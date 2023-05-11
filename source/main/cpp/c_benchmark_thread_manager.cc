#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_check.h"
#include "cbenchmark/private/c_time_helpers.h"

#include <cmath>
#include <thread>
#include <atomic>

#include "c_benchmark_thread_mutex.cc"

namespace BenchMark
{
    typedef s64 IterationCount;

    // TODO This is more like a ThreadMonitor so should be renamed
    class ThreadManager
    {
    public:
        explicit ThreadManager(int num_threads)
            : alive_threads_(num_threads)
            , start_stop_barrier_(num_threads)
        {
        }

        Mutex& GetBenchmarkMutex() const RETURN_CAPABILITY(benchmark_mutex_) { return benchmark_mutex_; }
        bool   StartStopBarrier() EXCLUDES(end_cond_mutex_) { return start_stop_barrier_.wait(); }
        void   NotifyThreadComplete() EXCLUDES(end_cond_mutex_)
        {
            start_stop_barrier_.removeThread();
            if (--alive_threads_ == 0)
            {
                MutexLock lock(end_cond_mutex_);
                end_condition_.notify_all();
            }
        }

        void WaitForAllThreads() EXCLUDES(end_cond_mutex_)
        {
            MutexLock lock(end_cond_mutex_);
            end_condition_.wait(lock.native_handle(), [this]() { return alive_threads_ == 0; });
        }

    private:
        mutable Mutex    benchmark_mutex_;
        std::atomic<int> alive_threads_;
        Barrier          start_stop_barrier_;
        Mutex            end_cond_mutex_;
        Condition        end_condition_;
    };

    // TODO to be implemented
    static double ProcessCPUUsage() { return 0.0; }  // Return the CPU usage of the current process
    static double ChildrenCPUUsage() { return 0.0; } // Return the CPU usage of the children of the current process
    static double ThreadCPUUsage() { return 0.0; }   // Return the CPU usage of the current thread

#define HAVE_STEADY_CLOCK 1

    static inline double ChronoClockNow()
    {
#if defined(HAVE_STEADY_CLOCK)
        using FpSeconds = std::chrono::duration<double, std::chrono::seconds::period>;
        return FpSeconds(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#else
        using FpSeconds = std::chrono::duration<double, std::chrono::seconds::period>;
        return FpSeconds(std::chrono::steady_clock::now().time_since_epoch()).count();
        return 0;
#endif
    }

    class ThreadTimer
    {
        explicit ThreadTimer()
        {
        }

    public:
        static ThreadTimer Create() { return ThreadTimer(); }

        // Called by each thread
        void StartTimer()
        {
            running_         = true;
            start_real_time_ = ChronoClockNow();
            start_cpu_time_  = g_TimeStart();
        }

        // Called by each thread
        void StopTimer()
        {
            BM_CHECK(running_);
            running_ = false;
            real_time_used_ += ChronoClockNow() - start_real_time_;

            // Floating point error may result in the subtraction producing a negative time.
            const double cpu_time = g_GetElapsedTimeInMs(start_cpu_time_);
            if (cpu_time > 0)
                cpu_time_used_ += cpu_time;
        }

        // Called by each thread
        void SetIterationTime(double seconds) { manual_time_used_ += seconds; }

        bool IsRunning() const { return running_; }

        // REQUIRES: timer is not running
        double real_time_used() const
        {
            BM_CHECK(!running_);
            return real_time_used_;
        }

        // REQUIRES: timer is not running
        double cpu_time_used() const
        {
            BM_CHECK(!running_);
            return cpu_time_used_;
        }

        // REQUIRES: timer is not running
        double manual_time_used() const
        {
            BM_CHECK(!running_);
            return manual_time_used_;
        }

    private:
        bool   running_         = false; // Is the timer running
        double start_real_time_ = 0;     // If running_
        time_t start_cpu_time_  = 0;     // If running_

        // Accumulated time so far (does not contain current slice if running_)
        double real_time_used_ = 0;
        double cpu_time_used_  = 0;

        // Manually set iteration time. User sets this with SetIterationTime(seconds).
        double manual_time_used_ = 0;
    };

} // namespace BenchMark