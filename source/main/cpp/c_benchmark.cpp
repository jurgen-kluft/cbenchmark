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
#include <cstdlib>

namespace BenchMark
{
    static void Report(BenchMarkReporter* reporter, const RunResults* run_results, ForwardAllocator* allocator, ScratchAllocator* scratch)
    {
        if (reporter != nullptr)
        {
            // If there are no aggregates, do output non-aggregates.
            if (!run_results->display_report_aggregates_only || !run_results->aggregates_only.Empty())
                reporter->ReportRuns(run_results->non_aggregates, allocator, scratch);
            if (!run_results->aggregates_only.Empty())
                reporter->ReportRuns(run_results->aggregates_only, allocator, scratch);
        };

        reporter->ReportEnd(allocator);
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

    static void RandomShuffle(Array<s32>& indices, u64 seed)
    {
        XorRandom rng(seed);
        for (int i = 0; i < indices.Size(); ++i)
        {
            s32 j = i + ((s32)rng.next() % (indices.Size() - i));
            std::swap(indices[i], indices[j]);
        }
    }

    static void RunBenchMarkInstances(Allocator* main_allocator, ForwardAllocator* forward_allocator, ScratchAllocator* scratch_allocator, BenchMarkGlobals* globals, const Array<BenchMarkInstance*>& benchmark_instances, BenchMarkReporter* reporter)
    {
        USE_SCRATCH(scratch_allocator);

        // Note the file_reporter can be null.
        BM_CHECK(reporter != nullptr);

        // Determine the width of the name field using a minimum width of 10.
        bool might_have_aggregates = globals->benchmark_repetitions > 1;
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
            reports_for_family = scratch_allocator->Construct<BenchMarkReporter::PerFamilyRunReports>();
        }

        if (reporter->ReportContext(context, forward_allocator, scratch_allocator))
        {
            reporter->Flush(forward_allocator, scratch_allocator);

            USE_SCRATCH(scratch_allocator);

            // Benchmarks to run
            Array<BenchMarkRunner*> runners;
            runners.Init(scratch_allocator, 0, benchmark_instances.Size());

            Array<RunResults*> run_results;
            run_results.Init(scratch_allocator, 0, benchmark_instances.Size());

            // Count the number of benchmark_instances with threads to warn the user in case
            // performance counters are used.
            s32 num_repetitions_total   = 0;
            s32 benchmarks_with_threads = 0;

            // Loop through all benchmark_instances
            for (s32 i = 0; i < benchmark_instances.Size(); ++i)
            {
                const BenchMarkInstance* benchmark = benchmark_instances[i];

                benchmarks_with_threads += (benchmark->threads() > 0);

                BenchMarkRunner* runner = CreateRunner(forward_allocator);
                InitRunner(runner, main_allocator, scratch_allocator, globals, benchmark);
                runners.PushBack(runner);

                const s32 num_repeats_of_this_instance = GetNumRepeats(runner);
                num_repetitions_total += num_repeats_of_this_instance;
                if (reports_for_family)
                    reports_for_family->num_runs_total += num_repeats_of_this_instance;

                RunResults* results = forward_allocator->Construct<RunResults>();
                results->non_aggregates.Init(forward_allocator, 0, num_repeats_of_this_instance);
                results->aggregates_only.Init(forward_allocator, 0, num_repeats_of_this_instance);
                InitRunResults(runner, globals, results);

                run_results.PushBack(results);
            }
            ASSERTS(runners.Size() == benchmark_instances.Size(), "Unexpected runner count.");

            Array<s32> repetition_indices;
            repetition_indices.Init(scratch_allocator, 0, num_repetitions_total);

            for (s32 runner_index = 0, num_runners = runners.Size(); runner_index != num_runners; ++runner_index)
            {
                const BenchMarkRunner* runner = runners[runner_index];
                s32                    i      = GetNumRepeats(runner);
                while (i--)
                    repetition_indices.PushBack(runner_index);
            }
            ASSERTS(repetition_indices.Size() == num_repetitions_total, "Unexpected number of repetition indexes.");

            if (globals->benchmark_enable_random_interleaving)
            {
                RandomShuffle(repetition_indices, (u64)globals->benchmark_random_interleaving_seed);
            }

            for (s32 i = 0; i < repetition_indices.Size(); ++i)
            {
                const s32        repetition_index = repetition_indices[i];
                BenchMarkRunner* runner           = runners[repetition_index];
                RunResults*      results          = run_results[repetition_index];

                BenchMarkRun*& report = results->non_aggregates.Alloc();
                report                = forward_allocator->Construct<BenchMarkRun>();

                DoOneRepetition(runner, forward_allocator, scratch_allocator, report, reports_for_family);
                if (HasRepeatsRemaining(runner))
                    continue;

                reporter->ReportRunsConfig(GetMinTime(runner), HasExplicitIters(runner), GetIters(runner), forward_allocator, scratch_allocator);

                AggregateResults(runner, forward_allocator, scratch_allocator, results->non_aggregates, results->aggregates_only);

                // Maybe calculate complexity report
                if (reports_for_family != nullptr)
                {
                    if (reports_for_family->num_runs_done == reports_for_family->num_runs_total)
                    {
                        USE_SCRATCH(scratch_allocator);

                        Array<BenchMarkRun*> additional_run_stats;
                        additional_run_stats.Init(scratch_allocator, 0, 2);
                        ComputeBigO(forward_allocator, scratch_allocator, reports_for_family->runs, additional_run_stats);

                        // run_results->aggregates_only.insert(run_results->aggregates_only.end(), additional_run_stats.begin(), additional_run_stats.end());
                        for (int i = 0; i < additional_run_stats.Size(); ++i)
                            results->aggregates_only.PushBack(additional_run_stats[i]);
                    }
                }

                Report(reporter, results, forward_allocator, scratch_allocator);

                // Destroy the reports
                for (int i = 0; i < results->non_aggregates.Size(); ++i)
                {
                    results->non_aggregates[i]->Reset();
                    forward_allocator->Destruct(results->non_aggregates[i]);
                }
                for (int i = 0; i < results->aggregates_only.Size(); ++i)
                {
                    results->aggregates_only[i]->Reset();
                    forward_allocator->Destruct(results->aggregates_only[i]);
                }
                forward_allocator->Destruct(results);
            }

            // Destroy the run results array
            run_results.Release();

            // Destroy the reports for family
            if (reports_for_family != nullptr)
            {
                scratch_allocator->Destruct(reports_for_family);
            }

            // Destroy all runners
            for (int i = 0; i < runners.Size(); ++i)
            {
                DestroyRunner(runners[i], forward_allocator);
            }
            runners.Release();
        }

        reporter->Finalize(forward_allocator, scratch_allocator);
        reporter->Flush(forward_allocator, scratch_allocator);
    }

