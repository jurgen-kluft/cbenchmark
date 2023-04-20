#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_exception.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_alloc.h"
#include "cbenchmark/private/c_benchmark_runner.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_instance.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

#include <cmath>

#include "c_benchmark_thread_manager.cc"

namespace BenchMark
{
    static inline bool IsZero(double n) { return std::abs(n) < std::numeric_limits<double>::epsilon(); }

    static constexpr IterationCount kMaxIterations  = 1000000000;
    const double                    kDefaultMinTime = 1.0;

    BenchMarkRun CreateRunReport(BenchMarkRun& report, const BenchMarkInstance* bmi, const ThreadManager::Result& results, IterationCount memory_iterations, double seconds, s64 repetition_index, s64 repeats)
    {
        // Create report about this benchmark run.
        report.run_name                  = bmi->name();
        report.family_index              = bmi->family_index();
        report.per_family_instance_index = bmi->per_family_instance_index();
        report.skipped                   = results.skipped_;
        report.skip_message              = results.skip_message_;
        report.report_label              = results.report_label_;

        // This is the total iterations across all threads.
        report.iterations       = results.iterations;
        report.time_unit        = bmi->time_unit();
        report.threads          = bmi->threads();
        report.repetition_index = repetition_index;
        report.repetitions      = repeats;

        if (report.skipped.Is(Skipped::NotSkipped))
        {
            if (bmi->use_manual_time())
            {
                report.real_accumulated_time = results.manual_time_used;
            }
            else
            {
                report.real_accumulated_time = results.real_time_used;
            }
            report.cpu_accumulated_time = results.cpu_time_used;
            report.complexity_n         = results.complexity_n;
            report.complexity           = bmi->complexity();
            report.complexity_lambda    = bmi->complexity_lambda();
            report.statistics           = bmi->statistics();
            report.counters             = results.counters;

            Counters::Finish(report.counters, results.iterations, seconds, bmi->threads());
        }
        return report;
    }

    // Execute one thread of benchmark bmi for the specified number of iterations.
    // Adds the stats collected for the thread into manager->results.
    void RunInThread(const BenchMarkInstance* bmi, IterationCount iters, int thread_id, ThreadManager* manager, PerfCountersMeasurement* perf_counters_measurement)
    {
        ThreadTimer timer(bmi->measure_process_cpu_time() ? ThreadTimer::CreateProcessCpuTime() : ThreadTimer::Create());

        BenchMarkState st = bmi->Run(iters, thread_id, &timer, manager, perf_counters_measurement);
        // BM_CHECK(st.skipped() || st.iterations() >= st.max_iterations) << "Benchmark returned before State::KeepRunning() returned false!";
        {
            MutexLock              l(manager->GetBenchmarkMutex());
            ThreadManager::Result& results = manager->results;
            results.iterations += st.Iterations();
            results.cpu_time_used += timer.cpu_time_used();
            results.real_time_used += timer.real_time_used();
            results.manual_time_used += timer.manual_time_used();
            results.complexity_n += st.GetComplexityLengthN();
            Counters::Increment(results.counters, st.counters_);
        }
        manager->NotifyThreadComplete();
    }

    double ComputeMinTime(const BenchMarkInstance* bmi, const BenchTimeType& iters_or_time)
    {
        if (!IsZero(bmi->min_time()))
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
        // BM_CHECK(iters_or_time.tag == BenchTimeType::ITERS);
        return iters_or_time.iters;
    }

    class BenchMarkRunner
    {
    public:
        BenchMarkRunner();

        void                                    Init(const BenchMarkInstance* b_, PerfCountersMeasurement* pmc_, BenchMarkReporter::PerFamilyRunReports* reports_for_family);
        int                                     GetNumRepeats() const { return repeats; }
        bool                                    HasRepeatsRemaining() const { return GetNumRepeats() != num_repetitions_done; }
        void                                    DoOneRepetition();
        RunResults&&                            GetResults();
        BenchMarkReporter::PerFamilyRunReports* GetReportsForFamily() const { return reports_for_family; }
        double                                  GetMinTime() const { return min_time; }
        bool                                    HasExplicitIters() const { return has_explicit_iteration_count; }
        IterationCount                          GetIters() const { return iters; }

        Allocator*                              mAllocator;
        RunResults                              run_results;
        BenchMarkInstance const*                instance;
        BenchMarkReporter::PerFamilyRunReports* reports_for_family;
        BenchTimeType                           parsed_benchtime_flag;
        double                                  min_time;
        double                                  min_warmup_time;
        bool                                    warmup_done;
        int                                     repeats;
        bool                                    has_explicit_iteration_count;
        int                                     num_repetitions_done = 0;

        void* operator new(u64 num_bytes, void* mem) { return mem; }
        void  operator delete(void* mem, void*) {}

        std::vector<std::thread> pool;
        // std::vector<MemoryManager::Result> memory_results;

