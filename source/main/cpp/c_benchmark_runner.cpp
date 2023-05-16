#include "ccore/c_target.h"
#include "ccore/c_debug.h"

#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_allocators.h"
#include "cbenchmark/private/c_benchmark_runner.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_instance.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

#include <cmath>

#include "c_benchmark_thread_manager.cc"

namespace BenchMark
{
    static constexpr IterationCount kMaxIterations  = 1000000000;
    static constexpr double         kDefaultMinTime = 1.0;

    void CreateRunReport(ForwardAllocator* allocator, BenchMarkRun* report, const BenchMarkInstance* bmi, const BenchMarkRunResult& results, IterationCount memory_iterations, double seconds, s64 repetition_index, s64 repeats)
    {
        // Create report about this benchmark run.
        report->run_name.CopyFrom(allocator, bmi->name());
        report->skipped      = results.skipped_;
        report->skip_message = results.skip_message_;

        report->report_format = results.report_format_;
        report->report_value  = results.report_value_;

        // This is the total iterations across all threads.
        report->iterations       = results.iterations;
        report->time_unit        = bmi->time_unit();
        report->threads          = bmi->threads();
        report->repetition_index = repetition_index;
        report->repetitions      = repeats;

        if (report->skipped.Is(Skipped::NotSkipped))
        {
            if (bmi->use_manual_time())
            {
                report->real_accumulated_time = results.manual_time_used;
            }
            else
            {
                report->real_accumulated_time = results.real_time_used;
            }
            report->cpu_accumulated_time = results.cpu_time_used;
            report->complexity_n         = results.complexity_n;
            report->complexity           = bmi->complexity();
            report->complexity_lambda    = bmi->complexity_lambda();
            report->statistics.Copy(allocator, bmi->statistics());
            report->counters.Copy(allocator, results.counters);

            Counters::Finish(report->counters, results.iterations, seconds, bmi->threads());
        }
    }

    // Execute one thread of benchmark bmi for the specified number of iterations.
    // Adds the stats collected for the thread into manager->results.
    void RunInThread(ForwardAllocator* allocator, const BenchMarkInstance* bmi, IterationCount iters, int thread_id, ThreadManager* manager, BenchMarkRunResult* results)
    {
        ThreadTimer timer(ThreadTimer::Create());

        BenchMarkState st;
        st.InitRun(allocator, bmi->name().function_name, iters, bmi->args(), bmi->counters()->Size(), thread_id, bmi->threads(), &timer, manager, results);

        bmi->run(st, allocator);

        ASSERTS(st.IsSkipped() || st.Iterations() >= st.max_iterations, "Benchmark returned before BenchMarkState::KeepRunning() returned false!");
        {
            results->iterations += st.Iterations();
            results->cpu_time_used += timer.cpu_time_used();
            results->real_time_used += timer.real_time_used();
            results->manual_time_used += timer.manual_time_used();
            results->complexity_n += st.GetComplexityLengthN();
            Counters::Increment(results->counters, st.counters_);
        }
        st.Shutdown();

        manager->NotifyThreadComplete();
    }

    double ComputeMinTime(const BenchMarkInstance* bmi, const BenchTimeType& iters_or_time)
    {
        if (!gIsZero(bmi->min_time()))
            return bmi->min_time();

        // If the flag was used to specify number of iters, then return the default min_time.
        if (iters_or_time.type == BenchTimeType::ITERS)
            return kDefaultMinTime;

        return iters_or_time.time;
    }

    IterationCount ComputeIters(const BenchMarkInstance& bmi, const BenchTimeType& iters_or_time)
    {
        if (bmi.iterations() != 0)
            return bmi.iterations();

        // We've already concluded that this flag is currently used to pass
        // iters but do a check here again anyway.
        ASSERT(iters_or_time.type == BenchTimeType::ITERS);
        return iters_or_time.iters;
    }

    class BenchMarkRunner
    {
    public:
        BenchMarkRunner();

        void           Init(Allocator* allocator, ScratchAllocator* scratch, BenchMarkGlobals* globals, const BenchMarkInstance* b_);
        int            GetNumRepeats() const { return repeats; }
        bool           HasRepeatsRemaining() const { return GetNumRepeats() != num_repetitions_done; }
        void           DoOneRepetition(ForwardAllocator* allocator, ScratchAllocator* scratch, BenchMarkRun* report, BenchMarkReporter::PerFamilyRunReports* reports_for_family);
        void           AggregateResults(ForwardAllocator* alloc, ScratchAllocator* scratch, const Array<BenchMarkRun*>& non_aggregates, Array<BenchMarkRun*>& aggregates_only) const;
        double         GetMinTime() const { return min_time; }
        bool           HasExplicitIters() const { return has_explicit_iteration_count; }
        IterationCount GetIters() const { return iters; }

