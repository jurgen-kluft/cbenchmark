#ifndef __CBENCHMARK_BENCHMARK_STATISTICS_H__
#define __CBENCHMARK_BENCHMARK_STATISTICS_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"

namespace BenchMark
{
    struct Statistic
    {
        typedef double(Func)(double* data, size_t size);

        Statistic() : name_(nullptr), compute_(nullptr), unit_({StatisticUnit::Time}) {}

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
        static const size_t MaxStatistics = 10;

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

        const Statistic& operator[](size_t index) const { return statistics_[index]; }
        size_t           size() const { return count_; }

        size_t    count_;
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

        u32 mode;
    };

    // Returns the number of reports that were aggregated into the result.
    s32 ComputeStats(const BenchMarkRun* reports, s32 num_reports, BenchMarkRun* result, s32 max_results);

    double StatisticsMean(const double* data, size_t size);
    double StatisticsMedian(const double* data, size_t size);
    double StatisticsStdDev(const double* data, size_t size);
    double StatisticsCV(const double* data, size_t size);

} // namespace BenchMark

#endif //__CBENCHMARK_BENCHMARK_STATISTICS_H__
