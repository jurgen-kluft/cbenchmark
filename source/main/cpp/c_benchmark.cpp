#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_macros.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_reporter_stdout.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"

namespace BenchMark
{
    static NullObserver  sNullObserver;
    static NullAllocator sNullAllocator;

    BenchMarkContext::BenchMarkContext()
        : mAllocator(&sNullAllocator)
        , mObserver(&sNullObserver)
    {
    }

    BenchMark::BenchMark(char const* testName, char const* filename, const int lineNumber, BenchMarkRun run, BenchMarkFixture* fixture)
        : mName(testName)
        , mFilename(filename)
        , mLineNumber(lineNumber)
        , mTimeConstraintExempt(false)
        , mBenchMarkRun(run)
        , mBenchMarkNext(0)
    {
        if (fixture != 0)
        {
            // add this test to the fixture's list
            if (fixture->mBenchMarkListHead == 0)
            {
                fixture->mBenchMarkListHead = this;
            }
            else
            {
                fixture->mBenchMarkListTail->mBenchMarkNext = this;
            }
            fixture->mBenchMarkListTail = this;
        }
    }

    BenchMarkFixture::BenchMarkFixture(const char* inName, const char* inFilename, const int inLineNumber, BenchMarkAllocator** inAllocator, BenchMarkSuite* inSuite)
        : mBenchMarkListHead(0)
        , mBenchMarkListTail(0)
        , mName(inName)
        , mFilename(inFilename)
        , mLineNumber(inLineNumber)
        , mTimeConstraintExempt(false)
        , mSetup(0)
        , mTeardown(0)
        , mAllocator(inAllocator)
        , mFixtureNext(0)
    {
        if (inSuite->mFixtureListHead == nullptr)
            inSuite->mFixtureListHead = this;
        else
            inSuite->mFixtureListTail->mFixtureNext = this;

        inSuite->mFixtureListTail = this;
    }

    BenchMarkSuite::BenchMarkSuite(const char* inName, const char* inFilename)
        : mName(inName)
        , mFilename(inFilename)
        , mFixtureListHead(0)
        , mFixtureListTail(0)
    {

    }

    int BenchMarkAllRun(BenchMarkContext& context, BenchMarkReporter& reporter, BenchMarkSuite* inSuiteList, int maxBenchMarkTimeInMs)
    {
        BenchMarkResults result(&reporter);

        time_t overallTime = TimeStamp();

        BenchMarkSuite* suiteList = inSuiteList;
        while (suiteList != 0)
        {
            int        numBenchMarks     = 0;
            BenchMarkSuite* curBenchMarkSuite = suiteList;

            BenchMarkFixture* curBenchMarkFixture = curBenchMarkSuite->mFixtureListHead;
            while (curBenchMarkFixture != 0)
            {
                ++numBenchMarks;
                curBenchMarkFixture = curBenchMarkFixture->mFixtureNext;
            }

            context.mObserver->BeginSuite(curBenchMarkSuite->mFilename, curBenchMarkSuite->mName);
            {
                time_t suiteStartTime = TimeStamp();
                result.onBenchMarkSuiteStart(curBenchMarkSuite->mName, numBenchMarks);

                curBenchMarkFixture = curBenchMarkSuite->mFixtureListHead;
                while (curBenchMarkFixture != 0)
                {
                    BenchMarkFixtureRun(curBenchMarkSuite, curBenchMarkFixture, context, result, maxBenchMarkTimeInMs);
                    curBenchMarkFixture = curBenchMarkFixture->mFixtureNext;
                }

                result.onBenchMarkSuiteEnd(curBenchMarkSuite->mName, (float)GetElapsedTimeInMs(suiteStartTime) / 1000.0f);
            }
            context.mObserver->EndSuite();

            suiteList = suiteList->mSuiteNext;
        }

        float const secondsElapsed = (float)GetElapsedTimeInMs(overallTime) / 1000.0f;
        reporter.reportSummary(secondsElapsed, result.mFailureCount, result.mBenchMarkCount);

        return result.mFailureCount;
    }
} // namespace BenchMark
