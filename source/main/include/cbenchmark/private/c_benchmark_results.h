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

#define BM_ARGS(...)                                                    \
    {                                                                   \
        const s32 args[] = {__VA_ARGS__};                               \
        settings->AddArgs(args, (s32)(sizeof(args) / sizeof(args[0]))); \
    }
#define BM_ADD_ARG(...)                                      \
    {                                                        \
        const s32 arg[] = {__VA_ARGS__};                     \
        settings->AddArg(arg, sizeof(arg) / sizeof(arg[0])); \
    }

#define BM_ADD_ARG_RANGE(lo, hi, multi) settings->AddArgRange(lo, hi, multi)
#define BM_ADD_ARG_DENSE_RANGE(start, limit, step) settings->AddArgDenseRange(start, limit, step)
#define BM_SET_ARG_NAMES(...)                                           \
    {                                                                   \
        const char* names[] = {__VA_ARGS__};                            \
        settings->SetArgNames(names, sizeof(names) / sizeof(names[0])); \
    }
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
                __unit.setup_     = BMSetup_Nil;                                             \
                __unit.teardown_  = BMTeardown_Nil;                                          \
                __unit.settings_  = BMSettings_Nil;                                          \
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
    void BMUnitSettings##name(Allocator* alloc, BenchMarkUnit* settings);           \
    namespace nsBMU##name                                                           \
    {                                                                               \
        extern BenchMarkUnit __unit;                                                \
        class SetBMUnitSettings                                                     \
        {                                                                           \
        public:                                                                     \
            inline SetBMUnitSettings() { __unit.settings_ = BMUnitSettings##name; } \
        };                                                                          \
    }                                                                               \
    void BMUnitSettings##name(Allocator* alloc, BenchMarkUnit* settings)

#define BM_FIXTURE(bmname)                                                                      \
    namespace nsBMF##bmname                                                                     \
    {                                                                                           \
        BenchMarkFixture __fixture;                                                             \
        class BMRegisterFixture                                                                 \
        {                                                                                       \
        public:                                                                                 \
            inline BMRegisterFixture(const char* _name, const char* _filename, int _lineNumber) \
            {                                                                                   \
                __fixture.setup      = BMSetup_Nil;                                             \
                __fixture.teardown   = BMTeardown_Nil;                                          \
                __fixture.settings   = BMSettings_Nil;                                          \
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
    void BMFixtureSettings(Allocator* alloc, BenchMarkUnit* settings);            \
    class SetBMFixtureSettings                                                    \
    {                                                                             \
    public:                                                                       \
        inline SetBMFixtureSettings() { __fixture.settings = BMFixtureSettings; } \
    };                                                                            \
    void BMFixtureSettings(Allocator* alloc, BenchMarkUnit* settings)

#define BM_SUITE(bmname)                                                                  \
    extern void RegisterBenchMarkSuite(BenchMarkSuite*);                                  \
    namespace nsBMS##bmname                                                               \
    {                                                                                     \
        void           BMSetup_Nil(const BenchMarkState&) {}                              \
        void           BMTeardown_Nil(const BenchMarkState&) {}                           \
        void           BMSettings_Nil(Allocator* alloc, BenchMarkUnit* settings) {}       \
        BenchMarkSuite __suite;                                                           \
        class BMSRegister                                                                 \
        {                                                                                 \
        public:                                                                           \
            inline BMSRegister(const char* _name, const char* _filename, int _lineNumber) \
            {                                                                             \
                __suite.setup      = BMSetup_Nil;                                         \
                __suite.teardown   = BMTeardown_Nil;                                      \
                __suite.settings   = BMSettings_Nil;                                      \
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
    void BMSuiteSettings(Allocator* alloc, BenchMarkUnit* settings);        \
    class SetBMSuiteSettings                                                \
    {                                                                       \
    public:                                                                 \
        inline SetBMSuiteSettings() { __suite.settings = BMSuiteSettings; } \
    };                                                                      \
    void BMSuiteSettings(Allocator* alloc, BenchMarkUnit* settings)

} // namespace BenchMark

#endif // __CBENCHMARK_RESULTS_H__
