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
    struct CounterFlags
    {
        CounterFlags(u32 flags = Defaults)
            : flags(flags)
        {
        }

        enum
        {
            IsRate                   = 1 << 16,                       // Mark the counter as a rate. It will be presented divided by the duration of the benchmark.
            AvgThreads               = 1 << 17,                       // Mark the counter as a thread-average quantity. It will be presented divided by the number of threads.
            AvgThreadsRate           = IsRate | AvgThreads,           // Mark the counter as a thread-average rate. See above.
            IsIterationInvariant     = 1 << 18,                       // Mark the counter as a constant value, valid/same for *every* iteration (will be *multiplied* by the iteration count)
            IsIterationInvariantRate = IsRate | IsIterationInvariant, // Mark the counter as a constant rate, when reporting, it will be *multiplied* by the iteration count and then divided by the duration of the benchmark.
            AvgIterations            = 1 << 19,                       // Mark the counter as a iteration-average quantity, it will be presented divided by the number of iterations.
            AvgIterationsRate        = IsRate | AvgIterations,        // Mark the counter as a iteration-average rate. See above.
            Invert                   = 1 << 31,                       // In the end, invert the result. This is always done last!
            Is1000                   = 1000,                          // 1'000 items per 1k
            Is1024                   = 1024,                          // 1'024 items per 1k
            Defaults                 = Is1000,
        };

        u32 flags;
    };

    // TimeUnit is passed to a benchmark in order to specify the order of magnitude for the measured time.
    struct TimeUnit
    {
        TimeUnit(u32 flags = Millisecond)
            : flags(flags)
        {
        }

        enum
        {
            Nanosecond,
            Microsecond,
            Millisecond,
            Second
        };

        inline const char* ToString() const
        {
            switch (flags)
            {
                case TimeUnit::Millisecond: return "ms";
                case TimeUnit::Microsecond: return "us";
                case TimeUnit::Nanosecond: return "ns";
            }
            return "s";
        }

        inline double GetTimeUnitMultiplier() const
        {
            switch (flags)
            {
                case TimeUnit::Millisecond: return 1e3;
                case TimeUnit::Microsecond: return 1e6;
                case TimeUnit::Nanosecond: return 1e9;
            }
            return 1.0;
        }

        u32 flags;
    };

    // Range Computations
    // Custom Counters
    // NOTE: Currently thinking of splitting this into Config and Runtime
    //       We cannot 'generate' data with PREPROCESSOR defines so we generate
    //       the Runtime from Config just before running the benchmark.

    struct Arg
    {
        s64 x;
    };
    struct ArgPair
    {
        s64 x, y;
    };
    struct Range
    {
        s64 start, end;
    };
    struct DenseRange
    {
        s64 start, end, step;
    };
    struct Counter
    {
        const char* const  name;
        CounterFlags const flags;
        double             value;
    };
    struct MinTime
    {
        double value;
    };
    struct MinWarmupTime
    {
        double value;
    };
    struct Iterations
    {
        s64 value;
    };
    struct Repetitions
    {
        s64 value;
    };

    static Counter items_per_second = {"items_per_second", CounterFlags::IsRate, 0};
    static Counter bytes_per_second = {"bytes_per_second", CounterFlags::IsRate, 0};

    // Counters:
    // By default there are 2 counters already defined, 'items_per_second' and 'bytes_per_second'.
    // For the user to define their own counters, they can use the following macro:
    // BM_COUNTERS(name, flags, name, flags, ...)

    typedef void (*SetVar)(BenchMarkConfig& c);

    struct BenchMarkRuntime;
    struct BenchMarkState;

    typedef void (*BM_Run)(BenchMarkRuntime& runtime, BenchMarkState& state);

    struct BenchMarkConfig
    {
        BM_Run m_run;

        BenchMarkConfig(BM_Run run, MinTime const* min_time, MinWarmupTime const* min_warmup_time, Iterations const* iterations, Repetitions const* repetitions)
            : m_run(run)
        {
        }

        void Set(Arg arg) {}
        void Set(ArgPair arg) {}
        void Set(Range arg) {}
        void Set(DenseRange arg) {}
        void Set(MinTime min_time) {}
        void Set(MinWarmupTime min_warmup_time) {}
        void Set(Iterations iterations) {}
        void Set(Repetitions repetitions) {}
        void Set(Counter counter) {}
    };

    BenchMarkConfig config;

#define BM_ARG(name, var) static const Arg name = {var}
#define BM_COUNTER(name, flags) static Counter name = {#name, flags, 0}

#define BM_MINTIME(time) static const MinTime min_time = {time}
#define BM_MINWARMUPTIME(time) static const MinWarmupTime min_warmup_time = {time}
#define BM_ITERATIONS(count) static const Iterations iterations = {count}
#define BM_REPETITIONS(count) static const Repetitions repetitions = {count}

#define BM_TEST(name)                                                  \
    namespace nsBM##name                                               \
    {                                                                  \
        void BM_Run(BenchMarkRuntime& runtime, BenchMarkState& state); \
    }                                                                  \
    namespace nsBM##name

    struct BenchMarkRuntime
    {
        bool         m_error;
        const char*  m_error_message;
        TimeUnit     m_time_unit;
        s32          m_ranges_count;
        s32          m_iterations_count;
        s32          m_counters_count;
        s64*         m_ranges_array;
        s64*         m_iterations_array;
        const char** m_counter_names;
        double*      m_counter_values;
        s32*         m_counter_flags;
    };

    struct BenchMarkState
    {
        // maybe have a high-performance 'forward' allocator for the benchmark itself
        s64 m_range[2];
    };

#define BM_RUN(name)                                                                        \
    BenchMarkConfig config(BM_Run, &min_time, &min_warmup_time, &iterations, &repetitions); \
    void            BM_Run(BenchMarkRuntime& runtime, BenchMarkState& state)

} // namespace BenchMark

#endif // __CBENCHMARK_RESULTS_H__
