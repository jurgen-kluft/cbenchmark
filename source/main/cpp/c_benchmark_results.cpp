#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_reporter.h"

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
