#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_macros.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_reporter_console.h"
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

    BenchMark::BenchMark(char const* testName, char const* filename, const int lineNumber, BenchMarkFunction run, BenchMarkFixture* fixture)
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

    BenchMarkFixture::BenchMarkFixture(const char* inName, const char* inFilename, const int inLineNumber, Allocator** inAllocator, BenchMarkSuite* inSuite)
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

    
} // namespace BenchMark
