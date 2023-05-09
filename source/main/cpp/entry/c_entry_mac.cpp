#ifdef TARGET_MAC
#    include "cbenchmark/cbenchmark.h"
#    include "cbenchmark/private/c_Time_Helpers.h"
#    include "cbenchmark/private/c_StringBuilder.h"
#    include "cbenchmark/private/c_BenchMark_alloc.h"
#    include "cbenchmark/private/c_BenchMark_Reporter_Console.h"

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

int         main(int argc, char** argv)
{
    BenchMark::InitTimer();

    StdOut                stdoutput;
    BenchMark::TextStream out_stream;
    BenchMark::TextStream err_stream;
    out_stream.out = &stdoutput;
    err_stream.out = &stdoutput;
    BenchMark::ConsoleReporter reporter(&out_stream, &err_stream);

    bool result = gRunBenchMark(reporter);

    return result ? 0 : -1;
}

#endif