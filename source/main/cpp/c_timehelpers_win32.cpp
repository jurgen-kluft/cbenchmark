#ifdef TARGET_PC

#include "cbenchmark/private/c_time_helpers.h"

#include <windows.h>
#include <cassert>

namespace BenchMark
{
	static LARGE_INTEGER s_frequency;
	static LARGE_INTEGER s_base;

	void g_InitTimer()
	{
		BOOL success = ::QueryPerformanceFrequency(&s_frequency);
		assert(success);
		success = ::QueryPerformanceCounter(&s_base);
		assert(success);
		s_frequency.QuadPart = s_frequency.QuadPart;
		(void) success;
	}

	time_t g_TimeStart()
	{
		LARGE_INTEGER current;
		BOOL const success = ::QueryPerformanceCounter(&current);
		assert(success);
		(void) success;
        current.QuadPart -= s_base.QuadPart;
        return *(time_t*)(&current.QuadPart);
	}

	double g_GetElapsedTimeInMs(time_t stamp)
	{
        LARGE_INTEGER baseTime = s_base;
        baseTime.QuadPart      = baseTime.QuadPart + ((LARGE_INTEGER&)(stamp)).QuadPart;

		LARGE_INTEGER curTime;
		BOOL const success = ::QueryPerformanceCounter(&curTime);
		assert(success);
		(void) success;

		LARGE_INTEGER elapsedTime;
		elapsedTime.QuadPart = curTime.QuadPart - baseTime.QuadPart;

		double const ms = double(elapsedTime.QuadPart) * 1000.0 / (double)s_frequency.QuadPart;
		return ms;
	}

	void g_SleepMs(int const ms)
	{
		::Sleep(ms);
	}

}

#endif
