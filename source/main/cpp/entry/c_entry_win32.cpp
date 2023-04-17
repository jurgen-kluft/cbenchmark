#ifdef TARGET_PC
#include <windows.h>

#include "cbenchmark/cbenchmark.h"
#include "cbenchmark/private/c_TimeHelpers.h"
#include "cbenchmark/private/c_BenchMarkReporterStdout.h"
#include "cbenchmark/private/c_BenchMarkReporterTeamCity.h"

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

class BenchMarkNullAllocator : public BenchMark::BenchMarkAllocator
{
public:
    BenchMarkNullAllocator() {}

    virtual void*        Allocate(unsigned int size, unsigned int alignment) { return nullptr; }
    virtual unsigned int Deallocate(void* ptr) { return 0; }
};

extern bool gRunBenchMark(BenchMark::BenchMarkReporter& reporter, BenchMark::BenchMarkContext& context);
int         main(int argc, char** argv)
{
    BenchMarkNullObserver  observer;
    BenchMarkNullAllocator dummy_allocator;

    BenchMark::BenchMarkReporterStdout stdout_reporter;
    BenchMark::BenchMarkReporter&      reporter = stdout_reporter;

    BenchMark::BenchMarkContext context;
    context.mAllocator = &dummy_allocator;
    context.mObserver  = &observer;

    BenchMark::g_InitTimer();

    bool result = gRunBenchMark(reporter, context);

    return result ? 0 : -1;
}

#endif