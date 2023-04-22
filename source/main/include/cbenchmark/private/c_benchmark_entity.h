#ifndef __CBENCHMARK_BENCHMARK_DECLARED_H__
#define __CBENCHMARK_BENCHMARK_DECLARED_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    class BenchMarkState;

    struct TimeSettings
    {
        enum EFlag
        {
            USE_DEFAULT_TIME_UNIT    = 1,
            MEASURE_PROCESS_CPU_TIME = 2,
            USE_REAL_TIME            = 4,
            USE_MANUAL_TIME          = 8,
        };

        TimeSettings()
            : flags(USE_DEFAULT_TIME_UNIT | MEASURE_PROCESS_CPU_TIME)
        {
        }

        inline bool UseDefaultTimeUnit() const { return (flags & USE_DEFAULT_TIME_UNIT) != 0; }
        inline bool MeasureProcessCpuTime() const { return (flags & MEASURE_PROCESS_CPU_TIME) != 0; }
        inline bool UseRealTime() const { return (flags & USE_REAL_TIME) != 0; }
        inline bool UseManualTime() const { return (flags & USE_MANUAL_TIME) != 0; }

        inline void Set(EFlag f, bool value) { flags = (flags & ~f) | (value ? f : 0); }
        inline void SetUseDefaultTimeUnit(bool value) { Set(USE_DEFAULT_TIME_UNIT, value); }
        inline void SetMeasureProcessCpuTime(bool value) { Set(MEASURE_PROCESS_CPU_TIME, value); }
        inline void SetUseRealTime(bool value) { Set(USE_REAL_TIME, value); }
        inline void SetUseManualTime(bool value) { Set(USE_MANUAL_TIME, value); }

        u32 flags;
    };

    // -----------------------------------------------------------------
    // BenchMarkEntity registration object. The BM_TEST() macro expands
    // into an BenchMarkEntity* object. Various properties can be set on
    // this object to change the execution of the benchmark.

    typedef void (*setup_function)(const BenchMarkState&);
    typedef void (*teardown_function)(const BenchMarkState&);
    typedef void (*run_function)(BenchMarkState&);
    typedef void (*settings_function)(Allocator* alloc, BenchMarkEntity* settings);

    class BenchMarkEntity
    {
    public:
        enum ESettings
        {
            MaxPermutations = 16,
        };

        bool                  enabled_;
        char*                 name_;
        TimeUnit              time_unit_;
        TimeSettings          time_settings_;
        AggregationReportMode aggregation_report_mode_;
        char*                 arg_names_[MaxPermutations]; // Args for all benchmark runs
        Arg                   args_[MaxPermutations];      // Args for all benchmark runs
        int                   range_multiplier_;
        int                   repetitions_;
        double                min_time_;
        double                min_warmup_time_;
        IterationCount        iterations_;
        BigO                  complexity_;
        BigO::Func*           complexity_lambda_;
        Statistics            statistics_;
        int                   thread_counts_[MaxPermutations];
        setup_function        setup_;
        teardown_function     teardown_;
        run_function          run_;
        settings_function     settings_;

        void SetEnabled(bool enabled);
        void AddArg(ArgVector a);
        void AddArgName(const char* name = nullptr);
        void AddNamedArg(const char* aname, ArgVector a);
        void SetArgRange(ArgVector a, ArgVector b, ArgVector c = ArgVector::empty, ArgVector d = ArgVector::empty);
        void SetNamedArgRange(const char* aname, ArgVector a, const char* bname, ArgVector b, const char* cname = nullptr, ArgVector c = ArgVector::empty, const char* dname = nullptr, ArgVector d = ArgVector::empty);
        void SetArgProduct(ArgVector a, ArgVector b);
        void AddCounter(const char* name, CounterFlags flags, double value = 0.0);
        void SetTimeUnit(TimeUnit tu);
        void SetMinTime(double min_time);
        void SetMinWarmupTime(double min_warmup_time);
        void SetIterations(IterationCount iters);
        void SetRepetitions(int repetitions);
        void SetFuncRun(run_function func);
        void SetFuncSettings(settings_function func);

    }; // namespace BenchMark
} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_DECLARED_H__