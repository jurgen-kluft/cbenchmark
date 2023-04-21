#ifdef TARGET_PC
#include <windows.h>

#include "cbenchmark/cbenchmark.h"
#include "cbenchmark/private/c_Time_Helpers.h"
#include "cbenchmark/private/c_BenchMark_alloc.h"
#include "cbenchmark/private/c_BenchMark_Reporter_Console.h"

class BenchMarkNullObserver : public BenchMark::BenchMarkObserver
{
public:
    virtual void BeginSuite(const char* filename, const char* suite_name) {}
    virtual void EndSuite() {}

    virtual void BeginFixture(const char* filename, const char* suite_name, const char* fixture_name) {}
    virtual void EndFixture() {}

    virtual void BeginBenchMark(const char* filename, const char* suite_name, const char* fixture_name, const char* test_name) {}
    virtual void EndBenchMark() {}
};


extern bool gRunBenchMark(BenchMark::BenchMarkReporter& reporter, BenchMark::BenchMarkContext& context);
int         main(int argc, char** argv)
{
    BenchMark::NullObserver  observer;
    BenchMark::NullAllocator dummy_allocator;

    BenchMark::ConsoleReporter     stdout_reporter;
    BenchMark::BenchMarkReporter& reporter = stdout_reporter;

    BenchMark::BenchMarkContext context;
    context.mAllocator = &dummy_allocator;
    context.mObserver  = &observer;

    BenchMark::InitTimer();

    bool result = gRunBenchMark(reporter, context);

    return result ? 0 : -1;
}

#endif