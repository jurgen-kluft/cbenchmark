#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_entity.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_alloc.h"

#include <string>

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
                BM_ARG(8, 32, 128);
                BM_NAMED_ARG(x, 8, 16, 32, 64, 128);

                BM_ARGRANGE({8, 512, 32}, {128, 512, 128});
                BM_NAMED_ARGRANGE("x", {8, 512, 32}, "y", {128, 512, 128}, "z", {4, 32, 8});

                BM_ARGPRODUCT({8, 16, 32, 64, 128}, {1, 2, 3, 4});

                BM_COUNTER("test", CounterFlags::IsRate);
                BM_COUNTER("test2", CounterFlags::IsRate);
                BM_COUNTER("test3", CounterFlags::IsRate);

                BM_TIMEUNIT(TimeUnit::Microsecond);
                BM_MINTIME(0);
                BM_MINWARMUPTIME(0);
                BM_ITERATIONS(0);
                BM_REPETITIONS(0);
            }

            BM_SETTINGS(test)
            {
                // Settings are initialized top to bottom, Suite -> Fixture -> Test.
                // Override any setting that was set in suite and fixture.
            }

            // BM_TEST_DISABLE(test);

            BM_TEST(test)
            {
                // here we have our benchmark code, for example
                // to benchmark memcpy we can do something like this:
                char* src = allocator->Malloc<char>(state.Range(0));
                char* dst = allocator->Malloc<char>(state.Range(0));
                
                memset(src, 'x', state.Range(0));

                // timing starts after 'BM_ITERATE'
                BM_ITERATE
                {
                    // state.PauseTiming();
                    //    If you need to do any setup before each iteration, do it here.
                    // state.ResumeTiming();

                    memcpy(dst, src, state.Range(0));
                }

                state.SetBytesProcessed(s64(state.Iterations()) * s64(state.Range(0)));

                allocator->Free(src);
                allocator->Free(dst);
            }
        }
    }
} // namespace BenchMark
