#ifndef __CBENCHMARK_REPORTERSTDOUT_H__
#define __CBENCHMARK_REPORTERSTDOUT_H__

#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_allocators.h"

namespace BenchMark
{
    // Simple reporter that outputs benchmark data to the console. This is the
    // default reporter used by RunSpecifiedBenchmarks().
    class ConsoleReporter : public BenchMarkReporter
    {
    public:
        enum OutputOptions
        {
            OO_None         = 0,
            OO_Color        = 1,
            OO_Tabular      = 2,
            OO_ColorTabular = OO_Color | OO_Tabular,
            OO_Defaults     = OO_ColorTabular
        };

        explicit ConsoleReporter(OutputOptions opts_ = OO_Defaults)
            : output_options_(opts_)
            , prev_counters_()
            , name_field_width_(0)
            , printed_header_(false)
        {
        }

        void Init(Allocator* allocator, ConsoleOutput* out, s32 max_line_width = 512);
        void Exit(Allocator* allocator);

        virtual bool ReportContext(const Context& context);
        virtual void ReportRuns(Array<BenchMarkRun*> const& reports);
        virtual void ReportRunsConfig(double /*min_time*/, bool /*has_explicit_iters*/, IterationCount /*iters*/);

    protected:
        virtual void PrintRunData(const BenchMarkRun& report);
        virtual void PrintHeader(const BenchMarkRun& report);

        virtual void Flush();
        virtual void Finalize();

        TextStream    output_stream_;
        TextStream    error_stream_;
        OutputOptions output_options_;
        Counters      prev_counters_;
        char*         line1_;
        char*         line2_;
        s32           max_line_width_;
        s32           name_field_width_;
        bool          printed_header_;
    };

} // namespace BenchMark

#endif // __CBENCHMARK_REPORTERSTDOUT_H__
