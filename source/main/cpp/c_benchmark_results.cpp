#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_reporter.h"

#include <string>

namespace BenchMark
{
    BM_SUITE(test_suite)
    {
        BM_FIXTURE(test_fixture)
        {
            // BM_SETUP()

            // BM_TEARDOWN()

            BM_TEST(test)
            {
                // BM_ARG({8, 32, 128, 512, 2048, 8192, 32768});
                // BM_ARGRANGE({0, 2, 8}, {128, 512});
                // BM_ARGRANGE({8, 64, 512, 4 << 10, 8 << 10, 9, 9, 9},{128, 512},{128, 512});
                BM_ARGPRODUCT({8, 16, 32, 64, 128}, {1, 2, 3, 4});

                BM_COUNTER(items_per_second, CounterFlags::IsRate);
                BM_COUNTER(bytes_per_second, CounterFlags::IsRate);
                BM_COUNTER(test, CounterFlags::IsRate);
                BM_COUNTER(test2, CounterFlags::IsRate);
                BM_COUNTER(test3, CounterFlags::IsRate);

                BM_TIMEUNIT(TimeUnit::Microsecond);
                BM_MINTIME(0);
                BM_MINWARMUPTIME(0);
                BM_ITERATIONS(0);
                BM_REPETITIONS(0);

                BM_RUN
                {
                    // here we have our benchmark code, for example
                    // to benchmark memcpy we can do something like this:
                    state.PauseTiming();
                    char* src = new char[state.Range(0)];
                    char* dst = new char[state.Range(0)];
                    memset(src, 'x', state.Range(0));
                    state.ResumeTiming();

                    BM_ITERATE
                    {
                        memcpy(dst, src, state.Range(0));
                    }

                    state.SetBytesProcessed(int64_t(state.Iterations()) * int64_t(state.Range(0)));
                    delete[] src;
                    delete[] dst;
                }
            }
        }
    }
} // namespace BenchMark

namespace BenchMark
{
    BenchMarkResults::BenchMarkResults(BenchMarkReporter* testReporter)
        : mBenchMarkReporter(testReporter)
        , mBenchMarkSuiteCount(0)
        , mBenchMarkFixtureCount(0)
        , mBenchMarkCount(0)
        , mFailureCount(0)
        , mExceptionBenchMarkCount(0)
        , mExceptionFailureCount(0)
    {
    }

    void BenchMarkResults::onBenchMarkSuiteStart(char const* testName, int inNumBenchMarks)
    {
        ++mBenchMarkSuiteCount;
        if (mBenchMarkReporter)
            mBenchMarkReporter->reportBenchMarkSuiteStart(inNumBenchMarks, testName);
    }

    void BenchMarkResults::onBenchMarkSuiteEnd(char const* testName, float secondsElapsed)
    {
        if (mBenchMarkReporter)
            mBenchMarkReporter->reportBenchMarkSuiteEnd(testName, secondsElapsed);
    }

    void BenchMarkResults::onBenchMarkFixtureStart(char const* testName, int inNumBenchMarks)
    {
        ++mBenchMarkFixtureCount;
        if (mBenchMarkReporter)
            mBenchMarkReporter->reportBenchMarkFixtureStart(inNumBenchMarks, testName);
    }

    void BenchMarkResults::onBenchMarkFixtureEnd(char const* testName, float secondsElapsed)
    {
        if (mBenchMarkReporter)
            mBenchMarkReporter->reportBenchMarkFixtureEnd(testName, secondsElapsed);
    }

    void BenchMarkResults::onBenchMarkStart(char const* testName)
    {
        ++mBenchMarkCount;
        if (mBenchMarkReporter)
            mBenchMarkReporter->reportBenchMarkStart(testName);
    }

    void BenchMarkResults::onBenchMarkFailure(char const* file, int line, char const* testName, char const* failure)
    {
        ++mFailureCount;
        if (mBenchMarkReporter)
            mBenchMarkReporter->reportFailure(file, line, testName, failure);
    }

    void BenchMarkResults::onBenchMarkEnd(char const* testName, float secondsElapsed)
    {
        if (mBenchMarkReporter)
            mBenchMarkReporter->reportBenchMarkEnd(testName, secondsElapsed);
    }
} // namespace BenchMark
