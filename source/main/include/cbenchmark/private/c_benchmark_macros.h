#ifndef __CBENCHMARK_BENCHMACROS_H__
#define __CBENCHMARK_BENCHMACROS_H__

#ifdef BENCHMARK
#error Benckmark redefines BENCHMARK
#endif

#define BENCHMARK_SUITE_LIST(NameOfList) \
    namespace BenchMark##NameOfList      \
    {                                    \
        BenchSuite* __head = 0;          \
        BenchSuite* __tail = 0;          \
    }

#define BENCHMARK_SUITE_DECLARE(NameOfList, NameOfSuite)                                                                  \
    namespace nsBenchSuite##NameOfSuite                                                                                   \
    {                                                                                                                     \
        extern BenchMark::BenchSuite   __suite;                                                                           \
        BenchMark::AddSuiteToSuiteList AddToList(BenchMark##NameOfList::__head, BenchMark##NameOfList::__tail, &__suite); \
    }

#define BENCHMARK_SUITE_RUN(Context, Reporter, NameOfList) BenchMark::RunAllBenchMarks(Context, Reporter, BenchMark##NameOfList::__head);

#define BENCHMARK_SUITE_BEGIN(NameOfSuite)                              \
    namespace nsBenchSuite##NameOfSuite                                 \
    {                                                                   \
        BenchMark::BenchSuite __suite("Suite_" #NameOfSuite, __FILE__); \
    }                                                                   \
    namespace nsBenchSuite##NameOfSuite

#define BENCHMARK_SUITE_END

#define BENCHMARK_FIXTURE(Name)                                                                            \
    namespace nsBenchFixture##Name                                                                         \
    {                                                                                                      \
        static BenchMark::BenchAllocator* BenchAllocator = 0;                                              \
        BenchMark::BenchFixture           __fixture(#Name, __FILE__, __LINE__, &BenchAllocator, &__suite); \
    }                                                                                                      \
    namespace nsBenchFixture##Name

#define BENCHMARK_FIXTURE_SETUP()                                                      \
    void                          FixtureSetup(BenchMark::BenchResults& benchResults); \
    BenchMark::SetSetupForFixture SetFixtureSetup(&__fixture, &FixtureSetup);          \
    void                          FixtureSetup(BenchMark::BenchResults& benchResults)

#define BENCHMARK_FIXTURE_TEARDOWN()                                                         \
    void                             FixtureTeardown(BenchMark::BenchResults& benchResults); \
    BenchMark::SetTeardownForFixture SetFixtureTeardown(&__fixture, &FixtureTeardown);       \
    void                             FixtureTeardown(BenchMark::BenchResults& benchResults)

#define BENCHMARK_TEST(Name)                                                                                     \
    namespace nsBench##Name                                                                                      \
    {                                                                                                            \
        void            BenchRun(const char* benchName, BenchMark::BenchResults& benchResults, int maxTimeInMs); \
        BenchMark::BenchMark __bench(#Name, __FILE__, __LINE__, &BenchRun, &__fixture);                               \
    }                                                                                                            \
    void nsBench##Name::BenchRun(const char* benchName, BenchMark::BenchResults& benchResults, int maxTimeInMs)

#endif ///< __CBENCHMARK_BENCHMACROS_H__
