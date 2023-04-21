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
        Array<BenchMarkRun> non_aggregates;
        Array<BenchMarkRun> aggregates_only;
        bool                display_report_aggregates_only = false;
        bool                file_report_aggregates_only    = false;
    };

    class BenchMarkRunner;
    void                                    CreateRunner(BenchMarkRunner*& r, Allocator* a, const BenchMarkInstance* b_, PerfCountersMeasurement* pcm_, BenchMarkReporter::PerFamilyRunReports* reports_for_family_);
    void                                    DestroyRunner(BenchMarkRunner*& r);
    int                                     GetNumRepeats(BenchMarkRunner* r);
    bool                                    HasRepeatsRemaining(BenchMarkRunner* r);
    void                                    DoOneRepetition(BenchMarkRunner* r);
    RunResults&&                            GetResults(BenchMarkRunner* r);
    BenchMarkReporter::PerFamilyRunReports* GetReportsForFamily(BenchMarkRunner* r);
    double                                  GetMinTime(BenchMarkRunner* r);
    bool                                    HasExplicitIters(BenchMarkRunner* r);
    IterationCount                          GetIters(BenchMarkRunner* r);
    void                                    StartStopBarrier(ThreadManager* tm);
    void                                    TimerStart(ThreadTimer* timer);
    void                                    TimerStop(ThreadTimer* timer);
    bool                                    TimerIsRunning(ThreadTimer* timer);
    void                                    TimerSetIterationTime(ThreadTimer* timer, double seconds);
    void                                    ThreadManagerSkipWithMessage(ThreadManager* m, const char* msg, Skipped skipped);
    void                                    ThreadManagerSetLabel(ThreadManager* m, const char* format, double value);

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_RUNNER_H__