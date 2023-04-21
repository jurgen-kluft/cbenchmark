#ifndef __CBENCHMARK_BENCHMARK_COMPLEXITY_H__
#define __CBENCHMARK_BENCHMARK_COMPLEXITY_H__

#include "cbenchmark/private/c_benchmark_alloc.h"

namespace BenchMark
{
    class BenchMarkRun;

    void ComputeBigO(Allocator* alloc, const Array<BenchMarkRun>& reports, Array<BenchMarkRun>& bigo);

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_COMPLEXITY_H__