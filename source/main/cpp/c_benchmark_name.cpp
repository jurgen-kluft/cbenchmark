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
    static char* gStringAppend2(char* str, char const* strEnd, const char* str1, const char* str2)
    {
        if (str1 != nullptr)
        {
            while (*str1 != '\0' && str < strEnd)
            {
                *str++ = *str1++;
            }
        }
        if (str2 != nullptr)
        {
            while (*str2 != '\0' && str < strEnd)
            {
                *str++ = *str2++;
            }
        }
        return str;
    }

    s32 BenchmarkName::FullNameLen() const
    {
        s32 len = gStringLength(function_name);
        len += 1 + gStringLength(args);
        len += 1 + gStringLength(min_time);
        len += 1 + gStringLength(min_warmup_time);
        len += 1 + gStringLength(iterations);
        len += 1 + gStringLength(repetitions);
        len += 1 + gStringLength(time_type);
        len += 1 + gStringLength(threads);
        return len;
    }

    char* BenchmarkName::FullName(char* dst, const char* dstEnd) const
    {
        dst = gStringAppend(dst, dstEnd, function_name);
        dst = gStringAppend2(dst, dstEnd, "/", args);
        dst = gStringAppend2(dst, dstEnd, "/", min_time);
        dst = gStringAppend2(dst, dstEnd, "/", min_warmup_time);
        dst = gStringAppend2(dst, dstEnd, "/", iterations);
        dst = gStringAppend2(dst, dstEnd, "/", repetitions);
        dst = gStringAppend2(dst, dstEnd, "/", time_type);
        dst = gStringAppend2(dst, dstEnd, "/", threads);
        return dst;
    }

    void BenchmarkName::CopyFrom(ForwardAllocator* alloc, BenchmarkName const& other)
    {
        Release();
        allocator = alloc;

        // first determine the size of the string
        const s32 len = other.FullNameLen();

        // allocate the string
        char* str = allocator->Checkout<char>(len + 1);
        str[len]  = '\0';
        {
            function_name = str;
            str           = gStringAppend(str, str + len, other.function_name);
            str           = gStringAppend(str, str + len, '\0');

            args = str;
            if (other.args != nullptr)
            {
                str = gStringAppend(str, str + len, other.args);
                str = gStringAppend(str, str + len, '\0');
            }
            if (other.min_time != nullptr)
            {
                str = gStringAppend(str, str + len, other.min_time);
                str = gStringAppend(str, str + len, '\0');
            }
            if (other.min_warmup_time != nullptr)
            {
                str = gStringAppend(str, str + len, other.min_warmup_time);
                str = gStringAppend(str, str + len, '\0');
            }
            if (other.iterations != nullptr)
            {
                str = gStringAppend(str, str + len, other.iterations);
                str = gStringAppend(str, str + len, '\0');
            }
            if (other.repetitions != nullptr)
            {
                str = gStringAppend(str, str + len, other.repetitions);
                str = gStringAppend(str, str + len, '\0');
            }
            if (other.time_type != nullptr)
            {
                str = gStringAppend(str, str + len, other.time_type);
                str = gStringAppend(str, str + len, '\0');
            }
            if (other.threads != nullptr)
            {
                str = gStringAppend(str, str + len, other.threads);
                str = gStringAppend(str, str + len, '\0');
            }
        }
        allocator->Commit(str);
    }

    void BenchmarkName::Release()
    {
        if (allocator != nullptr)
        {
            allocator->Deallocate(function_name);
            allocator->Deallocate(args);
            allocator->Deallocate(min_time);
            allocator->Deallocate(min_warmup_time);
            allocator->Deallocate(iterations);
            allocator->Deallocate(repetitions);
            allocator->Deallocate(time_type);
            allocator->Deallocate(threads);

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