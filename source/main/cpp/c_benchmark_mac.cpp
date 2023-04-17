#ifdef TARGET_MAC

#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"

#include <exception>

namespace BenchMark
{
    class AssertException : public std::exception
    {
        enum ESettings
        {
            DESCRIPTION_MAX_STR_LENGTH = 512,
            FILENAME_MAX_STR_LENGTH    = 256
        };

    public:
        AssertException(char const* description, char const* filename, const int lineNumber);

        char mDescription[DESCRIPTION_MAX_STR_LENGTH];
        char mFilename[FILENAME_MAX_STR_LENGTH];
        int  mLineNumber;
    };

    AssertException::AssertException(char const* description, char const* filename, const int lineNumber)
        : mLineNumber(lineNumber)
    {
        gStringCopy(mDescription, description, FILENAME_MAX_STR_LENGTH);
        gStringCopy(mFilename, filename, DESCRIPTION_MAX_STR_LENGTH);
    }

	void ReportAssert(char const* description, char const* filename, int const lineNumber)
	{
		UT_THROW1(AssertException(description, filename, lineNumber));
	}

    void BenchMarkBenchMarkRun(BenchMark* test, BenchMarkContext& context, BenchMarkResults& results, int const maxBenchMarkTimeInMs)
    {
        time_t testTime = g_TimeStart();

        results.onBenchMarkStart(test->mName);

        try
        {
            test->mBenchMarkRun(test->mName, results, maxBenchMarkTimeInMs);
        }
        catch (AssertException const& e)
        {
            results.onBenchMarkFailure(e.mFilename, e.mLineNumber, test->mName, e.mDescription);
        }
        catch (std::exception const& e)
        {
            StringBuilder stringBuilder(context.mAllocator);
            stringBuilder << "Unhandled exception: " << e.what();
            results.onBenchMarkFailure(test->mFilename, test->mLineNumber, test->mName, stringBuilder.getText());
        }
        catch (...)
        {
            results.onBenchMarkFailure(test->mFilename, test->mLineNumber, test->mName, "Unhandled exception: Crash!");
        }
        const int testTimeInMs = (int)((float)g_GetElapsedTimeInMs(testTime) / 1000.0f);
        if (maxBenchMarkTimeInMs > 0 && testTimeInMs > maxBenchMarkTimeInMs && !test->mTimeConstraintExempt)
        {
            StringBuilder stringBuilder(context.mAllocator);
            stringBuilder << "Global time constraint failed. Expected under ";
            stringBuilder << maxBenchMarkTimeInMs;
            stringBuilder << "ms but took ";
            stringBuilder << testTimeInMs;
            stringBuilder << "ms.";

            results.onBenchMarkFailure(test->mFilename, test->mLineNumber, test->mName, stringBuilder.getText());
        }
        results.onBenchMarkEnd(test->mName, testTimeInMs / 1000.0f);
    }

