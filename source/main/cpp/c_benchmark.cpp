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
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include <algorithm>

namespace BenchMark
{
    static s32  FLAGS_benchmark_repetitions                = 1;
    static bool FLAGS_benchmark_enable_random_interleaving = false;

    // Flushes streams after invoking reporter methods that write to them. This
    // ensures users get timely updates even when streams are not line-buffered.
    void FlushStreams(BenchMarkReporter* reporter)
    {
        if (!reporter)
            return;

        reporter->GetOutputStream()->flush();
        reporter->GetErrorStream()->flush();
    }

    // Reports in both display and file reporters.
    void Report(BenchMarkReporter* display_reporter, BenchMarkReporter* file_reporter, const RunResults* run_results)
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

        report_one(display_reporter, run_results->display_report_aggregates_only, run_results);
        if (file_reporter)
            report_one(file_reporter, run_results->file_report_aggregates_only, run_results);

        FlushStreams(display_reporter);
        FlushStreams(file_reporter);
    }

    template <typename T> T min(T a, T b) { return a < b ? a : b; }
    template <typename T> T max(T a, T b) { return a > b ? a : b; }

    struct XorRandom
    {
        u64 shuffle_table[4];

        XorRandom(u64 seed)
        {
            shuffle_table[0] = seed;
            shuffle_table[1] = 0;
            shuffle_table[2] = 0;
            shuffle_table[3] = 0;
            for (int i = 0; i < 4; ++i)
                next();
        }

        u64 next(void)
        {
            u64 s1           = shuffle_table[0];
            u64 s0           = shuffle_table[1];
            u64 result       = s0 + s1;
            shuffle_table[0] = s0;
            s1 ^= s1 << 23;
            shuffle_table[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
            return result;
        }
    }

    RandomShuffle(Array<s64>& indices)
    {
        XorRandom rng(0xdeadbeef);
        for (int i = 0; i < indices.Size(); ++i)
        {
            s64 j = i + (rng.next() % (indices.Size() - i));
            std::swap(indices[i], indices[j]);
        }
    }

    void RunBenchmarks(Allocator* allocator, const Array<BenchMarkInstance*>& benchmarks, BenchMarkReporter* display_reporter, BenchMarkReporter* file_reporter)
    {
        // Note the file_reporter can be null.
        BM_CHECK(display_reporter != nullptr);

        // Determine the width of the name field using a minimum width of 10.
        bool might_have_aggregates = FLAGS_benchmark_repetitions > 1;
        s64  name_field_width      = 10;
        s64  stat_field_width      = 0;
        for (int i = 0; i < benchmarks.Size(); ++i)
        {
            const BenchMarkInstance* benchmark = benchmarks[i];
            name_field_width                   = max<s64>(name_field_width, benchmark->name().FullNameLen());
            might_have_aggregates |= benchmark->repetitions() > 1;

            Statistics const& stats = benchmark->statistics();
            for (int j = 0; j < stats.Size(); ++j)
            {
                Statistic const& Stat = stats[j];
                stat_field_width      = max<s64>(stat_field_width, gStringLength(Stat.name_));
            }
        }
        if (might_have_aggregates)
            name_field_width += 1 + stat_field_width;

        // Print header here
        BenchMarkReporter::Context context;
        context.name_field_width = name_field_width;

        // Keep track of running times of all instances of each benchmark family.
        // std::map<int /*family_index*/, BenchMarkReporter::PerFamilyRunReports> per_family_reports;
        Array<int>                                     family_indices;
        Array<BenchMarkReporter::PerFamilyRunReports*> per_family_reports;

        if (display_reporter->ReportContext(context) && (!file_reporter || file_reporter->ReportContext(context)))
        {
            FlushStreams(display_reporter);
            FlushStreams(file_reporter);

            s32 num_repetitions_total = 0;

            // Benchmarks to run
            Array<BenchMarkRunner*> runners;
            runners.Init(allocator, 0, benchmarks.Size());

            // Count the number of benchmarks with threads to warn the user in case
            // performance counters are used.
            int benchmarks_with_threads = 0;

            // Loop through all benchmarks
            for (int i = 0; i < benchmarks.Size(); ++i)
            {
                const BenchMarkInstance* benchmark = benchmarks[i];

                BenchMarkReporter::PerFamilyRunReports* reports_for_family = nullptr;
                if (!benchmark->complexity().Is(BigO::O_None))
                {
                    s32 const family_index = family_indices.Find(benchmark->family_index());
                    if (family_index == -1)
                    {
                        family_indices.PushBack(benchmark->family_index());
                        reports_for_family = allocator->Construct<BenchMarkReporter::PerFamilyRunReports>();
                        per_family_reports.PushBack(reports_for_family);
                    }
                    else
                    {
                        reports_for_family = per_family_reports[family_index];
                    }
                }

                benchmarks_with_threads += (benchmark->threads() > 0);

                BenchMarkRunner* runner = allocator->Construct<BenchMarkRunner>(allocator, benchmark, reports_for_family);
                runners.PushBack(runner);

                int num_repeats_of_this_instance = GetNumRepeats(runner);
                num_repetitions_total += num_repeats_of_this_instance;
                if (reports_for_family)
                    reports_for_family->num_runs_total += num_repeats_of_this_instance;
            }
            BM_CHECK(runners.Size() == benchmarks.Size() && "Unexpected runner count.");

            Array<s64> repetition_indices;
            repetition_indices.Init(allocator, 0, num_repetitions_total);

            for (s32 runner_index = 0, num_runners = runners.Size(); runner_index != num_runners; ++runner_index)
            {
                const BenchMarkRunner* runner = runners[runner_index];
                int                    i      = GetNumRepeats(runner);
                while (i--)
                    repetition_indices.PushBack(runner_index);
            }
            BM_CHECK(repetition_indices.Size() == num_repetitions_total && "Unexpected number of repetition indexes.");

            if (FLAGS_benchmark_enable_random_interleaving)
            {
                RandomShuffle(repetition_indices);
            }

            for (int i = 0; i < repetition_indices.Size(); ++i)
            {
                const s64        repetition_index = repetition_indices[i];
                BenchMarkRunner* runner           = runners[repetition_index];

                DoOneRepetition(runner);
                if (HasRepeatsRemaining(runner))
                    continue;

                // FIXME: report each repetition separately, not all of them in bulk.

                display_reporter->ReportRunsConfig(GetMinTime(runner), HasExplicitIters(runner), GetIters(runner));
                if (file_reporter)
                    file_reporter->ReportRunsConfig(GetMinTime(runner), HasExplicitIters(runner), GetIters(runner));

                RunResults* run_results = GetResults(runner);

                // Maybe calculate complexity report
                if (auto* reports_for_family = GetReportsForFamily(runner))
                {
                    if (reports_for_family->num_runs_done == reports_for_family->num_runs_total)
                    {
                        Array<BenchMarkRun*> additional_run_stats;
                        additional_run_stats.Init(allocator, 0, 2);
                        ComputeBigO(allocator, reports_for_family->runs, additional_run_stats);

                        // run_results->aggregates_only.insert(run_results->aggregates_only.end(), additional_run_stats.begin(), additional_run_stats.end());
                        run_results->aggregates_only.PushBack(additional_run_stats);

                        const s32 family_index       = reports_for_family->runs.Front()->family_index;
                        const s32 family_index_index = family_indices.Find(family_index);

                        // Remove the family
                        family_indices.Erase(family_index_index);
                        per_family_reports.Erase(family_index_index);

                        // Destroy the family object, when using a forward allocator this does nothing
                        allocator->Destruct(reports_for_family);
                    }
                }

                Report(display_reporter, file_reporter, run_results);
            }
        }
        display_reporter->Finalize();
        if (file_reporter)
            file_reporter->Finalize();

        FlushStreams(display_reporter);
        FlushStreams(file_reporter);
    }

} // namespace BenchMark
