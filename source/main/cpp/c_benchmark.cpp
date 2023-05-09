#include "ccore/c_target.h"
#include "ccore/c_debug.h"

#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_instance.h"
#include "cbenchmark/private/c_benchmark_name.h"
#include "cbenchmark/private/c_benchmark_macros.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_reporter_console.h"
#include "cbenchmark/private/c_benchmark_check.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_runner.h"
#include "cbenchmark/private/c_benchmark_complexity.h"
#include "cbenchmark/private/c_benchmark_unit.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include <algorithm>

namespace BenchMark
{
    // Flushes streams after invoking reporter methods that write to them. This
    // ensures users get timely updates even when streams are not line-buffered.
    static void FlushStreams(BenchMarkReporter* reporter)
    {
        if (!reporter)
            return;
        reporter->Flush();
    }

    // Reports in both display and file reporters.
    static void Report(BenchMarkReporter* reporter, const RunResults* run_results)
    {
        auto report_one = [](BenchMarkReporter* reporter, bool aggregates_only, const RunResults* results)
        {
            BM_CHECK(reporter);
            // If there are no aggregates, do output non-aggregates.
            aggregates_only &= !results->aggregates_only.Empty();
            if (!aggregates_only)
                reporter->ReportRuns(results->non_aggregates);
            if (!results->aggregates_only.Empty())
                reporter->ReportRuns(results->aggregates_only);
        };

        report_one(reporter, run_results->display_report_aggregates_only, run_results);

        FlushStreams(reporter);
    }

    template <typename T> T min(T a, T b) { return a < b ? a : b; }
    template <typename T> T max(T a, T b) { return a > b ? a : b; }

    struct XorRandom
    {
        u64 s0, s1;
        inline XorRandom(u64 seed)
            : s0(seed)
            , s1(0)
        {
            next();
            next();
        }

        inline u64 next(void)
        {
            u64 ss1    = s0;
            u64 ss0    = s1;
            u64 result = ss0 + ss1;
            s0         = ss0;
            ss1 ^= ss1 << 23;
            s1 = ss1 ^ ss0 ^ (ss1 >> 18) ^ (ss0 >> 5);
            return result;
        }
    };

    static void RandomShuffle(Array<s32>& indices)
    {
        XorRandom rng(0xdeadbeef);
        for (int i = 0; i < indices.Size(); ++i)
        {
            s32 j = i + ((s32)rng.next() % (indices.Size() - i));
            std::swap(indices[i], indices[j]);
        }
    }

