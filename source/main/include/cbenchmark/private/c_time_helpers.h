#ifndef __CBENCHMARK_TIMEHELPERS_H__
#define __CBENCHMARK_TIMEHELPERS_H__

#include "cbenchmark/private/c_types.h"

namespace BenchMark
{
    typedef u64 time_t;

    void   g_InitTimer();
    time_t g_TimeStart();
    double g_GetElapsedTimeInMs(time_t start);
    
} // namespace BenchMark

#endif ///< __CBENCHMARK_TIMEHELPERS_H__
