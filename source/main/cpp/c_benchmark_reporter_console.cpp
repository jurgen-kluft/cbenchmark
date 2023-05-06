#include "cbenchmark/private/c_benchmark_reporter_console.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_stdout.h"

namespace BenchMark
{
    bool ConsoleReporter::ReportContext(const Context& context)
    {
        /* TO BE IMPLEMENTED */
        name_field_width_ = context.name_field_width;
        printed_header_ = false;
        prev_counters_.Clear();

        //PrintBasicContext(*error_stream_, context);
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

    static void SetWidthFormat(char* format, int width)
    {
        int i = 0;
        format[i++] = '%';
        if (width > 99)
            format[i++] = '0' + (width / 100);
        if (width > 9)
           format[i++] = '0' + ((width % 100) / 10);
        if (width > 0)
            format[i++] = '0' + (width % 10);
        format[i++] = 's';
        format[i++] = '\0';
    }

    void ConsoleReporter::PrintHeader(const BenchMarkRun& report)
    {
        char nameWidthFormat[8];
        SetWidthFormat(nameWidthFormat, static_cast<int>(name_field_width_));

        line1_[0] = '\0';
        line2_[0] = '\0';

        char* str = &line1_[0];
        str       = gStringFormatAppend(str, &line1_[639], nameWidthFormat, "Benchmark");
        str       = gStringFormatAppend(str, &line1_[639], "%13s", "Time");
        str       = gStringFormatAppend(str, &line1_[639], "%15s", "CPU");
        str       = gStringFormatAppend(str, &line1_[639], "%12s", "Iterations");

        if (!report.counters.counters.Empty())
        {
            if (output_options_ & OO_Tabular)
            {
                for (int i = 0; i < report.counters.counters.Size(); ++i)
                {
                    str = gStringFormatAppend(str, &line1_[639], " %10s", report.counters.counters[i].name);
                }
            }
            else
            {
                str = gStringFormatAppend(str, &line1_[639], " %s", " UserCounters...");
            }
        }

        char* str2 = &line2_[0];
        int width = str - &line2_[0];
        for (int i = 0; i < width; ++i)
            str2[i] = '-';

        (*output_stream_) << line2_ << "\n" << line1_ << "\n" << line2_ << "\n";
    }

} // namespace BenchMark
