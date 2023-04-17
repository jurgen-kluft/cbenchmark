#ifdef TARGET_MAC
#include <stdlib.h>

#include "cbenchmark/cbenchmark.h"
#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_test_reporter_stdout.h"
#include "cbenchmark/private/c_test_reporter_teamcity.h"


extern bool gRunBenchMark(BenchMark::BenchMarkReporter& reporter, BenchMark::BenchMarkContext& context);
int         main(int argc, char** argv)
{
    BenchMark::NullObserver observer;
	BenchMark::NullAllocator dummy_allocator;

    BenchMark::BenchMarkReporterStdout stdout_reporter;
    BenchMark::BenchMarkReporter&      reporter = stdout_reporter;

    BenchMark::BenchMarkContext context;
	context.mAllocator = &dummy_allocator;
	context.mObserver = &observer;

    BenchMark::g_InitTimer();

    bool result = gRunBenchMark(reporter, context);

    return result ? 0 : -1;
}

#endif