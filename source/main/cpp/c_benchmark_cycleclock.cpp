// ----------------------------------------------------------------------
// CycleClock
//    A CycleClock tells you the current time in Cycles.  The "time"
//    is actually time since power-on.  This is like time() but doesn't
//    involve a system call and is much more precise.
//
// NOTE: Not all cpu/platform/kernel combinations guarantee that this
// clock increments at a constant rate or is synchronized across all logical
// cpus in a system.
//
// If you need the above guarantees, please consider using a different
// API. There are efforts to provide an interface which provides a millisecond
// granularity and implemented as a memory read. A memory read is generally
// cheaper than the CycleClock for many architectures.
//
// Also, in some out of order CPU implementations, the CycleClock is not
// serializing. So if you're trying to count at cycles granularity, your
// data might be inaccurate due to out of order instruction execution.
// ----------------------------------------------------------------------

#ifndef __CBENCHMARK_CYCLECLOCK_H__
#define __CBENCHMARK_CYCLECLOCK_H__

#include "cbenchmark/private/c_types.h"

#include <cstdint>

#if defined(TARGET_MAC)
#    include <mach/mach_time.h>
#endif

#if defined(__clang__)
#    if defined(__ibmxl__)
#        if !defined(COMPILER_IBMXL)
#            define COMPILER_IBMXL
#        endif
#    elif !defined(COMPILER_CLANG)
#        define COMPILER_CLANG
#    endif
#elif defined(_MSC_VER)
#    if !defined(COMPILER_MSVC)
#        define COMPILER_MSVC
#    endif
#elif defined(__GNUC__)
#    if !defined(COMPILER_GCC)
#        define COMPILER_GCC
#    endif
#endif

#if defined(_WIN32)
#    define BENCHMARK_OS_WINDOWS 1
// WINAPI_FAMILY_PARTITION is defined in winapifamily.h.
// We include windows.h which implicitly includes winapifamily.h for compatibility.
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#    if defined(WINAPI_FAMILY_PARTITION)
#        if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#            define BENCHMARK_OS_WINDOWS_WIN32 1
#        elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#            define BENCHMARK_OS_WINDOWS_RT 1
#        endif
#    endif
#    if defined(__MINGW32__)
#        define BENCHMARK_OS_MINGW 1
#    endif
#elif defined(__APPLE__)
#    define BENCHMARK_OS_APPLE 1
#    include "TargetConditionals.h"
#    if defined(TARGET_OS_MAC)
#        define BENCHMARK_OS_MACOSX 1
#        if defined(TARGET_OS_IPHONE)
#            define BENCHMARK_OS_IOS 1
#        endif
#    endif
#endif

// For MSVC, we want to use '_asm rdtsc' when possible (since it works
// with even ancient MSVC compilers), and when not possible the
// __rdtsc intrinsic, declared in <intrin.h>.  Unfortunately, in some
// environments, <windows.h> and <intrin.h> have conflicting
// declarations of some other intrinsics, breaking compilation.
// Therefore, we simply declare __rdtsc ourselves. See also
// http://connect.microsoft.com/VisualStudio/feedback/details/262047
#if defined(COMPILER_MSVC) && !defined(_M_IX86) && !defined(_M_ARM64) && !defined(_M_ARM64EC)
extern "C" uint64_t __rdtsc();
#    pragma intrinsic(__rdtsc)
#endif

#define BENCHMARK_ALWAYS_INLINE

namespace BenchMark
{
    namespace CycleClock
    {
        void Init()
        {
            // Use standard methods for high precision timers to find the CPU frequency in cycles per second.
            // This is used to convert the cycle count to seconds.

            // Pseudo code:
            //   hpt_start = GetHighPrecisionTimer()
            //       sleep(1)
            //   hpt_end = GetHighPrecisionTimer()
            //   cc_start = Now()
            //       sleep(1)
            //   cc_end = Now()
            //   hpt_end = GetHighPrecisionTimer()
            //   
            //   cpu_frequency = (cc_end - cc_start) / (hpt_end - hpt_start)
        }

        // This should return the number of cycles since power-on.  Thread-safe.
        inline BENCHMARK_ALWAYS_INLINE s64 Now()
        {
#if defined(TARGET_OS_MAC)
            // this goes at the top because we need ALL Macs, regardless of
            // architecture, to return the number of "mach time units" that
            // have passed since startup.  See sysinfo.cc where
            // InitializeSystemInfo() sets the supposed cpu clock frequency of
            // macs to the number of mach time units per second, not actual
            // CPU clock frequency (which can change in the face of CPU
            // frequency scaling).  Also note that when the Mac sleeps, this
            // counter pauses; it does not continue counting, nor does it
            // reset to zero.
            return mach_absolute_time();
#elif defined(__i386__)
            int64_t ret;
            __asm__ volatile("rdtsc" : "=A"(ret));
            return ret;
#elif defined(__x86_64__) || defined(__amd64__)
            uint64_t low, high;
            __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
            return (high << 32) | low;
#elif defined(COMPILER_MSVC) && defined(_M_IX86)
            // Older MSVC compilers (like 7.x) don't seem to support the
            // __rdtsc intrinsic properly, so I prefer to use _asm instead
            // when I know it will work.  Otherwise, I'll use __rdtsc and hope
            // the code is being compiled with a non-ancient compiler.
            _asm rdtsc
#elif defined(COMPILER_MSVC)
            return __rdtsc();
#else
            // The soft failover to a generic implementation is automatic only for ARM.
            // For other platforms the developer is expected to make an attempt to create
            // a fast implementation and use generic version if nothing better is available.
#    error You need to define CycleClock for your OS and CPU

#endif
        }
    } // namespace CycleClock
} // namespace BenchMark

#endif // BENCHMARK_CYCLECLOCK_H_