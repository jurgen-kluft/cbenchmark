#ifndef __CBENCHMARK_TEST_H__
#define __CBENCHMARK_TEST_H__

namespace BenchMark
{
    class BenchMarkResults;
    class BenchMarkContext;
    class BenchMarkReporter;
    class BenchMarkAllocator;
    
    struct BenchMark;
    struct BenchMarkFixture;
    struct BenchMarkSuite;

    typedef void (*BenchMarkSetup)(BenchMarkResults& _results);
    typedef void (*BenchMarkTeardown)(BenchMarkResults& _results);
    typedef void (*BenchMarkRun)(const char* name, BenchMarkResults& _results, int const maxBenchMarkTimeInMs);

    struct BenchMark
    {
        BenchMark(char const* name, char const* filename, const int lineNumber, BenchMarkRun run, BenchMarkFixture* fixture);

        char const* const mName;
        char const* const mFilename;
        int const         mLineNumber;
        mutable bool      mTimeConstraintExempt;

        BenchMarkRun mBenchMarkRun;
        BenchMark*   mBenchMarkNext;
    };

    struct BenchMarkFixture
    {
        BenchMarkFixture(const char* name, const char* filename, const int lineNumber, BenchMarkAllocator** allocator, BenchMarkSuite* suite);

        BenchMark* mBenchMarkListHead;
        BenchMark* mBenchMarkListTail;

        char const* const mName;
        char const* const mFilename;
        int const         mLineNumber;
        mutable bool      mTimeConstraintExempt;

        BenchMarkSetup    mSetup;
        BenchMarkTeardown mTeardown;

        BenchMarkAllocator** mAllocator;
        BenchMarkFixture*    mFixtureNext;
    };

    struct BenchMarkSuite
    {
        BenchMarkSuite(const char* name, const char* filename);

        char const* const mName;
        char const* const mFilename;
        BenchMarkFixture*      mFixtureListHead;
        BenchMarkFixture*      mFixtureListTail;
        BenchMarkSuite*        mSuiteNext;
    };

    void BenchMarkFixtureRun(BenchMarkSuite* suite, BenchMarkFixture* fixture, BenchMarkContext& context, BenchMarkResults& results, int maxBenchMarkTimeInMs);
	int	 BenchMarkAllRun(BenchMarkContext& context, BenchMarkReporter& reporter, BenchMarkSuite* inSuiteList, int maxBenchMarkTimeInMs = 0);

    class SetRunForBenchMark
    {
    public:
        SetRunForBenchMark(BenchMark* test, BenchMarkRun run) { test->mBenchMarkRun = run; }
    };

    class SetSetupForFixture
    {
    public:
        SetSetupForFixture(BenchMarkFixture* fixture, BenchMarkSetup setup) { fixture->mSetup = setup; }
    };
    class SetTeardownForFixture
    {
    public:
        SetTeardownForFixture(BenchMarkFixture* fixture, BenchMarkTeardown teardown) { fixture->mTeardown = teardown; }
    };

    class AddSuiteToSuiteList
    {
    public:
        AddSuiteToSuiteList(BenchMarkSuite*& head, BenchMarkSuite*& tail, BenchMarkSuite* suite)
        {
            if (head == nullptr)
                head = suite;
            else
                tail->mSuiteNext = suite;

            tail = suite;
        }
    };

} // namespace BenchMark

#endif ///< __CBENCHMARK_TEST_H__