        IterationCount iters; // preserved between repetitions!
        // So only the first repetition has to find/calculate it,
        // the other repetitions will just use that precomputed iteration count.

        PerfCountersMeasurement* perf_counters_measurement_ptr = nullptr;

        struct IterationResults
        {
            ThreadManager::Result results;
            IterationCount        iters;
            double                seconds;
        };

        IterationResults DoNIterations();
        IterationCount   PredictNumItersNeeded(const IterationResults& i) const;
        bool             ShouldReportIterationResults(const IterationResults& i) const;
        double           GetMinTimeToApply() const;
        void             FinishWarmUp(const IterationCount& i);
        void             RunWarmUp();
    };

    // Public Interface
    void CreateRunner(BenchMarkRunner*& r, Allocator* a) { r = a->Construct<BenchMarkRunner>(a); }
    void DestroyRunner(BenchMarkRunner*& r) { r->mAllocator->Destruct(r); }

    int                                     GetNumRepeats(BenchMarkRunner* r) { return r->GetNumRepeats(); }
    bool                                    HasRepeatsRemaining(BenchMarkRunner* r) { return r->HasRepeatsRemaining(); }
    void                                    DoOneRepetition(BenchMarkRunner* r) { r->DoOneRepetition(); }
    RunResults&&                            GetResults(BenchMarkRunner* r) { return r->GetResults(); }
    BenchMarkReporter::PerFamilyRunReports* GetReportsForFamily(BenchMarkRunner* r) { return r->GetReportsForFamily(); }
    double                                  GetMinTime(BenchMarkRunner* r) { return r->GetMinTime(); }
    bool                                    HasExplicitIters(BenchMarkRunner* r) { return r->HasExplicitIters(); }
    IterationCount                          GetIters(BenchMarkRunner* r) { return r->GetIters(); }

    static double FLAGS_benchmark_min_time                = 0.5;
    static double FLAGS_benchmark_min_warmup_time         = 0.5;
    static bool   FLAGS_benchmark_report_aggregates_only  = true;
    static bool   FLAGS_benchmark_display_aggregates_only = false;
    static bool   FLAGS_benchmark_repetitions             = 1;

    BenchMarkRunner::BenchMarkRunner()
        : instance(nullptr)
        , reports_for_family(nullptr)
        , parsed_benchtime_flag(BenchTimeType())
        , min_time(0.0)
        , min_warmup_time(0.0)
        , warmup_done(false)
        , repeats(FLAGS_benchmark_repetitions)
        , has_explicit_iteration_count(false)
        , pool()
        , iters(0)
        , perf_counters_measurement_ptr(nullptr)
    {
    }

    void BenchMarkRunner::Init(const BenchMarkInstance* b_, PerfCountersMeasurement* pcm_, BenchMarkReporter::PerFamilyRunReports* reports_for_family_)
    {
        instance                     = (b_);
        reports_for_family           = (reports_for_family_);
        parsed_benchtime_flag        = (BenchTimeType(FLAGS_benchmark_min_time));
        min_time                     = (ComputeMinTime(b_, parsed_benchtime_flag));
        min_warmup_time              = ((!IsZero(instance->min_time()) && instance->min_warmup_time() > 0.0) ? instance->min_warmup_time() : FLAGS_benchmark_min_warmup_time);
        warmup_done                  = (!(min_warmup_time > 0.0));
        repeats                      = (instance->repetitions() != 0 ? instance->repetitions() : FLAGS_benchmark_repetitions);
        has_explicit_iteration_count = (instance->iterations() != 0 || parsed_benchtime_flag.type == BenchTimeType::ITERS);
        pool.reserve(instance->threads() - 1);
        iters                         = (has_explicit_iteration_count ? ComputeIters(*instance, parsed_benchtime_flag) : 1);
        perf_counters_measurement_ptr = (pcm_);

        run_results.display_report_aggregates_only = (FLAGS_benchmark_report_aggregates_only || FLAGS_benchmark_display_aggregates_only);
        run_results.file_report_aggregates_only    = FLAGS_benchmark_report_aggregates_only;
        if (instance->aggregation_report_mode().mode != AggregationReportMode::Unspecified)
        {
            run_results.display_report_aggregates_only = (instance->aggregation_report_mode().mode & AggregationReportMode::DisplayReportAggregatesOnly);
            run_results.file_report_aggregates_only    = (instance->aggregation_report_mode().mode & AggregationReportMode::FileReportAggregatesOnly);
            // BM_CHECK(FLAGS_benchmark_perf_counters.empty() || (perf_counters_measurement_ptr->num_counters() == 0)) << "Perf counters were requested but could not be set up.";
        }
    }

