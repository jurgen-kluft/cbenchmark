#include "cbenchmark/private/c_benchmark_reporter_console.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_stdout.h"

namespace BenchMark
{

    void ConsoleReporter::Initialize(Allocator* allocator, ConsoleOutput* out)
    {
        const unsigned int kOutStreamBufferSize = 1024;
        const unsigned int kErrStreamBufferSize = 256;

        output_stream_.out    = out;
        output_stream_.sos    = (char*)allocator->Allocate(kOutStreamBufferSize, 8);
        output_stream_.eos    = output_stream_.sos + kOutStreamBufferSize - 1;
        output_stream_.stream = output_stream_.sos;

        error_stream_.out    = out;
        error_stream_.sos    = (char*)allocator->Allocate(kErrStreamBufferSize, 8);
        error_stream_.eos    = error_stream_.sos + kErrStreamBufferSize - 1;
        error_stream_.stream = error_stream_.sos;
    }

    void ConsoleReporter::Shutdown(Allocator* allocator)
    {
        output_stream_.flush();
        error_stream_.flush();

        allocator->Deallocate(output_stream_.sos);
        allocator->Deallocate(error_stream_.sos);

        output_stream_.sos    = nullptr;
        output_stream_.eos    = nullptr;
        output_stream_.stream = nullptr;
        output_stream_.out    = nullptr;

        error_stream_.sos    = nullptr;
        error_stream_.eos    = nullptr;
        error_stream_.stream = nullptr;
        error_stream_.out    = nullptr;
    }

    bool ConsoleReporter::ReportContext(const Context& context, ForwardAllocator* allocator, ScratchAllocator* scratch)
    {
        /* TO BE IMPLEMENTED */
        name_field_width_ = context.name_field_width;
        prev_counters_.ClearReserve(16);
        printed_header_ = false;

        // PrintBasicContext(*error_stream_, context);
        return true;
    }

    void ConsoleReporter::ReportRuns(Array<BenchMarkRun*> const& reports, ForwardAllocator* allocator, ScratchAllocator* scratch)
    {
        for (s32 i = 0; i < reports.Size(); ++i)
        {
            BenchMarkRun* run          = reports[i];
            bool          print_header = !printed_header_;
            print_header               = print_header || ((output_options_ & OO_Tabular) && (!Counters::SameNames(run->counters, prev_counters_)));
            if (print_header)
            {
                printed_header_ = true;
                prev_counters_.Copy(allocator, run->counters);
                PrintHeader(*run, allocator, scratch);
            }

            PrintRunData(*run, scratch);
        }
    }

    void ConsoleReporter::ReportRunsConfig(double min_time, bool has_explicit_iters, IterationCount iters, ForwardAllocator* allocator, ScratchAllocator* scratch) {}

