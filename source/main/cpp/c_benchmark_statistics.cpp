#include "cbenchmark/private/c_benchmark_alloc.h"
#include "cbenchmark/private/c_benchmark_metrics.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_check.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include <vector>

namespace BenchMark
{

    auto StatisticsSum = [](const std::vector<double>& v) { return std::accumulate(v.begin(), v.end(), 0.0); };

    double StatisticsMean(const std::vector<double>& v)
    {
        if (v.empty())
            return 0.0;
        return StatisticsSum(v) * (1.0 / v.size());
    }

    double StatisticsMedian(const std::vector<double>& v)
    {
        if (v.size() < 3)
            return StatisticsMean(v);
        std::vector<double> copy(v);

        auto center = copy.begin() + v.size() / 2;
        std::nth_element(copy.begin(), center, copy.end());

        // did we have an odd number of samples?
        // if yes, then center is the median
        // it no, then we are looking for the average between center and the value
        // before
        if (v.size() % 2 == 1)
            return *center;
        auto center2 = copy.begin() + v.size() / 2 - 1;
        std::nth_element(copy.begin(), center2, copy.end());
        return (*center + *center2) / 2.0;
    }

    // Return the sum of the squares of this sample set
    auto SumSquares = [](const std::vector<double>& v) { return std::inner_product(v.begin(), v.end(), v.begin(), 0.0); };

    auto Sqr  = [](const double dat) { return dat * dat; };
    auto Sqrt = [](const double dat)
    {
        // Avoid NaN due to imprecision in the calculations
        if (dat < 0.0)
            return 0.0;
        return std::sqrt(dat);
    };

    double StatisticsStdDev(const std::vector<double>& v)
    {
        const auto mean = StatisticsMean(v);
        if (v.empty())
            return mean;

        // Sample standard deviation is undefined for n = 1
        if (v.size() == 1)
            return 0.0;

        const double avg_squares = SumSquares(v) * (1.0 / v.size());
        return Sqrt(v.size() / (v.size() - 1.0) * (avg_squares - Sqr(mean)));
    }

    double StatisticsCV(const std::vector<double>& v)
    {
        if (v.size() < 2)
            return 0.0;

        const auto stddev = StatisticsStdDev(v);
        const auto mean   = StatisticsMean(v);

        return stddev / mean;
    }

    void ComputeStats(Allocator* alloc, const Array<BenchMarkRun>& reports, Array<BenchMarkRun>& results)
    {
        typedef BenchMarkRun Run;

        auto error_count = 0;
        for (int i = 0; i < reports.Size(); i++)
        {
            if (!reports[i].skipped.IsSkipped())
                continue;
            error_count++;
        }

        if (reports.Size() - error_count < 2)
        {
            // We don't report aggregated data if there was a single run.
        }

        // Accumulators.
        std::vector<double> real_accumulated_time_stat;
        std::vector<double> cpu_accumulated_time_stat;

        real_accumulated_time_stat.reserve(reports.size());
        cpu_accumulated_time_stat.reserve(reports.size());

        // All repetitions should be run with the same number of iterations so we
        // can take this information from the first benchmark.
        const IterationCount run_iterations = reports.front().iterations;
        
        // create stats for user counters
        struct CounterStat
        {
            Counter c;
            Array<double> s;
        };

        std::map<std::string, CounterStat> counter_stats;
        for (Run const& r : reports)
        {
            for (auto const& cnt : r.counters)
            {
                auto it = counter_stats.find(cnt.first);
                if (it == counter_stats.end())
                {
                    it = counter_stats.emplace(cnt.first, CounterStat{cnt.second, std::vector<double>{}}).first;
                    it->second.s.reserve(reports.size());
                }
                else
                {
                    BM_CHECK_EQ(it->second.c.flags, cnt.second.flags);
                }
            }
        }

        // Populate the accumulators.
        for (BenchMarkRun const& run : reports)
        {
            BM_CHECK_EQ(reports[0].benchmark_name(), run.benchmark_name());
            BM_CHECK_EQ(run_iterations, run.iterations);
            if (run.skipped.IsSkipped())
                continue;
            real_accumulated_time_stat.emplace_back(run.real_accumulated_time);
            cpu_accumulated_time_stat.emplace_back(run.cpu_accumulated_time);

            // user counters
            for (auto const& cnt : run.counters_)
            {
                auto it = counter_stats.find(cnt.first);
                BM_CHECK_NE(it, counter_stats.end());
                it->second.s.emplace_back(cnt.second);
            }
        }

        // Only add label if it is same for all runs
        std::string report_label = reports[0].report_label;
        for (int i = 1; i < reports.size(); i++)
        {
            if (reports[i].report_label != report_label)
            {
                report_label = "";
                break;
            }
        }

        const double iteration_rescale_factor = double(reports.size()) / double(run_iterations);

        for (int i=0; i<reports[0].statistics.count_; i++)
        {
            const auto& Stat = reports[0].statistics[i];

            // Get the data from the accumulator to BenchMarkRun's.
            Run data;
            data.run_name                  = reports[0].run_name;
            data.family_index              = reports[0].family_index;
            data.per_family_instance_index = reports[0].per_family_instance_index;
            data.run_type                  = BenchMarkRun::RT_Aggregate;
            data.threads                   = reports[0].threads;
            data.repetitions               = reports[0].repetitions;
            data.repetition_index          = Run::no_repetition_index;
            data.aggregate_name            = Stat.name_;
            data.aggregate_unit            = Stat.unit_;
            data.report_label              = report_label;

            // It is incorrect to say that an aggregate is computed over
            // run's iterations, because those iterations already got averaged.
            // Similarly, if there are N repetitions with 1 iterations each,
            // an aggregate will be computed over N measurements, not 1.
            // Thus it is best to simply use the count of separate reports.
            data.iterations = reports.size();

            data.real_accumulated_time = Stat.compute_(real_accumulated_time_stat);
            data.cpu_accumulated_time  = Stat.compute_(cpu_accumulated_time_stat);

            if (data.aggregate_unit.unit == StatisticUnit::Time)
            {
                // We will divide these times by data.iterations when reporting, but the
                // data.iterations is not necessarily the scale of these measurements,
                // because in each repetition, these timers are sum over all the iters.
                // And if we want to say that the stats are over N repetitions and not
                // M iterations, we need to multiply these by (N/M).
                data.real_accumulated_time *= iteration_rescale_factor;
                data.cpu_accumulated_time *= iteration_rescale_factor;
            }

            data.time_unit = reports[0].time_unit;

            // user counters
            for (auto const& kv : counter_stats)
            {
                // Do *NOT* rescale the custom counters. They are already properly scaled.
                const auto uc_stat      = Stat.compute_(kv.second.s);
                auto       c            = Counter(uc_stat, counter_stats[kv.first].c.flags, counter_stats[kv.first].c.oneK);
                data.counters[kv.first] = c;
            }

            results.push_back(data);
        }

        return results;
    }

} // namespace BenchMark