        Allocator*               main_allocator_;
        ForwardAllocator*        forward_allocator_;
        ScratchAllocator*        scratch_allocator_;
        BenchMarkInstance const* instance;

        BenchTimeType parsed_benchtime_flag;
        double        min_time;
        double        min_warmup_time;
        bool          warmup_done;
        int           repeats;
        bool          has_explicit_iteration_count;
        int           num_repetitions_done = 0;

        void* operator new(u64 num_bytes, void* mem) { return mem; }
        void  operator delete(void* mem, void*) {}

        Array<std::thread*> thread_pool;
        // std::vector<MemoryManager::Result> memory_results;

        IterationCount iters; // preserved between repetitions!
        // So only the first repetition has to find/calculate it,
        // the other repetitions will just use that precomputed iteration count.

        struct IterationResults
        {
            IterationResults()
                : iters(0)
                , seconds(0)
            {
            }

            void Reset()
            {
                results.Reset();
                iters   = 0;
                seconds = 0;
            }

            void Initialize(Allocator* alloc, BenchMarkInstance const* instance) { results.Initialize(alloc, instance); }
            void Shutdown() { results.Shutdown(); }

            BenchMarkRunResult results;
            IterationCount     iters;
            double             seconds;
        };

        void           DoNIterations(IterationResults& iteration_results);
        IterationCount PredictNumItersNeeded(const IterationResults& i) const;
        bool           ShouldReportIterationResults(const IterationResults& i) const;
        double         GetMinTimeToApply() const;
        void           FinishWarmUp(const IterationCount& i);
        void           RunWarmUp();
    };

    // Public Interface
    BenchMarkRunner* CreateRunner(Allocator* a) { return a->Construct<BenchMarkRunner>(); }
    void             InitRunner(BenchMarkRunner* r, Allocator* a, ScratchAllocator* t, BenchMarkGlobals* globals, const BenchMarkInstance* b_) { r->Init(a, t, globals, b_); }
    void             DestroyRunner(BenchMarkRunner*& r, Allocator* a) { a->Destruct(r); }

    void InitRunResults(BenchMarkRunner* r, BenchMarkGlobals* globals, RunResults* results)
    {
        BenchMarkInstance const* instance       = r->instance;
        results->display_report_aggregates_only = (globals->benchmark_report_aggregates_only || globals->benchmark_display_aggregates_only);
        results->file_report_aggregates_only    = globals->benchmark_report_aggregates_only;
        if (instance->aggregation_report_mode().mode != AggregationReportMode::Unspecified)
        {
            results->display_report_aggregates_only = (instance->aggregation_report_mode().mode & AggregationReportMode::DisplayReportAggregatesOnly);
            results->file_report_aggregates_only    = (instance->aggregation_report_mode().mode & AggregationReportMode::FileReportAggregatesOnly);
        }
    }

    int    GetNumRepeats(const BenchMarkRunner* r) { return r->GetNumRepeats(); }
    bool   HasRepeatsRemaining(const BenchMarkRunner* r) { return r->HasRepeatsRemaining(); }
    void   DoOneRepetition(BenchMarkRunner* r, ForwardAllocator* allocator, ScratchAllocator* scratch, BenchMarkRun* report, BenchMarkReporter::PerFamilyRunReports* reports_for_family) { r->DoOneRepetition(allocator, scratch, report, reports_for_family); }
    void   AggregateResults(BenchMarkRunner* r, ForwardAllocator* alloc, ScratchAllocator* scratch, const Array<BenchMarkRun*>& non_aggregates, Array<BenchMarkRun*>& aggregates_only) { r->AggregateResults(alloc, scratch, non_aggregates, aggregates_only); }
    double GetMinTime(const BenchMarkRunner* r) { return r->GetMinTime(); }
    bool   HasExplicitIters(const BenchMarkRunner* r) { return r->HasExplicitIters(); }
    IterationCount GetIters(const BenchMarkRunner* r) { return r->GetIters(); }
    void           StartStopBarrier(ThreadManager* tm) { tm->StartStopBarrier(); }
    void           ThreadTimerStart(ThreadTimer* timer) { timer->StartTimer(); }
    void           ThreadTimerStop(ThreadTimer* timer) { timer->StopTimer(); }
    bool           ThreadTimerIsRunning(ThreadTimer* timer) { return timer->IsRunning(); }
    void           ThreadTimerSetIterationTime(ThreadTimer* timer, double seconds) { timer->SetIterationTime(seconds); }

