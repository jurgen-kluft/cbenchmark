#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_exception.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_alloc.h"
#include "cbenchmark/private/c_benchmark_name.h"
#include "cbenchmark/private/c_benchmark_instance.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_entity.h"

namespace BenchMark
{
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
        , family_index_(0)
        , per_family_instance_index_(0)
        , aggregation_report_mode_(AggregationReportMode::Default)
        , args_()
        , time_unit_(TimeUnit::Nanosecond)
        , measure_process_cpu_time_(false)
        , use_real_time_(false)
        , use_manual_time_(false)
        , complexity_(BigO::O_None)
        , complexity_lambda_(nullptr)
        , counters_()
        , statistics_()
        , repetitions_(1)
        , min_time_(0.0)
        , min_warmup_time_(0.0)
        , iterations_(0)
        , threads_(1)
    {
    }

    void BenchMarkInstance::Setup() const
    {
        if (setup_)
        {
            BenchMarkState st(name_.function_name, /*iters*/ 1, &args_, /*thread_id*/ 0, threads_, nullptr, nullptr, nullptr);
            setup_(st);
        }
    }

    void BenchMarkInstance::Teardown() const
    {
        if (teardown_)
        {
            BenchMarkState st(name_.function_name, /*iters*/ 1, &args_, /*thread_id*/ 0, threads_, nullptr, nullptr, nullptr);
            teardown_(st);
        }
    }

    BenchMarkState BenchMarkInstance::Run(IterationCount iters, int thread_id, ThreadTimer* timer, ThreadManager* manager, BenchMarkRunResult* results) const
    {
        BenchMarkState state(name_.function_name, iters, &args_, thread_id, threads_, timer, manager, results);
        benchmark_->run_(state);
        return state;
    }

    void BenchMarkInstance::init(Allocator* allocator, BenchMarkEntity* benchmark, int family_index, int per_family_instance_index, int thread_count, Arg* args)
    {
        benchmark_                 = benchmark;
        family_index_              = family_index;
        per_family_instance_index_ = (per_family_instance_index);
        aggregation_report_mode_   = (benchmark->aggregation_report_mode_);
        time_unit_                 = (benchmark->time_unit_);
        measure_process_cpu_time_  = (benchmark->measure_process_cpu_time_);
        use_real_time_             = (benchmark->use_real_time_);
        use_manual_time_           = (benchmark->use_manual_time_);
        complexity_                = (benchmark->complexity_);
        complexity_lambda_         = (benchmark->complexity_lambda_);
        statistics_                = (benchmark->statistics_);
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