    void ConsoleReporter::PrintRunData(const BenchMarkRun& result, ScratchAllocator* scratch)
    {
        USE_SCRATCH(scratch);

        char* nameWidthFormat = scratch->Alloc<char>(8 + 1);
        gSetWidthFormat(nameWidthFormat, static_cast<int>(name_field_width_));

        const s32   max_line_width = 1024;
        char* const name           = scratch->Alloc<char>(max_line_width + 1);
        name[max_line_width]       = '\0';

        char*             outStr    = name;
        const char* const outStrEnd = &name[max_line_width];
        outStr                      = result.run_name.FullName(outStr, outStrEnd);
        outStr                      = gStringAppendTerminator(outStr, outStrEnd);

        const char* const line       = outStr;
        auto              name_color = (result.report_big_o || result.report_rms) ? COLOR_BLUE : COLOR_GREEN;
        // name_color
        outStr = gStringFormatAppend(outStr, outStrEnd, nameWidthFormat, name);

        // TODO print 'skipped with error' or 'skipped with message'

        const double real_time = result.GetAdjustedRealTime();
        const double cpu_time  = result.GetAdjustedCPUTime();

        if (result.report_big_o)
        {
            const char* bigo = result.complexity.ToString();
            // COLOR_YELLOW
            outStr = gStringFormatAppend(outStr, outStrEnd, "%10.2f ", real_time);
            outStr = gStringFormatAppend(outStr, outStrEnd, "%-4s ", bigo);
            outStr = gStringFormatAppend(outStr, outStrEnd, "%10.2f ", cpu_time);
            outStr = gStringFormatAppend(outStr, outStrEnd, "%-4s ", bigo);
        }
        else if (result.report_rms)
        {
            // COLOR_YELLOW
            // printer(Out, , "%10.0f %-4s %10.0f %-4s ", real_time * 100, "%", cpu_time * 100, "%");
            outStr = gStringFormatAppend(outStr, outStrEnd, "%10.0f ", real_time * 100);
            outStr = gStringFormatAppend(outStr, outStrEnd, "%-4s ", "%");
            outStr = gStringFormatAppend(outStr, outStrEnd, "%10.0f ", cpu_time * 100);
            outStr = gStringFormatAppend(outStr, outStrEnd, "%-4s ", "%");
        }
        else if (result.run_type != BenchMarkRun::RT_Aggregate || result.aggregate_unit.unit == StatisticUnit::Time)
        {
            const char* timeLabel = result.time_unit.ToString();
            // printer(Out, COLOR_YELLOW, "%s %-4s %s %-4s ", real_time_str.c_str(), timeLabel, cpu_time_str.c_str(), timeLabel);
            outStr = gFormatTime(real_time, outStr, outStrEnd);
            outStr = gStringAppend(outStr, outStrEnd, ' ');
            outStr = gStringFormatAppend(outStr, outStrEnd, "%-4s ", timeLabel);
            outStr = gFormatTime(cpu_time, outStr, outStrEnd);
            outStr = gStringAppend(outStr, outStrEnd, ' ');
            outStr = gStringFormatAppend(outStr, outStrEnd, "%-4s ", timeLabel);
        }
        else
        {
            // ASSERT(result.aggregate_unit.unit == StatisticUnit::Percentage);
            // printer(Out, COLOR_YELLOW, "%10.2f %-4s %10.2f %-4s ", (100. * result.real_accumulated_time), "%", (100. * result.cpu_accumulated_time), "%");
            outStr = gStringFormatAppend(outStr, outStrEnd, "%10.2f ", (100. * result.real_accumulated_time));
            outStr = gStringFormatAppend(outStr, outStrEnd, "%-4s ", "%");
            outStr = gStringFormatAppend(outStr, outStrEnd, "%10.2f ", (100. * result.cpu_accumulated_time));
            outStr = gStringFormatAppend(outStr, outStrEnd, "%-4s ", "%");
        }

        if (!result.report_big_o && !result.report_rms)
        {
            // printer(Out, COLOR_CYAN, "%10lld", result.iterations);
            outStr = gStringFormatAppend(outStr, outStrEnd, "%10lld", result.iterations);
        }

        if (result.counters.Size() > 0)
        {
            char* const numberStr        = scratch->Alloc<char>(128 + 1);
            numberStr[128]               = '\0';
            const char* const outStr2End = &numberStr[128];

            for (s32 i = 0; i < result.counters.Size(); ++i)
            {
                const Counter& c = result.counters.counters[i];

                const char* cname = c.name;
                const char* unit  = "";

                char* outStr2 = numberStr;
                if (result.run_type == BenchMarkRun::RT_Aggregate && result.aggregate_unit.unit == StatisticUnit::Percentage)
                {
                    outStr2 = gStringFormatAppend(outStr2, outStr2End, "%.2f", 100. * c.value);
                    unit    = "%";
                }
                else
                {
                    outStr2 = gHumanReadableNumber(outStr2, outStr2End, c.value, c.flags.OneK());
                    if (c.flags.flags & CounterFlags::IsRate)
                        unit = (c.flags.flags & CounterFlags::Invert) ? "s" : "/s";
                }
                outStr2 = gStringAppendTerminator(outStr2, outStr2End);

                outStr = gStringAppend(outStr, outStrEnd, ' ');
                if (output_options_ & OO_Tabular)
                {
                    // printer(Out, COLOR_DEFAULT, " %*s%s", gStringLength(cname) - gStringLength(unit), outStr, unit);
                    gSetWidthFormat(nameWidthFormat, gStringLength(cname) - gStringLength(unit));
                    outStr = gStringFormatAppend(outStr, outStrEnd, nameWidthFormat, numberStr);
                }
                else
                {
                    // printer(Out, COLOR_DEFAULT, " %s=%s%s", cname, outStr, unit);
                    outStr = gStringAppend(outStr, outStrEnd, cname);
                    outStr = gStringAppend(outStr, outStrEnd, '=');
                    outStr = gStringAppend(outStr, outStrEnd, numberStr);
                }
                outStr = gStringAppend(outStr, outStrEnd, unit);
            }
            scratch->Deallocate(numberStr);
        }
        if (result.report_format != nullptr)
        {
            // printer(Out, COLOR_DEFAULT, " %s", result.report_label);
            outStr = gStringAppend(outStr, outStrEnd, ' ');
            outStr = gStringFormatAppend(outStr, outStrEnd, result.report_format, result.report_value);
        }
        outStr = gStringAppendTerminator(outStr, outStrEnd);

        (output_stream_ << line).endl();

        scratch->Deallocate(nameWidthFormat);
        scratch->Deallocate(name);
    }

