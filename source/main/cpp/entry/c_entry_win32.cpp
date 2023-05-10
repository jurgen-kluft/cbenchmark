#ifdef TARGET_PC

#    include "cbenchmark/cbenchmark.h"
#    include "cbenchmark/private/c_benchmark.h"
#    include "cbenchmark/private/c_benchmark_allocators.h"
#    include "cbenchmark/private/c_benchmark_instance.h"
#    include "cbenchmark/private/c_benchmark_reporter_console.h"
#    include "cbenchmark/private/c_time_helpers.h"
#    include "cbenchmark/private/c_stringbuilder.h"

#    include <cstdio>
#    include <cstring>

#    include <windows.h>

typedef WORD      PlatformColorCode;
PlatformColorCode GetPlatformColorCode(BenchMark::TextColor color)
{
    switch (color)
    {
        case BenchMark::COLOR_RED: return FOREGROUND_RED;
        case BenchMark::COLOR_GREEN: return FOREGROUND_GREEN;
        case BenchMark::COLOR_YELLOW: return FOREGROUND_RED | FOREGROUND_GREEN;
        case BenchMark::COLOR_BLUE: return FOREGROUND_BLUE;
        case BenchMark::COLOR_MAGENTA: return FOREGROUND_BLUE | FOREGROUND_RED;
        case BenchMark::COLOR_CYAN: return FOREGROUND_BLUE | FOREGROUND_GREEN;
        case BenchMark::COLOR_WHITE: // fall through to default
        default: return 0;
    }
}

class StdOut : public BenchMark::TextOutput
{
    bool has_default_color_attrs;
    WORD default_color_attrs;

public:
    StdOut()
        : has_default_color_attrs(false)
        , default_color_attrs(0)
    {
    }

    virtual void setColor(BenchMark::TextColor color)
    {
        const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (!has_default_color_attrs)
        {
            has_default_color_attrs = true;
            CONSOLE_SCREEN_BUFFER_INFO buffer_info;
            GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
            default_color_attrs = buffer_info.wAttributes;
        }

        fflush(stdout);
        SetConsoleTextAttribute(stdout_handle, GetPlatformColorCode(color) | FOREGROUND_INTENSITY);
    }

    virtual void resetColor()
    {
        fflush(stdout);

        // Restores the default text color.
        const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(stdout_handle, default_color_attrs);
    }

    virtual void print(const char* text)
    {
        // std::out / console writer
        fprintf(stdout, "%s", text);
    }
};

int main(int argc, char** argv)
{
    BenchMark::InitTimer();

    BenchMark::MainAllocator    main_allocator;
    BenchMark::BenchMarkGlobals globals;

    StdOut                stdoutput;
    BenchMark::TextStream out_stream;
    BenchMark::TextStream err_stream;
    out_stream.out = &stdoutput;
    err_stream.out = &stdoutput;

    const unsigned int kOutStreamBufferSize = 4 * 1024;
    const unsigned int kErrStreamBufferSize = 1 * 1024;

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