    BenchMarkRunner::IterationResults BenchMarkRunner::DoNIterations()
    {
        // BM_VLOG(2) << "Running " << instance->name().str() << " for " << iters << "\n";

        // TODO Need Allocator with Construct<> and Destroy<>
        ThreadManager* manager = mAllocator->Construct<ThreadManager>(instance->threads());

        // Run all but one thread in separate threads
        for (u64 ti = 0; ti < pool.size(); ++ti)
        {
            pool[ti] = std::thread(&RunInThread, &instance, iters, static_cast<int>(ti + 1), manager, perf_counters_measurement_ptr);
        }

        // And run one thread here directly.
        // (If we were asked to run just one thread, we don't create new threads.)
        // Yes, we need to do this here *after* we start the separate threads.
        RunInThread(instance, iters, 0, manager, perf_counters_measurement_ptr);

        // The main thread has finished. Now let's wait for the other threads.
        manager->WaitForAllThreads();
        for (std::thread& thread : pool)
            thread.join();

        IterationResults i;

        // Acquire the measurements/counters from the manager, UNDER THE LOCK!
        {
            MutexLock l(manager->GetBenchmarkMutex());
            i.results = manager->results;
        }

        // And get rid of the manager.
        mAllocator->Destruct(manager);

        // Adjust real/manual time stats since they were reported per thread.
        i.results.real_time_used /= instance->threads();
        i.results.manual_time_used /= instance->threads();
        
        // If we were measuring whole-process CPU usage, adjust the CPU time too.
        if (instance->measure_process_cpu_time())
            i.results.cpu_time_used /= instance->threads();

        // BM_VLOG(2) << "Ran in " << i.results.cpu_time_used << "/" << i.results.real_time_used << "\n";

        // By using KeepRunningBatch a benchmark can iterate more times than
        // requested, so take the iteration count from i.results.
        i.iters = i.results.iterations / instance->threads();

        // Base decisions off of real time if requested by this benchmark.
        i.seconds = i.results.cpu_time_used;
        if (instance->use_manual_time())
        {
            i.seconds = i.results.manual_time_used;
        }
        else if (instance->use_real_time())
        {
            i.seconds = i.results.real_time_used;
        }

        return i;
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

        // BM_VLOG(3) << "Next iters: " << next_iters << ", " << multiplier << "\n";
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

        for (;;)
        {
            instance->Setup();
            i_warmup = DoNIterations();
            instance->Teardown();

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
            // assert(iters > i_warmup.iters && "if we did more iterations than we want to do the next time, "
            //                                  "then we should have accepted the current iteration run.");
        }
    }

    void BenchMarkRunner::DoOneRepetition()
    {
        // assert(HasRepeatsRemaining() && "Already done all repetitions?");

        const bool is_the_first_repetition = num_repetitions_done == 0;

        // In case a warmup phase is requested by the benchmark, run it now.
        // After running the warmup phase the BenchMarkRunner should be in a state as
        // this warmup never happened except the fact that warmup_done is set. Every
        // other manipulation of the BenchMarkRunner instance would be a bug! Please
        // fix it.
        if (!warmup_done)
            RunWarmUp();

        IterationResults i;
        // We *may* be gradually increasing the length (iteration count)
        // of the benchmark until we decide the results are significant.
        // And once we do, we report those last results and exit.
        // Please do note that the if there are repetitions, the iteration count
        // is *only* calculated for the *first* repetition, and other repetitions
        // simply use that precomputed iteration count.
        for (;;)
        {
            instance->Setup();
            i = DoNIterations();
            instance->Teardown();

            // Do we consider the results to be significant?
            // If we are doing repetitions, and the first repetition was already done,
            // it has calculated the correct iteration time, so we have run that very
            // iteration count just now. No need to calculate anything. Just report.
            // Else, the normal rules apply.
            const bool results_are_significant = !is_the_first_repetition || has_explicit_iteration_count || ShouldReportIterationResults(i);

            if (results_are_significant)
                break; // Good, let's report them!

            // Nope, bad iteration. Let's re-estimate the hopefully-sufficient
            // iteration count, and run the benchmark again...

            iters = PredictNumItersNeeded(i);
            // assert(iters > i.iters && "if we did more iterations than we want to do the next time, "
            //                           "then we should have accepted the current iteration run.");
        }

        // Ok, now actually report.
        IterationCount memory_iterations = 0;

        BenchMarkRun& report = run_results.non_aggregates[run_results.num_non_aggregates++];

        CreateRunReport(report, instance, i.results, memory_iterations, i.seconds, num_repetitions_done, repeats);

        if (reports_for_family)
        {
            ++reports_for_family->num_runs_done;
            if (report.skipped.IsNotSkipped())
                reports_for_family->runs[reports_for_family->num_runs++] = &report;
        }

        ++num_repetitions_done;
    }

    RunResults&& BenchMarkRunner::GetResults()
    {
        // assert(!HasRepeatsRemaining() && "Did not run all repetitions yet?");

        // Calculate additional statistics over the repetitions of this instance->
        run_results.num_aggregates = ComputeStats(run_results.non_aggregates, run_results.num_non_aggregates, run_results.aggregates_only, run_results.max_aggregates);

        return std::move(run_results);
    }
} // namespace BenchMark