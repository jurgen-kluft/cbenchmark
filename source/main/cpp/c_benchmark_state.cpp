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
    BenchMarkState::BenchMarkState()
        : alloc_(nullptr)
        , name_(nullptr)
        , max_iterations(0)
        , range_(nullptr)
        , thread_index_(0)
        , threads_(0)
        , timer_(nullptr)
        , manager_(nullptr)
        , results_(nullptr)
        , total_iterations_(0)
        , started_(false)
        , finished_(false)
        , skipped_({Skipped::NotSkipped})
    {
    }

    void BenchMarkState::Init(const char* name, IterationCount max_iters, Array<s32> const* range, s32 thread_index, s32 threads)
    {
        name_          = name;
        max_iterations = max_iters;
        range_         = range;
        thread_index_  = thread_index;
        threads_       = threads;
    }

    void BenchMarkState::InitRun(Allocator* alloc, const char* name, IterationCount max_iters, Array<s32> const* range, s32 thread_index, s32 threads, ThreadTimer* timer, ThreadManager* manager, BenchMarkRunResult* results)
    {
        alloc_         = alloc;
        name_          = name;
        max_iterations = max_iters;
        range_         = range;
        thread_index_  = thread_index;
        threads_       = threads;
        timer_         = timer;
        manager_       = manager;
        results_       = results;

        // counters, we always have
    }

    static void ResultSkipWithMessage(BenchMarkRunResult* rr, const char* msg, Skipped skipped)
    {
        if (rr->skip_message_)
        {
            rr->allocator->Dealloc((void*)rr->skip_message_);
            rr->skip_message_ = nullptr;
        
        }
        if (rr->skipped_.IsNotSkipped())
        {
            s32 const msg_len      = (s32)gStringLength(msg);
            char*     skip_message = rr->allocator->Alloc<char>(msg_len + 1);
            gStringCopy(skip_message, msg, msg_len);
            rr->skip_message_ = skip_message;
            rr->skipped_      = skipped;
        }
    }
    static void ResultSetLabel(BenchMarkRunResult* rr, const char* format, double value)
    {
        if (rr->report_label_)
        {
            rr->allocator->Dealloc((void*)rr->report_label_);
            rr->report_label_ = nullptr;
        }
        s32 const len     = 63;
        char*     label   = rr->allocator->Alloc<char>(len + 1);
        const char* labelEnd = label + len;
        rr->report_label_ = label;
        gStringFormatAppend(label, labelEnd, format, value);
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
        total_iterations_ = 0;
        finished_         = true;
        StartStopBarrier(manager_);
    }
} // namespace BenchMark