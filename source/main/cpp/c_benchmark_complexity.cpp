#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_check.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_types.h"

#include <algorithm>
#include <cmath>

namespace BenchMark
{
    // integer power function
    template <typename T> T ipow(T x, u32 y)
    {
        T temp   = x;
        T result = 1;
        while (y > 0)
        {
            if ((y & 1) == 1)
            {
                result *= temp;
            }
            y    = y >> 1;
            temp = temp * temp;
        }
        return result;
    }

    // double power function
    double dpow(double x, u32 y)
    {
        double temp   = x;
        double result = 1;
        while (y > 0)
        {
            if ((y & 1) == 1)
            {
                result *= temp;
            }
            y    = y >> 1;
            temp = temp * temp;
        }
        return result;
    }

    // Internal function to calculate the different scalability forms
    BigO::Func* FittingCurve(BigO complexity)
    {
        static const double kLog2E = 1.44269504088896340736;
        switch (complexity.bigo)
        {
            case BigO::O_N: return [](IterationCount n) -> double { return static_cast<double>(n); };
            case BigO::O_N_Squared: return [](IterationCount n) -> double { return ipow(n, 2); };
            case BigO::O_N_Cubed: return [](IterationCount n) -> double { return ipow(n, 3); };
            case BigO::O_Log_N: return [](IterationCount n) { return kLog2E * log(static_cast<double>(n)); };
            case BigO::O_N_Log_N: return [](IterationCount n) { return kLog2E * n * log(static_cast<double>(n)); };
            case BigO::O_1:
            default: return [](IterationCount) { return 1.0; };
        }
    }

    struct LeastSq
    {
        LeastSq()
            : coef(0.0)
            , rms(0.0)
            , complexity(BigO::O_None)
        {
        }

        double coef;
        double rms;
        BigO   complexity;
    };

    // Find the coefficient for the high-order term in the running time, by
    // minimizing the sum of squares of relative error, for the fitting curve
    // given by the lambda expression.
    //   - n             : Vector containing the size of the benchmark tests.
    //   - time          : Vector containing the times for the benchmark tests.
    //   - fitting_curve : lambda expression (e.g. [](s64 n) {return n; };).

    // For a deeper explanation on the algorithm logic, please refer to
    // https://en.wikipedia.org/wiki/Least_squares#Least_squares,_regression_analysis_and_statistics

    LeastSq MinimalLeastSq(const Array<s64>& n, const Array<double>& time, BigO::Func* fitting_curve)
    {
        double sigma_gn_squared = 0.0;
        double sigma_time       = 0.0;
        double sigma_time_gn    = 0.0;

        // Calculate least square fitting parameter
        for (s32 i = 0; i < n.Size(); ++i)
        {
            double gn_i = fitting_curve(n[i]);
            sigma_gn_squared += gn_i * gn_i;
            sigma_time += time[i];
            sigma_time_gn += time[i] * gn_i;
        }

        LeastSq result;
        result.complexity = BigO::O_Lambda;

        // Calculate complexity.
        result.coef = sigma_time_gn / sigma_gn_squared;

        // Calculate RMS
        double rms = 0.0;
        for (s32 i = 0; i < n.Size(); ++i)
        {
            double fit = result.coef * fitting_curve(n[i]);
            rms += dpow((time[i] - fit), 2);
        }

        // Normalized RMS by the mean of the observed values
        double mean = sigma_time / n.Size();
        result.rms  = sqrt(rms / n.Size()) / mean;

        return result;
    }

    // Find the coefficient for the high-order term in the running time, by
    // minimizing the sum of squares of relative error.
    //   - n          : Vector containing the size of the benchmark tests.
    //   - time       : Vector containing the times for the benchmark tests.
    //   - complexity : If different than O_Auto, the fitting curve will stick to
    //                  this one. If it is O_Auto, it will be calculated the best
    //                  fitting curve.
    LeastSq MinimalLeastSq(const Array<s64>& n, const Array<double>& time, const BigO complexity)
    {
        BM_CHECK_EQ(n.Size(), time.Size());
        BM_CHECK_GE(n.Size(), 2); // Do not compute fitting curve is less than two
                                  // benchmark runs are given
        BM_CHECK_NE(complexity, oNone);

        LeastSq best_fit;

        if (complexity.Is(BigO::O_Auto))
        {
            u32 const fit_curves[] = {BigO::O_Log_N, BigO::O_N, BigO::O_N_Log_N, BigO::O_N_Squared, BigO::O_N_Cubed};

            // Take O(1) as default best fitting curve
            best_fit            = MinimalLeastSq(n, time, FittingCurve(BigO::O_1));
            best_fit.complexity = BigO::O_1;

            // Compute all possible fitting curves and stick to the best one
            for (const auto& fit : fit_curves)
            {
                LeastSq current_fit = MinimalLeastSq(n, time, FittingCurve(fit));
                if (current_fit.rms < best_fit.rms)
                {
                    best_fit            = current_fit;
                    best_fit.complexity = fit;
                }
            }
        }
        else
        {
            best_fit            = MinimalLeastSq(n, time, FittingCurve(complexity));
            best_fit.complexity = complexity;
        }

        return best_fit;
    }

