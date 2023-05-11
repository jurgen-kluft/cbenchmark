#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_allocators.h"
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
        , aggregation_report_mode_(AggregationReportMode::Default)
        , args_()
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

    void BenchMarkInstance::run(BenchMarkState& state, Allocator* allocator) const { benchmark_->run_(state, allocator); }

    void BenchMarkInstance::initialize(ForwardAllocator* allocator, BenchMarkUnit* benchmark, Array<s32> const& args, int thread_count)
    {
        benchmark_               = benchmark;
        aggregation_report_mode_ = (benchmark->aggregation_report_mode_);
        time_unit_               = (benchmark->time_unit_);
        time_settings_           = (benchmark->time_settings_);
        complexity_              = (benchmark->complexity_);
        complexity_lambda_       = (benchmark->complexity_lambda_);
        statistics_              = (&benchmark->statistics_);
        repetitions_             = (benchmark->repetitions_);
        min_time_                = (benchmark->min_time_);
        min_warmup_time_         = (benchmark->min_warmup_time_);
        iterations_              = (benchmark->iterations_);
        threads_                 = (thread_count);

        args_.Init(allocator, 0, args.Size());
        for (s32 i = 0; i < args.Size(); ++i)
            args_.PushBack(args[i]);

        // 'Reserve' enough memory for the name and parts.
        char* str = allocator->Checkout<char>(512);
        {
            name_.function_name = str;

            // Name/{ArgName:}Arg/{ArgName:}Arg/..
            str = gStringAppend(str, nullptr, benchmark->name);
            for (s32 i = 0; i < args_.Size(); ++i)
            {
                if (i != 0)
                {
                    str = gStringAppend(str, nullptr, '/');
                }

                if (benchmark_->arg_names_count_ > i)
                {
                    str = gStringFormatAppend(str, nullptr, "%s:", benchmark_->arg_names_[i]);
                }

                str = gStringFormatAppend(str, nullptr, "%d", args_[i]);
            }
            *str++ = '\0'; // Terminate

            if (!gIsZero(benchmark->min_time_))
            {
                name_.min_time = str;
                str            = gStringFormatAppend(str, nullptr, "min_time:%0.3f", benchmark->min_time_);
                *str++         = '\0'; // Terminate
            }

            if (!gIsZero(benchmark->min_warmup_time_))
            {
                name_.min_warmup_time = str;
                str                   = gStringFormatAppend(str, nullptr, "min_warmup_time:%0.3f", benchmark->min_warmup_time_);
                *str++                = '\0'; // Terminate
            }

            if (benchmark->iterations_ != 0)
            {
                name_.iterations = str;
                str              = gStringFormatAppend(str, nullptr, "iterations:%lu", static_cast<unsigned long>(benchmark->iterations_));
                *str++           = '\0'; // Terminate
            }

            if (benchmark->repetitions_ != 0)
            {
                name_.repetitions = str;
                str               = gStringFormatAppend(str, nullptr, "repeats:%d", benchmark->repetitions_);
                *str++            = '\0'; // Terminate
            }

            s32 time_types  = 0;
            name_.time_type = str;
            if (benchmark->time_settings_.MeasureProcessCpuTime())
            {
                str = gStringAppend(str, nullptr, "process_time");
                time_types++;
            }

            if (benchmark->time_settings_.UseManualTime())
            {
                if (time_types > 0)
                {
                    str = gStringAppend(str, nullptr, '/');
                }
                name_.time_type = str;
                str             = gStringAppend(str, nullptr, "manual_time");
                time_types++;
            }
            else if (benchmark->time_settings_.UseRealTime())
            {
                if (time_types > 0)
                {
                    str = gStringAppend(str, nullptr, '/');
                }
                name_.time_type = str;
                str             = gStringAppend(str, nullptr, "real_time");
                time_types++;
            }

            if (time_types > 0)
            {
                *str++ = '\0'; // Terminate
            }
            else
            {
                name_.time_type = nullptr;
            }

            if (!benchmark->thread_counts_.Empty())
            {
                name_.threads = str;
                str           = gStringFormatAppend(str, nullptr, "threads:%d", threads_);
                *str++        = '\0'; // Terminate
            }

            *str++ = '\0'; // Terminate
        }
        allocator->Commit(str);

        setup_    = benchmark->setup_;
        teardown_ = benchmark->teardown_;
    }

} // namespace BenchMark