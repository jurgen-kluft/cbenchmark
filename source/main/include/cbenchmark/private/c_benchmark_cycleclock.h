#ifndef __CBENCHMARK_CYCLECLOCK_H__
#define __CBENCHMARK_CYCLECLOCK_H__

#include "cbenchmark/private/c_types.h"

namespace BenchMark
{
    namespace CycleClock
    {
        // This should return the number of cycles since power-on.  Thread-safe.
        s64 Now();
    }
}
#endif // __CBENCHMARK_CYCLECLOCK_H__
