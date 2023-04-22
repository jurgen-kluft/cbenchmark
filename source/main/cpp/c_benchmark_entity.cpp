#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_check.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_entity.h"

namespace BenchMark
{
        void BenchMarkEntity::SetEnabled(bool enabled)
        {
            
        }
        void BenchMarkEntity::AddArg(ArgVector a)
        {
            
        }
        void BenchMarkEntity::AddArgName(const char* name = nullptr)
        {
            
        }
        void BenchMarkEntity::AddNamedArg(const char* aname, ArgVector a)
        {
            
        }
        void BenchMarkEntity::SetArgRange(ArgVector a, ArgVector b, ArgVector c = ArgVector::empty, ArgVector d = ArgVector::empty)
        {
            
        }
        void BenchMarkEntity::SetNamedArgRange(const char* aname, ArgVector a, const char* bname, ArgVector b, const char* cname = nullptr, ArgVector c = ArgVector::empty, const char* dname = nullptr, ArgVector d = ArgVector::empty)
        {
            
        }
        void BenchMarkEntity::SetArgProduct(ArgVector a, ArgVector b)
        {
            
        }
        void BenchMarkEntity::AddCounter(const char* name, CounterFlags flags, double value = 0.0)
        {
            
        }
        void BenchMarkEntity::SetTimeUnit(TimeUnit tu)
        {
            
        }
        void BenchMarkEntity::SetMinTime(double min_time)
        {
            
        }
        void BenchMarkEntity::SetMinWarmupTime(double min_warmup_time)
        {
            
        }
        void BenchMarkEntity::SetIterations(IterationCount iters)
        {
            
        }
        void BenchMarkEntity::SetRepetitions(int repetitions)
        {
            
        }
        void BenchMarkEntity::SetFuncRun(run_function func)
        {
            
        }
        void BenchMarkEntity::SetFuncSettings(settings_function func)
        {
            
        }

    // inline void SetArgProduct(ArgVector a, ArgVector b)
    // {
    //     for (s64 i = 0; i < a.size(); ++i)
    //     {
    //         if (a.args[i] == -1)
    //             break;
    //         for (s64 j = 0; j < b.size(); ++j)
    //         {
    //             if (b.args[i] == -1)
    //                 break;
    //             settings->add_arg({a.args[i], b.args[j]});
    //         }
    //     }
    // }

}