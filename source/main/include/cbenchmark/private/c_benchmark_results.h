#ifndef __CBENCHMARK_RESULTS_H__
#define __CBENCHMARK_RESULTS_H__

#include "cbenchmark/private/c_types.h"

namespace BenchMark
{
    class BenchMarkReporter;

    class BenchMarkResults
    {
    public:
        explicit BenchMarkResults(BenchMarkReporter* reporter = 0);

        void onBenchMarkSuiteStart(char const* testName, int inNumBenchMarks);
        void onBenchMarkSuiteEnd(char const* testName, float secondsElapsed);
        void onBenchMarkFixtureStart(char const* testName, int inNumBenchMarks);
        void onBenchMarkFixtureEnd(char const* testName, float secondsElapsed);
        void onBenchMarkStart(char const* testName);
        void onBenchMarkFailure(char const* file, int line, char const* testName, char const* failure);
        void onBenchMarkEnd(char const* testName, float secondsElapsed);

        BenchMarkReporter* mBenchMarkReporter;
        int                mBenchMarkSuiteCount;
        int                mBenchMarkFixtureCount;
        int                mBenchMarkCount;
        int                mFailureCount;
        int                mExceptionBenchMarkCount;
        int                mExceptionFailureCount;
    };

    // Counter flags
    enum CounterFlags
    {
        kIsRate                   = 1 << 16,                         // Mark the counter as a rate. It will be presented divided by the duration of the benchmark.
        kAvgThreads               = 1 << 17,                         // Mark the counter as a thread-average quantity. It will be presented divided by the number of threads.
        kAvgThreadsRate           = kIsRate | kAvgThreads,           // Mark the counter as a thread-average rate. See above.
        kIsIterationInvariant     = 1 << 18,                         // Mark the counter as a constant value, valid/same for *every* iteration (will be *multiplied* by the iteration count)
        kIsIterationInvariantRate = kIsRate | kIsIterationInvariant, // Mark the counter as a constant rate, when reporting, it will be *multiplied* by the iteration count and then divided by the duration of the benchmark.
        kAvgIterations            = 1 << 19,                         // Mark the counter as a iteration-average quantity, it will be presented divided by the number of iterations.
        kAvgIterationsRate        = kIsRate | kAvgIterations,        // Mark the counter as a iteration-average rate. See above.
        kInvert                   = 1 << 31,                         // In the end, invert the result. This is always done last!
        kIs1000                   = 1000,                            // 1'000 items per 1k
        kIs1024                   = 1024,                            // 1'024 items per 1k
        kDefaults                 = kIs1000,
    };

    // TimeUnit is passed to a benchmark in order to specify the order of magnitude for the measured time.
    enum TimeUnit
    {
        kNanosecond,
        kMicrosecond,
        kMillisecond,
        kSecond
    };

    inline const char* GetTimeUnitString(TimeUnit unit)
    {
        switch (unit)
        {
            case kMillisecond: return "ms";
            case kMicrosecond: return "us";
            case kNanosecond: return "ns";
        }
        return "s";
    }

    inline double GetTimeUnitMultiplier(TimeUnit unit)
    {
        switch (unit)
        {
            case kMillisecond: return 1e3;
            case kMicrosecond: return 1e6;
            case kNanosecond: return 1e9;
        }
        return 1.0;
    }

    // Range Computations
    // Custom Counters
    //
    class BenchMarkConfig
    {
    public:
        TimeUnit     m_time_unit;
        s32          m_ranges_count;
        s32          m_iterations_count;
        s32          m_counters_count;
        s64*         m_ranges_array;
        s64*         m_iterations_array;
        const char** m_counter_names;
        double*      m_counter_values;
        s32*         m_counter_flags;

        double* m_min_time;
        double* m_min_warmup_time;
        s64*    m_iterations;
        s64*    m_repetitions;
    };

    class BenchMarkState
    {
    public:
        // maybe have a high-performance 'forward' allocator for the benchmark itself

        s64 m_range[2];
    };
} // namespace BenchMark

#endif // __CBENCHMARK_RESULTS_H__
