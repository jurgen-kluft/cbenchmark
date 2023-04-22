#ifndef __CBENCHMARK_BENCHMARK_H__
#define __CBENCHMARK_BENCHMARK_H__

#include "cbenchmark/private/c_benchmark_alloc.h"

namespace BenchMark
{
    class BMSuite;
    class BenchMarkReporter;

    void RunBenchMarkSuite(Allocator* allocator, BMSuite* suite, BenchMarkReporter* display_reporter, BenchMarkReporter* file_reporter);

} // namespace BenchMark

#endif ///< __CBENCHMARK_TEST_H__
