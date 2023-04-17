#ifndef __CBENCHMARK_METRICS_H__
#define __CBENCHMARK_METRICS_H__

#include "cbenchmark/private/c_types.h"

namespace BenchMark
{
    struct BenchMarkMetrics
    {
        s64 m_min_time;
        s64 m_max_time;
        s64 m_total_time;
        s64 m_total_operations;
        s64 m_total_items;
        s64 m_total_bytes;
        s64 m_iterstamp;
        s64 m_timestamp;
    };

    void StartCollecting(BenchMarkMetrics& metrics);
    void StopCollecting(BenchMarkMetrics& metrics);

    void PauseCollecting(BenchMarkMetrics& metrics);
    void ResumeCollecting(BenchMarkMetrics& metrics);

    void Reset(BenchMarkMetrics& metrics);
    void Merge(BenchMarkMetrics& metrics, BenchMarkMetrics const& other);

} // namespace BenchMark

#endif // __CBENCHMARK_METRICS_H__
