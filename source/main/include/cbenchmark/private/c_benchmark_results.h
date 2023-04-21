#ifndef __CBENCHMARK_RESULTS_H__
#define __CBENCHMARK_RESULTS_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_types.h"

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
        BMRegisterCounter(BenchMarkConfig& config, u64 id, CounterFlags flags, double value) { config.m_counters[config.m_num_counters++] = {id, flags, value}; }
    };

    enum EBenchMarkCounters
    {
        items_per_second = 0,
        bytes_per_second = 1,
    };

#define BM_COUNTER(id, flags) BMRegisterCounter registerCounter_##id(config, id, flags, 0)

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

#define BM_ITERATE \
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
