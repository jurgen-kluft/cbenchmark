#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_check.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_entity.h"
#include "cbenchmark/private/c_benchmark_instance.h"

#include <limits>

namespace BenchMark
{
    s32 BenchMarkEntity::BuildArgs() { return 0; 
    }

    void BenchMarkEntity::SetDefaults()
    {
        // TODO set all members to sane defaults
        AddStatisticsComputer(Statistic("mean", StatisticsMean, {StatisticUnit::Time}));
        AddStatisticsComputer(Statistic("median", StatisticsMedian, {StatisticUnit::Time}));
        AddStatisticsComputer(Statistic("stddev", StatisticsStdDev, {StatisticUnit::Time}));
        AddStatisticsComputer(Statistic("cv", StatisticsCV, {StatisticUnit::Percentage}));
    }

    void BenchMarkEntity::ReportAggregatesOnly(bool value) { aggregation_report_mode_ = value ? AggregationReportMode::ReportAggregatesOnly : AggregationReportMode::Default; }

    void BenchMarkEntity::DisplayAggregatesOnly(bool value)
    {
        // If we were called, the report mode is no longer 'unspecified', in any case.
        aggregation_report_mode_.mode = static_cast<u32>(aggregation_report_mode_.mode | AggregationReportMode::Default);
        aggregation_report_mode_.mode = static_cast<u32>(aggregation_report_mode_.mode & ~AggregationReportMode::DisplayReportAggregatesOnly);
        if (value)
            aggregation_report_mode_.mode = static_cast<u32>(aggregation_report_mode_.mode | AggregationReportMode::DisplayReportAggregatesOnly);
    }

    void BenchMarkEntity::AddStatisticsComputer(Statistic stat) {}

    void BenchMarkEntity::SetEnabled(bool enabled) { enabled_ = enabled; }

    void BenchMarkEntity::SetDefaultArgNames()
    {
        arg_names_[0] = "x";
        arg_names_[1] = "y";
        arg_names_[2] = "z";
    }
    void BenchMarkEntity::SetNamedArg(int i, const char* name, const s32* args, int argc)
    {
        if (i >= sizeof(arg_names_) / sizeof(arg_names_[0]))
            return;
        arg_names_[i]       = name;
        arg_arrays_[i]      = args;
        arg_array_sizes_[i] = argc;
    }
    void BenchMarkEntity::SetArgRange(int idx, s32 lo, s32 hi, int multiplier)
    {
        arg_names_[idx]  = nullptr;
        arg_ranges_[idx] = ArgRange(lo, hi, multiplier, 1);
    }
    void BenchMarkEntity::SetNamedArgRange(int idx, const char* name, s32 lo, s32 hi, int multiplier)
    {
        arg_names_[idx]  = name;
        arg_ranges_[idx] = ArgRange(lo, hi, multiplier, 1);
    }

    void BenchMarkEntity::SetNamedArgProduct(const char* aname, std::initializer_list<s64> a, const char* bname, std::initializer_list<s64> b, const char* cname, std::initializer_list<s64> c)
    {
        arg_names_[0] = aname;
        arg_names_[1] = bname;
        arg_names_[2] = cname;
        SetArgProduct(a, b, c);
    }

    void BenchMarkEntity::SetArgProduct(std::initializer_list<s64> a, std::initializer_list<s64> b, std::initializer_list<s64> c)
    {
        arg_product_[0] = Array<s64>();
        arg_product_[1] = Array<s64>();
        arg_product_[2] = Array<s64>();

        if (a.size() > 0)
        {
            arg_product_[0].Init(allocator, (s32)a.size(), (s32)a.size());
            for (auto& i : a)
                arg_product_[0].PushBack(i);
        }
        if (b.size() > 0)
        {
            arg_product_[1].Init(allocator, (s32)b.size(), (s32)b.size());
            for (auto& i : b)
                arg_product_[1].PushBack(i);
        }
        if (c.size() > 0)
        {
            arg_product_[2].Init(allocator, (s32)c.size(), (s32)c.size());
            for (auto& i : c)
                arg_product_[2].PushBack(i);
        }
    }

    void BenchMarkEntity::SetThreadCounts(s32 const* thread_counts, s32 thread_counts_size)
    {
        thread_counts_ = thread_counts;
        thread_counts_size_ = thread_counts_size;
    }

    void BenchMarkEntity::SetComplexity(BigO complexity) { complexity_ = complexity; }
    void BenchMarkEntity::SetComplexity(BigO::Func* complexity_lambda) { complexity_lambda_ = complexity_lambda; }
    void BenchMarkEntity::AddCounter(const char* name, CounterFlags flags, double value) {}
    void BenchMarkEntity::SetTimeUnit(TimeUnit tu) { time_unit_ = tu; }
    void BenchMarkEntity::SetMinTime(double min_time) { min_time_ = min_time; }
    void BenchMarkEntity::SetMinWarmupTime(double min_warmup_time) { min_warmup_time_ = min_warmup_time; }
    void BenchMarkEntity::SetIterations(IterationCount iters) { iterations_ = iters; }
    void BenchMarkEntity::SetRepetitions(int repetitions) { repetitions_ = repetitions; }
    void BenchMarkEntity::SetFuncRun(run_function func) { run = func; }
    void BenchMarkEntity::SetFuncSettings(settings_function func) { settings = func; }

    // Append the powers of 'mult' in the closed interval [lo, hi].
    // Returns iterator to the start of the inserted range.

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

    void BenchMarkEntity::CreateRange(s32 start, s32 limit, s32 mult, Array<s64>& out, Allocator* alloc)
    {
        // Compute the number of elements in the range.
        const s32 size = ComputeAddRangeSize(start, limit, mult);
        out.Init(alloc, 0, size);
        AddRange(start, limit, mult, out, alloc);
    }

    void BenchMarkEntity::CreateDenseRange(s32 start, s32 limit, s32 step, Array<s64>& out, Allocator* alloc)
    {
        out.Init(alloc, 0, (limit - start) / step + 1);
        for (s32 arg = start; arg <= limit; arg += step)
        {
            out.PushBack(arg);
        }
    }

} // namespace BenchMark