    static void RunBenchMarkInstances(Allocator* allocator, ScratchAllocator* scratch, BenchMarkGlobals* globals, const Array<BenchMarkInstance*>& benchmark_instances, BenchMarkReporter* reporter)
    {
        // Note the file_reporter can be null.
        BM_CHECK(reporter != nullptr);

        // Determine the width of the name field using a minimum width of 10.
        bool might_have_aggregates = globals->FLAGS_benchmark_repetitions > 1;
        s32  name_field_width      = 10;
        s32  stat_field_width      = 0;
        for (int i = 0; i < benchmark_instances.Size(); ++i)
        {
            const BenchMarkInstance* benchmark = benchmark_instances[i];
            name_field_width                   = max<s32>(name_field_width, benchmark->name().FullNameLen());
            might_have_aggregates |= benchmark->repetitions() > 1;

            Array<Statistic> const& stats = benchmark->statistics();
            for (int j = 0; j < stats.Size(); ++j)
            {
                Statistic const& Stat = stats[j];
                stat_field_width      = max<s32>(stat_field_width, gStringLength(Stat.name_));
            }
        }
        if (might_have_aggregates)
            name_field_width += 1 + stat_field_width;

        // Print header here
        BenchMarkReporter::Context context;
        context.name_field_width = name_field_width;

        BenchMarkReporter::PerFamilyRunReports* reports_for_family = nullptr;
        if (!benchmark_instances[0]->complexity().Is(BigO::O_None))
        {
            reports_for_family = scratch->Construct<BenchMarkReporter::PerFamilyRunReports>();
        }

        if (reporter->ReportContext(context))
        {
            FlushStreams(reporter);

            s32 num_repetitions_total = 0;

            // Benchmarks to run
            Array<BenchMarkRunner*> runners;
            runners.Init(scratch, 0, benchmark_instances.Size());

            Array<RunResults*> run_results;
            run_results.Init(scratch, 0, benchmark_instances.Size());

            // Count the number of benchmark_instances with threads to warn the user in case
            // performance counters are used.
            int benchmarks_with_threads = 0;

            // Loop through all benchmark_instances
            for (int i = 0; i < benchmark_instances.Size(); ++i)
            {
                const BenchMarkInstance* benchmark = benchmark_instances[i];

                benchmarks_with_threads += (benchmark->threads() > 0);

                BenchMarkRunner* runner = CreateRunner(scratch);
                InitRunner(runner, allocator, scratch, globals, benchmark);
                runners.PushBack(runner);

                const int num_repeats_of_this_instance = GetNumRepeats(runner);
                num_repetitions_total += num_repeats_of_this_instance;
                if (reports_for_family)
                    reports_for_family->num_runs_total += num_repeats_of_this_instance;

                RunResults* results = scratch->Construct<RunResults>();
                results->non_aggregates.Init(scratch, 0, num_repeats_of_this_instance);
                results->aggregates_only.Init(scratch, 0, num_repeats_of_this_instance);
                InitRunResults(runner, globals, *results);
                run_results.PushBack(results);
            }
            BM_CHECK(runners.Size() == benchmark_instances.Size() && "Unexpected runner count.");

            Array<s32> repetition_indices;
            repetition_indices.Init(scratch, 0, num_repetitions_total);

            for (s32 runner_index = 0, num_runners = runners.Size(); runner_index != num_runners; ++runner_index)
            {
                const BenchMarkRunner* runner = runners[runner_index];
                int                    i      = GetNumRepeats(runner);
                while (i--)
                    repetition_indices.PushBack(runner_index);
            }
            BM_CHECK(repetition_indices.Size() == num_repetitions_total && "Unexpected number of repetition indexes.");

            if (globals->FLAGS_benchmark_enable_random_interleaving)
            {
                RandomShuffle(repetition_indices);
            }

            for (int i = 0; i < repetition_indices.Size(); ++i)
            {
                const s32        repetition_index = repetition_indices[i];
                BenchMarkRunner* runner           = runners[repetition_index];
                RunResults*      results          = run_results[repetition_index];

                BenchMarkRun*& report = results->non_aggregates.Alloc();
                report                = allocator->Construct<BenchMarkRun>();

                DoOneRepetition(runner, report, reports_for_family);
                if (HasRepeatsRemaining(runner))
                    continue;

                // FIXME: report each repetition separately, not all of them in bulk.

                reporter->ReportRunsConfig(GetMinTime(runner), HasExplicitIters(runner), GetIters(runner));

                AggregateResults(runner, allocator, results->non_aggregates, results->aggregates_only);

                // Maybe calculate complexity report
                if (reports_for_family != nullptr)
                {
                    if (reports_for_family->num_runs_done == reports_for_family->num_runs_total)
                    {
                        Array<BenchMarkRun*> additional_run_stats;
                        additional_run_stats.Init(allocator, 0, 2);
                        ComputeBigO(allocator, scratch, reports_for_family->runs, additional_run_stats);

                        // run_results->aggregates_only.insert(run_results->aggregates_only.end(), additional_run_stats.begin(), additional_run_stats.end());
                        results->aggregates_only.PushBack(additional_run_stats);
                    }
                }

                Report(reporter, results);

                // Destroy the reports
                for (int i = 0; i < results->non_aggregates.Size(); ++i)
                {
                    scratch->Destruct(results->non_aggregates[i]);
                }
                for (int i = 0; i < results->aggregates_only.Size(); ++i)
                {
                    scratch->Destruct(results->aggregates_only[i]);
                }
                scratch->Destruct(results);
            }

            // Destroy the reports for family
            if (reports_for_family != nullptr)
            {
                scratch->Destruct(reports_for_family);
            }

            // Destroy all runners
            for (int i = 0; i < runners.Size(); ++i)
            {
                DestroyRunner(runners[i], scratch);
            }
        }

        reporter->Finalize();
        FlushStreams(reporter);
    }

    // Permutations is determined by the number of inputs to repeat a benchmark on.
    // If this is "large" then warn the user during configuration.
    static constexpr size_t kMaxPerms = 100;

    static bool CreateBenchMarkInstances(Allocator* allocator, BenchMarkUnit* benchmark, Array<BenchMarkInstance*>& benchmark_instances)
    {
        const Array<s32>& thread_counts     = benchmark->thread_counts_;
        const s32         num_thread_counts = thread_counts.Empty() ? 1 : thread_counts.Size();

        // Have BenchMarkUnit create the arguments for this benchmark
        const s32 perms = benchmark->BuildArgs();
        benchmark_instances.Init(allocator, 0, perms * num_thread_counts);

        s32 per_benchmark_instance_index = 0;
        for (s32 arg_index = 0; arg_index < perms; ++arg_index)
        {
            for (s32 i = 0; i < num_thread_counts; ++i)
            {
                const s32 num_threads = thread_counts.Empty() ? 1 : thread_counts[i];

                BenchMarkInstance*& instance = benchmark_instances.Alloc();
                instance                     = allocator->Construct<BenchMarkInstance>();
                instance->init(allocator, benchmark, per_benchmark_instance_index, num_threads);

                benchmark_instances.PushBack(instance);
                ++per_benchmark_instance_index;
            }
        }

        return true;
    }