    typedef BenchMarkRun Run;

    // TODO Could benefit from an additional 'temp' allocator 
    void ComputeBigO(Allocator* alloc, Allocator* temp, const Array<Run*>& reports, Array<Run*>& bigo)
    {
        if (reports.Size() < 2)
            return;

        // Accumulators.
        Array<s64>    n;
        Array<double> real_time;
        Array<double> cpu_time;

        n.Init(temp, 0, reports.Size());
        real_time.Init(temp, 0, reports.Size());
        cpu_time.Init(temp, 0, reports.Size());

        // Populate the accumulators.
        for (s32 i = 0; i < reports.Size(); ++i)
        {
            const Run* run = reports[i];

            // BM_CHECK_GT(run->complexity_n, 0) << "Did you forget to call SetComplexityN?";
            n.PushBack(run->complexity_n);
            real_time.PushBack(run->real_accumulated_time / run->iterations);
            cpu_time.PushBack(run->cpu_accumulated_time / run->iterations);
        }

        LeastSq result_cpu;
        LeastSq result_real;

        const Run* run0 = reports[0];
        if (run0->complexity.Is(BigO::O_Lambda))
        {
            result_cpu  = MinimalLeastSq(n, cpu_time, run0->complexity_lambda);
            result_real = MinimalLeastSq(n, real_time, run0->complexity_lambda);
        }
        else
        {
            result_cpu  = MinimalLeastSq(n, cpu_time, run0->complexity);
            result_real = MinimalLeastSq(n, real_time, result_cpu.complexity);
        }

        // Drop the 'args' when reporting complexity.

        // TODO Seems to use BenchMarkName structure to construct a run name.

        BenchmarkName run_name = run0->run_name;
        run_name.args          = nullptr;

        // Get the data from the accumulator to Run's.
        Run*& big_o                      = bigo.Alloc();
        big_o                            = alloc->Construct<Run>();
        big_o->run_name                  = run_name;
        big_o->family_index              = run0->family_index;
        big_o->per_family_instance_index = run0->per_family_instance_index;
        big_o->run_type                  = Run::RT_Aggregate;
        big_o->repetitions               = run0->repetitions;
        big_o->repetition_index          = Run::no_repetition_index;
        big_o->threads                   = run0->threads;
        big_o->aggregate_name            = "BigO";
        big_o->aggregate_unit            = {StatisticUnit::Time};
        big_o->report_label              = run0->report_label;
        big_o->iterations                = 0;
        big_o->real_accumulated_time     = result_real.coef;
        big_o->cpu_accumulated_time      = result_cpu.coef;
        big_o->report_big_o              = true;
        big_o->complexity                = result_cpu.complexity;

        // All the time results are reported after being multiplied by the
        // time unit multiplier. But since RMS is a relative quantity it
        // should not be multiplied at all. So, here, we _divide_ it by the
        // multiplier so that when it is multiplied later the result is the
        // correct one.
        const double multiplier = run0->time_unit.GetTimeUnitMultiplier();

        // Only add label to mean/stddev if it is same for all runs
        Run*& rms                      = bigo.Alloc();
        rms                            = alloc->Construct<Run>();
        rms->run_name                  = run_name;
        rms->family_index              = run0->family_index;
        rms->per_family_instance_index = run0->per_family_instance_index;
        rms->run_type                  = Run::RT_Aggregate;
        rms->aggregate_name            = "RMS";
        rms->aggregate_unit            = {StatisticUnit::Percentage};
        rms->report_label              = big_o->report_label;
        rms->iterations                = 0;
        rms->repetition_index          = Run::no_repetition_index;
        rms->repetitions               = run0->repetitions;
        rms->threads                   = run0->threads;
        rms->real_accumulated_time     = result_real.rms / multiplier;
        rms->cpu_accumulated_time      = result_cpu.rms / multiplier;
        rms->report_rms                = true;
        rms->complexity                = result_cpu.complexity;
        // don't forget to keep the time unit, or we won't be able to recover the correct value.
        rms->time_unit = run0->time_unit;
    }

} // namespace BenchMark
