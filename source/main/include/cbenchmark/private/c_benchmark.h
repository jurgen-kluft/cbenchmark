#ifndef __CBENCHMARK_RUN_BENCHMARK_H__
#define __CBENCHMARK_RUN_BENCHMARK_H__

namespace BenchMark
{
    struct Allocators;
    class BenchMarkReporter;
    struct BenchMarkGlobals;

    void RunBenchMarks(Allocators& allocators, BenchMarkGlobals* globals, BenchMarkReporter* display_reporter, BenchMarkReporter* file_reporter);

} // namespace BenchMark

#endif // __CBENCHMARK_RUN_BENCHMARK_H__
