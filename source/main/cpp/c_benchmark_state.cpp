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
    BenchMarkState::BenchMarkState(const char* name, IterationCount max_iters, Array<s64> const* range, int thread_index, int threads, ThreadTimer* timer, ThreadManager* manager, BenchMarkRunResult* results)
        : name_(name)
        , max_iterations(max_iters)
        , range_(range)
        , thread_index_(thread_index)
        , threads_(threads)
        , timer_(timer)
        , manager_(manager)
        , results_(results)
    {
    }

    static void ResultSkipWithMessage(BenchMarkRunResult* rr, const char* msg, Skipped skipped)
    {
        if (rr->skipped_.IsNotSkipped())
        {
            gStringCopy(rr->skip_message_, msg, sizeof(rr->skip_message_) - 1);
            rr->skipped_ = skipped;
        }
    }
    static void ResultSetLabel(BenchMarkRunResult* rr, const char* format, double value)
    {
        gStringFormatAppend(rr->report_label_, &rr->report_label_[64] - 1, format, value);
    }

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
        ResultSkipWithMessage(results_, msg, skipped_);
        total_iterations_ = 0;
        if (TimerIsRunning(timer_))
            TimerStop(timer_);
    }

    void BenchMarkState::SkipWithError(const char* msg)
    {
        skipped_ = {Skipped::SkippedWithError};
        ResultSkipWithMessage(results_, msg, skipped_);
        total_iterations_ = 0;
        if (TimerIsRunning(timer_))
            TimerStop(timer_);
    }

    void BenchMarkState::SetIterationTime(double seconds) { TimerSetIterationTime(timer_, seconds); }
    void BenchMarkState::SetLabel(const char* format, double value) { ResultSetLabel(results_, format, value); }

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