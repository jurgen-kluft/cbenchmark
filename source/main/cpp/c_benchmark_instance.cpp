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

    BenchMarkRunResult::BenchMarkRunResult()
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

    void BenchMarkRunResult::Reset()
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

    void BenchMarkRunResult::Initialize(Allocator* alloc, BenchMarkInstance const* instance)
    { 
        Reset();
        if (instance->counters() != nullptr)
            counters.Initialize(alloc, instance->counters()->Size());
    }

    void BenchMarkRunResult::Shutdown()
    { 
        counters.Release();
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
        , args_(nullptr)
        , threads_(1)
    {
    }

    void BenchMarkInstance::run(BenchMarkState& state, Allocator* allocator) const { benchmark_->run_(state, allocator); }

    void BenchMarkInstance::initialize(ForwardAllocator* allocator, BenchMarkUnit* benchmark, Array<s32> const* args, int thread_count)
    {
        benchmark_ = benchmark;
        threads_   = (thread_count);
        args_      = args;

        // 'Reserve' enough memory for the name and parts.
        const s32 nameSize       = 511;
        char*     str            = allocator->Checkout<char>(nameSize + 1);
        str[nameSize]            = '\0';
        const char* const strEnd = str + nameSize;
        {
            name_.function_name = str;

            // Name/{ArgName:}Arg/{ArgName:}Arg/..
            str = gStringAppend(str, strEnd, benchmark->name);
            str = gStringAppendTerminator(str, strEnd);

            name_.args = str;
            if (args_ != nullptr)
            {
                for (s32 i = 0; i < args_->Size(); ++i)
                {
                    if (str > name_.args)
                    {
                        str = gStringAppend(str, strEnd, '/');
                    }

                    if (benchmark_->args_count_ > i && benchmark_->args_[i].name_ != nullptr)
                    {
                        str = gStringFormatAppend(str, strEnd, "%s:", benchmark_->args_[i].name_);
                    }

                    str = gStringFormatAppend(str, strEnd, "%d", (*args_)[i]);
                }
            }
            str = gStringAppendTerminator(str, strEnd);

            name_.min_time = str;
            if (!gIsZero(benchmark->min_time_))
            {
                str = gStringFormatAppend(str, strEnd, "min_time:%0.3f", benchmark->min_time_);
            }
            str = gStringAppendTerminator(str, strEnd);

            name_.min_warmup_time = str;
            if (!gIsZero(benchmark->min_warmup_time_))
            {
                str = gStringFormatAppend(str, strEnd, "min_warmup_time:%0.3f", benchmark->min_warmup_time_);
            }
            str = gStringAppendTerminator(str, strEnd);

            name_.iterations = str;
            if (benchmark->iterations_ != 0)
            {
                str = gStringFormatAppend(str, strEnd, "iterations:%lu", static_cast<unsigned long>(benchmark->iterations_));
            }
            str = gStringAppendTerminator(str, strEnd);

            name_.repetitions = str;
            if (benchmark->repetitions_ != 0)
            {
                str = gStringFormatAppend(str, strEnd, "repeats:%d", benchmark->repetitions_);
            }
            str = gStringAppendTerminator(str, strEnd);

            s32 time_types  = 0;
            name_.time_type = str;
            if (benchmark->time_settings_.MeasureProcessCpuTime())
            {
                str = gStringAppend(str, strEnd, "process_time");
                time_types++;
            }
            str = gStringAppendTerminator(str, strEnd);

            if (benchmark->time_settings_.UseManualTime())
            {
                if (time_types > 0)
                {
                    str = gStringAppend(str, strEnd, '/');
                }
                name_.time_type = str;
                str             = gStringAppend(str, strEnd, "manual_time");
                time_types++;
            }
            else if (benchmark->time_settings_.UseRealTime())
            {
                if (time_types > 0)
                {
                    str = gStringAppend(str, strEnd, '/');
                }
                name_.time_type = str;
                str             = gStringAppend(str, strEnd, "real_time");
                time_types++;
            }
            str = gStringAppendTerminator(str, strEnd);

            name_.threads = str;
            if (!benchmark->thread_counts_.Empty())
            {
                str = gStringFormatAppend(str, strEnd, "threads:%d", threads_);
            }

            str = gStringAppendTerminator(str, strEnd);
        }
        allocator->Commit(str);
    }

} // namespace BenchMark