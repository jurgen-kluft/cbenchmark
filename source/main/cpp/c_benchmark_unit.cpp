#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_check.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_unit.h"
#include "cbenchmark/private/c_benchmark_instance.h"

#include <limits>

namespace BenchMark
{
    namespace nranges
    {
        template <typename T> T min(T a, T b) { return a < b ? a : b; }
        template <typename T> T max(T a, T b) { return a > b ? a : b; }

        // Creates a list of integer values for the given range and multiplier.
        // This can be used together with ArgsProduct() to allow multiple ranges
        // with different multipliers.
        static s32 ComputeRange(s64 lo, s64 hi, int mult, Array<s64>* dst);

        // Creates a list of integer values for the given range and step.
        static s32 ComputeDenseRange(s64 lo, s64 hi, int step, Array<s64>* dst);

    } // namespace nranges

    Arg_t::Arg_t()
        : count_(0)
        , count_only_(true)
        , mode_(Arg_t::Arg_Uninitialized)
        , args_()
        , name_(nullptr)
    {
    }

    void Arg_t::SetName(char const* name) { name_ = name; }

    void Arg_t::AddValue(s64 value)
    {
        if (count_only_)
        {
            mode_ = Arg_t::Arg_Value;
            count_++;
            return;
        }
        if (mode_ == Arg_Value)
        {
            args_.PushBack(value);
        }
    }

    void Arg_t::Range(s32 lo, s32 hi, s32 multiplier)
    {
        if (count_only_)
        {
            mode_  = Arg_t::Arg_Range;
            count_ = nranges::ComputeRange(lo, hi, multiplier, nullptr);
            return;
        }
        if (mode_ == Arg_Range)
        {
            nranges::ComputeRange(lo, hi, multiplier, &args_);
            count_ = args_.Size();
        }
    }

    void Arg_t::DenseRange(s32 start, s32 limit, s32 step)
    {
        if (count_only_)
        {
            mode_  = Arg_t::Arg_DenseRange;
            count_ = nranges::ComputeDenseRange(start, limit, step, nullptr);
            return;
        }
        if (mode_ == Arg_DenseRange)
        {
            nranges::ComputeDenseRange(start, limit, step, &args_);
            count_ = args_.Size();
        }
    }

    void Arg_t::SetSequence(s64 const* argv, s32 argc)
    {
        if (count_only_)
        {
            mode_  = Arg_t::Arg_Sequence;
            count_ = argc;
            return;
        }
        if (mode_ == Arg_t::Arg_Sequence)
        {
            count_ = argc;
            for (s32 i = 0; i < argc; ++i)
                args_.PushBack(argv[i]);
        }
    }

    static void IncreasePermuteVector(s32* permute_vector, s32 const* original, s32 size)
    {
        // permute_vector is an array of integers, we decrease it like any number and
        // carry the overflow to the next digit.
        for (s32 i = size - 1; i >= 0; --i)
        {
            permute_vector[i]++;
            if (permute_vector[i] < original[i])
            {
                break;
            }
            else
            {
                permute_vector[i] = 0;
            }
        }
    }

