#ifndef __CBENCHMARK_BENCHMARK_INSTANCE_H__
#define __CBENCHMARK_BENCHMARK_INSTANCE_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_name.h"
#include "cbenchmark/private/c_benchmark_unit.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    class BenchMarkState;
    class ThreadTimer;
    class ThreadManager;
    class PerfCountersMeasurement;

    struct BenchMarkGlobals
    {
        BenchMarkGlobals();

        double FLAGS_benchmark_min_time;
        double FLAGS_benchmark_min_warmup_time;
        bool   FLAGS_benchmark_report_aggregates_only;
        bool   FLAGS_benchmark_display_aggregates_only;
        s32    FLAGS_benchmark_repetitions;
        bool   FLAGS_benchmark_enable_random_interleaving;
    };

    static BenchMarkGlobals g_benchmark_globals;

    struct BenchMarkRunResult
    {
        BenchMarkRunResult()
            : allocator(nullptr)
            , iterations(0)
            , real_time_used(0.0)
            , cpu_time_used(0.0)
            , manual_time_used(0.0)
            , complexity_n(0)
            , skipped_(Skipped::NotSkipped)
            , report_label_(nullptr)
            , skip_message_(nullptr)
        {
        }

        Allocator*     allocator;
        IterationCount iterations;
        double         real_time_used;
        double         cpu_time_used;
        double         manual_time_used;
        s64            complexity_n;
        Counters       counters;
        Skipped        skipped_;
        const char*    report_label_;
        const char*    skip_message_;

        void Merge(const BenchMarkRunResult& other);
    };

    // Information kept per benchmark we may want to run
    class BenchMarkInstance
    {
    public:
        BenchMarkInstance();

        void                    init(Allocator* allocator, BenchMarkUnit* benchmark, int per_family_instance_index, int thread_count);
        const BenchmarkName&    name() const { return name_; }
        Array<s32> const*       args() const { return args_; }
        int                     per_family_instance_index() const { return per_family_instance_index_; }
        AggregationReportMode   aggregation_report_mode() const { return aggregation_report_mode_; }
        TimeUnit                time_unit() const { return time_unit_; }
        bool                    measure_process_cpu_time() const { return time_settings_.MeasureProcessCpuTime(); }
        bool                    use_real_time() const { return time_settings_.UseRealTime(); }
        bool                    use_manual_time() const { return time_settings_.UseManualTime(); }
        BigO                    complexity() const { return complexity_; }
        BigO::Func*             complexity_lambda() const { return complexity_lambda_; }
        Array<Statistic> const& statistics() const { return *statistics_; }
        int                     repetitions() const { return repetitions_; }
        double                  min_time() const { return min_time_; }
        double                  min_warmup_time() const { return min_warmup_time_; }
        IterationCount          iterations() const { return iterations_; }
        int                     threads() const { return threads_; }
        setup_function          setup() const { return setup_; }
        teardown_function       teardown() const { return teardown_; }
        void                    run(BenchMarkState& state, Allocator* allocator) const;

    private:
        BenchmarkName           name_;
        BenchMarkUnit*          benchmark_;
        int                     per_family_instance_index_;
        AggregationReportMode   aggregation_report_mode_;
        Array<s32>*             args_;
        TimeUnit                time_unit_;
        TimeSettings            time_settings_;
        BigO                    complexity_;
        BigO::Func*             complexity_lambda_;
        Array<Counter> const*   counters_;
        Array<Statistic> const* statistics_;
        int                     repetitions_;
        double                  min_time_;
        double                  min_warmup_time_;
        IterationCount          iterations_;
        int                     threads_; // Number of concurrent threads to us

        setup_function    setup_    = nullptr;
        teardown_function teardown_ = nullptr;
    };

} // namespace BenchMark

#endif //__CBENCHMARK_BENCHMARK_INSTANCE_H__