    BenchMarkRunner::BenchMarkRunner()
        : main_allocator_(nullptr)
        , scratch_allocator_(nullptr)
        , instance(nullptr)
        , parsed_benchtime_flag(BenchTimeType())
        , min_time(0.0)
        , min_warmup_time(0.0)
        , warmup_done(false)
        , repeats(0)
        , has_explicit_iteration_count(false)
        , thread_pool()
        , iters(0)
    {
    }

    void BenchMarkRunner::Init(Allocator* allocator, ScratchAllocator* scratch, BenchMarkGlobals* globals, const BenchMarkInstance* b_)
    {
        main_allocator_              = (allocator);
        scratch_allocator_           = (scratch);
        instance                     = (b_);
        parsed_benchtime_flag        = (BenchTimeType(globals->benchmark_min_time));
        min_time                     = (ComputeMinTime(b_, parsed_benchtime_flag));
        min_warmup_time              = ((!gIsZero(instance->min_time()) && instance->min_warmup_time() > 0.0) ? instance->min_warmup_time() : globals->benchmark_min_warmup_time);
        warmup_done                  = (!(min_warmup_time > 0.0));
        repeats                      = (instance->repetitions() != 0 ? instance->repetitions() : globals->benchmark_repetitions);
        has_explicit_iteration_count = (instance->iterations() != 0 || parsed_benchtime_flag.type == BenchTimeType::ITERS);

        iters = (has_explicit_iteration_count ? ComputeIters(*instance, parsed_benchtime_flag) : 1);
    }

    // TODO could really benefit from a temporary allocator

    void BenchMarkRunner::DoNIterations(BenchMarkRunner::IterationResults& iteration_results)
    {
        USE_SCRATCH(scratch_allocator_);

        // "Running " << instance->name << " for " << iters

        ThreadManager* manager = scratch_allocator_->Construct<ThreadManager>(instance->threads());

        // Initialize the thread pool
        thread_pool.Init(scratch_allocator_, instance->threads() - 1, instance->threads() - 1);

        // Initialize the results array
        Array<BenchMarkRunResult*> results;
        results.Init(scratch_allocator_, 0, thread_pool.Capacity());

        // Prepare allocators for each thread
        Array<ForwardAllocator*> forward_allocators;
        forward_allocators.Init(scratch_allocator_, 0, thread_pool.Capacity() + 1);
        for (s32 ti = 0; ti < thread_pool.Capacity() + 1; ++ti)
        {
            ForwardAllocator* allocator = scratch_allocator_->Construct<ForwardAllocator>();
            allocator->Initialize(main_allocator_, instance->memory_required());
            forward_allocators.PushBack(allocator);
        }

        // Run all but one thread in separate threads
        for (s32 ti = 0; ti < thread_pool.Capacity(); ++ti)
        {
            BenchMarkRunResult*& result = results.Alloc();
            result                      = scratch_allocator_->Construct<BenchMarkRunResult>();
            result->Initialize(scratch_allocator_, instance);
            thread_pool[ti] = scratch_allocator_->Construct<std::thread>(&RunInThread, forward_allocators[1 + ti], instance, iters, static_cast<int>(ti + 1), manager, result);
        }

        // And run one thread here directly and use the results from iteration_results.
        // (If we were asked to run just one thread, we don't create new threads.)
        // Yes, we need to do this here *after* we start the separate threads.
        RunInThread(forward_allocators[0], instance, iters, 0, manager, &iteration_results.results);

        // The main thread has finished. Now let's wait for the other threads.
        manager->WaitForAllThreads();
        for (s32 ti = 0; ti < thread_pool.Size(); ++ti)
        {
            thread_pool[ti]->join();
        }
        for (s32 ti = 0; ti < thread_pool.Size(); ++ti)
        {
            scratch_allocator_->Destruct(thread_pool[ti]);
        }
        thread_pool.Release();

        // Destroy the allocators.
        for (s32 ti = 0; ti < forward_allocators.Size(); ++ti)
        {
            ForwardAllocator*& allocator = forward_allocators[ti];
            allocator->Release();
            scratch_allocator_->Destruct(allocator);
        }
        forward_allocators.Release();

        // Merge all the results together.
        for (s32 ti = 0; ti < results.Size(); ++ti)
        {
            BenchMarkRunResult* rr = results[ti];
            iteration_results.results.Merge(*rr);
            rr->Shutdown();
            scratch_allocator_->Destruct(rr);
        }
        results.Release();

        // And get rid of the manager.
        scratch_allocator_->Destruct(manager);

        // Adjust real/manual time stats since they were reported per thread.
        iteration_results.results.real_time_used /= instance->threads();
        iteration_results.results.manual_time_used /= instance->threads();

        // If we were measuring whole-process CPU usage, adjust the CPU time too.
        if (instance->measure_process_cpu_time())
            iteration_results.results.cpu_time_used /= instance->threads();

        // "Ran in " << i.results.cpu_time_used << "/" << i.results.real_time_used

        // By using KeepRunningBatch a benchmark can iterate more times than
        // requested, so take the iteration count from i.results.
        iteration_results.iters = iteration_results.results.iterations / instance->threads();

        // Base decisions off of real time if requested by this benchmark.
        iteration_results.seconds = iteration_results.results.cpu_time_used;
        if (instance->use_manual_time())
        {
            iteration_results.seconds = iteration_results.results.manual_time_used;
        }
        else if (instance->use_real_time())
        {
            iteration_results.seconds = iteration_results.results.real_time_used;
        }
    }

