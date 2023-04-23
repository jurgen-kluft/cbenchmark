#ifndef __CBENCHMARK_BENCHMARK_H__
#define __CBENCHMARK_BENCHMARK_H__

namespace BenchMark
{
    class Allocator;
    class BenchMarkReporter;
    struct BenchMarkGlobals;

    void RunBenchMarks(Allocator* allocator, Allocator* temp, BenchMarkGlobals* globals, BenchMarkReporter* display_reporter, BenchMarkReporter* file_reporter);

} // namespace BenchMark

#endif ///< __CBENCHMARK_BENCHMARK_H__
