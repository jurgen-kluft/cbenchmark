#ifndef __CBENCHMARK_BENCHMARK_UNIT_H__
#define __CBENCHMARK_BENCHMARK_UNIT_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    class BenchMarkUnit;
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
    // BenchMarkUnit registration object. The BM_TEST() macro expands
    // into an BenchMarkUnit* object. Various properties can be set on
    // this object to change the execution of the benchmark.

    typedef void (*setup_function)(const BenchMarkState&);
    typedef void (*teardown_function)(const BenchMarkState&);
    typedef void (*run_function)(BenchMarkState&, Allocator* alloc);
    typedef void (*settings_function)(Allocator* alloc, BenchMarkUnit* settings);

    struct Args
    {
        inline Args(s32 const* _args, s32 _argc)
            : args(_args)
            , argc(_argc)
        {
        }
        s32 const* args;
        s32        argc;
    };

    class BenchMarkUnit
    {
    public:
        Allocator*            allocator;                //
        Array<Args>           final_args_;              // {...}[]
        TimeUnit              time_unit_;               // time unit to use for output
        TimeSettings          time_settings_;           //
        AggregationReportMode aggregation_report_mode_; //
        bool                  count_only_;              // Settings are applied in two passes
        s32                   arg_names_count_;         //
        Array<const char*>    arg_names_;               // Arg names (default = x,y,z,w)
        s32                   args_count_;              //
        Array<Args>           args_;                    // {x,y,z,w}[]
        s32                   arg_count_;               //
        Array<Args>           arg_;                     // x[], y[], z[], w[]
        s32                   arg_ranges_count_;        //
        Array<ArgRange>       arg_ranges_;              // x:{lo,hi,multi} * y:{lo,hi,multi} * z:{lo,hi,multi} * w:{lo,hi,multi} permutations
        int                   thread_counts_size_;
        Array<s32>            thread_counts_;
        int                   range_multiplier_;
        int                   repetitions_;
        double                min_time_;
        double                min_warmup_time_;
        IterationCount        iterations_;
        s32                   counters_size_;
        Counters              counters_;
        BigO                  complexity_;
        BigO::Func*           complexity_lambda_;
        s32                   statistics_count_;
        Array<Statistic>      statistics_;
        setup_function        setup_;
        teardown_function     teardown_;
        settings_function     settings_;
        run_function          run_;
        // -----------------------------------------------------------------
        BenchMarkUnit* next;       // the fixture has a singly-linked list of benchmarks
        const char*    name;       // the name of the benchmark
        const char*    filename;   // the source file this benchmark was declared
        int            disabled;   // 0 = enabled, 1 = disabled, should this benchmark be run?
        int            lineNumber; // the line number in the source file

        s32 BuildArgs();

        void PrepareSettings(bool count_only);
        void SetDefaults();
        void SetEnabled(bool enabled);
        bool IsDisabled() const { return disabled != 0; }
        void AddArgs(s32 const* args, s32 argc);
        void AddArg(const s32* args, s32 argc);
        void AddRange(s32 lo, s32 hi, s32 multiplier, s32 mode);
        void AddArgRange(s32 lo, s32 hi, s32 multiplier = 8) { AddRange(lo, hi, multiplier, 1); }
        void AddArgDenseRange(s32 start, s32 limit, s32 step = 32) { AddRange(start, limit, step, 2); }
        void SetArgNames(const char** names, s32 names_size);
        void SetThreadCounts(s32 const* thread_counts, s32 thread_counts_size);
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
        static void CreateRange(s32 lo, s32 hi, s32 multi, Array<s64>& out, Allocator* alloc);
        // Creates a list of integer values for the given range and step.
        static void CreateDenseRange(s32 start, s32 limit, s32 step, Array<s64>& out, Allocator* alloc);

    }; // namespace BenchMark
} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_UNIT_H__