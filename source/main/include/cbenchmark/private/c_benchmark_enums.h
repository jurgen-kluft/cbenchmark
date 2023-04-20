#ifndef __CBENCHMARK_BENCHMARK_ENUMS_H__
#define __CBENCHMARK_BENCHMARK_ENUMS_H__

#include "cbenchmark/private/c_types.h"

namespace BenchMark
{
    typedef s64 IterationCount;

    // Counter flags
    struct CounterFlags
    {
        CounterFlags(u32 flags = Defaults)
            : flags(flags)
        {
        }

        enum
        {
            IsRate                   = 1 << 16,                       // Mark the counter as a rate. It will be presented divided by the duration of the benchmark.
            AvgThreads               = 1 << 17,                       // Mark the counter as a thread-average quantity. It will be presented divided by the number of threads.
            AvgThreadsRate           = IsRate | AvgThreads,           // Mark the counter as a thread-average rate. See above.
            IsIterationInvariant     = 1 << 18,                       // Mark the counter as a constant value, valid/same for *every* iteration (will be *multiplied* by the iteration count)
            IsIterationInvariantRate = IsRate | IsIterationInvariant, // Mark the counter as a constant rate, when reporting, it will be *multiplied* by the iteration count and then divided by the duration of the benchmark.
            AvgIterations            = 1 << 19,                       // Mark the counter as a iteration-average quantity, it will be presented divided by the number of iterations.
            AvgIterationsRate        = IsRate | AvgIterations,        // Mark the counter as a iteration-average rate. See above.
            Invert                   = 1 << 31,                       // In the end, invert the result. This is always done last!
            Is1000                   = 1000,                          // 1'000 items per 1k
            Is1024                   = 1024,                          // 1'024 items per 1k
            Defaults                 = Is1000,
        };

        inline bool operator==(const CounterFlags& f) const { return f.flags == flags; }
        inline bool operator!=(const CounterFlags& f) const { return f.flags != flags; }

        u32 flags;
    };

    // TimeUnit is passed to a benchmark in order to specify the order of magnitude for the measured time.
    struct TimeUnit
    {
        TimeUnit(u32 flags = Millisecond)
            : flags(flags)
        {
        }

        enum
        {
            Nanosecond  = 9,
            Microsecond = 6,
            Millisecond = 3,
            Second      = 0,
        };

        inline const char* ToString() const
        {
            switch (flags)
            {
                case TimeUnit::Millisecond: return "ms";
                case TimeUnit::Microsecond: return "us";
                case TimeUnit::Nanosecond: return "ns";
            }
            return "s";
        }

        inline double GetTimeUnitMultiplier() const
        {
            switch (flags)
            {
                case TimeUnit::Millisecond: return 1e3;
                case TimeUnit::Microsecond: return 1e6;
                case TimeUnit::Nanosecond: return 1e9;
            }
            return 1.0;
        }

        u32 flags;
    };

    class BigO
    {
    public:
        enum EEnum
        {
            O_1,
            O_N,
            O_Log_N,
            O_N_Squared,
            O_N_Cubed,
            O_N_Log_N,
            O_Exponential,
            O_Lambda,
            O_Auto,
            O_None
        };

        BigO(u32 bo = O_None)
            : bigo(bo)
        {
        }

        inline const char* ToString() const
        {
            switch (flags)
            {
                case BigO::O_1: return "O(1)";
                case BigO::O_N: return "O(N)";
                case BigO::O_Log_N: return "O(logN)";
                case BigO::O_N_Squared: return "O(N^2)";
                case BigO::O_N_Cubed: return "O(N^3)";
                case BigO::O_N_Log_N: return "O(NlogN)";
                case BigO::O_Exponential: return "O(2^N)";
            }
            return "O(?)";
        }

        inline bool Is(EEnum e) const { return bigo == e; }

        typedef double(Func)(IterationCount);

        u32 bigo;
    };

    struct StatisticUnit
    {
        enum
        {
            Time       = 1,
            Percentage = 100,
        };

        u32 unit;
    };

    struct Skipped
    {
        enum
        {
            NotSkipped         = 0,
            SkippedWithMessage = 1,
            SkippedWithError   = -1,
        };

        bool Is(u32 s) const { return skipped == s; }
        bool IsNot(u32 s) const { return skipped != s; }

        bool IsSkipped() const { return skipped != 0; }
        bool IsNotSkipped() const { return skipped == 0; }

        s32 skipped;
    };

    struct BenchTimeType
    {
        enum
        {
            ITERS,
            TIME
        };

        BenchTimeType(u32 type = ITERS, IterationCount iters = 0)
            : type(type)
            , iters(iters)
        {
        }
        BenchTimeType(double time)
            : type(TIME)
            , time(time)
        {
        }

        union
        {
            IterationCount iters;
            double         time;
        };

        u32 type;
    };
} // namespace BenchMark

#endif //__CBENCHMARK_BENCHMARK_ENUMS_H__
