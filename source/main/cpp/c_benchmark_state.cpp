#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
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
    BenchMarkState::BenchMarkState()
        : alloc_(nullptr)
        , name_(nullptr)
        , max_iterations(0)
        , range_(nullptr)
        , complexity_n_(0)
        , thread_index_(0)
        , threads_(0)
        , timer_(nullptr)
        , manager_(nullptr)
        , results_(nullptr)
        , total_iterations_(0)
        , batch_leftover_(0)
        , started_(false)
        , finished_(false)
        , skipped_(Skipped::NotSkipped)
    {
    }

    void BenchMarkState::Init(const char* name, IterationCount max_iters, Array<s32> const* range, s32 thread_index, s32 threads)
    {
        name_             = name;
        max_iterations    = max_iters;
        range_            = range;
        thread_index_     = thread_index;
        threads_          = threads;
        alloc_            = nullptr;
        name_             = nullptr;
        complexity_n_     = 0;
        timer_            = nullptr;
        manager_          = nullptr;
        results_          = nullptr;
        total_iterations_ = 0;
        batch_leftover_   = 0;
        started_          = false;
        finished_         = false;
        skipped_          = Skipped::NotSkipped;
    }

    void BenchMarkState::InitRun(Allocator* alloc, const char* name, IterationCount max_iters, Array<s32> const* range, s32 counters, s32 thread_index, s32 threads, ThreadTimer* timer, ThreadManager* manager, BenchMarkRunResult* results)
    {
        Init(name, max_iters, range, thread_index, threads);

        alloc_            = alloc;
        timer_            = timer;
        manager_          = manager;
        results_          = results;

        if (counters > 0)
            counters_.Initialize(alloc, counters);
    }

    void BenchMarkState::Shutdown() 
    { 
        counters_.Release();
    }

    static void ResultSkipWithMessage(BenchMarkRunResult* rr, const char* msg, Skipped skipped)
    {
        if (rr->skip_message_ != nullptr)
            rr->skip_message_ = nullptr;

        if (rr->skipped_.IsNotSkipped())
        {
            rr->skip_message_ = msg;
            rr->skipped_      = skipped;
        }
    }
    static void ResultSetLabel(BenchMarkRunResult* rr, const char* format, double value)
    {
        if (rr->report_format_ != nullptr)
        {
            rr->report_format_ = nullptr;
            rr->report_value_  = 0.0;
        }
        rr->report_format_ = format;
        rr->report_value_  = value;
    }

    void BenchMarkState::PauseTiming()
    {
        // Add in time accumulated so far
        BM_CHECK(started_ && !finished_ && !skipped());
        ThreadTimerStop(timer_);
    }

    void BenchMarkState::ResumeTiming()
    {
        BM_CHECK(started_ && !finished_ && !skipped());
        ThreadTimerStart(timer_);
    }

    void BenchMarkState::SkipWithMessage(const char* msg)
    {
        skipped_ = {Skipped::SkippedWithMessage};
        ResultSkipWithMessage(results_, msg, skipped_);
        total_iterations_ = 0;
        if (ThreadTimerIsRunning(timer_))
            ThreadTimerStop(timer_);
    }

    void BenchMarkState::SkipWithError(const char* msg)
    {
        skipped_ = {Skipped::SkippedWithError};
        ResultSkipWithMessage(results_, msg, skipped_);
        total_iterations_ = 0;
        if (ThreadTimerIsRunning(timer_))
            ThreadTimerStop(timer_);
    }

    void BenchMarkState::SetIterationTime(double seconds) { ThreadTimerSetIterationTime(timer_, seconds); }
    void BenchMarkState::SetLabel(const char* format, double value) { ResultSetLabel(results_, format, value); }

    void BenchMarkState::StartKeepRunning()
    {
        BM_CHECK(!started_ && !finished_);
        started_          = true;
        total_iterations_ = skipped_.IsSkipped() ? 0 : max_iterations;
        StartStopBarrier(manager_);
        if (skipped_.IsNotSkipped())
            ResumeTiming();
    }

    void BenchMarkState::FinishKeepRunning()
    {
        BM_CHECK(started_ && (!finished_ || skipped_.IsNotSkipped()));
        if (skipped_.IsNotSkipped())
        {
            PauseTiming();
        }
        total_iterations_ = 0;
        finished_         = true;
        StartStopBarrier(manager_);
    }
} // namespace BenchMark