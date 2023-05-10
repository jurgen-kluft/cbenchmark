#include "cbenchmark/private/c_benchmark_reporter_console.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_stdout.h"

namespace BenchMark
{

    bool ConsoleReporter::ReportContext(const Context& context)
    {
        /* TO BE IMPLEMENTED */
        name_field_width_ = context.name_field_width;
        prev_counters_.ClearReserve(16);
        printed_header_ = false;

        // PrintBasicContext(*error_stream_, context);
        return false;
    }

    void ConsoleReporter::ReportRuns(Array<BenchMarkRun*> const& reports)
    {
        for (s32 i = 0; i < reports.Size(); ++i)
        {
            BenchMarkRun* run          = reports[i];
            bool          print_header = !printed_header_;
            print_header               = print_header || ((output_options_ & OO_Tabular) && (!Counters::SameNames(run->counters, prev_counters_)));
            if (print_header)
            {
                printed_header_ = true;
                prev_counters_.Copy(run->counters);
                PrintHeader(*run);
            }
        }
    }

    void ConsoleReporter::ReportRunsConfig(double min_time, bool has_explicit_iters, IterationCount iters)
    {

    }


    void ConsoleReporter::PrintRunData(const BenchMarkRun& result)
    {
        char nameWidthFormat[8];
        gSetWidthFormat(nameWidthFormat, static_cast<int>(name_field_width_));

        char* str = &line2_[0];
        str       = result.run_name.FullName(str, &line2_[639]);
        str[0]    = '\0';

        auto name_color = (result.report_big_o || result.report_rms) ? COLOR_BLUE : COLOR_GREEN;
        // name_color
        char* line = gStringFormatAppend(&line1_[0], &line1_[639], nameWidthFormat, str);

        // TODO print 'skipped with error' or 'skipped with message'

        const double real_time = result.GetAdjustedRealTime();
        const double cpu_time  = result.GetAdjustedCPUTime();

        if (result.report_big_o)
        {
            const char* bigo = result.complexity.ToString();
            // COLOR_YELLOW
            line = gStringFormatAppend(line, &line1_[639], "%10.2f ", real_time);
            line = gStringFormatAppend(line, &line1_[639], "%-4s ", bigo);
            line = gStringFormatAppend(line, &line1_[639], "%10.2f ", cpu_time);
            line = gStringFormatAppend(line, &line1_[639], "%-4s ", bigo);
        }
        else if (result.report_rms)
        {
            // COLOR_YELLOW
            // printer(Out, , "%10.0f %-4s %10.0f %-4s ", real_time * 100, "%", cpu_time * 100, "%");
            line = gStringFormatAppend(line, &line1_[639], "%10.0f ", real_time * 100);
            line = gStringFormatAppend(line, &line1_[639], "%-4s ", "%");
            line = gStringFormatAppend(line, &line1_[639], "%10.0f ", cpu_time * 100);
            line = gStringFormatAppend(line, &line1_[639], "%-4s ", "%");
        }
        else if (result.run_type != BenchMarkRun::RT_Aggregate || result.aggregate_unit.unit == StatisticUnit::Time)
        {
            const char* timeLabel = result.time_unit.ToString();
            // printer(Out, COLOR_YELLOW, "%s %-4s %s %-4s ", real_time_str.c_str(), timeLabel, cpu_time_str.c_str(), timeLabel);
            line = gFormatTime(real_time, line, &line1_[639]);
            line = gStringFormatAppend(line, &line1_[639], "%s", " ");
            line = gStringFormatAppend(line, &line1_[639], "%-4s ", timeLabel);
            line = gFormatTime(cpu_time, line, &line1_[639]);
            line = gStringFormatAppend(line, &line1_[639], "%s", " ");
            line = gStringFormatAppend(line, &line1_[639], "%-4s ", timeLabel);
        }
        else
        {
            // ASSERT(result.aggregate_unit.unit == StatisticUnit::Percentage);
            // printer(Out, COLOR_YELLOW, "%10.2f %-4s %10.2f %-4s ", (100. * result.real_accumulated_time), "%", (100. * result.cpu_accumulated_time), "%");
            line = gStringFormatAppend(line, &line1_[639], "%10.2f ", (100. * result.real_accumulated_time));
            line = gStringFormatAppend(line, &line1_[639], "%-4s ", "%");
            line = gStringFormatAppend(line, &line1_[639], "%10.2f ", (100. * result.cpu_accumulated_time));
            line = gStringFormatAppend(line, &line1_[639], "%-4s ", "%");
        }

        if (!result.report_big_o && !result.report_rms)
        {
            // printer(Out, COLOR_CYAN, "%10lld", result.iterations);
            line = gStringFormatAppend(line, &line1_[639], "%10lld", result.iterations);
        }

        for (s32 i = 0; i < result.counters.Size(); ++i)
        {
            const Counter& c = result.counters.counters[i];

            const char* cname = c.name;
            const char* unit  = "";
            
            char* str = &line2_[0];
            if (result.run_type == BenchMarkRun::RT_Aggregate && result.aggregate_unit.unit == StatisticUnit::Percentage)
            {
                str = gStringFormatAppend(str, &line2_[639], "%.2f", 100. * c.value);
                unit = "%";
            }
            else
            {
                str = gHumanReadableNumber(str, &line2_[639], c.value, c.flags.OneK());
                if (c.flags.flags & CounterFlags::IsRate)
                    unit = (c.flags.flags & CounterFlags::Invert) ? "s" : "/s";
            }

            if (output_options_ & OO_Tabular)
            {
                //printer(Out, COLOR_DEFAULT, " %*s%s", gStringLength(cname) - gStringLength(unit), str, unit);
                gSetWidthFormat(nameWidthFormat, gStringLength(cname) - gStringLength(unit));
                line = gStringAppend(line, &line1_[639], " ");
                line = gStringFormatAppend(line, &line1_[639], nameWidthFormat, str);
                line = gStringAppend(line, &line1_[639], unit);
            }
            else
            {
                //printer(Out, COLOR_DEFAULT, " %s=%s%s", cname, str, unit);
                line = gStringAppend(line, &line1_[639], " ");
                line = gStringAppend(line, &line1_[639], cname);
                line = gStringAppend(line, &line1_[639], "=");
                line = gStringAppend(line, &line1_[639], str);
                line = gStringAppend(line, &line1_[639], unit);
            }
        }

        *line = '\0';

        ((*output_stream_) << line1_).endl();
        return;
    }

    void ConsoleReporter::PrintHeader(const BenchMarkRun& report)
    {
        char nameWidthFormat[8];
        gSetWidthFormat(nameWidthFormat, static_cast<int>(name_field_width_));

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
        *str = '\0';

        char* str2  = &line2_[0];
        int   width = (int)(str - &line2_[0]);
        for (int i = 0; i < width; ++i)
            str2[i] = '-';
        *str2 = '\0';

        ((((*output_stream_) << line2_).endl() << line1_).endl() << line2_).endl();
    }

    void         ConsoleReporter::Flush()
    {

    }

    void ConsoleReporter::Finalize()
    {

    }


} // namespace BenchMark