    s32 BenchMarkUnit::BuildArgs(Allocator* alloc, Array<Array<s32>*>& args)
    {
        // check what we have:
        // - arg values, []{ x:{a,b,c,...}, y:{a,b,c,...}, ...}
        // - arg sequence, []{ x:{a,b,c,...}, y:{a,b,c,...}, ...]
        // - arg range, multiplier mode, []{ x: range, y: range, ...}
        // - arg dense range, step mode (dense), []{ x: range, y: range, ...}

        args_count_ = 0;
        for (s32 i = 0; i < Max_Args; ++i)
        {
            if (args_[i].mode_ == 0 || args_[i].count_ == 0)
                break;
            args_count_++;
        }

        s8 mode = 0;
        for (s32 i = 0; i < args_count_; ++i)
            mode |= args_[i].mode_;

        if (mode == Arg_t::Arg_Value)
        {
            // there is no need to compute a 'product' of values.
            // {args_[0].args_[0], args_[1].args_[0], args_[2].args_[0], ...}
            s32 iters = 0;
            for (s32 i = 0; i < args_count_; ++i)
                iters = iters == 0 ? args_[i].count_ : nranges::min(args_[i].count_, iters);

            args.Init(alloc, 0, iters);

            for (s32 i = 0; i < iters; ++i)
            {
                Array<s32>* arg = alloc->Construct<Array<s32>>();
                for (s32 j = 0; j < args_count_; ++j)
                {
                    if (args_[i].mode_ == 0 || args_[i].count_ == 0)
                        continue;
                    arg->PushBack(static_cast<s32>(args_[j].args_[i]));
                }
                args.PushBack(arg);
            }
        }
        else if (mode != 0)
        {
            // compute the total number of permutations and
            // initialize the permute target and vector.
            s32 iters = 1;
            s32 permute_vector[Max_Args];
            s32 permute_target[Max_Args];
            for (s32 i = 0; i < args_count_; ++i)
            {
                permute_vector[i] = 0;
                permute_target[i] = args_[i].count_;
                iters *= args_[i].count_;
            }

            // compute all the permutations
            args.Init(alloc, 0, iters);
            for (s32 i = 0; i < iters; ++i)
            {
                Array<s32>* arg = alloc->Construct<Array<s32>>();
                arg->Init(alloc, 0, args_count_);
                for (s32 j = 0; j < args_count_; ++j)
                {
                    arg->PushBack(static_cast<s32>(args_[j].args_[permute_vector[j]]));
                }
                args.PushBack(arg);

                IncreasePermuteVector(permute_vector, permute_target, args_count_);
            }
        }

        if (args.Size() == 0)
        {
            args.Init(alloc, 0, 1);
            args.PushBack(nullptr);
        }

        return args.Size();
    }

    Arg_t* BenchMarkUnit::Arg(s32 index)
    {
        if (index < Max_Args)
            return &args_[index];
        else
            return nullptr;
    }

    Arg_t* BenchMarkUnit::Arg(s32 index, const char* name)
    {
        if (index < Max_Args)
        {
            args_[index].SetName(name);
            return &args_[index];
        }
        return nullptr;
    }

    void BenchMarkUnit::ReportAggregatesOnly(bool value) { aggregation_report_mode_ = value ? AggregationReportMode::ReportAggregatesOnly : AggregationReportMode::Default; }

    void BenchMarkUnit::DisplayAggregatesOnly(bool value)
    {
        // If we were called, the report mode is no longer 'unspecified', in any case.
        aggregation_report_mode_.mode = static_cast<u32>(aggregation_report_mode_.mode | AggregationReportMode::Default);
        aggregation_report_mode_.mode = static_cast<u32>(aggregation_report_mode_.mode & ~AggregationReportMode::DisplayReportAggregatesOnly);
        if (value)
            aggregation_report_mode_.mode = static_cast<u32>(aggregation_report_mode_.mode | AggregationReportMode::DisplayReportAggregatesOnly);
    }

    void BenchMarkUnit::PrepareSettings()
    {
        thread_counts_      = Array<s32>();
        statistics_         = Array<Statistic>();
        counters_size_      = 2;
        thread_counts_size_ = 1;
        statistics_count_   = 4;

        args_count_ = sizeof(args_) / sizeof(args_[0]);
        for (s32 i = 0; i < args_count_; ++i)
        {
            args_[i].count_      = 0;
            args_[i].mode_       = Arg_t::Arg_Uninitialized;
            args_[i].count_only_ = true;
            args_[i].args_       = Array<s64>();
            args_[i].name_       = nullptr;
        }
        count_only_ = true;
    }

