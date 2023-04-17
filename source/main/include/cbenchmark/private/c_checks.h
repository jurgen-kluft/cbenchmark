#ifndef __CBENCHMARK_CHECKS_H__
#define __CBENCHMARK_CHECKS_H__

#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_stringbuilder.h"

namespace BenchMark
{
	inline bool check(bool value, bool expected)
	{
		return value == expected;
	}

	class BenchMarkAllocator;

	template< typename Expected, typename Actual >
	void checkEqual(BenchMarkResults& results, Expected const& expected, Actual const& actual, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
	{
		if (!(expected == actual))
		{
			BenchMark::StringBuilder stringBuilder(allocator);
			stringBuilder << "Expected ";
			stringBuilder << expected;
			stringBuilder << " but was ";
			stringBuilder << actual;
			results.onBenchMarkFailure(filename, line, testName, stringBuilder.getText());
		}
	}

	template< typename Expected, typename Actual >
	void checkNotEqual(BenchMarkResults& results, Expected const& expected, Actual const& actual, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
	{
		if (expected == actual)
		{
			BenchMark::StringBuilder stringBuilder(allocator);
			stringBuilder << "Expected ";
			stringBuilder << expected;
			stringBuilder << " but was ";
			stringBuilder << actual;
			results.onBenchMarkFailure(filename, line, testName, stringBuilder.getText());
		}
	}

	void checkEqual(BenchMarkResults& results, char const* expected, char const* actual, char const* testName, char const* filename, int line, BenchMarkAllocator* allocator);
	void checkEqual(BenchMarkResults& results, char* expected, char* actual, char const* testName, char const* filename, int line, BenchMarkAllocator* allocator);
	void checkEqual(BenchMarkResults& results, char* expected, char const* actual, char const* testName, char const* filename, int line, BenchMarkAllocator* allocator);
	void checkEqual(BenchMarkResults& results, char const* expected, char* actual, char const* testName, char const* filename, int line, BenchMarkAllocator* allocator);

	template< typename Expected, typename Actual, typename Tolerance >
	bool areClose(Expected const& expected, Actual const& actual, Tolerance const tolerance)
	{
		return (actual >= expected - tolerance) && (actual <= expected + tolerance);
	}

	template< typename Expected, typename Actual, typename Tolerance >
	void checkClose(BenchMarkResults& results, Expected const& expected, Actual const& actual, Tolerance const& tolerance, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
	{
		if (!areClose(expected, actual, tolerance))
		{ 
			BenchMark::StringBuilder stringBuilder(allocator);
			stringBuilder << "Expected " << expected << " +/- " << tolerance << " but was " << actual;
			results.onBenchMarkFailure(filename, line, testName, stringBuilder.getText());
		}
	}

	template< typename Expected, typename Actual >
	void checkArrayEqual(BenchMarkResults& results, Expected const& expected, Actual const& actual, int const count, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
	{
		bool equal = true;
		for (int i = 0; i < count; ++i)
			equal &= (expected[i] == actual[i]);

		if (!equal)
		{
			BenchMark::StringBuilder stringBuilder(allocator);
			stringBuilder << "Expected [ ";
			for (int i = 0; i < count; ++i)
				stringBuilder << expected[i] << " ";
			stringBuilder << "] but was [ ";
			for (int i = 0; i < count; ++i)
				stringBuilder << actual[i] << " ";
			stringBuilder << "]";
			results.onBenchMarkFailure(filename, line, testName, stringBuilder.getText());
		}
	}

	template< typename Expected, typename Actual, typename Tolerance >
	bool checkClose2(Expected const& expected, Actual const& actual, Tolerance const tolerance)
	{
		return ((actual >= expected - tolerance) && (actual <= expected + tolerance));
	}

	template< typename Expected, typename Actual, typename Tolerance >
	void checkArrayClose(BenchMarkResults& results, Expected const& expected, Actual const& actual, int const count, Tolerance const& tolerance, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
	{
		bool equal = true;
		for (int i = 0; i < count; ++i)
			equal &= areClose(expected[i], actual[i], tolerance);

		if (!equal)
		{
			BenchMark::StringBuilder stringBuilder(allocator);
			stringBuilder << "Expected [ ";    
			for (int i = 0; i < count; ++i)
				stringBuilder << expected[i] << " ";
			stringBuilder << "] +/- " << tolerance << " but was [ ";
			for (int i = 0; i < count; ++i)
				stringBuilder << actual[i] << " ";
			stringBuilder << "]";
			results.onBenchMarkFailure(filename, line, testName, stringBuilder.getText());
		}
	}
}

#endif	///< __CBENCHMARK_CHECKS_H__
