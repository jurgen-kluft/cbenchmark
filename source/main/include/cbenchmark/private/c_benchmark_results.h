#ifndef __CBENCHMARK_RESULTS_H__
#define __CBENCHMARK_RESULTS_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_entity.h"

namespace BenchMark
{
    typedef s64 IterationCount;

#define BENCHMARK_BUILTIN_EXPECT(x, y) x

    class BenchMarkReporter;
    struct BenchMarkState;

#define BM_ARG              \
    settings->AddArgName(); \
    settings->AddArg

#define BM_ARGRANGE settings->SetArgRange
#define BM_ARGPRODUCT settings->SetArgProduct

#define BM_NAMED_ARG settings->AddNamedArg
#define BM_NAMED_ARGRANGE settings->SetNamedArgRange

#define BM_COUNTER settings->AddCounter
#define BM_TIMEUNIT settings->SetTimeUnit
#define BM_MINTIME settings->SetMinTime
#define BM_MINWARMUPTIME settings->SetMinWarmupTime
#define BM_ITERATIONS settings->SetIterations
#define BM_REPETITIONS settings->SetRepetitions

#define BM_ITERATE while (state.KeepRunning())

    class BMUnit
    {
    public:
        BMUnit*           next;
        BenchMarkEntity*  entity;
        run_function      run;
        settings_function settings;
        const char*       name;
        const char*       filename;
        int               disabled;
        int               lineNumber;
    };

    class BMFixture
    {
    public:
        void AddBMUnit(BMUnit* bmu)
        {
            if (head == nullptr)
                head = bmu;
            else
                tail->next = bmu;
            tail = bmu;
        }
        BMUnit*           head;
        BMUnit*           tail;
        BMFixture*        next;
        settings_function settings;
        setup_function    setup;
        teardown_function teardown;
        const char*       name;
        const char*       filename;
        int               disabled;
        int               lineNumber;
    };

