#ifndef __CBENCHMARK_BENCHMARK_TYPES_H__
#define __CBENCHMARK_BENCHMARK_TYPES_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_alloc.h"

namespace BenchMark
{
    typedef s64 IterationCount;

    struct Arg
    {
        s32 argv[4];
    };

    struct ArgRange
    {
        ArgRange() {}
        ArgRange(s32 _a, s32 _b, s32 _c, s32 _mode)
            : a(_a)
            , b(_b)
            , c(_c)
            , mode(_mode)
        {
        }

        s32             a, b, c;
        s32             mode; // 0 = un-initialized, 1 = multiplier, 2 = step
        static ArgRange empty;
    };

    struct Counter
    {
        u64          id;
        CounterFlags flags;
        double       value;
        Counter*     next;
    };

    struct CounterType
    {
        enum
        {
            BytesProcessed = 0,
            ItemsProcessed = 1,
            User1          = 2,
            User2          = 3,
            User3          = 4,
            User4          = 5,
            User5          = 6,
            User6          = 7,
        };
    };

    struct Counters
    {
        Array<Counter> counters;

        Counters() {}
        Counters(Counters const& other) { counters.Copy(other.counters); }

        static double Finish(Counter const& c, IterationCount iterations, double cpu_time, double num_threads)
        {
            double v = c.value;
            if (c.flags.flags & CounterFlags::IsRate)
            {
                v /= cpu_time;
            }
            if (c.flags.flags & CounterFlags::AvgThreads)
            {
                v /= num_threads;
            }
            if (c.flags.flags & CounterFlags::IsIterationInvariant)
            {
                v *= iterations;
            }
            if (c.flags.flags & CounterFlags::AvgIterations)
            {
                v /= iterations;
            }
            if (c.flags.flags & CounterFlags::Invert)
            { // Invert is *always* last.
                v = 1.0 / v;
            }
            return v;
        }

        static void Finish(Counters& c, IterationCount iterations, double cpu_time, double num_threads)
        {
            for (s32 i = 0; i < c.counters.Size(); ++i)
            {
                c.counters[i].value = Finish(c.counters[i], iterations, cpu_time, num_threads);
            }
        }

        static s32 Find(Counters const& c, u64 id)
        {
            for (s32 i = 0; i < c.counters.Size(); ++i)
            {
                if (c.counters[i].id == id)
                {
                    return i;
                }
            }
            return -1;
        }

        static void Increment(Counters& l, Counters const& r)
        {
            // add counters present in both or just in *l
            for (s32 i = 0; i < l.counters.Size(); ++i)
            {
                Counter& lc = l.counters[i];

                const s32 it = Find(r, lc.id);
                if (it >= 0)
                {
                    lc.value += r.counters[it].value;
                }
            }
            // add counters present in r, but not in *l
            for (s32 i = 0; i < r.counters.Size(); ++i)
            {
                Counter const& rc = r.counters[i];
                const s32      it = Find(l, rc.id);
                if (it < 0)
                {
                    l.counters.PushBack(rc);
                }
            }
        }
    };

    struct MinTime
    {
        double value;
    };

    struct MinWarmupTime
    {
        double value;
    };

    struct Iterations
    {
        IterationCount value;
    };

    struct Repetitions
    {
        s64 value;
    };

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_TYPES_H__
