#ifndef __CBENCHMARK_BENCHMARK_UNIT_H__
#define __CBENCHMARK_BENCHMARK_UNIT_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_array.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    class BenchMarkUnit;
    class BenchMarkState;
    class BenchMarkInstance;

    // -----------------------------------------------------------------
    // BenchMarkUnit registration object. The BM_TEST() macro expands
    // into an BenchMarkUnit* object. Various properties can be set on
    // this object to change the execution of the benchmark.

    typedef void (*setup_function)(const BenchMarkState&);
    typedef void (*teardown_function)(const BenchMarkState&);
    typedef void (*run_function)(BenchMarkState&, Allocator* alloc);
    typedef void (*settings_function)(BenchMarkUnit* settings);

    struct Arg_t
    {
        Arg_t();

        enum EArg
        {
            Arg_Uninitialized = 0x0,
            Arg_Value         = 0x1,
            Arg_Range         = 0x2,
            Arg_DenseRange    = 0x4,
            Arg_Sequence      = 0x8,
        };

        template <typename... Args> void Sequence(Args&&... _args)
        {
            const s64 argv[] = {_args...};
            SetSequence(argv, sizeof...(Args));
        }
        void SetSequence(s64 const* argv, s32 argc);

        void SetName(char const* name);
        void AddValue(s64 value);
        void Range(s32 lo, s32 hi, s32 multiplier = 8);
        void DenseRange(s32 start, s32 limit, s32 step = 32);

        char const* name_;
        s8          count_only_;
        s8          mode_;
        s32         count_; //
        Array<s64>  args_;  //
    };

    class BenchMarkUnit
    {
    public:
        enum ESettings
        {
            Max_Args = 8,
        };

        TimeUnit              time_unit_;               // time unit to use for output
        TimeSettings          time_settings_;           //
        AggregationReportMode aggregation_report_mode_; //
        bool                  count_only_;              // Settings are applied in two passes
        int                   args_count_;
        Arg_t                 args_[Max_Args];
        int                   thread_counts_size_;
        Array<s32>            thread_counts_;
        int                   range_multiplier_;
        int                   repetitions_;
        double                min_time_;
        double                min_warmup_time_;
        s64                   memory_required_;
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

        s32    BuildArgs(Allocator* alloc, Array<Array<s32>*>& args);
        Arg_t* Arg(s32 index);
        Arg_t* Arg(s32 index, const char* name);

        template <typename... Args> void Args(Args&&... _args)
        {
            const s64 argv[] = {_args...};
            for (s32 i = 0; i < sizeof...(Args); ++i)
                Arg(i)->AddValue(argv[i]);
        }

        void PrepareSettings();
        void ApplySettings(Allocator* allocator);
        void ReleaseSettings();

        void SetEnabled(bool enabled);
        bool IsDisabled() const { return disabled != 0; }

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
        void SetMemoryRequired(s64 required);
        void SetIterations(IterationCount iters);
        void SetRepetitions(int repetitions);
        void SetFuncRun(run_function func);
        void SetFuncSettings(settings_function func);

    }; // namespace BenchMark
} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_UNIT_H__