    class BMSuite
    {
    public:
        void AddBMFixture(BMFixture* bm)
        {
            if (head == nullptr)
                head = bm;
            else
                tail->next = bm;
            tail = bm;
        }
        BMFixture*        head;
        BMFixture*        tail;
        BMSuite*          next;
        settings_function settings;
        setup_function    setup;
        teardown_function teardown;
        const char*       name;
        const char*       filename;
        int               disabled;
        int               lineNumber;
    };

#define BM_TEST_DISABLE(name)                    \
    namespace nsBMU##name                        \
    {                                            \
        extern BMUnit __unit;                    \
    }                                            \
    class SetBMUnitDisable##name                 \
    {                                            \
    public:                                      \
        inline SetBMUnitDisable##name()          \
        {                                        \
            nsBMU##name::__unit.disabled = true; \
        }                                        \
    }

#define BM_TEST(bmname)                                                                      \
    void BM_Run_##bmname(BenchMarkState& state);                                             \
    namespace nsBMU##bmname                                                                  \
    {                                                                                        \
        static BenchMarkEntity __entity;                                                     \
        static BMUnit          __unit;                                                       \
        class BMRegisterUnit                                                                 \
        {                                                                                    \
        public:                                                                              \
            inline BMRegisterUnit(const char* _name, const char* _filename, int _lineNumber) \
            {                                                                                \
                __unit.entity     = &__entity;                                               \
                __unit.run        = BM_Run_##bmname;                                         \
                __unit.name       = _name;                                                   \
                __unit.filename   = _filename;                                               \
                __unit.lineNumber = _lineNumber;                                             \
                __fixture.AddBMUnit(&__unit);                                                \
            }                                                                                \
        };                                                                                   \
        static BMRegisterUnit __register(#bmname, __FILE__, __LINE__ + 2);                   \
    }                                                                                        \
    void BM_Run_##bmname(BenchMarkState& state)

#define BM_SETTINGS(name)                                                   \
    void BMUnitSettings##name(Allocator* alloc, BenchMarkEntity* settings); \
    namespace nsBMU##name                                                   \
    {                                                                       \
        extern BMUnit __unit;                                               \
        class SetBMUnitSettings                                             \
        {                                                                   \
        public:                                                             \
            inline SetBMUnitSettings()                                      \
            {                                                               \
                __unit.settings = BMUnitSettings##name;                     \
            }                                                               \
        };                                                                  \
    }                                                                       \
    void BMUnitSettings##name(Allocator* alloc, BenchMarkEntity* settings)

#define BM_FIXTURE(bmname)                                                                      \
    namespace nsBMF##bmname                                                                     \
    {                                                                                           \
        static BMFixture __fixture;                                                             \
        class BMRegisterFixture                                                                 \
        {                                                                                       \
        public:                                                                                 \
            inline BMRegisterFixture(const char* _name, const char* _filename, int _lineNumber) \
            {                                                                                   \
                __fixture.name       = _name;                                                   \
                __fixture.filename   = _filename;                                               \
                __fixture.lineNumber = _lineNumber;                                             \
                __suite.AddBMFixture(&__fixture);                                               \
            }                                                                                   \
        };                                                                                      \
        static BMRegisterFixture __register(#bmname, __FILE__, __LINE__ + 2);                   \
    }                                                                                           \
    namespace nsBMF##bmname

#define BM_FIXTURE_TEARDOWN                         \
    void BMFixtureTeardown(const BenchMarkState&);  \
    class SetBMFixtureTeardown                      \
    {                                               \
    public:                                         \
        inline SetBMFixtureTeardown()               \
        {                                           \
            __fixture.teardown = BMFixtureTeardown; \
        }                                           \
    };                                              \
    void BMFixtureTeardown(const BenchMarkState&)

#define BM_FIXTURE_SETUP                        \
    void BMFixtureSetup(const BenchMarkState&); \
    class SetBMFixtureSetup                     \
    {                                           \
    public:                                     \
        inline SetBMFixtureSetup()              \
        {                                       \
            __fixture.setup = BMFixtureSetup;   \
        }                                       \
    };                                          \
    void BMFixtureSetup(const BenchMarkState&)

#define BM_FIXTURE_SETTINGS                                              \
    void BMFixtureSettings(Allocator* alloc, BenchMarkEntity* settings); \
    class SetBMFixtureSettings                                           \
    {                                                                    \
    public:                                                              \
        inline SetBMFixtureSettings()                                    \
        {                                                                \
            __fixture.settings = BMFixtureSettings;                      \
        }                                                                \
    };                                                                   \
    void BMFixtureSettings(Allocator* alloc, BenchMarkEntity* settings)

#define BM_SUITE(bmname)                                                                  \
    extern void RegisterBenchMarkSuite(BMSuite*);                                         \
    namespace nsBMS##bmname                                                               \
    {                                                                                     \
        static BMSuite __suite;                                                           \
        class BMSRegister                                                                 \
        {                                                                                 \
        public:                                                                           \
            inline BMSRegister(const char* _name, const char* _filename, int _lineNumber) \
            {                                                                             \
                __suite.name       = _name;                                               \
                __suite.filename   = _filename;                                           \
                __suite.lineNumber = _lineNumber;                                         \
                RegisterBenchMarkSuite(&__suite);                                         \
            }                                                                             \
        };                                                                                \
        static BMSRegister __register(#bmname, __FILE__, __LINE__ + 2);                   \
    }                                                                                     \
    namespace nsBMS##bmname

#define BM_SUITE_TEARDOWN                        \
    void BMSuiteTeardown(const BenchMarkState&); \
    class SetBMSuiteTeardown                     \
    {                                            \
    public:                                      \
        inline SetBMSuiteTeardown()              \
        {                                        \
            __suite.teardown = BMSuiteTeardown;  \
        }                                        \
    };                                           \
    void BMSuiteTeardown(const BenchMarkState&)

#define BM_SUITE_SETUP                        \
    void BMSuiteSetup(const BenchMarkState&); \
    class SetBMSuiteSetup                     \
    {                                         \
    public:                                   \
        inline SetBMSuiteSetup()              \
        {                                     \
            __suite.setup = BMSuiteSetup;     \
        }                                     \
    };                                        \
    void BMSuiteSetup(const BenchMarkState&)

#define BM_SUITE_SETTINGS                                              \
    void BMSuiteSettings(Allocator* alloc, BenchMarkEntity* settings); \
    class SetBMSuiteSettings                                           \
    {                                                                  \
    public:                                                            \
        inline SetBMSuiteSettings()                                    \
        {                                                              \
            __suite.settings = BMSuiteSettings;                        \
        }                                                              \
    };                                                                 \
    void BMSuiteSettings(Allocator* alloc, BenchMarkEntity* settings)

} // namespace BenchMark

#endif // __CBENCHMARK_RESULTS_H__