    // Permutations is determined by the number of inputs to repeat a benchmark on.
    // If this is "large" then warn the user during configuration.
    static constexpr size_t kMaxPerms = 100;

    static void DestroyBenchMarkInstances(ForwardAllocator* forward_allocator, Array<BenchMarkInstance*>& benchmark_instances)
    {
        for (s32 i = 0; i < benchmark_instances.Size(); ++i)
        {
            benchmark_instances[i]->release(forward_allocator);
            forward_allocator->Destruct(benchmark_instances[i]);
        }
        benchmark_instances.Release();
    }

    static bool CreateBenchMarkInstances(ForwardAllocator* forward_allocator, ScratchAllocator* scratch_allocator, BenchMarkUnit* benchmark, Array<BenchMarkInstance*>& benchmark_instances)
    {
        USE_SCRATCH(scratch_allocator);

        const Array<s32>& thread_counts     = benchmark->thread_counts_;
        const s32         num_thread_counts = thread_counts.Empty() ? 1 : thread_counts.Size();

        // Have BenchMarkUnit create the arguments for the instances
        Array<Array<s32>> args;
        const s32          perms = benchmark->BuildArgs(scratch_allocator, args);
        benchmark_instances.Init(forward_allocator, 0, perms * num_thread_counts);

        for (s32 i = 0; i < num_thread_counts; ++i)
        {
            for (s32 arg_index = 0; arg_index < perms; ++arg_index)
            {
                const s32 num_threads = thread_counts.Empty() ? 1 : thread_counts[i];

                BenchMarkInstance* instance = forward_allocator->Construct<BenchMarkInstance>();
                instance->initialize(forward_allocator, benchmark, args[arg_index], num_threads);

                benchmark_instances.PushBack(instance);
            }
        }

        // Each arg has been given to the instances, so we can destroy the args array.
        // The indivual args are owned by the instances and will be destroyed when the instances are released.
        for (s32 i = 0; i < args.Size(); ++i)
            args[i].Release();
        args.Release();

        return true;
    }

