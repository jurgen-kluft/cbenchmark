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
    s32 BenchMarkUnit::BuildArgs(Allocator* alloc, Array<Array<s32>>& args)
    {
        // check what we have:
        // - args, []{x,y,..,...}
        // - arg, []{ x:{a,b,c,...}, y:{a,b,c,...}, ...]
        // - arg range, multiplier mode, []{ x: range, y: range, ...}
        // - arg range, step mode (dense), []{ x: range, y: range, ...}

        return 0;
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
        args_               = Array<Args>();
        arg_                = Array<Args>();
        arg_ranges_         = Array<ArgRange>();
        arg_names_          = Array<const char*>();
        thread_counts_      = Array<s32>();
        statistics_         = Array<Statistic>();
        args_count_         = 0;
        arg_count_          = 0;
        arg_ranges_count_   = 0;
        arg_names_count_    = 0;
        counters_size_      = 2;
        thread_counts_size_ = 0;
        statistics_count_   = 4;
    }

    void BenchMarkUnit::ApplySettings()
    {
        args_.Init(allocator, 0, args_count_);
        arg_.Init(allocator, 0, arg_count_);
        arg_ranges_.Init(allocator, 0, arg_ranges_count_);
        arg_names_.Init(allocator, 0, arg_names_count_);
        thread_counts_.Init(allocator, 0, thread_counts_size_);
        counters_.counters.Init(allocator, 0, counters_size_);
        statistics_.Init(allocator, 0, statistics_count_);

        AddStatisticsComputer(Statistic("mean", StatisticsMean, {StatisticUnit::Time}));
        AddStatisticsComputer(Statistic("median", StatisticsMedian, {StatisticUnit::Time}));
        AddStatisticsComputer(Statistic("stddev", StatisticsStdDev, {StatisticUnit::Time}));
        AddStatisticsComputer(Statistic("cv", StatisticsCV, {StatisticUnit::Percentage}));

        AddCounter("items per second", CounterFlags::IsRate);
        AddCounter("bytes per second", CounterFlags::IsRate);
    }

    void BenchMarkUnit::ReleaseSettings()
    {
        PrepareSettings();
    }

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

    void BenchMarkUnit::AddArgs(s32 const* args, s32 argc)
    {
        if (count_only_)
        {
            args_count_++;
            return;
        }
        args_.PushBack(Args(args, argc));
    }

    void BenchMarkUnit::AddArg(const s32* args, s32 argc)
    {
        if (count_only_)
        {
            arg_count_++;
            return;
        }
        arg_.PushBack(Args(args, argc));
    }

    void BenchMarkUnit::AddRange(s32 lo, s32 hi, s32 multiplier, ArgRange::EMode mode)
    {
        if (count_only_)
        {
            arg_ranges_count_++;
            return;
        }
        arg_ranges_.PushBack(ArgRange(lo, hi, multiplier, mode));
    }

    void BenchMarkUnit::SetArgNames(const char** names, s32 names_size)
    {
        if (count_only_)
        {
            arg_names_count_ = names_size;
            return;
        }
        for (s32 i = 0; i < names_size; ++i)
            arg_names_.PushBack(names[i]);
    }

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
    void BenchMarkUnit::SetIterations(IterationCount iters) { iterations_ = iters; }
    void BenchMarkUnit::SetRepetitions(int repetitions) { repetitions_ = repetitions; }
    void BenchMarkUnit::SetFuncRun(run_function func) { run_ = func; }
    void BenchMarkUnit::SetFuncSettings(settings_function func) { settings_ = func; }

    // Append the powers of 'mult' in the closed interval [lo, hi].
    // Returns iterator to the start of the inserted range.

    namespace internal
    {
        template <typename T> T min(T a, T b) { return a < b ? a : b; }
        template <typename T> T max(T a, T b) { return a > b ? a : b; }

        static s32 AddPowers(Array<s64>& dst, s64 lo, s64 hi, int mult)
        {
            BM_CHECK_GE(lo, 0);
            BM_CHECK_GE(hi, lo);
            BM_CHECK_GE(mult, 2);

            const s32 start_offset = dst.Size();

            static const s64 kmax = std::numeric_limits<s64>::max();

            // Space out the values in multiples of "mult"
            for (s64 i = static_cast<s64>(1); i <= hi; i *= static_cast<s64>(mult))
            {
                if (i >= lo)
                {
                    dst.PushBack(i);
                }
                // Break the loop here since multiplying by
                // 'mult' would move outside of the range of s64
                if (i > kmax / mult)
                    break;
            }

            return start_offset;
        }

        static void AddNegatedPowers(Array<s64>& dst, s64 lo, s64 hi, int mult)
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

            const s32 it = AddPowers(dst, hi_complement, lo_complement, mult);

            for (int i = it; i < dst.Size(); i++)
                dst[i] *= -1;
            for (int i = it, j = dst.Size() - 1; i < j; i++, j--)
            {
                const s64 tmp = dst[i];
                dst[i]        = dst[j];
                dst[j]        = tmp;
            }
        }

        static void AddRange(s64 lo, s64 hi, int mult, Array<s64>& dst, Allocator* alloc)
        {
            BM_CHECK_GE(hi, lo);
            BM_CHECK_GE(mult, 2);

            dst.PushBack(lo);

            // Handle lo == hi as a special case, so we then know
            // lo < hi and so it is safe to add 1 to lo and subtract 1
            // from hi without falling outside of the range of s64.
            if (lo == hi)
                return;

            // Ensure that lo_inner <= hi_inner below.
            if (lo + 1 == hi)
            {
                dst.PushBack(hi);
                return;
            }

            // Add all powers of 'mult' in the range [lo+1, hi-1] (inclusive).
            const auto lo_inner = static_cast<s64>(lo + 1);
            const auto hi_inner = static_cast<s64>(hi - 1);

            // Insert negative values
            if (lo_inner < 0)
            {
                AddNegatedPowers(dst, lo_inner, min(hi_inner, s64{-1}), mult);
            }

            // Treat 0 as a special case
            if (lo < 0 && hi >= 0)
            {
                dst.PushBack(0);
            }

            // Insert positive values
            if (hi_inner > 0)
            {
                AddPowers(dst, max(lo_inner, s64{1}), hi_inner, mult);
            }

            // Add "hi" (if different from last value).
            if (hi != dst.Back())
            {
                dst.PushBack(hi);
            }
        }

        static s32 AddedPowersSize(s64 lo, s64 hi, int mult)
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
                    size += 1;
                }
                // Break the loop here since multiplying by
                // 'mult' would move outside of the range of s64
                if (i > kmax / mult)
                    break;
            }
            return size;
        }

        static s32 AddedNegatedPowersSize(s64 lo, s64 hi, int mult)
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

            return AddedPowersSize(hi_complement, lo_complement, mult);
        }

        static s32 ComputeAddRangeSize(s64 lo, s64 hi, int mult)
        {
            BM_CHECK_GE(hi, lo);
            BM_CHECK_GE(mult, 2);

            s32 size = 1;

            // Handle lo == hi as a special case, so we then know
            // lo < hi and so it is safe to add 1 to lo and subtract 1
            // from hi without falling outside of the range of s64.
            if (lo == hi)
                return size;

            // Ensure that lo_inner <= hi_inner below.
            if (lo + 1 == hi)
            {
                size += 1;
                return size;
            }

            // Add all powers of 'mult' in the range [lo+1, hi-1] (inclusive).
            const auto lo_inner = static_cast<s64>(lo + 1);
            const auto hi_inner = static_cast<s64>(hi - 1);

            // Insert negative values
            if (lo_inner < 0)
            {
                size += AddedNegatedPowersSize(lo_inner, min(hi_inner, s64{-1}), mult);
            }

            // Treat 0 as a special case
            if (lo < 0 && hi >= 0)
            {
                size += 1;
            }

            // Insert positive values
            if (hi_inner > 0)
            {
                size += AddedPowersSize(max(lo_inner, s64{1}), hi_inner, mult);
            }

            size += 1;
            return size;
        }
    } // namespace internal

    void BenchMarkUnit::CreateRange(s32 start, s32 limit, s32 mult, Array<s64>& out, Allocator* alloc)
    {
        // Compute the number of elements in the range.
        const s32 size = internal::ComputeAddRangeSize(start, limit, mult);
        out.Init(alloc, 0, size);
        internal::AddRange(start, limit, mult, out, alloc);
    }

    void BenchMarkUnit::CreateDenseRange(s32 start, s32 limit, s32 step, Array<s64>& out, Allocator* alloc)
    {
        out.Init(alloc, 0, (limit - start) / step + 1);
        for (s32 arg = start; arg <= limit; arg += step)
        {
            out.PushBack(arg);
        }
    }

} // namespace BenchMark