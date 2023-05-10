#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_exception.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_allocators.h"
#include "cbenchmark/private/c_benchmark_name.h"

namespace BenchMark
{
    static void StringAppend(char*& str, char const* strEnd, const char* sep, const char* append)
    {
        while (sep != nullptr && *sep != '\0' && str < strEnd)
        {
            *str++ = *sep++;
        }
        while (*append != '\0' && str < strEnd)
        {
            *str++ = *append++;
        }
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
        char* str = dst;
        StringAppend(str, dstEnd, nullptr, function_name);
        StringAppend(str, dstEnd, "/", args);
        StringAppend(str, dstEnd, "/", min_time);
        StringAppend(str, dstEnd, "/", min_warmup_time);
        StringAppend(str, dstEnd, "/", iterations);
        StringAppend(str, dstEnd, "/", repetitions);
        StringAppend(str, dstEnd, "/", time_type);
        StringAppend(str, dstEnd, "/", threads);

        return str;
    }
} // namespace BenchMark