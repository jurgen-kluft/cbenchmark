#include "ccore/c_target.h"
#include "cbenchmark/cbenchmark.h"

#include "cunittest/cunittest.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(test_benchmark)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(test)
        {
            BenchMark::MainAllocator main_allocator;
            BenchMark::BenchMarkGlobals globals;
            BenchMark::ConsoleReporter reporter;
            
            BenchMark::gRunBenchMark(&main_allocator, &globals, reporter);


        }
    }
}
UNITTEST_SUITE_END