    namespace BenchMarkSuiteList
    {
        static BenchMarkSuite* head = nullptr;
        static BenchMarkSuite* tail = nullptr;
    } // namespace BenchMarkSuiteList

    void RegisterBenchMarkSuite(BenchMarkSuite* suite)
    {
        if (BenchMarkSuiteList::head == nullptr)
        {
            BenchMarkSuiteList::head = suite;
            BenchMarkSuiteList::tail = suite;
        }
        else
        {
            BenchMarkSuiteList::tail->next = suite;
            BenchMarkSuiteList::tail       = suite;
        }
    }

    static void RunBenchMarkSuite(Allocator* allocator, ScratchAllocator* scratch, BenchMarkGlobals* globals, BenchMarkSuite* suite, BenchMarkReporter* reporter)
    {
        if (suite->disabled)
            return;

        // Report the details of this benchmark suite ?
        // - name / filename / line number
        // - list
        //   - fixture name(num units)

        // A benchmark-suite has a list of benchmark-fixtures where every fixture has a list of benchmark-units.

        BenchMarkFixture* fixture = suite->head;
        while (fixture != nullptr)
        {
            // Report the details of this fixture ?
            // - name / filename / line number
            // - num units

            if (fixture->disabled)
            {
                // Report this fixture as disabled
                fixture = fixture->next;
                continue;
            }

            BenchMarkUnit* unit = fixture->head;
            while (unit != nullptr)
            {
                if (!unit->IsDisabled())
                {
                    // Apply the settings for this benchmark unit, from Suite, Fixture and Unit
                    // Two passes
                    for (s32 i = 0; i < 2; ++i)
                    {
                        unit->PrepareSettings(i == 0);
                        suite->settings(allocator, unit);
                        fixture->settings(allocator, unit);
                        unit->settings_(allocator, unit);
                    }

                    Array<BenchMarkInstance*> benchmark_instances;
                    if (CreateBenchMarkInstances(allocator, unit, benchmark_instances))
                    {
                        // Report the details of this benchmark unit ?
                        // - name / filename / line number

                        RunBenchMarkInstances(allocator, scratch, globals, benchmark_instances, reporter);
                    }

                    // Destroy the benchmark instances
                    for (int i = 0; i < benchmark_instances.Size(); ++i)
                    {
                        allocator->Destruct(benchmark_instances[i]);
                    }

                    // Reset the settings for this benchmark unit (release memory)
                    unit->PrepareSettings(true);
                }

                unit = unit->next;
            }
            fixture = fixture->next;
        }
    }

    bool RunBenchMarks(Allocator* allocator, ScratchAllocator* scratch_allocator, BenchMarkGlobals* globals, BenchMarkReporter* reporter)
    {
        BenchMarkSuite* suite = BenchMarkSuiteList::head;
        while (suite != nullptr)
        {
            if (!suite->disabled)
            {
                RunBenchMarkSuite(allocator, scratch_allocator, globals, suite, reporter);
            }
            suite = suite->next;
        }
        return true;
    }

    class MainAllocator : public Allocator
    {
    public:
        virtual void* v_Allocate(unsigned int size, unsigned int alignment)
        {
            return aligned_alloc(alignment, size);
        }

        virtual void  v_Deallocate(void* ptr)
        {
            free(ptr);
        }
    };

    typedef unsigned char u8;

    class MainScratchAllocator : public ScratchAllocator
    {
        u8* buffer_;

        u8* buffer_begin_;
        u8* buffer_end_;

    public:
        void Init(Allocator* alloc, u32 size)
        {
            buffer_begin_ = (u8*)alloc->Allocate(size);
            buffer_end_   = buffer_begin_ + size;
            buffer_       = buffer_begin_;
        }

    protected:
        virtual void* v_Allocate(unsigned int size, unsigned int alignment = sizeof(void*))
        {
            u8* p   = (u8*)((uintptr_t)(buffer_ + alignment - 1) & ~(alignment - 1));
            buffer_ = p + size;
            if (buffer_ > buffer_end_)
            {
                ASSERT(false);
                return nullptr;
            }
            return p;
        }

        virtual void v_Deallocate(void* ptr) {}
        virtual void v_Reset() { buffer_ = buffer_begin_; }
    };

    bool gRunBenchMark(BenchMark::BenchMarkReporter& reporter)
    {
        // Setup the allocators
        MainAllocator main_allocator;
        MainScratchAllocator scratch_allocator;
        scratch_allocator.Init(&main_allocator, 8 * 1024 * 1024);

        // Setup the globals
        BenchMarkGlobals globals;

        // Run the benchmarks
        return BenchMark::RunBenchMarks(&main_allocator, &scratch_allocator, &globals, &reporter);
    }

} // namespace BenchMark
