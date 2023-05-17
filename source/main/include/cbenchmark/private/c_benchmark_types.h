#ifndef __CBENCHMARK_BENCHMARK_TYPES_H__
#define __CBENCHMARK_BENCHMARK_TYPES_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_array.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_allocators.h"
#include "cbenchmark/private/c_utils.h"

namespace BenchMark
{
    typedef s64 IterationCount;

    struct Counter
    {
        const char*  name;
        CounterFlags flags;
        double       value;
    };
    
    struct Counters
    {
        Array<Counter> counters;

        Counters() {}
        Counters(Counters const& other) = delete;
        
        inline void Initialize(Allocator* allocator, s32 reserve) { counters.Init(allocator, 0, reserve); }
        inline s32  Size() const { return counters.Size(); }
        inline void Clear() { counters.Clear(); }
        inline void ClearReserve(s32 reserve) { counters.ClearReserve(reserve); }
        inline void Copy(Allocator* alloc, Counters const& other) 
        {
            counters.Init(alloc, 0, other.counters.Size());
            for (s32 i = 0; i < other.counters.Size(); ++i)
                counters.PushBack(other.counters[i]);
        }

        inline void Release() { counters.Release(); }

        static bool SameNames(Counters const& left, Counters const& right)
        {
            if (&left == &right)
                return true;
            if (left.counters.Size() != right.counters.Size())
                return false;
            for (s32 i = 0; i < left.counters.Size(); ++i)
            {
                const Counter& lc = left.counters[i];
                if (FindByName(right, lc.name) < 0)
                    return false;
            }
            return true;
        }

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

        static s32 FindByName(Counters const& c, const char* name)
        {
            for (s32 i = 0; i < c.counters.Size(); ++i)
            {
                if (gCompareStrings(c.counters[i].name, name) == 0)
                {
                    return i;
                }
            }
            return -1;
        }

        static void Increment(Counters& l, Counters const& r)
        {
            // add counters present in both or just in l
            for (s32 i = 0; i < l.counters.Size(); ++i)
            {
                Counter& lc = l.counters[i];

                const s32 it = FindByName(r, lc.name);
                if (it >= 0)
                {
                    lc.value += r.counters[it].value;
                }
            }
            // add counters present in r, but not in l
            for (s32 i = 0; i < r.counters.Size(); ++i)
            {
                Counter const& rc = r.counters[i];
                const s32      it = FindByName(l, rc.name);
                if (it < 0)
                {
                    l.counters.PushBack(rc);
                }
            }
        }
    };

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_TYPES_H__
