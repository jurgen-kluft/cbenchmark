#ifndef __CBENCHMARK_BENCHMARK_STATE_H__
#define __CBENCHMARK_BENCHMARK_STATE_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_statistics.h"

namespace BenchMark
{
    // BenchMarkState is passed to a running Benchmark and contains state for the benchmark to use.
    class BenchMarkState
    {
    public:
        // Returns true if the benchmark should continue through another iteration.
        // NOTE: A benchmark may not return from the test until KeepRunning() has
        // returned false.
        bool KeepRunning();

        // Returns true if the benchmark should run n more iterations.
        // REQUIRES: 'n' > 0.
        // NOTE: A benchmark must not return from the test until KeepRunningBatch()
        // has returned false.
        // NOTE: KeepRunningBatch() may overshoot by up to 'n' iterations.
        //
        // Intended usage:
        //   while (state.KeepRunningBatch(1000)) {
        //     // process 1000 elements
        //   }
        bool KeepRunningBatch(IterationCount n);

        // REQUIRES: timer is running and 'SkipWithMessage(...)' or
        //   'SkipWithError(...)' has not been called by the current thread.
        // Stop the benchmark timer.  If not called, the timer will be
        // automatically stopped after the last iteration of the benchmark loop.
        //
        // For threaded benchmarks the PauseTiming() function only pauses the timing
        // for the current thread.
        //
        // NOTE: The "real time" measurement is per-thread. If different threads
        // report different measurements the largest one is reported.
        //
        // NOTE: PauseTiming()/ResumeTiming() are relatively
        // heavyweight, and so their use should generally be avoided
        // within each benchmark iteration, if possible.
        void PauseTiming();

        // REQUIRES: timer is not running and 'SkipWithMessage(...)' or
        //   'SkipWithError(...)' has not been called by the current thread.
        // Start the benchmark timer.  The timer is NOT running on entrance to the
        // benchmark function. It begins running after control flow enters the
        // benchmark loop.
        //
        // NOTE: PauseTiming()/ResumeTiming() are relatively
        // heavyweight, and so their use should generally be avoided
        // within each benchmark iteration, if possible.
        void ResumeTiming();

        // REQUIRES: 'SkipWithMessage(...)' or 'SkipWithError(...)' has not been
        //            called previously by the current thread.
        // Report the benchmark as resulting in being skipped with the specified
        // 'msg'.
        // After this call the user may explicitly 'return' from the benchmark.
        //
        // If the ranged-for style of benchmark loop is used, the user must explicitly
        // break from the loop, otherwise all future iterations will be run.
        // If the 'KeepRunning()' loop is used the current thread will automatically
        // exit the loop at the end of the current iteration.
        //
        // For threaded benchmarks only the current thread stops executing and future
        // calls to `KeepRunning()` will block until all threads have completed
        // the `KeepRunning()` loop. If multiple threads report being skipped only the
        // first skip message is used.
        //
        // NOTE: Calling 'SkipWithMessage(...)' does not cause the benchmark to exit
        // the current scope immediately. If the function is called from within
        // the 'KeepRunning()' loop the current iteration will finish. It is the users
        // responsibility to exit the scope as needed.
        void SkipWithMessage(const char* msg);

        // REQUIRES: 'SkipWithMessage(...)' or 'SkipWithError(...)' has not been
        //            called previously by the current thread.
        // Report the benchmark as resulting in an error with the specified 'msg'.
        // After this call the user may explicitly 'return' from the benchmark.
        //
        // If the ranged-for style of benchmark loop is used, the user must explicitly
        // break from the loop, otherwise all future iterations will be run.
        // If the 'KeepRunning()' loop is used the current thread will automatically
        // exit the loop at the end of the current iteration.
        //
        // For threaded benchmarks only the current thread stops executing and future
        // calls to `KeepRunning()` will block until all threads have completed
        // the `KeepRunning()` loop. If multiple threads report an error only the
        // first error message is used.
        //
        // NOTE: Calling 'SkipWithError(...)' does not cause the benchmark to exit
        // the current scope immediately. If the function is called from within
        // the 'KeepRunning()' loop the current iteration will finish. It is the users
        // responsibility to exit the scope as needed.
        void SkipWithError(const char* msg);

        // Returns true if 'SkipWithMessage(...)' or 'SkipWithError(...)' was called.
        bool IsSkipped() const { return skipped_.IsNot(Skipped::NotSkipped); }

        // Returns true if an error has been reported with 'SkipWithError(...)'.
        bool ErrorOccurred() const { return skipped_.Is(Skipped::SkippedWithError); }