    void BenchMarkUnit::ApplySettings(Allocator* allocator)
    {
        count_only_ = false;

        for (s32 i = 0; i < args_count_; ++i)
        {
            if (args_[i].count_ > 0)
                args_[i].args_.Init(allocator, 0, args_[i].count_);
            args_[i].count_only_ = false;
        }

        thread_counts_.Init(allocator, 0, thread_counts_size_);
        counters_.counters.Init(allocator, 0, counters_size_);
        statistics_.Init(allocator, 0, statistics_count_);

        AddStatisticsComputer(Statistic("mean", StatisticsMean, {StatisticUnit::Time}));
        AddStatisticsComputer(Statistic("median", StatisticsMedian, {StatisticUnit::Time}));
        AddStatisticsComputer(Statistic("stddev", StatisticsStdDev, {StatisticUnit::Time}));
        AddStatisticsComputer(Statistic("cv", StatisticsCV, {StatisticUnit::Percentage}));

        AddCounter("items per second", CounterFlags::IsRate);
        AddCounter("bytes per second", CounterFlags::IsRate);

        // ensure everything has defaults if they are unspecified
        if (time_unit_.IsUnspecified())
            time_unit_.SetDefault();
        if (time_settings_.IsUnspecified())
            time_settings_.SetDefaults();
        if (aggregation_report_mode_.IsUnspecified())
            aggregation_report_mode_.SetDefault();
        if (thread_counts_.Size() == 0)
            thread_counts_.PushBack(1);
        if (repetitions_ == 0)
            repetitions_ = 1;
        if (min_time_ == 0)
            min_time_ = 0.5;
        if (min_warmup_time_ == 0)
            min_warmup_time_ = 0.5;
        if (memory_required_ == 0)
            memory_required_ = 1 << 20; // 1 MB
    }

    void BenchMarkUnit::ReleaseSettings() { PrepareSettings(); }

    void BenchMarkUnit::AddStatisticsComputer(Statistic stat)
    {
        if (count_only_)
        {
            statistics_count_++;
            return;
        }
        statistics_.PushBack(stat);
    }

    void BenchMarkUnit::SetEnabled(bool enabled) { disabled = enabled ? 0 : 1; }

    void BenchMarkUnit::SetThreadCounts(s32 const* thread_counts, s32 thread_counts_size)
    {
        if (count_only_)
        {
            thread_counts_size_ = thread_counts_size;
            return;
        }
        for (s32 i = 0; i < thread_counts_size; ++i)
            thread_counts_.PushBack(thread_counts[i]);
    }

    void BenchMarkUnit::AddCounter(const char* name, CounterFlags flags, double value)
    {
        if (count_only_)
        {
            counters_size_ += 1;
            return;
        }

        // TODO Need to figure out the id of a counter, this should be done from a global
        // registration box where names are registered. So here we could ask that box
        // what the id is given a name.

        counters_.counters.PushBack({name, 0, flags, value});
    }

    void BenchMarkUnit::SetComplexity(BigO complexity) { complexity_ = complexity; }
    void BenchMarkUnit::SetComplexity(BigO::Func* complexity_lambda) { complexity_lambda_ = complexity_lambda; }
    void BenchMarkUnit::SetTimeUnit(TimeUnit tu) { time_unit_ = tu; }
    void BenchMarkUnit::SetMinTime(double min_time) { min_time_ = min_time; }
    void BenchMarkUnit::SetMinWarmupTime(double min_warmup_time) { min_warmup_time_ = min_warmup_time; }
    void BenchMarkUnit::SetMemoryRequired(s64 required) { memory_required_ = required; }
    void BenchMarkUnit::SetIterations(IterationCount iters) { iterations_ = iters; }
    void BenchMarkUnit::SetRepetitions(int repetitions) { repetitions_ = repetitions; }
    void BenchMarkUnit::SetFuncRun(run_function func) { run_ = func; }
    void BenchMarkUnit::SetFuncSettings(settings_function func) { settings_ = func; }

    // Append the powers of 'mult' in the closed interval [lo, hi].
    // Returns iterator to the start of the inserted range.

