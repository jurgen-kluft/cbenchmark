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

class StdOut : public BenchMark::ConsoleOutput
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
    BenchMark::g_InitTimer();

    BenchMark::MainAllocator    main_allocator;
    BenchMark::ForwardAllocator forward_allocator;
    BenchMark::BenchMarkGlobals globals;
    forward_allocator.Initialize(&main_allocator, 128 * 1024);

    StdOut                     stdoutput;
    BenchMark::ConsoleReporter reporter;
    reporter.Initialize(&forward_allocator, &stdoutput);

    bool result = BenchMark::gRunBenchMark(&main_allocator, &globals, reporter);

    reporter.Shutdown(&forward_allocator);
    forward_allocator.Release();
    
    return result ? 0 : -1;
}

#endif