#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_exception.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_alloc.h"
#include "cbenchmark/private/c_benchmark_name.h"

namespace BenchMark
{
    static s32 StringLen(const char* str)
    {
        if (str == nullptr)
            return 0;

        s32 len = 0;
        while (str[len] != '\0')
            ++len;
        return len;
    }

    static void StringAppend(char*& str, const char* sep, const char* append)
    {
        while (sep != nullptr && *sep != '\0')
        {
            *str++ = *sep++;
        }
        while (*append != '\0')
        {
            *str++ = *append++;
        }
    }

    char* BenchmarkName::FullName(Allocator* allocator) const 
    {
        s32 len = 0;
        len += StringLen(function_name);
        len += StringLen(args);
        len += StringLen(min_time);
        len += StringLen(min_warmup_time);
        len += StringLen(iterations);
        len += StringLen(repetitions);
        len += StringLen(time_type);
        len += StringLen(threads);

        char* full_name = (char*)allocator->Allocate(len + 8);
        
        char* str = full_name;
        StringAppend(str, nullptr, function_name);
        StringAppend(str, "/", args);
        StringAppend(str, "/", min_time);
        StringAppend(str, "/", min_warmup_time);
        StringAppend(str, "/", iterations);
        StringAppend(str, "/", repetitions);
        StringAppend(str, "/", time_type);
        StringAppend(str, "/", threads);

        return full_name;
    }
} // namespace BenchMark