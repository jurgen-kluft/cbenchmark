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
    class ScratchAllocator;
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
    BenchMarkRunner* CreateRunner(Allocator* a);
    void             InitRunner(BenchMarkRunner* r, Allocator* a, ScratchAllocator* t, BenchMarkGlobals* globals, const BenchMarkInstance* b_);
    void             InitRunResults(BenchMarkRunner* r, BenchMarkGlobals* globals, RunResults& results);
    void             DestroyRunner(BenchMarkRunner*& r, Allocator* a);
    int              GetNumRepeats(const BenchMarkRunner* r);
    bool             HasRepeatsRemaining(const BenchMarkRunner* r);
    void             DoOneRepetition(BenchMarkRunner* r, BenchMarkRun* report, BenchMarkReporter::PerFamilyRunReports* reports_for_family);
    void             AggregateResults(BenchMarkRunner* r, Allocator* alloc, const Array<BenchMarkRun*>& non_aggregates, Array<BenchMarkRun*>& aggregates_only);
    double           GetMinTime(const BenchMarkRunner* r);
    bool             HasExplicitIters(const BenchMarkRunner* r);
    IterationCount   GetIters(const BenchMarkRunner* r);
    void             StartStopBarrier(ThreadManager* tm);
    void             ThreadTimerStart(ThreadTimer* timer);
    void             ThreadTimerStop(ThreadTimer* timer);
    bool             ThreadTimerIsRunning(ThreadTimer* timer);
    void             ThreadTimerSetIterationTime(ThreadTimer* timer, double seconds);

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_RUNNER_H__