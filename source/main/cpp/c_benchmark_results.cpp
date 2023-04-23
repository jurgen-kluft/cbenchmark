#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_unit.h"
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
                { // (1) either this
                    // {x,y,z}
                    // {x,y,z}
                    // {x,y,z}
                    // ...
                    BM_ARGS(8, 64 );
                    BM_ARGS(8, 128 );
                    BM_ARGS(16, 64 );
                    BM_ARGS(16, 128 );
                    BM_ARGS(32, 64 );
                    BM_ARGS(32, 128 );
                    BM_ARGS(64, 64 );
                    BM_ARGS(64, 128 );
                    BM_ARGS(128, 64 );
                    BM_ARGS(128, 128 );
                }

                { // (2) or this 
                    // These 2 named arguments will be permuted together, and will result in:
                    // { x: 8, y: 64 }
                    // { x: 8, y: 128 }
                    // { x: 16, y: 64 }
                    // { x: 16, y: 128 }
                    // { x: 32, y: 64 }
                    // { x: 32, y: 128 }
                    // { x: 64, y: 64 }
                    // { x: 64, y: 128 }
                    // { x: 128, y: 64 }
                    // { x: 128, y: 128 }
                    BM_ADD_ARG(8, 16, 32, 64, 128);
                    BM_ADD_ARG(64, 128);
                }

                { // (3) or this
                    // This will have the same result as (2)
                    BM_ADD_ARG_RANGE(8, 128, 8);
                    BM_ADD_ARG_DENSE_RANGE(64, 128, 64);
                }

                // The above 3 examples emit arguments with 2 values each, let's say x and y.
                BM_SET_ARG_NAMES("x", "y");

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

            BM_SETTINGS(test)
            {
                // Settings are initialized top to bottom, Suite -> Fixture -> Test.
                // Override any setting that was set in suite and fixture.
            }

            // BM_TEST_DISABLE(test);

            BM_TEST(memcpy)
            {
                // here we have our benchmark code, for example
                // to benchmark memcpy we can do something like this:
                char* src = allocator->Alloc<char>(state.Range(0));
                char* dst = allocator->Alloc<char>(state.Range(0));

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

                allocator->Dealloc(src);
                allocator->Dealloc(dst);
            }
        }
    }
} // namespace BenchMark
