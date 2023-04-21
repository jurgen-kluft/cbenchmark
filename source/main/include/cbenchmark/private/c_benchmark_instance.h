#ifndef __CBENCHMARK_BENCHMARK_INSTANCE_H__
#define __CBENCHMARK_BENCHMARK_INSTANCE_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_name.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    class BenchMarkEntity;
    class BenchMarkState;
    class ThreadTimer;
    class ThreadManager;
    class PerfCountersMeasurement;

    struct BenchMarkRunResult
    {
        BenchMarkRunResult()
            : iterations(0)
            , real_time_used(0.0)
            , cpu_time_used(0.0)
            , manual_time_used(0.0)
            , complexity_n(0)
            , skipped_({Skipped::NotSkipped})
        {
            report_label_[0] = '\0';
            skip_message_[0] = '\0';
        }

        IterationCount iterations;
        double         real_time_used;
        double         cpu_time_used;
        double         manual_time_used;
        s64            complexity_n;
        Counters       counters;
        Skipped        skipped_;
        char           report_label_[64];
        char           skip_message_[64];

        void Merge(const BenchMarkRunResult& other);
    };

    // Information kept per benchmark we may want to run
    class BenchMarkInstance
    {
    public:
        BenchMarkInstance();

        void                  init(Allocator* allocator, BenchMarkEntity * benchmark, int family_index, int per_family_instance_index, int thread_count, Arg* args);
        const BenchmarkName&  name() const { return name_; }
        int                   family_index() const { return family_index_; }
        int                   per_family_instance_index() const { return per_family_instance_index_; }
        AggregationReportMode aggregation_report_mode() const { return aggregation_report_mode_; }
        TimeUnit              time_unit() const { return time_unit_; }
        bool                  measure_process_cpu_time() const { return measure_process_cpu_time_; }
        bool                  use_real_time() const { return use_real_time_; }
        bool                  use_manual_time() const { return use_manual_time_; }
        BigO                  complexity() const { return complexity_; }
        BigO::Func*           complexity_lambda() const { return complexity_lambda_; }
        Statistics const&     statistics() const { return statistics_; }
        int                   repetitions() const { return repetitions_; }
        double                min_time() const { return min_time_; }
        double                min_warmup_time() const { return min_warmup_time_; }
        IterationCount        iterations() const { return iterations_; }
        int                   threads() const { return threads_; }

        void           Setup() const;
        void           Teardown() const;
        BenchMarkState Run(IterationCount iters, int threadid, ThreadTimer* timer, ThreadManager* manager, BenchMarkRunResult* results) const;

    private:
        BenchmarkName            name_;
        BenchMarkEntity * benchmark_;
        int                      family_index_;
        int                      per_family_instance_index_;
        AggregationReportMode    aggregation_report_mode_;
        Array<s64>               args_;
        TimeUnit                 time_unit_;
        bool                     measure_process_cpu_time_;
        bool                     use_real_time_;
        bool                     use_manual_time_;
        BigO                     complexity_;
        BigO::Func*              complexity_lambda_;
        Counters                 counters_;
        Statistics               statistics_;
        int                      repetitions_;
        double                   min_time_;
        double                   min_warmup_time_;
        IterationCount           iterations_;
        int                      threads_; // Number of concurrent threads to us

        typedef void (*callback_function)(const BenchMarkState&);
        callback_function setup_    = nullptr;
        callback_function teardown_ = nullptr;
    };

} // namespace BenchMark

#endif //__CBENCHMARK_BENCHMARK_INSTANCE_H__
