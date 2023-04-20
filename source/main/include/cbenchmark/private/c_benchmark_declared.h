#ifndef __CBENCHMARK_BENCHMARK_DECLARED_H__
#define __CBENCHMARK_BENCHMARK_DECLARED_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    class BenchMarkState;

    // ------------------------------------------------------
    // BenchMarkDeclared registration object.  The BENCHMARK() macro expands
    // into an internal::BenchMarkDeclared* object.  Various methods can
    // be called on this object to change the properties of the benchmark.
    // Each method returns "this" so that multiple method calls can
    // chained into one expression.
    class BenchMarkDeclared
    {
    public:
        virtual ~BenchMarkDeclared();

        // Note: the following methods all return "this" so that multiple
        // method calls can be chained together in one expression.

        // Specify the name of the benchmark
        BenchMarkDeclared* Name(const char* name);

        // Have "setup" and/or "teardown" invoked once for every benchmark run.
        // If the benchmark is multi-threaded (will run in k threads concurrently),
        // the setup callback will be be invoked exactly once (not k times) before
        // each run with k threads. Time allowing (e.g. for a short benchmark), there
        // may be multiple such runs per benchmark, each run with its own
        // "setup"/"teardown".
        //
        // If the benchmark uses different size groups of threads (e.g. via
        // ThreadRange), the above will be true for each size group.
        //
        // The callback will be passed a State object, which includes the number
        // of threads, thread-index, benchmark arguments, etc.
        //
        // The callback must not be NULL or self-deleting.
        BenchMarkDeclared* Setup(void (*setup)(const BenchMarkState&));
        BenchMarkDeclared* Teardown(void (*teardown)(const BenchMarkState&));

        // Pass this benchmark object to *func, which can customize
        // the benchmark by calling various methods like Arg, Args,
        // Threads, etc.
        BenchMarkDeclared* Apply(void (*func)(BenchMarkDeclared* benchmark));

        // Set the range multiplier for non-dense range. If not called, the range
        // multiplier kRangeMultiplier will be used.
        BenchMarkDeclared* RangeMultiplier(int multiplier);

        // Set the minimum amount of time to use when running this benchmark. This
        // option overrides the `benchmark_min_time` flag.
        // REQUIRES: `t > 0` and `Iterations` has not been called on this benchmark.
        BenchMarkDeclared* MinTime(double t);

        // Set the minimum amount of time to run the benchmark before taking runtimes
        // of this benchmark into account. This
        // option overrides the `benchmark_min_warmup_time` flag.
        // REQUIRES: `t >= 0` and `Iterations` has not been called on this benchmark.
        BenchMarkDeclared* MinWarmUpTime(double t);

        // Specify the amount of iterations that should be run by this benchmark.
        // This option overrides the `benchmark_min_time` flag.
        // REQUIRES: 'n > 0' and `MinTime` has not been called on this benchmark.
        //
        // NOTE: This function should only be used when *exact* iteration control is
        //   needed and never to control or limit how long a benchmark runs, where
        // `--benchmark_min_time=<N>s` or `MinTime(...)` should be used instead.
        BenchMarkDeclared* Iterations(IterationCount n);

        // Specify the amount of times to repeat this benchmark. This option overrides
        // the `benchmark_repetitions` flag.
        // REQUIRES: `n > 0`
        BenchMarkDeclared* Repetitions(int n);

        // Specify if each repetition of the benchmark should be reported separately
        // or if only the final statistics should be reported. If the benchmark
        // is not repeated then the single result is always reported.
        // Applies to *ALL* reporters (display and file).
        BenchMarkDeclared* ReportAggregatesOnly(bool value = true);

        // Same as ReportAggregatesOnly(), but applies to display reporter only.
        BenchMarkDeclared* DisplayAggregatesOnly(bool value = true);

        // By default, the CPU time is measured only for the main thread, which may
        // be unrepresentative if the benchmark uses threads internally. If called,
        // the total CPU time spent by all the threads will be measured instead.
        // By default, only the main thread CPU time will be measured.
        BenchMarkDeclared* MeasureProcessCPUTime();

        // If a particular benchmark should use the Wall clock instead of the CPU time
        // (be it either the CPU time of the main thread only (default), or the
        // total CPU usage of the benchmark), call this method. If called, the elapsed
        // (wall) time will be used to control how many iterations are run, and in the
        // printing of items/second or MB/seconds values.
        // If not called, the CPU time used by the benchmark will be used.
        BenchMarkDeclared* UseRealTime();

        // If a benchmark must measure time manually (e.g. if GPU execution time is being
        // measured), call this method. If called, each benchmark iteration should call
        // SetIterationTime(seconds) to report the measured time, which will be used
        // to control how many iterations are run, and in the printing of items/second
        // or MB/second values.
        BenchMarkDeclared* UseManualTime();

        // Set the asymptotic computational complexity for the benchmark. If called
        // the asymptotic computational complexity will be shown on the output.
        BenchMarkDeclared* Complexity(BigO complexity = {BigO::O_Auto});

        // Set the asymptotic computational complexity for the benchmark. If called
        // the asymptotic computational complexity will be shown on the output.
        BenchMarkDeclared* Complexity(BigO::Func* complexity);

        // Add this statistics to be computed over all the values of benchmark run
        BenchMarkDeclared* ComputeStatistics(const char*& name, Statistic::Func* statistics, StatisticUnit unit = {StatisticUnit::Time});

        virtual void Run(BenchMarkState& state) = 0;

        TimeUnit GetTimeUnit() const;

    protected:
        explicit BenchMarkDeclared(const char*& name);
        void SetName(const char*& name);

    public:
        const char* GetName() const;
        int         ArgsCnt() const;
        const char* GetArgName(int arg) const;

    private:
        friend class BenchmarkFamilies;
        friend class BenchmarkInstance;

        char*                 name_;
        AggregationReportMode aggregation_report_mode_;
        char*                 arg_names_[16]; // Args for all benchmark runs
        Arg                   args_[16];      // Args for all benchmark runs

        TimeUnit       time_unit_;
        bool           use_default_time_unit_;
        int            range_multiplier_;
        double         min_time_;
        double         min_warmup_time_;
        IterationCount iterations_;
        int            repetitions_;
        bool           measure_process_cpu_time_;
        bool           use_real_time_;
        bool           use_manual_time_;
        BigO           complexity_;
        BigO::Func*    complexity_lambda_;
        Statistics     statistics_;

        // int            thread_counts_[16];

        typedef void (*callback_function)(const BenchMarkState&);
        callback_function setup_;
        callback_function teardown_;

    }; // namespace BenchMark
} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_DECLARED_H__