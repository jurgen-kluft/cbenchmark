#ifndef __CBENCHMARK_TIMEHELPERS_H__
#define __CBENCHMARK_TIMEHELPERS_H__

#include "cbenchmark/private/c_types.h"

namespace BenchMark
{
    typedef u64 time_t;

    void   InitTimer();
    time_t TimeStamp();
    double GetElapsedTimeInMs(time_t start);
    
} // namespace BenchMark

#endif ///< __CBENCHMARK_TIMEHELPERS_H__