        // REQUIRES: called exactly once per iteration of the benchmarking loop.
        // Set the manually measured time for this benchmark iteration, which
        // is used instead of automatically measured time if UseManualTime() was
        // specified.
        //
        // For threaded benchmarks the final value will be set to the largest
        // reported values.
        void SetIterationTime(double seconds);

        // Set the number of bytes processed by the current benchmark
        // execution.  This routine is typically called once at the end of a
        // throughput oriented benchmark.
        //
        // REQUIRES: a benchmark has exited its benchmarking loop.
        inline void SetBytesProcessed(s64 bytes)
        {
            counters_[CounterType::BytesProcessed].value = static_cast<double>(bytes);
            counters_[CounterType::BytesProcessed].flags = {CounterFlags::IsRate | CounterFlags::Is1024};
        }

        inline s64 GetBytesProcessed() const { return (s64)counters_[CounterType::BytesProcessed].value; }

        // If this routine is called with complexity_n > 0 and complexity report is
        // requested for the
        // family benchmark, then current benchmark will be part of the computation
        // and complexity_n will
        // represent the length of N.
        inline void SetComplexityN(s64 complexity_n) { complexity_n_ = complexity_n; }
        inline s64  GetComplexityLengthN() const { return complexity_n_; }

        // If this routine is called with items > 0, then an items/s
        // label is printed on the benchmark report line for the currently
        // executing benchmark. It is typically called at the end of a processing
        // benchmark where a processing items/second output is desired.
        //
        // REQUIRES: a benchmark has exited its benchmarking loop.
        inline void SetItemsProcessed(s64 items)
        {
            counters_[CounterType::ItemsProcessed].value = static_cast<double>(items);
            counters_[CounterType::ItemsProcessed].flags = CounterFlags::IsRate;
        }

        inline s64 GetItemsProcessed() const { return (s64)counters_[CounterType::ItemsProcessed].value; }

        // If this routine is called, the specified label is printed at the
        // end of the benchmark report line for the currently executing
        // benchmark.  Example:
        //  static void BM_Compress(benchmark::BenchMarkState& state) {
        //    ...
        //    double compress = input_size / output_size;
        //    state.SetLabel(StrFormat("compress:%.1f%%", 100.0*compression));
        //  }
        // Produces output that looks like:
        //  BM_Compress   50         50   14115038  compress:27.3%
        //
        // REQUIRES: a benchmark has exited its benchmarking loop.
        void SetLabel(const char* label);

        // Range arguments for this run. CHECKs if the argument has been set.
        inline s64 Range(s32 pos = 0) const
        {
            //            assert(range_.size() > pos);
            return range_[pos];
        }

        // Number of threads concurrently executing the benchmark.
        // inline int threads() const { return threads_; }

        // Index of the executing thread. Values from [0, threads).
        // inline int thread_index() const { return thread_index_; }

        inline IterationCount Iterations() const
        {
            // if (BENCHMARK_BUILTIN_EXPECT(!started_, false))
            if (!started_)
            {
                return 0;
            }
            return max_iterations - total_iterations_ + batch_leftover_;
        }

        inline const char* Name() const { return name_; }

    private:
        // items we expect on the first cache line (ie 64 bytes of the struct)
        // When total_iterations_ is 0, KeepRunning() and friends will return false.
        // May be larger than max_iterations.
        IterationCount total_iterations_;

        // When using KeepRunningBatch(), batch_leftover_ holds the number of
        // iterations beyond max_iters that were run. Used to track
        // completed_iterations_ accurately.
        IterationCount batch_leftover_;

    public:
        const IterationCount max_iterations;

    private:
        bool    started_;
        bool    finished_;
        Skipped skipped_;

        // items we don't need on the first cache line
        s64 range_[4];
        s64 complexity_n_;

    public:
        // Container for user-defined counters.
        Counters counters_;

    private:
        BenchMarkState(const char* name, IterationCount max_iters, s64* ranges, int num_ranges);

        void StartKeepRunning();
        // Implementation of KeepRunning() and KeepRunningBatch().
        // is_batch must be true unless n is 1.
        bool KeepRunningInternal(IterationCount n, bool is_batch);
        void FinishKeepRunning();

        const char* name_;

        // const int   thread_index_;
        // const int   threads_;

        // internal::ThreadTimer* const             timer_;
        // internal::ThreadManager* const           manager_;
        // internal::PerfCountersMeasurement* const perf_counters_measurement_;

        friend class BenchmarkInstance;
    };

} // namespace BenchMark

#endif //__CBENCHMARK_BENCHMARK_STATE_H__
