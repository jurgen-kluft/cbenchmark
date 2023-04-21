#ifndef __CBENCHMARK_BENCHMARK_DECLARED_H__
#define __CBENCHMARK_BENCHMARK_DECLARED_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    class BenchMarkState;

    // -----------------------------------------------------------------
    // BenchMarkEntity registration object. The BM_TEST() macro expands
    // into an BenchMarkEntity* object. Various properties be set on this
    // object to change the execution of the benchmark.
    class BenchMarkEntity
    {
    public:
        enum ESettings
        {
            MaxPermutations = 16,
        };
        typedef void (*callback_function)(const BenchMarkState&);

        char*                 name_;
        AggregationReportMode aggregation_report_mode_;
        char*                 arg_names_[MaxPermutations]; // Args for all benchmark runs
        Arg                   args_[MaxPermutations];      // Args for all benchmark runs
        TimeUnit              time_unit_;
        bool                  use_default_time_unit_;
        int                   range_multiplier_;
        double                min_time_;
        double                min_warmup_time_;
        IterationCount        iterations_;
        int                   repetitions_;
        bool                  measure_process_cpu_time_;
        bool                  use_real_time_;
        bool                  use_manual_time_;
        BigO                  complexity_;
        BigO::Func*           complexity_lambda_;
        Statistics            statistics_;
        int                   thread_counts_[MaxPermutations];
        callback_function     setup_;
        callback_function     teardown_;
        callback_function     run_;

    }; // namespace BenchMark
} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_DECLARED_H__