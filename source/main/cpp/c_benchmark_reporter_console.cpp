#include "cbenchmark/private/c_benchmark_reporter_console.h"
#include "cbenchmark/private/c_stdout.h"

namespace BenchMark
{
    bool ConsoleReporter::ReportContext(const Context& context)
    {
        /* TO BE IMPLEMENTED */
        name_field_width_ = context.name_field_width;
        printed_header_ = false;
        prev_counters_.Clear();

        PrintBasicContext(*error_stream_, context);
        return false;
    }
    void ConsoleReporter::ReportRuns(Array<BenchMarkRun*> const& reports)
    {
        /* TO BE IMPLEMENTED */
        return;
    }

    void ConsoleReporter::PrintRunData(const BenchMarkRun& report)
    {
        /* TO BE IMPLEMENTED */
        return;
    }
    void ConsoleReporter::PrintHeader(const BenchMarkRun& report)
    {
        /* TO BE IMPLEMENTED */
        return;
    }

} // namespace BenchMark
