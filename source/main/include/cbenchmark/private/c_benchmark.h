#ifndef __CBENCHMARK_RUN_BENCHMARK_H__
#define __CBENCHMARK_RUN_BENCHMARK_H__

#include "cbenchmark/private/c_benchmark_allocators.h"
#include "cbenchmark/private/c_benchmark_instance.h"
#include "cbenchmark/private/c_benchmark_reporter_console.h"

namespace BenchMark
{
    class MainAllocator;
    class BenchMarkGlobals;
    class BenchMarkReporter;

    bool gRunBenchMark(MainAllocator* allocator, BenchMarkGlobals* globals, BenchMarkReporter& reporter);

} // namespace BenchMark

#endif // __CBENCHMARK_RUN_BENCHMARK_H__