    namespace nranges
    {
        static s32 AddPowers(s64 lo, s64 hi, int mult, Array<s64>* dst)
        {
            BM_CHECK_GE(lo, 0);
            BM_CHECK_GE(hi, lo);
            BM_CHECK_GE(mult, 2);

            static const s64 kmax = std::numeric_limits<s64>::max();

            // Space out the values in multiples of "mult"
            s32 size = 0;
            for (s64 i = static_cast<s64>(1); i <= hi; i *= static_cast<s64>(mult))
            {
                if (i >= lo)
                {
                    if (dst != nullptr)
                        dst->PushBack(i);
                    size++;
                }
                // Break the loop here since multiplying by
                // 'mult' would move outside of the range of s64
                if (i > kmax / mult)
                    break;
            }
            return size;
        }

        static s32 AddNegatedPowers(s64 lo, s64 hi, int mult, Array<s64>* dst)
        {
            // We negate lo and hi so we require that they cannot be equal to 'min'.
            BM_CHECK_GT(lo, std::numeric_limits<s64>::min());
            BM_CHECK_GT(hi, std::numeric_limits<s64>::min());
            BM_CHECK_GE(hi, lo);
            BM_CHECK_LE(hi, 0);

            // Add positive powers, then negate and reverse.
            // Casts necessary since small integers get promoted
            // to 'int' when negating.
            const auto lo_complement = static_cast<s64>(-lo);
            const auto hi_complement = static_cast<s64>(-hi);

            if (dst != nullptr)
            {
                const s32 it   = dst->Size();
                const s32 size = AddPowers(hi_complement, lo_complement, mult, dst);

                for (int i = it; i < dst->Size(); i++)
                    (*dst)[i] *= -1;
                for (int i = it, j = dst->Size() - 1; i < j; i++, j--)
                {
                    const s64 tmp = (*dst)[i];
                    (*dst)[i]     = (*dst)[j];
                    (*dst)[j]     = tmp;
                }
                return size;
            }
            else
            {
                const s32 size = AddPowers(hi_complement, lo_complement, mult, dst);
                return size;
            }
        }

        static s32 ComputeRange(s64 lo, s64 hi, int mult, Array<s64>* dst)
        {
            BM_CHECK_GE(hi, lo);
            BM_CHECK_GE(mult, 2);

            s32 size = 1;

            if (dst != nullptr)
                dst->PushBack(lo);

            // Handle lo == hi as a special case, so we then know
            // lo < hi and so it is safe to add 1 to lo and subtract 1
            // from hi without falling outside of the range of s64.
            if (lo == hi)
                return size;

            // Ensure that lo_inner <= hi_inner below.
            if (lo + 1 == hi)
            {
                dst->PushBack(hi);
                size += 1;
                return size;
            }

            // Add all powers of 'mult' in the range [lo+1, hi-1] (inclusive).
            const auto lo_inner = static_cast<s64>(lo + 1);
            const auto hi_inner = static_cast<s64>(hi - 1);

            // Insert negative values
            if (lo_inner < 0)
            {
                size += AddNegatedPowers(lo_inner, min(hi_inner, s64{-1}), mult, dst);
            }

            // Treat 0 as a special case
            if (lo < 0 && hi >= 0)
            {
                if (dst != nullptr)
                    dst->PushBack(0);
                size += 1;
            }

            // Insert positive values
            if (hi_inner > 0)
            {
                size += AddPowers(max(lo_inner, s64{1}), hi_inner, mult, dst);
            }

            // Add "hi" (if different from last value).
            size += 1;
            if (dst != nullptr)
            {
                if (hi != dst->Back())
                    dst->PushBack(hi);
            }

            return size;
        }

        static s32 ComputeDenseRange(s64 lo, s64 hi, int step, Array<s64>* dst)
        {
            s64 size = 0;
            for (s64 i = lo; i <= hi; i += step)
                size++;

            if (dst)
            {
                for (s64 i = lo; i <= hi; i += step)
                    dst->PushBack(i);
            }
            return static_cast<s32>(size);
        }

    } // namespace nranges

} // namespace BenchMark