    template <typename T> T min(T a, T b) { return a < b ? a : b; }
    template <typename T> T max(T a, T b) { return a > b ? a : b; }

    IterationCount BenchMarkRunner::PredictNumItersNeeded(const IterationResults& i) const
    {
        // See how much iterations should be increased by.
        // Note: Avoid division by zero with max(seconds, 1ns).
        double multiplier = GetMinTimeToApply() * 1.4 / max(i.seconds, 1e-9);
        // If our last run was at least 10% of FLAGS_benchmark_min_time then we
        // use the multiplier directly.
        // Otherwise we use at most 10 times expansion.
        // NOTE: When the last run was at least 10% of the min time the max
        // expansion should be 14x.
        const bool is_significant = (i.seconds / GetMinTimeToApply()) > 0.1;
        multiplier                = is_significant ? multiplier : 10.0;

        // So what seems to be the sufficiently-large iteration count? Round up.
        const IterationCount max_next_iters = static_cast<IterationCount>(std::lround(max(multiplier * static_cast<double>(i.iters), static_cast<double>(i.iters) + 1.0)));
        // But we do have *some* limits though..
        const IterationCount next_iters = min(max_next_iters, kMaxIterations);

        // "Next iters: " << next_iters << ", " << multiplier
        return next_iters; // round up before conversion to integer.
    }

    bool BenchMarkRunner::ShouldReportIterationResults(const IterationResults& i) const
    {
        // Determine if this run should be reported;
        // Either it has run for a sufficient amount of time
        // or because an error was reported.
        return i.results.skipped_.IsNotSkipped() || i.iters >= kMaxIterations || // Too many iterations already.
               i.seconds >= GetMinTimeToApply() ||                               // The elapsed time is large enough.
               // CPU time is specified but the elapsed real time greatly exceeds
               // the minimum time.
               // Note that user provided timers are except from this test.
               ((i.results.real_time_used >= 5 * GetMinTimeToApply()) && !instance->use_manual_time());
    }

    double BenchMarkRunner::GetMinTimeToApply() const
    {
        // In order to re-use functionality to run and measure benchmarks for running
        // a warmup phase of the benchmark, we need a way of telling whether to apply
        // min_time or min_warmup_time. This function will figure out if we are in the
        // warmup phase and therefore need to apply min_warmup_time or if we already
        // in the benchmarking phase and min_time needs to be applied.
        return warmup_done ? min_time : min_warmup_time;
    }

    void BenchMarkRunner::FinishWarmUp(const IterationCount& i)
    {
        warmup_done = true;
        iters       = i;
    }

