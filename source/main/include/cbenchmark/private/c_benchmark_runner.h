#ifndef __CBENCHMARK_BENCHMARK_RUNNER_H__
#define __CBENCHMARK_BENCHMARK_RUNNER_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"
#include "cbenchmark/private/c_benchmark_reporter.h"

namespace BenchMark
{
    class Allocator;
    class BenchMarkRun;
    class BenchMarkInstance;
    class PerfCountersMeasurement;

    struct RunResults
    {
        s32           num_non_aggregates = 0;
        s32           max_non_aggregates = 0;
        BenchMarkRun* non_aggregates;
        s32           num_aggregates = 0;
        s32           max_aggregates = 0;
        BenchMarkRun* aggregates_only;
        bool          display_report_aggregates_only = false;
        bool          file_report_aggregates_only    = false;
    };

    class BenchMarkRunner;
    void                                    CreateRunner(BenchMarkRunner*& r, Allocator* a);
    void                                    DestroyRunner(BenchMarkRunner*& r);
    int                                     GetNumRepeats(BenchMarkRunner* r);
    bool                                    HasRepeatsRemaining(BenchMarkRunner* r);
    void                                    DoOneRepetition(BenchMarkRunner* r);
    RunResults&&                            GetResults(BenchMarkRunner* r);
    BenchMarkReporter::PerFamilyRunReports* GetReportsForFamily(BenchMarkRunner* r);
    double                                  GetMinTime(BenchMarkRunner* r);
    bool                                    HasExplicitIters(BenchMarkRunner* r);
    IterationCount                          GetIters(BenchMarkRunner* r);

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_RUNNER_H__