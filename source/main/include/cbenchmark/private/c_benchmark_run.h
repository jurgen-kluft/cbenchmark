#ifndef __CBENCHMARK_BENCHMARK_RUN_H__
#define __CBENCHMARK_BENCHMARK_RUN_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"
#include "cbenchmark/private/c_benchmark_name.h"

namespace BenchMark
{
    class BenchMarkState;

    class BenchMarkRun
    {
    public:
        static const s64 no_repetition_index = -1;

        enum RunType
        {
            RT_Iteration,
            RT_Aggregate
        };

        BenchMarkRun()
            : run_name()
            , run_type(RT_Iteration)
            , aggregate_name(nullptr)
            , aggregate_unit({StatisticUnit::Time})
            , report_format(nullptr)
            , report_value(0.0)
            , skipped(Skipped::NotSkipped)
            , skip_message(nullptr)
            , iterations(1)
            , threads(1)
            , repetition_index(0)
            , repetitions(0)
            , time_unit(TimeUnit::Nanosecond)
            , real_accumulated_time(0)
            , cpu_accumulated_time(0)
            , max_heapbytes_used(0)
            , complexity(BigO::O_None)
            , complexity_lambda()
            , complexity_n(0)
            , statistics()
            , report_big_o(false)
            , report_rms(false)
            , counters()
            //, memory_result(NULL)
            , allocs_per_iter(0.0)
        {
        }

        const char* BenchMarkName(Allocator* alloc);

        BenchmarkName run_name;
        RunType       run_type;
        const char*   aggregate_name;
        StatisticUnit aggregate_unit;
        const char*   report_format; // Empty if not set by benchmark.
        double        report_value;
        Skipped       skipped;
        const char*   skip_message;

        IterationCount iterations;
        s64            threads;
        s64            repetition_index;
        s64            repetitions;
        TimeUnit       time_unit;
        double         real_accumulated_time;
        double         cpu_accumulated_time;

        // Return a value representing the real time per iteration in the unit
        // specified by 'time_unit'.
        // NOTE: If 'iterations' is zero the returned value represents the
        // accumulated time.
        double GetAdjustedRealTime() const;

        // Return a value representing the cpu time per iteration in the unit
        // specified by 'time_unit'.
        // NOTE: If 'iterations' is zero the returned value represents the
        // accumulated time.
        double GetAdjustedCPUTime() const;

        // This is set to 0.0 if memory tracing is not enabled.
        double max_heapbytes_used;

        // Keep track of arguments to compute asymptotic complexity
        BigO        complexity;
        BigO::Func* complexity_lambda;
        s64         complexity_n;

        // what statistics to compute from the measurements
        Array<Statistic> statistics;

        // Inform print function whether the current run is a complexity report
        bool report_big_o;
        bool report_rms;

        Counters counters;

        // Memory metrics.
        // const MemoryManager::Result* memory_result;
        double allocs_per_iter;
    };

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_RUN_H__