    namespace BenchMarkSuiteList
    {
        static BenchMarkSuite* head = nullptr;
        static BenchMarkSuite* tail = nullptr;
    } // namespace BenchMarkSuiteList

    static s32 gRegisterCounterName(const char** const counters_array, const char**& counters, const char* name)
    {
        // In the array find 'name', if found return the index, if not found add it to the array
        s32          index = 0;
        const char** iter  = counters_array;
        while (iter != counters)
        {
            if (gCompareStrings(*iter, name) == 0)
                return index;
            ++iter;
            ++index;
        }
        counters[0] = name;
        ++counters;
        return index;
    }

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

    // A benchmark-suite has a list of benchmark-fixtures where every fixture has a list of benchmark-units.
    static void RunBenchMarkSuite(Allocator* main_allocator, ForwardAllocator* forward_allocator, ScratchAllocator* scratch_allocator, BenchMarkGlobals* globals, BenchMarkSuite* suite, BenchMarkReporter* reporter)
    {
        // Report the details of this benchmark suite ?
        // - name / filename / line number
        // - list
        //   - fixture name(num units)

        BenchMarkFixture* fixture = suite->head;
        while (fixture != nullptr)
        {
            // Report the details of this fixture ?
            // - name / filename / line number / num units

            if (fixture->disabled)
            {
                // Report this fixture as disabled?
                fixture = fixture->next;
                continue;
            }

            BenchMarkUnit* unit = fixture->head;
            while (unit != nullptr)
            {
                if (!unit->IsDisabled())
                {
                    forward_allocator->Reset();

                    // Apply the settings for this benchmark unit, from Suite, Fixture and Unit
                    // Two passes
                    for (s32 i = 0; i < 2; ++i)
                    {
                        switch (i)
                        {
                            case 0: unit->PrepareSettings(); break;
                            case 1: unit->ApplySettings(forward_allocator); break;
                        }
                        suite->settings(unit);
                        fixture->settings(unit);
                        unit->settings_(unit);
                    }

                    Array<BenchMarkInstance*> benchmark_instances;
                    if (CreateBenchMarkInstances(forward_allocator, scratch_allocator, unit, benchmark_instances))
                    {
                        // Report the details of this benchmark unit ?
                        // - name / filename / line number

                        RunBenchMarkInstances(main_allocator, forward_allocator, scratch_allocator, globals, benchmark_instances, reporter);
                    }

                    // Destroy the benchmark instances
                    DestroyBenchMarkInstances(forward_allocator, benchmark_instances);

                    // Reset the settings for this benchmark unit (release memory)
                    unit->ReleaseSettings();
                }

                unit = unit->next;
            }
            fixture = fixture->next;
        }
    }

    static bool RunBenchMarks(Allocator* main_allocator, BenchMarkGlobals* globals, BenchMarkReporter* reporter)
    {
        ScratchAllocator _scratch_allocator;
        ForwardAllocator _forward_allocator;
        _scratch_allocator.Initialize(main_allocator, (s64)1024 * 1024);
        _forward_allocator.Initialize(main_allocator, (s64)8 * 1024 * 1024);

        ScratchAllocator* scratch_allocator = &_scratch_allocator;
        ForwardAllocator* forward_allocator = &_forward_allocator;

        BenchMarkSuite* suite = BenchMarkSuiteList::head;
        while (suite != nullptr)
        {
            if (!suite->disabled)
            {
                RunBenchMarkSuite(main_allocator, forward_allocator, scratch_allocator, globals, suite, reporter);
            }
            suite = suite->next;
        }
        return true;
    }

    bool gRunBenchMark(MainAllocator* allocator, BenchMarkGlobals* globals, BenchMark::BenchMarkReporter& reporter) { return BenchMark::RunBenchMarks(allocator, globals, &reporter); }

} // namespace BenchMark
