#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_enums.h"
#include "cbenchmark/private/c_benchmark_types.h"
#include "cbenchmark/private/c_benchmark_check.h"

#include <cmath>
#include <thread>
#include <atomic>

#include "c_benchmark_thread_mutex.cc"

namespace BenchMark
{
    typedef s64 IterationCount;

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

        struct Result
        {
            IterationCount iterations       = 0;
            double         real_time_used   = 0;
            double         cpu_time_used    = 0;
            double         manual_time_used = 0;
            s64            complexity_n     = 0;
            const char*    report_label_    = nullptr;
            const char*    skip_message_    = nullptr;
            Skipped        skipped_         = {Skipped::NotSkipped};
            Counters       counters;
        };
        GUARDED_BY(GetBenchmarkMutex()) Result results;

    private:
        mutable Mutex    benchmark_mutex_;
        std::atomic<int> alive_threads_;
        Barrier          start_stop_barrier_;
        Mutex            end_cond_mutex_;
        Condition        end_condition_;
    };

    double ProcessCPUUsage() { return 0.0; }  // Return the CPU usage of the current process
    double ChildrenCPUUsage() { return 0.0; } // Return the CPU usage of the children of the current process
    double ThreadCPUUsage() { return 0.0; }   // Return the CPU usage of the current thread

#define HAVE_STEADY_CLOCK 1

    inline double ChronoClockNow()
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
        explicit ThreadTimer(bool measure_process_cpu_time_)
            : measure_process_cpu_time(measure_process_cpu_time_)
        {
        }

    public:
        static ThreadTimer Create() { return ThreadTimer(/*measure_process_cpu_time_=*/false); }
        static ThreadTimer CreateProcessCpuTime() { return ThreadTimer(/*measure_process_cpu_time_=*/true); }

        // Called by each thread
        void StartTimer()
        {
            running_         = true;
            start_real_time_ = ChronoClockNow();
            start_cpu_time_  = ReadCpuTimerOfChoice();
        }

        // Called by each thread
        void StopTimer()
        {
            BM_CHECK(running_);
            running_ = false;
            real_time_used_ += ChronoClockNow() - start_real_time_;

            // Floating point error may result in the subtraction producing a negative time.
            const double cpu_time = ReadCpuTimerOfChoice() - start_cpu_time_;
            if (cpu_time > 0)
                cpu_time_used_ += cpu_time;
        }

        // Called by each thread
        void SetIterationTime(double seconds) { manual_time_used_ += seconds; }

        bool running() const { return running_; }

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
        double ReadCpuTimerOfChoice() const
        {
            if (measure_process_cpu_time)
                return ProcessCPUUsage();
            return ThreadCPUUsage();
        }

        // should the thread, or the process, time be measured?
        const bool measure_process_cpu_time;

        bool   running_         = false; // Is the timer running
        double start_real_time_ = 0;     // If running_
        double start_cpu_time_  = 0;     // If running_

        // Accumulated time so far (does not contain current slice if running_)
        double real_time_used_ = 0;
        double cpu_time_used_  = 0;
        // Manually set iteration time. User sets this with SetIterationTime(seconds).
        double manual_time_used_ = 0;
    };

} // namespace BenchMark