#ifndef __CBENCHMARK_BENCHMARK_STATISTICS_H__
#define __CBENCHMARK_BENCHMARK_STATISTICS_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_array.h"
#include "cbenchmark/private/c_benchmark_allocators.h"

namespace BenchMark
{
    class BenchMarkRun;

    double StatisticsMean(ScratchAllocator* scratch, const Array<double>& data);
    double StatisticsMedian(ScratchAllocator* scratch, const Array<double>& data);
    double StatisticsStdDev(ScratchAllocator* scratch, const Array<double>& data);
    double StatisticsCV(ScratchAllocator* scratch, const Array<double>& data);

    struct Statistic
    {
        typedef double (*Func)(ScratchAllocator* scratch, const Array<double>& values);

        Statistic()
            : name_(nullptr)
            , compute_(nullptr)
            , unit_({StatisticUnit::Time})
        {
        }

        Statistic(const char* name, Func compute, StatisticUnit unit = {StatisticUnit::Time})
            : name_(name)
            , compute_(compute)
            , unit_(unit)
        {
        }

        const char*   name_;
        Func          compute_;
        StatisticUnit unit_;
    };

    struct AggregationReportMode
    {
        enum
        {
            Unspecified                 = 0,                                                     // The mode has not been manually specified
            Default                     = 1U << 0U,                                              // The mode is user-specified.
            FileReportAggregatesOnly    = 1U << 1U,                                              // File reporter should only output aggregates.
            DisplayReportAggregatesOnly = 1U << 2U,                                              // Display reporter should only output aggregates
            ReportAggregatesOnly        = FileReportAggregatesOnly | DisplayReportAggregatesOnly // Both reporters should only display aggregates.
        };

        AggregationReportMode()
            : mode(Unspecified)
        {
        }
        AggregationReportMode(u32 mode)
            : mode(mode)
        {
        }

        inline bool IsUnspecified() const { return mode == Unspecified; }
        inline void SetDefault() { mode = Default; }

        u32 mode;
    };

    // Returns the number of reports that were aggregated into the result.
    void ComputeStats(ForwardAllocator* alloc, ScratchAllocator* scratch, const Array<BenchMarkRun*>& reports, Array<BenchMarkRun*>& result);

} // namespace BenchMark

#endif //__CBENCHMARK_BENCHMARK_STATISTICS_H__
