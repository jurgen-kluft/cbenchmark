#ifdef TARGET_PC

#include "cbenchmark/cbenchmark.h"
#include "cbenchmark/private/c_Time_Helpers.h"
#include "cbenchmark/private/c_StringBuilder.h"
#include "cbenchmark/private/c_BenchMark_alloc.h"
#include "cbenchmark/private/c_BenchMark_Reporter_Console.h"

#    include <windows.h>


extern bool gRunBenchMark(BenchMark::BenchMarkReporter& reporter);

int         main(int argc, char** argv)
{
    BenchMark::TextStream         out_stream;
    BenchMark::TextStream         err_stream;

    // TODO give the text stream objects enough memory to write their log to

    BenchMark::ConsoleReporter    reporter(&out_stream, &err_stream);

    BenchMark::InitTimer();

    bool result = gRunBenchMark(reporter);

    return result ? 0 : -1;
}

#endif