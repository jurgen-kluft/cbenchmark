#ifndef __CBENCHMARK_RESULTS_H__
#define __CBENCHMARK_RESULTS_H__

#include "cbenchmark/private/c_types.h"

namespace BenchMark
{
    typedef s64 IterationCount;

#define BENCHMARK_BUILTIN_EXPECT(x, y) x

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
            Nanosecond  = 9,
            Microsecond = 6,
            Millisecond = 3,
            Second      = 0,
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

    class Complexity
    {
    public:
        enum
        {
            O_1,
            O_N,
            O_Log_N,
            O_N_Squared,
            O_N_Cubed,
            O_Exponential,
            O_Unknown
        };

        Complexity(u32 flags = O_Unknown)
            : flags(flags)
        {
        }

        inline const char* ToString() const
        {
            switch (flags)
            {
                case Complexity::O_1: return "O(1)";
                case Complexity::O_N: return "O(N)";
                case Complexity::O_Log_N: return "O(logN)";
                case Complexity::O_N_Squared: return "O(N^2)";
                case Complexity::O_N_Cubed: return "O(N^3)";
                case Complexity::O_Exponential: return "O(2^N)";
            }
            return "O(?)";
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
        {
            args[0] = -1;
            args[1] = -1;
            args[2] = -1;
            args[3] = -1;
        }
        Arg(s64 arg0, s64 arg1 = -1, s64 arg2 = -1, s64 arg3 = -1)
        {
            args[0] = arg0;
            args[1] = arg1;
            args[2] = arg2;
            args[3] = arg3;
        }

        s64 args[4];
        s32 size() const { return sizeof(args) / sizeof(args[0]); }
    };

    struct ArgVector
    {
        s64 args[16];
        s64 size() const { return sizeof(args) / sizeof(args[0]); }
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
        Arg                  m_args[64];
        s64                  m_num_counters;
        Counter              m_counters[8];
        TimeUnit const*      m_time_unit;
        MinTime const*       m_min_time;
        MinWarmupTime const* m_min_warmup_time;
        Iterations const*    m_iterations;
        Repetitions const*   m_repetitions;

        BenchMarkConfig() {}

        BenchMarkConfig(BM_Run run, TimeUnit const* time_unit, MinTime const* min_time, MinWarmupTime const* min_warmup_time, Iterations const* iterations, Repetitions const* repetitions)
            : m_run(run)
            , m_num_args(0)
            , m_num_counters(0)
            , m_time_unit(time_unit)
            , m_min_time(min_time)
            , m_min_warmup_time(min_warmup_time)
            , m_iterations(iterations)
            , m_repetitions(repetitions)
        {
        }

        void WriteArg(ArgVector a)
        {
            for (s64 i = 0; i < a.size(); ++i)
            {
                if (a.args[i] == -1)
                    break;
                m_args[m_num_args].args[i] = a.args[i];
            }
            m_num_args++;
        }
    };

#define BM_ARG                   \
    class BMArg                  \
    {                            \
    public:                      \
        BMArgs(ArgVector a)      \
        {                        \
            config.WriteArgs(a); \
        }                        \
    };                           \
    static BMArg args_list

#define BM_ARGRANGE                          \
    class BMArgRange                         \
    {                                        \
    public:                                  \
        BMArgRange(ArgVector a, ArgVector b) \
        {                                    \
        }                                    \
    };                                       \
    BMArgRange arg_range

#define BM_ARGPRODUCT                                        \
    class BMArgProduct                                       \
    {                                                        \
    public:                                                  \
        BMArgProduct(ArgVector a, ArgVector b)               \
        {                                                    \
            for (s64 i = 0; i < a.size(); ++i)               \
            {                                                \
                if (a.args[i] == -1)                         \
                    break;                                   \
                for (s64 j = 0; j < b.size(); ++j)           \
                {                                            \
                    if (b.args[i] == -1)                     \
                        break;                               \
                    config.WriteArg({a.args[i], b.args[j]}); \
                }                                            \
            }                                                \
        }                                                    \
    };                                                       \
    BMArgProduct args_product

    class BMRegisterCounter
    {
    public:
        BMRegisterCounter(BenchMarkConfig& config, const char* name, CounterFlags flags, double value) { config.m_counters[config.m_num_counters++] = {name, flags, value}; }
    };

#define BM_COUNTER(name, flags) BMRegisterCounter registerCounter_##name(config, #name, flags, 0)

#define BM_TIMEUNIT(tu) static const TimeUnit time_unit(tu)
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
        s64     m_range[4];
        Counter m_counters[8];
        s64     m_iterations;
        s64     m_max_iterations;
        s64     m_complexity_n;

        void Error(const char* reason);

        bool Skipped() const;

        void FinishKeepRunning();

        void PauseTiming();
        void ResumeTiming();

        s64 Range(s32 i) const { return m_range[i]; }
        s64 Iterations() const { return m_iterations; }

        void SetItemsProcessed(s64 items) { m_counters[0].value = (double)items; }
        void SetBytesProcessed(s64 bytes) { m_counters[1].value = (double)bytes; }

        void SetComplexityN(s64 n) { m_complexity_n = n; }
    };

#define BM_ITERATE
    for (state.m_iterations = 0; state.m_iterations < state.m_max_iterations; ++state.m_iterations)


#define BM_RUN                                           \
    class BMInitConfig                                   \
    {                                                    \
    public:                                              \
        BMInitConfig(BenchMarkConfig& config)            \
        {                                                \
            config.m_run             = BM_Run;           \
            config.m_time_unit       = &time_unit;       \
            config.m_min_time        = &min_time;        \
            config.m_min_warmup_time = &min_warmup_time; \
            config.m_iterations      = &iterations;      \
            config.m_repetitions     = &repetitions;     \
        }                                                \
    };                                                   \
    BMInitConfig init_config(config);                    \
    void         BM_Run(BenchMarkRuntime& runtime, BenchMarkState& state)

#define BM_SUITE(name)                                                        \
    namespace nsBM##name                                                      \
    {                                                                         \
        static const TimeUnit      time_unit       = {TimeUnit::Millisecond}; \
        static const MinTime       min_time        = {5.0};                   \
        static const MinWarmupTime min_warmup_time = {1.0};                   \
        static const Iterations    iterations      = {1000};                  \
        static const Repetitions   repetitions     = {5};                     \
    }                                                                         \
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
