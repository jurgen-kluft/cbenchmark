#ifndef __CBENCHMARK_TESTREPORTER_H__
#define __CBENCHMARK_TESTREPORTER_H__

#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_allocators.h"
#include "cbenchmark/private/c_stringbuilder.h"

namespace BenchMark
{
    class BenchMarkRun;

    class BenchMarkReporter
    {
    public:
        struct Context
        {
            Context() : name_field_width(0), executable_name(nullptr) {}

            // CPUInfo const&    cpu_info;
            // SystemInfo const& sys_info;

            // The number of chars in the longest benchmark name.
            s32                name_field_width;
            const char*         executable_name;
        };

        struct PerFamilyRunReports
        {
            PerFamilyRunReports()
                : num_runs_total(0)
                , num_runs_done(0)
                , runs()
            {
            }

            int                  num_runs_total; // How many runs will all instances of this benchmark perform?
            int                  num_runs_done;  // How many runs have happened already?
            Array<BenchMarkRun*> runs;           // The reports about (non-errneous!) runs of this family.
        };

        virtual ~BenchMarkReporter() {}

        // Called once for every suite of benchmarks run.
        // The parameter "context" contains information that the
        // reporter may wish to use when generating its report, for example the
        // platform under which the benchmarks are running. The benchmark run is
        // never started if this function returns false, allowing the reporter
        // to skip runs based on the context information.
        virtual bool ReportContext(const Context& context, ForwardAllocator* allocator, ScratchAllocator* scratch) = 0;

        // Called once for each group of benchmark runs, gives information about the configurations of the runs.
        virtual void ReportRunsConfig(double min_time, bool has_explicit_iters, IterationCount iters, ForwardAllocator* allocator, ScratchAllocator* scratch) = 0;

        // Called once for each group of benchmark runs, gives information about
        // cpu-time and heap memory usage during the benchmark run. If the group
        // of runs contained more than two entries then 'report' contains additional
        // elements representing the mean and standard deviation of those runs.
        // Additionally if this group of runs was the last in a family of benchmarks
        // 'reports' contains additional entries representing the asymptotic
        // complexity and RMS of that benchmark family.
        virtual void ReportRuns(Array<BenchMarkRun*> const& reports, ForwardAllocator* allocator, ScratchAllocator* scratch) = 0;

        // Called once and only once after ever group of benchmarks is run and reported.
        virtual void Flush(ForwardAllocator* allocator, ScratchAllocator* scratch) = 0;
        virtual void Finalize(ForwardAllocator* allocator, ScratchAllocator* scratch) = 0;
    };

} // namespace BenchMark

#endif ///<__XUNITTEST_TESTREPORTER_H__
