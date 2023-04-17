#include "cbenchmark/private/c_benchmark_metrics.h"
#include "cbenchmark/private/c_time_helpers.h"

namespace BenchMark
{
    void Reset(BenchMarkMetrics& m)
    {
        m.m_min_time         = 0x7fffffffffffffff;
        m.m_max_time         = 0;
        m.m_total_time       = 0;
        m.m_total_operations = 0;
        m.m_total_items      = 0;
        m.m_total_bytes      = 0;
        m.m_iterstamp        = 0;
        m.m_timestamp        = 0;
    }

    void Merge(BenchMarkMetrics& m, BenchMarkMetrics const& other)
    {
        // Choose best min and max time
        if (other.m_min_time < m.m_min_time)
            m.m_min_time = other.m_min_time;
        if (other.m_max_time > m.m_max_time)
            m.m_max_time = other.m_max_time;

        if (other.m_total_time < m.m_total_time)
        {
            m.m_total_time       = other.m_total_time;
            m.m_total_operations = other.m_total_operations;
            m.m_total_items      = other.m_total_items;
            m.m_total_bytes      = other.m_total_bytes;
        }
    }

    void StartCollecting(BenchMarkMetrics& m)
    {
        m.m_iterstamp = m.m_total_operations;
        m.m_timestamp = TimeStamp();
    }

    void StopCollecting(BenchMarkMetrics& m)
    {
        // Get iterations count & duration of the phase
        const s64 duration   = (s64)(TimeStamp() - m.m_timestamp);
        const s64 iterations = m.m_total_operations - m.m_iterstamp;

        // Get min & max time of the phase
        if (iterations > 0)
        {
            const s64 min_time = (duration / iterations);
            if (min_time < m.m_min_time)
                m.m_min_time = min_time;

            const s64 max_time = (duration / iterations);
            if (max_time > m.m_max_time)
                m.m_max_time = max_time;
        }

        m.m_total_time += duration;
    }
} // namespace BenchMark