    void BenchMarkFixtureRun(BenchMarkSuite* suite, BenchMarkFixture* fixture, BenchMarkContext& context, BenchMarkResults& results, int maxBenchMarkTimeInMs)
    {
        enum EStep
        {
            FIXTURE_SETUP,
            FIXTURE_UNITTESTS,
            FIXTURE_TEARDOWN,
        };

        // The fixture allocator will track the number of allocations and deallocations
        BenchMarkAllocatorEx fixtureAllocator(context.mAllocator);
        *fixture->mAllocator = &fixtureAllocator;

        int numBenchMarks = 0;
        if (fixture->mBenchMarkListHead != 0)
        {
            BenchMark* curBenchMark = fixture->mBenchMarkListHead;
            while (curBenchMark != 0)
            {
                numBenchMarks++;
                curBenchMark = curBenchMark->mBenchMarkNext;
            }
        }

        time_t testTime = g_TimeStart();

        results.onBenchMarkFixtureStart(fixture->mName, numBenchMarks);

        EStep step = FIXTURE_SETUP;
        try
        {
            // Remember allocation count X
            int iAllocCntX       = fixtureAllocator.GetNumAllocations();
            int iMemLeakCnt      = 0;
            int iExtraDeallocCnt = 0;

            if (fixture->mSetup != 0)
            {
                fixture->mSetup(results);
            }

            if (fixture->mBenchMarkListHead != 0)
            {
                step          = FIXTURE_UNITTESTS;
                BenchMark* curBenchMark = fixture->mBenchMarkListHead;
                while (curBenchMark != 0)
                {
                    // Remember allocation count Y
                    int iAllocCntY = fixtureAllocator.GetNumAllocations();

                    curBenchMark->mBenchMarkRun(curBenchMark->mName, results, maxBenchMarkTimeInMs);

                    // Compare allocation count with Y
                    // If different => memory leak error
                    if (iAllocCntY != fixtureAllocator.GetNumAllocations())
                    {
                        int iAllocCountDifference = (fixtureAllocator.GetNumAllocations() - iAllocCntY);

                        StringBuilder str(context.mAllocator);
                        if (iAllocCountDifference > 0)
                        {
                            iMemLeakCnt += iAllocCountDifference;
                            str << "memory leak detected, leaked memory allocations: ";
                            str << iAllocCountDifference;
                        }
                        else
                        {
                            iExtraDeallocCnt += -1 * iAllocCountDifference;
                            str << "extra memory deallocations detected, unmatching deallocations: ";
                            str << -1 * iAllocCountDifference;
                        }

                        results.onBenchMarkFailure(curBenchMark->mFilename, curBenchMark->mLineNumber, curBenchMark->mName, str.getText());
                    }
                    curBenchMark = curBenchMark->mBenchMarkNext;
                }
            }

            step = FIXTURE_TEARDOWN;
            if (fixture->mTeardown != 0)
                fixture->mTeardown(results);

            // Compare allocation count with X
            // If different => Fixture memory leak error (probably the combination of Setup() and Teardown()
            if (iAllocCntX != (fixtureAllocator.GetNumAllocations() - iMemLeakCnt))
            {
                StringBuilder str(context.mAllocator);

                str << "memory leak detected in setup()/teardown(), leaked memory allocations: ";
                str << iMemLeakCnt;

                results.onBenchMarkFailure(fixture->mFilename, fixture->mLineNumber, fixture->mName, str.getText());
            }

            if (iAllocCntX != (fixtureAllocator.GetNumAllocations() - iExtraDeallocCnt))
            {
                StringBuilder str(context.mAllocator);

                str << "extra deallocations detected in setup()/teardown(), extra deallocations: ";
                str << iExtraDeallocCnt;

                results.onBenchMarkFailure(fixture->mFilename, fixture->mLineNumber, fixture->mName, str.getText());
            }
        }
        catch (std::exception const& e)
        {
            StringBuilder stringBuilder(context.mAllocator);
            if (step == FIXTURE_SETUP)
                stringBuilder << "Unhandled exception in setup of fixture " << fixture->mName;
            else if (step == FIXTURE_TEARDOWN)
                stringBuilder << "Unhandled exception in teardown of fixture " << fixture->mName;
            else
                stringBuilder << "Unhandled exception in fixture " << fixture->mName;

            stringBuilder << " : " << e.what();
            results.onBenchMarkFailure(fixture->mFilename, fixture->mLineNumber, fixture->mName, stringBuilder.getText());
        }
        catch (...)
        {
            StringBuilder stringBuilder(context.mAllocator);
            if (step == FIXTURE_SETUP)
                stringBuilder << "Unhandled exception in setup of fixture " << fixture->mName;
            else if (step == FIXTURE_TEARDOWN)
                stringBuilder << "Unhandled exception in teardown of fixture " << fixture->mName;
            else
                stringBuilder << "Unhandled exception in fixture " << fixture->mName;

            results.onBenchMarkFailure(fixture->mFilename, fixture->mLineNumber, fixture->mName, stringBuilder.getText());
        }

        const int testTimeInMs = (int)((float)g_GetElapsedTimeInMs(testTime) / 1000.0f);
        if (maxBenchMarkTimeInMs > 0 && testTimeInMs > maxBenchMarkTimeInMs && !fixture->mTimeConstraintExempt)
        {
            StringBuilder stringBuilder(context.mAllocator);
            stringBuilder << "Fixture time constraint failed. Expected under ";
            stringBuilder << maxBenchMarkTimeInMs;
            stringBuilder << "ms but took ";
            stringBuilder << testTimeInMs;
            stringBuilder << "ms.";

            results.onBenchMarkFailure(fixture->mFilename, fixture->mLineNumber, fixture->mName, stringBuilder.getText());
        }
        results.onBenchMarkFixtureEnd(fixture->mName, testTimeInMs / 1000.0f);
    }
} // namespace BenchMark

#endif
