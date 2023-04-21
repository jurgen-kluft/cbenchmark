#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_exception.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_runner.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_instance.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_statistics.h"
#include "cbenchmark/private/c_benchmark_alloc.h"

namespace BenchMark
{
    const char* BenchMarkRun::BenchMarkName(Allocator* alloc)
    {
        s32 len = run_name.FullNameLen();
        if (run_type == RT_Aggregate)
        {
            len += 1 + gStringLength(aggregate_name);
        }
        char* name    = (char*)alloc->Allocate(len + 1, 1);
        char* nameEnd = name + len;
        nameEnd[0]    = '\0';

        char* str = name;
        str       = run_name.FullName(str, nameEnd);
        if (run_type == RT_Aggregate)
        {
            gStringAppend(str, nameEnd, "_");
            gStringAppend(str, nameEnd, aggregate_name);
        }
    }

    double BenchMarkRun::GetAdjustedRealTime() const
    {
        double new_time = real_accumulated_time * time_unit.GetTimeUnitMultiplier();
        if (iterations != 0)
            new_time /= static_cast<double>(iterations);
        return new_time;
    }

    double BenchMarkRun::GetAdjustedCPUTime() const
    {
        double new_time = cpu_accumulated_time * time_unit.GetTimeUnitMultiplier();
        if (iterations != 0)
            new_time /= static_cast<double>(iterations);
        return new_time;
    }
} // namespace BenchMark