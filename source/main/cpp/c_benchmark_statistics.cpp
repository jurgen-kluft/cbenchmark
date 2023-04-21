#include "cbenchmark/private/c_benchmark_alloc.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_check.h"
#include "cbenchmark/private/c_utils.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>

namespace BenchMark
{
    auto StatisticsSum = [](const Array<double>& v)
    {
        double sum = 0.0;
        for (auto i = 0; i < v.Size(); ++i)
        {
            sum += v[i];
        };
        return sum;
    };

    double StatisticsMean(const Array<double>& v)
    {
        if (v.Empty())
            return 0.0;
        return StatisticsSum(v) * (1.0 / v.Size());
    }

    double StatisticsMedian(const Array<double>& v)
    {
        if (v.Size() < 3)
            return StatisticsMean(v);

        Array<double> copy;
        copy.Copy(v);

        double* center = copy.Begin() + (v.Size() / 2);
        std::nth_element(copy.Begin(), center, copy.End());

        // did we have an odd number of samples?
        // if yes, then center is the median
        // it no, then we are looking for the average between center and the value
        // before
        if (v.Size() % 2 == 1)
            return *center;
        double* center2 = copy.Begin() + v.Size() / 2 - 1;
        std::nth_element(copy.Begin(), center2, copy.End());

        return (*center + *center2) / 2.0;
    }

    // Return the sum of the squares of this sample set
    auto SumSquares = [](const Array<double>& v) { return std::inner_product(v.Begin(), v.End(), v.Begin(), 0.0); };

    auto Sqr  = [](const double dat) { return dat * dat; };
    auto Sqrt = [](const double dat)
    {
        // Avoid NaN due to imprecision in the calculations
        if (dat < 0.0)
            return 0.0;
        return std::sqrt(dat);
    };

    double StatisticsStdDev(const Array<double>& v)
    {
        const auto mean = StatisticsMean(v);
        if (v.Empty())
            return mean;

        // Sample standard deviation is undefined for n = 1
        if (v.Size() == 1)
            return 0.0;

        const double avg_squares = SumSquares(v) * (1.0 / v.Size());
        return Sqrt(v.Size() / (v.Size() - 1.0) * (avg_squares - Sqr(mean)));
    }

    double StatisticsCV(const Array<double>& v)
    {
        if (v.Size() < 2)
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
            results.Copy(reports);
            return;
        }

        // Accumulators.
        Array<double> real_accumulated_time_stat;
        Array<double> cpu_accumulated_time_stat;

        real_accumulated_time_stat.Init(alloc, 0, reports.Size());
        cpu_accumulated_time_stat.Init(alloc, 0, reports.Size());

        // All repetitions should be run with the same number of iterations so we
        // can take this information from the first benchmark.
        const IterationCount run_iterations = reports.Front().iterations;

        // create stats for user counters
        struct CounterStat
        {
            CounterStat() = default;
            Counter       c;
            Array<double> s;
        };

        struct CounterStats
        {
            Array<CounterStat> stats;

            s32 Find(Counter const& c) const
            {
                for (int i = 0; i < stats.Size(); i++)
                {
                    if (stats[i].c.id == c.id)
                        return i;
                }
                return stats.Size();
            }

            s32 End() const { return stats.Size(); }
        };

        CounterStats counter_stats;

        for (int i = 0; i < reports.Size(); i++)
        {
            auto const& r = reports[i];

            //for (auto const& cnt : r.counters)
            for (int j = 0; j < r.counters.counters.Size(); j++)
            {
                auto const& cnt = r.counters.counters[j];

                auto it = counter_stats.Find(cnt);
                if (it == counter_stats.End())
                {
                    CounterStat& c = counter_stats.stats.Alloc();
                    c.c = cnt;
                    c.s.Init(alloc, 0, reports.Size());
                }
                else
                {
                    BM_CHECK_EQ(counter_stats.stats[it].c.flags, cnt.flags);
                }
            }
        }

        // Populate the accumulators.
        for (int i = 0; i < reports.Size(); i++)
        {
            auto const& run = reports[i];

            BM_CHECK_EQ(gCompareStrings(reports[0].benchmark_name(), run.benchmark_name()), 0);
            BM_CHECK_EQ(run_iterations, run.iterations);
            if (run.skipped.IsSkipped())
                continue;
            
            real_accumulated_time_stat.PushBack(run.real_accumulated_time);
            cpu_accumulated_time_stat.PushBack(run.cpu_accumulated_time);

            // user counters
            for (int j = 0; j < run.counters.counters.Size(); j++)
            {
                Counter const& cnt = run.counters.counters[j];
                auto it = counter_stats.Find(cnt);
                BM_CHECK_NE(it, counter_stats.End());
                CounterStat& stat = counter_stats.stats[it];
                stat.s.PushBack(cnt.value);
            }
        }

        // Only add label if it is same for all runs
        const char* report_label = reports[0].report_label;
        for (int i = 1; i < reports.Size(); i++)
        {
            if (gCompareStrings(reports[i].report_label, report_label) != 0)
            {
                report_label = "";
                break;
            }
        }

        const double iteration_rescale_factor = double(reports.Size()) / double(run_iterations);

        // Reserve space for the results.
        results.Init(alloc, 0, reports[0].statistics.count_);

        for (int i = 0; i < reports[0].statistics.count_; i++)
        {
            const auto& Stat = reports[0].statistics[i];

            // Get the data from the accumulator to BenchMarkRun's.
            Run& data = results.Alloc();
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
            data.iterations = reports.Size();

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
            for (int j = 0; j < counter_stats.stats.Size(); j++)
            {
                CounterStat const& kv = counter_stats.stats[j];
                // Do *NOT* rescale the custom counters. They are already properly scaled.
                const auto uc_stat      = Stat.compute_(kv.s);
                Counter& c = data.counters.counters.Alloc();
                c.value = uc_stat;
                c.flags = kv.c.flags;
            }
        }
    }

} // namespace BenchMark