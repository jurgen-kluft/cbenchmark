#ifndef __CBENCHMARK_BENCHMARK_RUNNER_H__
#define __CBENCHMARK_BENCHMARK_RUNNER_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_alloc.h"

namespace BenchMark
{
    class Allocator;
    class BenchMarkRun;
    class BenchMarkInstance;
    class PerfCountersMeasurement;
    class ThreadManager;
    class ThreadTimer;

    struct RunResults
    {
        Array<BenchMarkRun*> non_aggregates;
        Array<BenchMarkRun*> aggregates_only;
        bool                 display_report_aggregates_only = false;
        bool                 file_report_aggregates_only    = false;
    };

    class BenchMarkRunner;
    void                                    CreateRunner(BenchMarkRunner*& r, Allocator* a, const BenchMarkInstance* b_, PerfCountersMeasurement* pcm_, BenchMarkReporter::PerFamilyRunReports* reports_for_family_);
    void                                    DestroyRunner(BenchMarkRunner*& r);
    int                                     GetNumRepeats(const BenchMarkRunner* r);
    bool                                    HasRepeatsRemaining(const BenchMarkRunner* r);
    void                                    DoOneRepetition(BenchMarkRunner* r);
    RunResults*                             GetResults(BenchMarkRunner* r);
    BenchMarkReporter::PerFamilyRunReports* GetReportsForFamily(const BenchMarkRunner* r);
    double                                  GetMinTime(const BenchMarkRunner* r);
    bool                                    HasExplicitIters(const BenchMarkRunner* r);
    IterationCount                          GetIters(const BenchMarkRunner* r);
    void                                    StartStopBarrier(ThreadManager* tm);
    void                                    TimerStart(ThreadTimer* timer);
    void                                    TimerStop(ThreadTimer* timer);
    bool                                    TimerIsRunning(ThreadTimer* timer);
    void                                    TimerSetIterationTime(ThreadTimer* timer, double seconds);

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_RUNNER_H__