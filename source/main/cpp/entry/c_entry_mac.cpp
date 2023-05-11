#ifdef TARGET_MAC
#    include "cbenchmark/cbenchmark.h"
#    include "cbenchmark/private/c_benchmark.h"
#    include "cbenchmark/private/c_benchmark_allocators.h"
#    include "cbenchmark/private/c_benchmark_instance.h"
#    include "cbenchmark/private/c_benchmark_reporter_console.h"
#    include "cbenchmark/private/c_time_helpers.h"
#    include "cbenchmark/private/c_stringbuilder.h"

#    include <stdlib.h>
#    include <stdio.h>

typedef const char* PlatformColorCode;

PlatformColorCode GetPlatformColorCode(BenchMark::TextColor color)
{
    switch (color)
    {
        case BenchMark::COLOR_RED: return "1";
        case BenchMark::COLOR_GREEN: return "2";
        case BenchMark::COLOR_YELLOW: return "3";
        case BenchMark::COLOR_BLUE: return "4";
        case BenchMark::COLOR_MAGENTA: return "5";
        case BenchMark::COLOR_CYAN: return "6";
        case BenchMark::COLOR_WHITE: return "7";
        default: return nullptr;
    }
}

class StdOut : public BenchMark::TextOutput
{
public:
    StdOut() {}

    virtual void setColor(BenchMark::TextColor color)
    {
        const char* color_code = GetPlatformColorCode(color);
        if (color_code)
        {
            // "\033[0;3%sm", color_code);
            fprintf(stdout, "\033[0;3%sm", color_code);
        }
    }

    virtual void resetColor() { fprintf(stdout, "\033[m"); }

    virtual void print(const char* text) { fprintf(stdout, "%s", text); }
};

extern bool gRunBenchMark(BenchMark::BenchMarkReporter& reporter);


int main(int argc, char** argv)
{
    BenchMark::g_InitTimer();

    BenchMark::MainAllocator    main_allocator;
    BenchMark::BenchMarkGlobals globals;

    StdOut                stdoutput;
    BenchMark::TextStream out_stream;
    BenchMark::TextStream err_stream;
    out_stream.out = &stdoutput;
    err_stream.out = &stdoutput;

    const unsigned int kOutStreamBufferSize = 1024;
    const unsigned int kErrStreamBufferSize = 256;

    out_stream.sos    = (char*)main_allocator.Allocate(kOutStreamBufferSize, 8);
    out_stream.eos    = out_stream.sos + kOutStreamBufferSize - 1;
    out_stream.stream = out_stream.sos;

    err_stream.sos    = (char*)main_allocator.Allocate(kErrStreamBufferSize, 8);
    err_stream.eos    = err_stream.sos + kErrStreamBufferSize - 1;
    err_stream.stream = err_stream.sos;

    BenchMark::ConsoleReporter reporter(&out_stream, &err_stream);

    bool result = BenchMark::gRunBenchMark(&main_allocator, &globals, reporter);

    main_allocator.Deallocate(out_stream.sos);
    main_allocator.Deallocate(err_stream.sos);

    return result ? 0 : -1;
}

#endif