#ifndef __CBENCHMARK_RESULTS_H__
#define __CBENCHMARK_RESULTS_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_unit.h"

namespace BenchMark
{
    class BenchMarkReporter;
    class BenchMarkState;
    typedef s64 IterationCount;

#define BENCHMARK_BUILTIN_EXPECT(x, y) x

#define BM_ARGS(...) settings->Args(__VA_ARGS__)
#define BM_ARG(index) settings->Arg(index)
#define BM_ARG_NAME(index, name) settings->Arg(index, name)
#define SEQUENCE(...) Sequence(__VA_ARGS__)
#define RANGE(lo, hi, multi) Range(lo, hi, multi)
#define DENSE_RANGE(start, limit, step) DenseRange(start, limit, step)

#define BM_THREAD_COUNTS(...)             \
    const s32 tcvector[] = {__VA_ARGS__}; \
    settings->SetThreadCounts(tcvector, (s32)(sizeof(tcvector) / sizeof(tcvector[0])))

#define BM_COUNTER settings->AddCounter
#define BM_TIMEUNIT settings->SetTimeUnit
#define BM_MINTIME settings->SetMinTime
#define BM_MINWARMUPTIME settings->SetMinWarmupTime
#define BM_ITERATIONS settings->SetIterations
#define BM_REPETITIONS settings->SetRepetitions

#define BM_ITERATE while (state.KeepRunning())

    class BenchMarkFixture
    {
    public:
        void AddUnit(BenchMarkUnit* bmu)
        {
            if (head == nullptr)
                head = bmu;
            else
                tail->next = bmu;
            tail = bmu;
        }
        BenchMarkUnit*    head;
        BenchMarkUnit*    tail;
        BenchMarkFixture* next;
        settings_function settings;
        setup_function    setup;
        teardown_function teardown;
        const char*       name;
        const char*       filename;
        int               disabled;
        int               lineNumber;
    };

