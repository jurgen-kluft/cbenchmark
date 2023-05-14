#ifndef __CBENCHMARK_BENCHMARK_INSTANCE_H__
#define __CBENCHMARK_BENCHMARK_INSTANCE_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_array.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_name.h"
#include "cbenchmark/private/c_benchmark_unit.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    class Allocator;
    class ForwardAllocator;
    class BenchMarkState;
    class ThreadTimer;
    class ThreadManager;
    class PerfCountersMeasurement;

    class BenchMarkGlobals
    {
    public:
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
            , counters()
            , skipped_(Skipped::NotSkipped)
            , report_format_(nullptr)
            , report_value_(0.0)
            , skip_message_(nullptr)
        {
        }

        void Reset()
        {
            allocator        = nullptr;
            iterations       = 0;
            real_time_used   = 0.0;
            cpu_time_used    = 0.0;
            manual_time_used = 0.0;
            complexity_n     = 0;
            counters.Clear();
            skipped_       = Skipped::NotSkipped;
            report_format_ = nullptr;
            report_value_  = 0.0;
            skip_message_  = nullptr;
        }

        Allocator*     allocator;
        IterationCount iterations;
        double         real_time_used;
        double         cpu_time_used;
        double         manual_time_used;
        s64            complexity_n;
        Counters       counters;
        Skipped        skipped_;
        const char*    report_format_;
        double         report_value_;
        const char*    skip_message_;

        void Merge(const BenchMarkRunResult& other);
    };

    // Information kept per benchmark we may want to run
    class BenchMarkInstance
    {
    public:
        BenchMarkInstance();

        void initialize(ForwardAllocator* allocator, BenchMarkUnit* benchmark, Array<s32> const* args, int thread_count);
        void run(BenchMarkState& state, Allocator* allocator) const;

        const BenchmarkName& name() const { return name_; }
        Array<s32> const*    args() const { return args_; }
        int                  threads() const { return threads_; }

        AggregationReportMode   aggregation_report_mode() const { return benchmark_->aggregation_report_mode_; }
        TimeUnit                time_unit() const { return benchmark_->time_unit_; }
        bool                    measure_process_cpu_time() const { return benchmark_->time_settings_.MeasureProcessCpuTime(); }
        bool                    use_real_time() const { return benchmark_->time_settings_.UseRealTime(); }
        bool                    use_manual_time() const { return benchmark_->time_settings_.UseManualTime(); }
        BigO                    complexity() const { return benchmark_->complexity_; }
        BigO::Func*             complexity_lambda() const { return benchmark_->complexity_lambda_; }
        Array<Statistic> const& statistics() const { return benchmark_->statistics_; }
        int                     repetitions() const { return benchmark_->repetitions_; }
        double                  min_time() const { return benchmark_->min_time_; }
        double                  min_warmup_time() const { return benchmark_->min_warmup_time_; }
        s64                     memory_required() const { return benchmark_->memory_required_; }
        IterationCount          iterations() const { return benchmark_->iterations_; }
        setup_function          setup() const { return benchmark_->setup_; }
        teardown_function       teardown() const { return benchmark_->teardown_; }

    private:
        BenchMarkUnit*    benchmark_;
        BenchmarkName     name_;
        Array<s32> const* args_;
        int               threads_; // Number of concurrent threads to us
    };

} // namespace BenchMark

#endif //__CBENCHMARK_BENCHMARK_INSTANCE_H__
