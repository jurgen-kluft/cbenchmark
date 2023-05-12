#ifndef __CBENCHMARK_BENCHMARK_NAME_H__
#define __CBENCHMARK_BENCHMARK_NAME_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    class ForwardAllocator;

    struct BenchmarkName
    {
        BenchmarkName()
            : allocator(nullptr)
            , function_name(nullptr)
            , args(nullptr)
            , min_time(nullptr)
            , min_warmup_time(nullptr)
            , iterations(nullptr)
            , repetitions(nullptr)
            , time_type(nullptr)
            , threads(nullptr)
        {
        }

        BenchmarkName(const BenchmarkName& other);

        ForwardAllocator* allocator;
        char*             function_name;
        char*             args;
        char*             min_time;
        char*             min_warmup_time;
        char*             iterations;
        char*             repetitions;
        char*             time_type;
        char*             threads;

        // Return the full name of the benchmark with each non-empty field separated by a '/'
        s32   FullNameLen() const;
        char* FullName(char* dst, const char* dstEnd) const;
        void  CopyFrom(ForwardAllocator* alloc, BenchmarkName const& other);
        void  Release();
    };

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_NAME_H__