    class BenchMarkSuite
    {
    public:
        void AddFixture(BenchMarkFixture* bm)
        {
            if (head == nullptr)
                head = bm;
            else
                tail->next = bm;
            tail = bm;
        }
        BenchMarkFixture* head;
        BenchMarkFixture* tail;
        BenchMarkSuite*   next;
        settings_function settings;
        setup_function    setup;
        teardown_function teardown;
        const char*       name;
        const char*       filename;
        int               disabled;
        int               lineNumber;
    };

#define BM_UNIT(bmname)                                                                      \
    void BM_Run_##bmname(BenchMarkState& state, Allocator* allocator);                       \
    namespace nsBMU##bmname                                                                  \
    {                                                                                        \
        BenchMarkUnit __unit;                                                                \
        class BMRegisterUnit                                                                 \
        {                                                                                    \
        public:                                                                              \
            inline BMRegisterUnit(const char* _name, const char* _filename, int _lineNumber) \
            {                                                                                \
                if (__unit.setup_ == nullptr)                                                \
                    __unit.setup_ = BMSetup_Nil;                                             \
                if (__unit.teardown_ == nullptr)                                             \
                    __unit.teardown_ = BMTeardown_Nil;                                       \
                if (__unit.settings_ == nullptr)                                             \
                    __unit.settings_ = BMSettings_Nil;                                       \
                __unit.run_       = BM_Run_##bmname;                                         \
                __unit.name       = _name;                                                   \
                __unit.filename   = _filename;                                               \
                __unit.lineNumber = _lineNumber;                                             \
                __fixture.AddUnit(&__unit);                                                  \
            }                                                                                \
        };                                                                                   \
        BMRegisterUnit __register(#bmname, __FILE__, __LINE__ + 2);                          \
    }                                                                                        \
    void BM_Run_##bmname(BenchMarkState& state, Allocator* allocator)

#define BM_UNIT_DISABLE(name)                                                    \
    namespace nsBMU##name { extern BenchMarkUnit __unit; }                       \
    class SetBMUnitDisable##name                                                 \
    {                                                                            \
    public:                                                                      \
        inline SetBMUnitDisable##name() { nsBMU##name::__unit.disabled = true; } \
    }

#define BM_SETTINGS(name)                                                           \
    void BMUnitSettings##name(BenchMarkUnit* settings);                             \
    namespace nsBMU##name                                                           \
    {                                                                               \
        extern BenchMarkUnit __unit;                                                \
        class SetBMUnitSettings                                                     \
        {                                                                           \
        public:                                                                     \
            inline SetBMUnitSettings() { __unit.settings_ = BMUnitSettings##name; } \
        };                                                                          \
        SetBMUnitSettings gSetBMUnitSettings;                                       \
    }                                                                               \
    void BMUnitSettings##name(BenchMarkUnit* settings)

#define BM_FIXTURE(bmname)                                                                      \
    namespace nsBMF##bmname                                                                     \
    {                                                                                           \
        BenchMarkFixture __fixture;                                                             \
        class BMRegisterFixture                                                                 \
        {                                                                                       \
        public:                                                                                 \
            inline BMRegisterFixture(const char* _name, const char* _filename, int _lineNumber) \
            {                                                                                   \
                if (__fixture.setup == nullptr)                                                 \
                    __fixture.setup = BMSetup_Nil;                                              \
                if (__fixture.teardown == nullptr)                                              \
                    __fixture.teardown = BMTeardown_Nil;                                        \
                if (__fixture.settings == nullptr)                                              \
                    __fixture.settings = BMSettings_Nil;                                        \
                __fixture.name       = _name;                                                   \
                __fixture.filename   = _filename;                                               \
                __fixture.lineNumber = _lineNumber;                                             \
                __suite.AddFixture(&__fixture);                                                 \
            }                                                                                   \
        };                                                                                      \
        BMRegisterFixture __register(#bmname, __FILE__, __LINE__ + 2);                          \
    }                                                                                           \
    namespace nsBMF##bmname

#define BM_FIXTURE_DISABLE(name)                                                       \
    namespace nsBMF##name { extern BenchMarkFixture __fixture; }                       \
    class SetBMFixtureDisable##name                                                    \
    {                                                                                  \
    public:                                                                            \
        inline SetBMFixtureDisable##name() { nsBMF##name::__fixture.disabled = true; } \
    }

#define BM_FIXTURE_TEARDOWN                                                       \
    void BMFixtureTeardown(const BenchMarkState&);                                \
    class SetBMFixtureTeardown                                                    \
    {                                                                             \
    public:                                                                       \
        inline SetBMFixtureTeardown() { __fixture.teardown = BMFixtureTeardown; } \
    };                                                                            \
    void BMFixtureTeardown(const BenchMarkState&)

#define BM_FIXTURE_SETUP                                                 \
    void BMFixtureSetup(const BenchMarkState&);                          \
    class SetBMFixtureSetup                                              \
    {                                                                    \
    public:                                                              \
        inline SetBMFixtureSetup() { __fixture.setup = BMFixtureSetup; } \
    };                                                                   \
    void BMFixtureSetup(const BenchMarkState&)

#define BM_FIXTURE_SETTINGS                                                       \
    void BMFixtureSettings(BenchMarkUnit* settings);                              \
    class SetBMFixtureSettings                                                    \
    {                                                                             \
    public:                                                                       \
        inline SetBMFixtureSettings() { __fixture.settings = BMFixtureSettings; } \
    };                                                                            \
    SetBMFixtureSettings gSetBMFixtureSettings;                                   \
    void                 BMFixtureSettings(BenchMarkUnit* settings)

#define BM_SUITE(bmname)                                                                  \
    extern void RegisterBenchMarkSuite(BenchMarkSuite*);                                  \
    namespace nsBMS##bmname                                                               \
    {                                                                                     \
        void           BMSetup_Nil(const BenchMarkState&) {}                              \
        void           BMTeardown_Nil(const BenchMarkState&) {}                           \
        void           BMSettings_Nil(BenchMarkUnit* settings) {}                         \
        BenchMarkSuite __suite;                                                           \
        class BMSRegister                                                                 \
        {                                                                                 \
        public:                                                                           \
            inline BMSRegister(const char* _name, const char* _filename, int _lineNumber) \
            {                                                                             \
                if (__suite.setup == nullptr)                                             \
                    __suite.setup = BMSetup_Nil;                                          \
                if (__suite.teardown == nullptr)                                          \
                    __suite.teardown = BMTeardown_Nil;                                    \
                if (__suite.settings == nullptr)                                          \
                    __suite.settings = BMSettings_Nil;                                    \
                __suite.name       = _name;                                               \
                __suite.filename   = _filename;                                           \
                __suite.lineNumber = _lineNumber;                                         \
                RegisterBenchMarkSuite(&__suite);                                         \
            }                                                                             \
        };                                                                                \
        BMSRegister __register(#bmname, __FILE__, __LINE__ + 2);                          \
    }                                                                                     \
    namespace nsBMS##bmname

#define BM_SUITE_DISABLE(name)                                                     \
    namespace nsBMS##name { extern BenchMarkSuite __suite; }                       \
    class SetBMSuiteDisable##name                                                  \
    {                                                                              \
    public:                                                                        \
        inline SetBMSuiteDisable##name() { nsBMS##name::__suite.disabled = true; } \
    }

#define BM_SUITE_TEARDOWN                                                   \
    void BMSuiteTeardown(const BenchMarkState&);                            \
    class SetBMSuiteTeardown                                                \
    {                                                                       \
    public:                                                                 \
        inline SetBMSuiteTeardown() { __suite.teardown = BMSuiteTeardown; } \
    };                                                                      \
    void BMSuiteTeardown(const BenchMarkState&)

#define BM_SUITE_SETUP                                             \
    void BMSuiteSetup(const BenchMarkState&);                      \
    class SetBMSuiteSetup                                          \
    {                                                              \
    public:                                                        \
        inline SetBMSuiteSetup() { __suite.setup = BMSuiteSetup; } \
    };                                                             \
    void BMSuiteSetup(const BenchMarkState&)

#define BM_SUITE_SETTINGS                                                   \
    void BMSuiteSettings(BenchMarkUnit* settings);                          \
    class SetBMSuiteSettings                                                \
    {                                                                       \
    public:                                                                 \
        inline SetBMSuiteSettings() { __suite.settings = BMSuiteSettings; } \
    };                                                                      \
    SetBMSuiteSettings gSetBMSuiteSettings;                                 \
    void               BMSuiteSettings(BenchMarkUnit* settings)

} // namespace BenchMark

#endif // __CBENCHMARK_RESULTS_H__
