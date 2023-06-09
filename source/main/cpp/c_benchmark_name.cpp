#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_allocators.h"
#include "cbenchmark/private/c_benchmark_name.h"

namespace BenchMark
{
    BenchmarkName::BenchmarkName(const BenchmarkName& other)
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
        CopyFrom(other.allocator, other);
    }

    BenchmarkName& BenchmarkName::operator = (const BenchmarkName& other)
    {
        CopyFrom(other.allocator, other);
        return *this;
    }

    s32 BenchmarkName::FullNameLen() const
    {
        s32 len = gStringLength(function_name) + 1;
        len += 1 + gStringLength(args) + 1;
        len += 1 + gStringLength(min_time) + 1;
        len += 1 + gStringLength(min_warmup_time) + 1;
        len += 1 + gStringLength(iterations) + 1;
        len += 1 + gStringLength(repetitions) + 1;
        len += 1 + gStringLength(time_type) + 1;
        len += 1 + gStringLength(threads) + 1;
        return len;
    }

    char* BenchmarkName::FullName(char* dst, const char* dstEnd) const
    {
        dst = gStringAppend(dst, dstEnd, function_name);
        if (args != nullptr && *args != '\0')
            dst = gStringAppend2(dst, dstEnd, "/", args);
        if (min_time != nullptr && *min_time != '\0')
            dst = gStringAppend2(dst, dstEnd, "/", min_time);
        if (min_warmup_time != nullptr && *min_warmup_time != '\0')
            dst = gStringAppend2(dst, dstEnd, "/", min_warmup_time);
        if (iterations != nullptr && *iterations != '\0')
            dst = gStringAppend2(dst, dstEnd, "/", iterations);
        if (repetitions != nullptr && *repetitions != '\0')
            dst = gStringAppend2(dst, dstEnd, "/", repetitions);
        if (time_type != nullptr && *time_type != '\0')
            dst = gStringAppend2(dst, dstEnd, "/", time_type);
        if (threads != nullptr && *threads != '\0')
            dst = gStringAppend2(dst, dstEnd, "/", threads);
        return dst;
    }

    void BenchmarkName::CopyFrom(ForwardAllocator* alloc, BenchmarkName const& other)
    {
        Release();
        allocator = alloc;

        // first determine the size of the string
        s32 const len = other.FullNameLen();

        // allocate the string
        char* str                = allocator->Checkout<char>(len + 1);
        str[len]                 = '\0';
        const char* const strEnd = str + len;
        {
            function_name   = str;
            str             = gStringAppend(str, strEnd, other.function_name);
            str             = gStringAppend(str, strEnd, '\0');
            args            = str;
            str             = gStringAppend(str, strEnd, other.args);
            str             = gStringAppend(str, strEnd, '\0');
            min_time        = str;
            str             = gStringAppend(str, strEnd, other.min_time);
            str             = gStringAppend(str, strEnd, '\0');
            min_warmup_time = str;
            str             = gStringAppend(str, strEnd, other.min_warmup_time);
            str             = gStringAppend(str, strEnd, '\0');
            iterations      = str;
            str             = gStringAppend(str, strEnd, other.iterations);
            str             = gStringAppend(str, strEnd, '\0');
            repetitions     = str;
            str             = gStringAppend(str, strEnd, other.repetitions);
            str             = gStringAppend(str, strEnd, '\0');
            time_type       = str;
            str             = gStringAppend(str, strEnd, other.time_type);
            str             = gStringAppend(str, strEnd, '\0');
            threads         = str;
            str             = gStringAppend(str, strEnd, other.threads);
            str             = gStringAppend(str, strEnd, '\0');
        }
        allocator->Commit(str);
    }

    void BenchmarkName::Release()
    {
        if (allocator != nullptr)
        {
            allocator->Deallocate(function_name);

            function_name   = nullptr;
            args            = nullptr;
            min_time        = nullptr;
            min_warmup_time = nullptr;
            iterations      = nullptr;
            repetitions     = nullptr;
            time_type       = nullptr;
            threads         = nullptr;
        }
    }

} // namespace BenchMark