#ifndef __CBENCHMARK_BENCHMARK_ENTITY_H__
#define __CBENCHMARK_BENCHMARK_ENTITY_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

#include <initializer_list>

namespace BenchMark
{
    class BenchMarkState;
    class BenchMarkInstance;

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
            : flags(USE_DEFAULT_TIME_UNIT)
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
    typedef void (*run_function)(BenchMarkState&, Allocator* alloc);
    typedef void (*settings_function)(Allocator* alloc, BenchMarkEntity* settings);

    class BenchMarkEntity
    {
    public:
        Allocator*            allocator;
        bool                  enabled_;
        char*                 name_;
        TimeUnit              time_unit_;
        TimeSettings          time_settings_;
        AggregationReportMode aggregation_report_mode_;
        int                   argc_;
        const char*           arg_names_[4];        // Arg names (default = x,y,z,w)
        Array<Array<s64>>     args_;                // {x,y,z,w}[]
        ArgRange              arg_ranges_[4];       // x:{lo,hi,multi} * y:{lo,hi,multi} * z:{lo,hi,multi} * w:{lo,hi,multi} permutations
        ArgDenseRange         arg_dense_ranges_[4]; // x:{start,limit,step} * y:{start,limit,step} * z:{start,limit,step} * w:{start,limit,step} permutations
        Array<s64>            arg_product_[4];      // x:{...} * y:{...} * z:{...} * w:{...} permutations
        int const*            thread_counts_;
        int                   num_thread_counts_;
        int                   range_multiplier_;
        int                   repetitions_;
        double                min_time_;
        double                min_warmup_time_;
        IterationCount        iterations_;
        Counters              counters_;
        BigO                  complexity_;
        BigO::Func*           complexity_lambda_;
        Statistics            statistics_;
        setup_function        setup_;
        teardown_function     teardown_;
        run_function          run_;
        settings_function     settings_;

        s32  BuildArgs();
        void SetDefaults();
        void SetEnabled(bool enabled);
        void SetDefaultArgNames();
        void AddArg(Arg a);
        void SetNamedArg(const char* name, std::initializer_list<s64> values);
        void SetArgRange(ArgRange a, ArgRange b, ArgRange c = ArgRange::empty);
        void SetNamedArgRange(const char* aname, ArgRange a, const char* bname, ArgRange b = ArgRange::empty, const char* cname = nullptr, ArgRange c = ArgRange::empty);
        void SetArgProduct(std::initializer_list<s64> a, std::initializer_list<s64> b, std::initializer_list<s64> c = std::initializer_list<s64>());
        void SetRangeMultiplier(int multiplier);
        void SetComplexity(BigO complexity);
        void SetComplexity(BigO::Func* complexity_lambda_);
        void AddCounter(const char* name, CounterFlags flags, double value = 0.0);
        void SetTimeUnit(TimeUnit tu);
        void DisplayAggregatesOnly(bool value);
        void ReportAggregatesOnly(bool value);
        void AddStatisticsComputer(Statistic stat);
        void SetMinTime(double min_time);
        void SetMinWarmupTime(double min_warmup_time);
        void SetIterations(IterationCount iters);
        void SetRepetitions(int repetitions);
        void SetFuncRun(run_function func);
        void SetFuncSettings(settings_function func);

        // Creates a list of integer values for the given range and multiplier.
        // This can be used together with ArgsProduct() to allow multiple ranges
        // with different multipliers.
        static void CreateRange(s64 lo, s64 hi, int multi, Array<s64>& out, Allocator* alloc);
        // Creates a list of integer values for the given range and step.
        static void CreateDenseRange(s64 start, s64 limit, int step, Array<s64>& out, Allocator* alloc);

    }; // namespace BenchMark
} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_ENTITY_H__