    void BenchMarkRunner::RunWarmUp()
    {
        // Use the same mechanisms for warming up the benchmark as used for actually
        // running and measuring the benchmark.
        IterationResults i_warmup;

        // Dont use the iterations determined in the warmup phase for the actual
        // measured benchmark phase. While this may be a good starting point for the
        // benchmark and it would therefore get rid of the need to figure out how many
        // iterations are needed if min_time is set again, this may also be a complete
        // wrong guess since the warmup loops might be considerably slower (e.g
        // because of caching effects).
        const IterationCount i_backup = iters;

        BenchMarkState state;
        state.Init(instance->name().function_name, /*iters*/ 1, instance->args(), /*thread_id*/ 0, instance->threads());

        for (;;)
        {
            instance->setup()(state);
            {
                i_warmup.Reset();
                DoNIterations(i_warmup);
            }
            instance->teardown()(state);

            const bool finish = ShouldReportIterationResults(i_warmup);
            if (finish)
            {
                FinishWarmUp(i_backup);
                break;
            }

            // Although we are running "only" a warmup phase where running enough
            // iterations at once without measuring time isn't as important as it is for
            // the benchmarking phase, we still do it the same way as otherwise it is
            // very confusing for the user to know how to choose a proper value for
            // min_warmup_time if a different approach on running it is used.
            iters = PredictNumItersNeeded(i_warmup);
            ASSERTS(iters > i_warmup.iters, "if we did more iterations than we want to do the next time, then we should have accepted the current iteration run.");
        }

        state.Shutdown();
    }

    void BenchMarkRunner::DoOneRepetition(ForwardAllocator* allocator, ScratchAllocator* scratch, BenchMarkRun* report, BenchMarkReporter::PerFamilyRunReports* reports_for_family)
    {
        ASSERTS(HasRepeatsRemaining(), "Already done all repetitions?");

        USE_SCRATCH(scratch);

        const bool is_the_first_repetition = num_repetitions_done == 0;

        // In case a warmup phase is requested by the benchmark, run it now.
        // After running the warmup phase the BenchMarkRunner should be in a state as
        // this warmup never happened except the fact that warmup_done is set. Every
        // other manipulation of the BenchMarkRunner instance would be a bug! Please
        // fix it.
        if (!warmup_done)
            RunWarmUp();

        IterationResults results;
        results.Initialize(scratch, instance);

        // We *may* be gradually increasing the length (iteration count)
        // of the benchmark until we decide the results are significant.
        // And once we do, we report those last results and exit.
        // Please do note that the if there are repetitions, the iteration count
        // is *only* calculated for the *first* repetition, and other repetitions
        // simply use that precomputed iteration count.
        BenchMarkState state;
        state.Init(instance->name().function_name, /*iters*/ 1, instance->args(), /*thread_id*/ 0, instance->threads());

        for (;;)
        {
            instance->setup()(state);
            {
                results.Reset();
                DoNIterations(results);
            }
            instance->teardown()(state);

            // Do we consider the results to be significant?
            // If we are doing repetitions, and the first repetition was already done,
            // it has calculated the correct iteration time, so we have run that very
            // iteration count just now. No need to calculate anything. Just report->
            // Else, the normal rules apply.
            const bool results_are_significant = !is_the_first_repetition || has_explicit_iteration_count || ShouldReportIterationResults(results);

            if (results_are_significant)
                break; // Good, let's report them!

            // Nope, bad iteration. Let's re-estimate the hopefully-sufficient
            // iteration count, and run the benchmark again...

            iters = PredictNumItersNeeded(results);
            ASSERTS(iters > results.iters, "if we did more iterations than we want to do the next time, then we should have accepted the current iteration run.");
        }

        state.Shutdown();

        // Ok, now actually report
        IterationCount memory_iterations = 0;

        CreateRunReport(allocator, report, instance, results.results, memory_iterations, results.seconds, num_repetitions_done, repeats);

        if (reports_for_family)
        {
            ++reports_for_family->num_runs_done;
            if (report->skipped.IsNotSkipped())
                reports_for_family->runs.PushBack(report);
        }

        results.Shutdown();
        ++num_repetitions_done;
    }

    void BenchMarkRunner::AggregateResults(ForwardAllocator* alloc, ScratchAllocator* scratch, const Array<BenchMarkRun*>& non_aggregates, Array<BenchMarkRun*>& aggregates_only) const
    {
        ASSERT(!HasRepeatsRemaining() && "Did not run all repetitions yet?");

        // Calculate additional statistics over the repetitions of this instance
        ComputeStats(alloc, scratch, non_aggregates, aggregates_only);
    }
} // namespace BenchMark