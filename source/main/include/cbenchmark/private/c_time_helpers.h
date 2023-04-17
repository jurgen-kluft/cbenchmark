#ifndef __CBENCHMARK_TIMEHELPERS_H__
#define __CBENCHMARK_TIMEHELPERS_H__

namespace BenchMark
{
    struct time_t
    {
        unsigned int m_time[2];
    };

    void   g_InitTimer();
    time_t g_TimeStart();
    double g_GetElapsedTimeInMs(time_t start);
    void   g_SleepMs(int ms);

} // namespace BenchMark

#endif ///< __CBENCHMARK_TIMEHELPERS_H__
