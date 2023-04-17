#ifndef __CBENCHMARK_TYPES_H__
#define __CBENCHMARK_TYPES_H__

namespace BenchMark
{
#ifdef TARGET_PC

    typedef int s32;
    typedef unsigned int u32;

    typedef unsigned __int64 u64;
    typedef __int64 s64;

#elif defined(TARGET_MAC)

    typedef int s32;
    typedef unsigned int u32;

    typedef unsigned long  u64;
    typedef long  s64;

#endif

} // namespace BenchMark

#endif ///< __CBENCHMARK_TYPES_H__
