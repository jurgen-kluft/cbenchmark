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
        Arg()
            : arg0(-1)
            , arg1(-1)
            , arg2(-1)
            , arg3(-1)
        {
        }
        Arg(s64 arg0, s64 arg1 = -1, s64 arg2 = -1, s64 arg3 = -1)
            : arg0(arg0)
            , arg1(arg1)
            , arg2(arg2)
            , arg3(arg3)
        {
        }

        union
        {
            s64 arg0, arg1, arg2, arg3;
            s64 args[4];
        };

        s32 size() const { return 4; }
    };

    struct ArgVector
    {
        s64              args[16];
        s64              size() const;
        static ArgVector Empty;
    };

    struct Counter
    {
        const char*  name;
        CounterFlags flags;
        double       value;
    };

    // Single occurrence variables
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

    // Counters:
    // By default there are 2 counters already defined, 'items_per_second' and 'bytes_per_second'.
    // For the user to define their own counters, they can use the following macro:
    // BM_COUNTERS(name, flags, name, flags, ...)

    struct BenchMarkRuntime;
    struct BenchMarkState;

    typedef void (*BM_Run)(BenchMarkRuntime& runtime, BenchMarkState& state);

    struct BenchMarkConfig
    {
        BM_Run               m_run;
        s64                  m_num_args;
        Arg                  m_args[32];
        s64                  m_num_counters;
        Counter              m_counters[8];
        MinTime const*       m_min_time;
        MinWarmupTime const* m_min_warmup_time;
        Iterations const*    m_iterations;
        Repetitions const*   m_repetitions;

        BenchMarkConfig() {}

        BenchMarkConfig(BM_Run run, MinTime const* min_time, MinWarmupTime const* min_warmup_time, Iterations const* iterations, Repetitions const* repetitions)
            : m_run(run)
            , m_num_args(0)
            , m_num_counters(0)
            , m_min_time(min_time)
            , m_min_warmup_time(min_warmup_time)
            , m_iterations(iterations)
            , m_repetitions(repetitions)
        {
        }

        void WriteArgs(Arg& a)
        {
            for (s64 i = 0; i < a.size(); ++i)
                m_args[m_num_args].args[0] = a.args[i];
            m_num_args++;
        }
        void WriteArgs(ArgVector& a)
        {
            for (s64 i = 0; i < a.size(); ++i)
                m_args[m_num_args].args[0] = a.args[i];
            m_num_args++;
        }
    };

#define BM_ARGS                                                                             \
    class BMArgs                                                                            \
    {                                                                                       \
    public:                                                                                 \
        BMArgs(ArgVector a, ArgVector b = ArgVector::Empty, ArgVector c = ArgVector::Empty) \
        {                                                                                   \
            config.WriteArgs(a);                                                            \
            config.WriteArgs(b);                                                            \
            config.WriteArgs(c);                                                            \
        }                                                                                   \
    };                                                                                      \
    static BMArgs args_list


#define BM_ARGSPRODUCT                               \
    class BMProductArgs                              \
    {                                                \
    public:                                          \
        BMProductArgs(ArgVector a, ArgVector b)      \
        {                                            \
            for (s64 i = 0; i < a.size(); ++i)       \
                for (s64 j = 0; j < b.size(); ++j)   \
                {                                    \
                    Arg arg(a.args[i], b.args[j]); \
                    config.WriteArgs(arg);           \
                }                                    \
        }                                            \
    };                                               \
    BMProductArgs args_product

    class BMRegisterCounter
    {
    public:
        BMRegisterCounter(BenchMarkConfig& config, const char* name, CounterFlags flags, double value) { config.m_counters[config.m_num_counters++] = {name, flags, value}; }
    };

#define BM_COUNTER(name, flags) BMRegisterCounter registerCounter_##name(config, #name, flags, 0)

#define BM_MINTIME(time) static const MinTime min_time = {time}
#define BM_MINWARMUPTIME(time) static const MinWarmupTime min_warmup_time = {time}
#define BM_ITERATIONS(count) static const Iterations iterations = {count}
#define BM_REPETITIONS(count) static const Repetitions repetitions = {count}

#define BM_TEST(name)                                                             \
    namespace nsBM##name                                                          \
    {                                                                             \
        BenchMarkConfig config;                                                   \
        void            BM_Run(BenchMarkRuntime& runtime, BenchMarkState& state); \
    }                                                                             \
    namespace nsBM##name

    struct BenchMarkRuntime
    {
        bool        m_error;
        const char* m_error_message;
    };

    struct BenchMarkState
    {
        // maybe have a high-performance 'forward' allocator for the benchmark itself
        s64     m_range[2];
        Counter m_counters[8];

        void SetItemsProcessed(s64 items) { m_counters[0].value = (double)items; }
        void SetBytesProcessed(s64 bytes) { m_counters[1].value = (double)bytes; }
    };

#define BM_RUN                                           \
    class BMInitConfig                                   \
    {                                                    \
    public:                                              \
        BMInitConfig(BenchMarkConfig& config)            \
        {                                                \
            config.m_run             = BM_Run;           \
            config.m_min_time        = &min_time;        \
            config.m_min_warmup_time = &min_warmup_time; \
            config.m_iterations      = &iterations;      \
            config.m_repetitions     = &repetitions;     \
        }                                                \
    };                                                   \
    BMInitConfig init_config(config);                    \
    void         BM_Run(BenchMarkRuntime& runtime, BenchMarkState& state)

#define BM_SUITE(name)                                       \
    namespace nsBM##name                                     \
    {                                                        \
        static const MinTime       min_time        = {5.0};  \
        static const MinWarmupTime min_warmup_time = {1.0};  \
        static const Iterations    iterations      = {1000}; \
        static const Repetitions   repetitions     = {5};    \
    }                                                        \
    namespace nsBM##name

#define BM_FIXTURE(name)                                     \
    namespace nsBM##name                                     \
    {                                                        \
        static const MinTime       min_time        = {5.0};  \
        static const MinWarmupTime min_warmup_time = {1.0};  \
        static const Iterations    iterations      = {1000}; \
        static const Repetitions   repetitions     = {5};    \
    }                                                        \
    namespace nsBM##name

} // namespace BenchMark

#endif // __CBENCHMARK_RESULTS_H__
