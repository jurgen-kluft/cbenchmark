#ifndef __CBENCHMARK_BENCHMARK_H__
#define __CBENCHMARK_BENCHMARK_H__

namespace BenchMark
{
    class Allocator;
    class BenchMarkReporter;

    void RunBenchMarks(Allocator* allocator, BenchMarkReporter* display_reporter, BenchMarkReporter* file_reporter);

} // namespace BenchMark

#endif ///< __CBENCHMARK_BENCHMARK_H__
