#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_stdout.h"

namespace BenchMark
{
    const char* LocalDateTimeString() { return "unknown date and time"; }

    BenchMarkReporter::BenchMarkReporter()
        : output_stream_(nullptr)
        , error_stream_(nullptr)
    {
    }

    BenchMarkReporter::~BenchMarkReporter() {}

    void BenchMarkReporter::PrintBasicContext(TextStreamWriter& out, Context const& context)
    {
        // BM_CHECK(out) << "cannot be null";

        // Date/time information is not available on QuRT.
        // Attempting to get it via this call cause the binary to crash.
        out << LocalDateTimeString() << "\n";

        if (context.executable_name)
            out << "Running " << context.executable_name << "\n";

            // const CPUInfo& info = context.cpu_info;
            // out << "Run on (" << info.num_cpus << " X " << (info.cycles_per_second / 1000000.0) << " MHz CPU " << ((info.num_cpus > 1) ? "s" : "") << ")\n";
            // if (info.caches.size() != 0)
            // {
            //     out << "CPU Caches:\n";
            //     for (auto& CInfo : info.caches)
            //     {
            //         out << "  L" << CInfo.level << " " << CInfo.type << " " << (CInfo.size / 1024) << " KiB";
            //         if (CInfo.num_sharing != 0)
            //             out << " (x" << (info.num_cpus / CInfo.num_sharing) << ")";
            //         out << "\n";
            //     }
            // }

            // if (!info.load_avg.empty())
            // {
            //     out << "Load Average: ";
            //     for (auto It = info.load_avg.begin(); It != info.load_avg.end();)
            //     {
            //         out << StrFormat("%.2f", *It++);
            //         if (It != info.load_avg.end())
            //             out << ", ";
            //     }
            //     out << "\n";
            // }

            // std::map<std::string, std::string>* global_context = internal::GetGlobalContext();

            // if (global_context != nullptr)
            // {
            //     for (const auto& kv : *global_context)
            //     {
            //         out << kv.first << ": " << kv.second << "\n";
            //     }
            // }

            // if (CPUInfo::Scaling::ENABLED == info.scaling)
            // {
            //     out << "***WARNING*** CPU scaling is enabled, the benchmark "
            //            "real time measurements may be noisy and will incur extra "
            //            "overhead.\n";
            // }

#ifndef NDEBUG
        out << "***WARNING*** Library was built as DEBUG. Timings may be affected.\n";
#endif
    }

} // namespace BenchMark
