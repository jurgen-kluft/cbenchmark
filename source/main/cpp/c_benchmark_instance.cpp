#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_alloc.h"
#include "cbenchmark/private/c_benchmark_name.h"
#include "cbenchmark/private/c_benchmark_instance.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_unit.h"

namespace BenchMark
{
    BenchMarkGlobals::BenchMarkGlobals()
    {
        FLAGS_benchmark_min_time                   = 0.5;
        FLAGS_benchmark_min_warmup_time            = 0.5;
        FLAGS_benchmark_report_aggregates_only     = true;
        FLAGS_benchmark_display_aggregates_only    = false;
        FLAGS_benchmark_repetitions                = 1;
        FLAGS_benchmark_enable_random_interleaving = false;
    }

    void BenchMarkRunResult::Merge(const BenchMarkRunResult& other)
    {
        iterations += other.iterations;
        cpu_time_used += other.cpu_time_used;
        real_time_used += other.real_time_used;
        manual_time_used += other.manual_time_used;
        complexity_n += other.complexity_n;
        Counters::Increment(counters, other.counters);

        // TODO

        // Merge Skipped
        // Skipped        skipped_;

        // Merge ReportLabel
        // char           report_label_[64];

        // Merge SkipMessage
        // char           skip_message_[64];
    }

    BenchMarkInstance::BenchMarkInstance()
        : name_()
        , benchmark_(nullptr)
        , per_family_instance_index_(0)
        , aggregation_report_mode_(AggregationReportMode::Default)
        , args_(nullptr)
        , time_unit_(TimeUnit::Nanosecond)
        , time_settings_()
        , complexity_(BigO::O_None)
        , complexity_lambda_(nullptr)
        , counters_(nullptr)
        , statistics_(nullptr)
        , repetitions_(1)
        , min_time_(0.0)
        , min_warmup_time_(0.0)
        , iterations_(0)
        , threads_(1)
    {
    }

    void BenchMarkInstance::run(BenchMarkState& state, Allocator* allocator) const
    {
        benchmark_->run_(state, allocator);
    }

    void BenchMarkInstance::init(Allocator* allocator, BenchMarkUnit* benchmark, int per_family_instance_index, int thread_count)
    {
        benchmark_                 = benchmark;
        per_family_instance_index_ = (per_family_instance_index);
        aggregation_report_mode_   = (benchmark->aggregation_report_mode_);
        time_unit_                 = (benchmark->time_unit_);
        time_settings_             = (benchmark->time_settings_);
        complexity_                = (benchmark->complexity_);
        complexity_lambda_         = (benchmark->complexity_lambda_);
        statistics_                = (&benchmark->statistics_);
        repetitions_               = (benchmark->repetitions_);
        min_time_                  = (benchmark->min_time_);
        min_warmup_time_           = (benchmark->min_warmup_time_);
        iterations_                = (benchmark->iterations_);
        threads_                   = (thread_count);

        // args_ = (args);

        // name_.function_name = benchmark->name_;

        // size_t arg_i = 0;
        // for (const auto& arg : args)
        // {
        //     if (!name_.args.empty())
        //     {
        //         name_.args += '/';
        //     }

        //     if (arg_i < benchmark->arg_names_.size())
        //     {
        //         const auto& arg_name = benchmark->arg_names_[arg_i];
        //         if (!arg_name.empty())
        //         {
        //             name_.args += StrFormat("%s:", arg_name.c_str());
        //         }
        //     }

        //     name_.args += StrFormat("%" PRId64, arg);
        //     ++arg_i;
        // }

        // if (!IsZero(benchmark->min_time_))
        // {
        //     name_.min_time = StrFormat("min_time:%0.3f", benchmark->min_time_);
        // }

        // if (!IsZero(benchmark->min_warmup_time_))
        // {
        //     name_.min_warmup_time = StrFormat("min_warmup_time:%0.3f", benchmark->min_warmup_time_);
        // }

        // if (benchmark->iterations_ != 0)
        // {
        //     name_.iterations = StrFormat("iterations:%lu", static_cast<unsigned long>(benchmark->iterations_));
        // }

        // if (benchmark->repetitions_ != 0)
        // {
        //     name_.repetitions = StrFormat("repeats:%d", benchmark->repetitions_);
        // }

        // if (benchmark->measure_process_cpu_time_)
        // {
        //     name_.time_type = "process_time";
        // }

        // if (benchmark->use_manual_time_)
        // {
        //     if (!name_.time_type.empty())
        //     {
        //         name_.time_type += '/';
        //     }
        //     name_.time_type += "manual_time";
        // }
        // else if (benchmark->use_real_time_)
        // {
        //     if (!name_.time_type.empty())
        //     {
        //         name_.time_type += '/';
        //     }
        //     name_.time_type += "real_time";
        // }

        // if (!benchmark->thread_counts_.empty())
        // {
        //     name_.threads = StrFormat("threads:%d", threads_);
        // }

        setup_    = benchmark->setup_;
        teardown_ = benchmark->teardown_;
    }

} // namespace BenchMark