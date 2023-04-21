#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_exception.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include "cbenchmark/private/c_benchmark_runner.h"
#include "cbenchmark/private/c_benchmark_run.h"
#include "cbenchmark/private/c_benchmark_instance.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_check.h"

namespace BenchMark
{
    void BenchMarkState::PauseTiming()
    {
        // Add in time accumulated so far
        BM_CHECK(started_ && !finished_ && !skipped());
        TimerStop(timer_);
    }

    void BenchMarkState::ResumeTiming()
    {
        BM_CHECK(started_ && !finished_ && !skipped());
        TimerStart(timer_);
    }

    void BenchMarkState::SkipWithMessage(const char* msg)
    {
        skipped_ = {Skipped::SkippedWithMessage};
        ThreadManagerSkipWithMessage(manager_, msg, skipped_);
        total_iterations_ = 0;
        if (TimerIsRunning(timer_))
            TimerStop(timer_);
    }

    void BenchMarkState::SkipWithError(const char* msg)
    {
        skipped_ = {Skipped::SkippedWithError};
        ThreadManagerSkipWithMessage(manager_, msg, skipped_);
        total_iterations_ = 0;
        if (TimerIsRunning(timer_))
            TimerStop(timer_);
    }

    void BenchMarkState::SetIterationTime(double seconds) { TimerSetIterationTime(timer_, seconds); }
    void BenchMarkState::SetLabel(const char* format, double value) { ThreadManagerSetLabel(manager_, format, value); }

    void BenchMarkState::StartKeepRunning()
    {
        BM_CHECK(!started_ && !finished_);
        started_          = true;
        total_iterations_ = Skipped().IsSkipped() ? 0 : max_iterations;
        StartStopBarrier(manager_);
        if (Skipped().IsNotSkipped())
            ResumeTiming();
    }

    void BenchMarkState::FinishKeepRunning()
    {
        BM_CHECK(started_ && (!finished_ || Skipped().IsNotSkipped()));
        if (Skipped().IsNotSkipped())
        {
            PauseTiming();
        }
        // Total iterations has now wrapped around past 0. Fix this.
        total_iterations_ = 0;
        finished_         = true;
        StartStopBarrier(manager_);
    }
} // namespace BenchMark