#ifdef TARGET_PC

#    include "cbenchmark/cbenchmark.h"
#    include "cbenchmark/private/c_Time_Helpers.h"
#    include "cbenchmark/private/c_StringBuilder.h"
#    include "cbenchmark/private/c_BenchMark_alloc.h"
#    include "cbenchmark/private/c_BenchMark_Reporter_Console.h"

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
        const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        fprintf(stdout_handle, "%s", text);
    }
};

extern bool gRunBenchMark(BenchMark::BenchMarkReporter& reporter);

int main(int argc, char** argv)
{
    StdOut                stdoutput;
    BenchMark::TextStream out_stream;
    BenchMark::TextStream err_stream;
    out_stream.out = &stdoutput;
    err_stream.out = &stdoutput;

    // TODO give the text stream objects enough memory to write their log to

    BenchMark::ConsoleReporter reporter(&out_stream, &err_stream);

    BenchMark::InitTimer();

    bool result = gRunBenchMark(reporter);

    return result ? 0 : -1;
}

#endif