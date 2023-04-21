#ifndef __CBENCHMARK_BENCHMARK_STATISTICS_H__
#define __CBENCHMARK_BENCHMARK_STATISTICS_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"

namespace BenchMark
{
    struct Statistic
    {
        typedef double(Func)(const Array<double>& values);

        Statistic()
            : name_(nullptr)
            , compute_(nullptr)
            , unit_({StatisticUnit::Time})
        {
        }

        Statistic(const char*& name, Func* compute, StatisticUnit unit = {StatisticUnit::Time})
            : name_(name)
            , compute_(compute)
            , unit_(unit)
        {
        }

        const char*   name_;
        Func*         compute_;
        StatisticUnit unit_;
    };

    struct Statistics
    {
        static const s32 MaxStatistics = 10;

        Statistics()
            : count_(0)
        {
        }

        void Add(const Statistic& statistic)
        {
            if (count_ < MaxStatistics)
            {
                statistics_[count_++] = statistic;
            }
        }

        const Statistic& operator[](s32 index) const { return statistics_[index]; }
        s32              Size() const { return count_; }

        s32       count_;
        Statistic statistics_[MaxStatistics];
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

        AggregationReportMode() : mode(Unspecified) {}
        AggregationReportMode(u32 mode) : mode(mode) {}

        u32 mode;
    };

    // Returns the number of reports that were aggregated into the result.
    void ComputeStats(Allocator* alloc, const Array<BenchMarkRun>& reports, Array<BenchMarkRun>& result);

    double StatisticsMean(const Array<double>& data);
    double StatisticsMedian(const Array<double>& data);
    double StatisticsStdDev(const Array<double>& data);
    double StatisticsCV(const Array<double>& data);

} // namespace BenchMark

#endif //__CBENCHMARK_BENCHMARK_STATISTICS_H__