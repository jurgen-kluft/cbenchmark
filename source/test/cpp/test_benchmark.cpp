#include "ccore/c_target.h"
#include "cbenchmark/cbenchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_unit.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_allocators.h"
#include "cbenchmark/private/c_time_helpers.h"

#include "cunittest/cunittest.h"

#include <string>

using namespace ncore;

namespace BenchMark
{
    BM_SUITE(test_suite)
    {
        BM_FIXTURE(test_fixture)
        {
            BM_FIXTURE_SETUP {}
            BM_FIXTURE_TEARDOWN {}

            BM_FIXTURE_SETTINGS
            {
                {                      // (1) either this
                    BM_ARGS(8, 64);    // {x,y}
                    BM_ARGS(8, 128);   // {x,y}
                    BM_ARGS(16, 64);   // {x,y}
                    BM_ARGS(16, 128);  // {x,y}
                    BM_ARGS(32, 64);   // {x,y}
                    BM_ARGS(32, 128);  // {x,y}
                    BM_ARGS(64, 64);   // {x,y}
                    BM_ARGS(64, 128);  // {x,y}
                    BM_ARGS(128, 64);  // {x,y}
                    BM_ARGS(128, 128); // {x,y}

                    BM_ARG_NAME(0, "x");
                    BM_ARG_NAME(1, "y");
                }

                {                                   // (2) or this
                    BM_ARG(0)->SEQUENCE(8, 16, 32, 64, 128); // x
                    BM_ARG(1)->SEQUENCE(64, 128);            // y

                    BM_ARG_NAME(0, "x");
                    BM_ARG_NAME(1, "y");

                    // These 2 named arguments will be permuted together, and will result in:
                    // { 8, 64 }, x and y
                    // { 8, 128 }, x and y
                    // { 16, 64 }, x and y
                    // { 16, 128 }, x and y
                    // { 32, 64 }, x and y
                    // { 32, 128 }, x and y
                    // { 64, 64 }, x and y
                    // { 64, 128 }, x and y
                    // { 128, 64 }, x and y
                    // { 128, 128 }, x and y
                }

                {                                        // (3) or this
                    BM_ARG_NAME(0, "x")->RANGE(8, 128, 8);         // x = 8, 16, 32, 64, 128
                    BM_ARG_NAME(1, "y")->DENSE_RANGE(64, 128, 64); // y = 64, 128
                    // Will have the same result as (2)
                }

                // Run the benchmark with 1, 2, 4, 8, 16 threads (increases the permutations)
                BM_THREAD_COUNTS(1, 2, 4, 8, 16);

                BM_COUNTER("test", CounterFlags::IsRate);
                BM_COUNTER("test2", CounterFlags::IsRate);
                BM_COUNTER("test3", CounterFlags::IsRate);

                BM_TIMEUNIT(TimeUnit::Microsecond);
                BM_MINTIME(0);
                BM_MINWARMUPTIME(0);
                BM_ITERATIONS(0);
                BM_REPETITIONS(0);
            }

            BM_SETTINGS(memcpy)
            {
                // Settings are initialized top to bottom, Suite -> Fixture -> Test.
                // Override any setting that was set in suite and fixture.
                
                BM_MEMORY_REQUIRED(8*1024*1024);
            }

            // BM_UNIT_DISABLE(memcpy);

            BM_UNIT(memcpy)
            {
                // here we have our benchmark code, for example
                // to benchmark memcpy we can do something like this:
                char* src = allocator->Alloc<char>(state.Range(0));
                char* dst = allocator->Alloc<char>(state.Range(0));

                // 'allocator' is available in each benchmark (thread-safe)

                memset(src, 'x', state.Range(0));

                // timing starts here at 'BM_ITERATE'
                BM_ITERATE
                {
                    // state.PauseTiming();
                    //    If you need to do any specific setup before each iteration, do it here.
                    //    But this can impact timings since PauseTiming and ResumeTiming are costly
                    //    and cause inaccuracy.
                    // state.ResumeTiming();

                    memcpy(dst, src, state.Range(0));
                }

                state.SetBytesProcessed(s64(state.Iterations()) * s64(state.Range(0)));

                allocator->Dealloc(src);
                allocator->Dealloc(dst);
            }
        }
    }
} // namespace BenchMark

class StdTest : public BenchMark::ConsoleOutput
{

public:
    StdTest()
    {
    }

    virtual void setColor(BenchMark::TextColor color)
    {

    }

    virtual void resetColor()
    {

    }

    virtual void print(const char* text)
    {
        // std::out / console writer
        fprintf(stdout, "%s", text);
    }
};

UNITTEST_SUITE_BEGIN(test_benchmark)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(test)
        {
            BenchMark::g_InitTimer();

            BenchMark::MainAllocator    main_allocator;
            BenchMark::ForwardAllocator forward_allocator;
            BenchMark::BenchMarkGlobals globals;
            forward_allocator.Initialize(&main_allocator, 128 * 1024);

            StdTest                    stdoutput;
            BenchMark::ConsoleReporter reporter;
            reporter.Initialize(&forward_allocator, &stdoutput);

            BenchMark::gRunBenchMark(&main_allocator, &globals, reporter);

            forward_allocator.Release();
        }
    }
}
UNITTEST_SUITE_END
