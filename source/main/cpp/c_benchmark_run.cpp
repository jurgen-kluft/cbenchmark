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

namespace BenchMark
{
    const char* BenchMarkRun::BenchMarkName()
    {
        char* name = benchmark_name;
        const char* nameEnd = name + sizeof(benchmark_name) - 1;
        gStringAppend(name, nameEnd, run_name);
        if (run_type == RT_Aggregate)
        {
            gStringAppend(name, nameEnd, "_");
            gStringAppend(name, nameEnd, aggregate_name);
        }
        return benchmark_name;
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