    void ConsoleReporter::PrintHeader(const BenchMarkRun& report, ForwardAllocator* allocator, ScratchAllocator* scratch)
    {
        USE_SCRATCH(scratch);

        char* nameWidthFormat = scratch->Alloc<char>(8 + 1);
        gSetWidthFormat(nameWidthFormat, static_cast<int>(name_field_width_));

        const s32   max_line_width = 1024;
        char* const line           = scratch->Alloc<char>(max_line_width + 1);
        line[0]                    = '\0';

        char*             outStr    = line;
        char const* const outStrEnd = &line[max_line_width];
        outStr                      = gStringFormatAppend(outStr, outStrEnd, nameWidthFormat, "Benchmark");
        outStr                      = gStringFormatAppend(outStr, outStrEnd, "%13s", "Time");
        outStr                      = gStringFormatAppend(outStr, outStrEnd, "%15s", "CPU");
        outStr                      = gStringFormatAppend(outStr, outStrEnd, "%12s", "Iterations");

        if (!report.counters.counters.Empty())
        {
            if (output_options_ & OO_Tabular)
            {
                for (int i = 0; i < report.counters.counters.Size(); ++i)
                {
                    outStr = gStringFormatAppend(outStr, outStrEnd, " %10s", report.counters.counters[i].name);
                }
            }
            else
            {
                outStr = gStringFormatAppend(outStr, outStrEnd, " %s", " UserCounters...");
            }
        }
        outStr = gStringAppendTerminator(outStr, outStrEnd);

        const int   width = (int)(outStr - line);
        char* const line2 = outStr;
        char*       str2  = line2;
        for (int i = 0; i < width; ++i)
            str2 = gStringAppend(str2, outStrEnd, '-');
        str2 = gStringAppend(str2, outStrEnd, '\0');

        (output_stream_ << line2).endl();
        (output_stream_ << line).endl();
        (output_stream_ << line2).endl();

        scratch->Deallocate(nameWidthFormat);
        scratch->Deallocate(line);
    }

    void ConsoleReporter::Flush(ForwardAllocator* allocator, ScratchAllocator* scratch) { output_stream_.flush(); }
    void ConsoleReporter::Finalize(ForwardAllocator* allocator, ScratchAllocator* scratch) {}

} // namespace BenchMark
