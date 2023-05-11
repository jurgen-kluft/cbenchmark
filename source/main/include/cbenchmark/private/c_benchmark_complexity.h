#ifndef __CBENCHMARK_BENCHMARK_COMPLEXITY_H__
#define __CBENCHMARK_BENCHMARK_COMPLEXITY_H__

#include "cbenchmark/private/c_benchmark_allocators.h"
#include "cbenchmark/private/c_benchmark_array.h"

namespace BenchMark
{
    class BenchMarkRun;

    void ComputeBigO(Allocator* alloc, ScratchAllocator* scratch, const Array<BenchMarkRun*>& reports, Array<BenchMarkRun*>& bigo);

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_COMPLEXITY_H__