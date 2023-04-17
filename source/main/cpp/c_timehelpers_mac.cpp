#ifdef TARGET_MAC

#include "cbenchmark/private/c_time_helpers.h"

#include <assert.h>

#ifdef __APPLE__
#define HAVE_MACH_TIMER
#include <mach/mach_time.h>
#endif

#include <unistd.h>

namespace BenchMark
{
	static uint64_t s_numer;
	static uint64_t s_denom;
	static uint64_t s_base;

	void g_InitTimer()
	{
		mach_timebase_info_data_t rate_nsec;
		mach_timebase_info(&rate_nsec);
		s_numer = rate_nsec.numer;
		s_denom = rate_nsec.denom;
		s_base = mach_absolute_time();
	}

	time_t g_TimeStart()
    {
        uint64_t start = mach_absolute_time() - s_base;
        return *(time_t*)(&start);
	}

	double g_GetElapsedTimeInMs(time_t stamp)
	{
		uint64_t const last = s_base + *(uint64_t*)(&stamp);
		uint64_t const current = mach_absolute_time();
		double const ms = (double)((current - last) * s_denom) / (s_numer * 1000000);
		return ms;
	}

	void g_SleepMs(int const ms)
	{
		usleep(ms * 1000);
	}

}

#endif
