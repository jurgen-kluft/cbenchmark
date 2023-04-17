#include "cbenchmark/private/c_checks.h"
#include "cbenchmark/private/c_utils.h"
#include "cbenchmark/private/c_config.h"

namespace BenchMark
{
	namespace
	{
		void gCheckStringsEqual(BenchMarkResults& results, char const* const expected, char const* const actual, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
		{
			if (gAreStringsEqual(expected, actual) == false)
			{
				BenchMark::StringBuilder stringBuilder(allocator);
				stringBuilder << "Expected " << expected << " but was " << actual;
				results.onBenchMarkFailure(filename, line, testName, stringBuilder.getText());
			}
		}
	}

	void checkEqual(BenchMarkResults& results, char const* const expected, char const* const actual, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
	{
		gCheckStringsEqual(results, expected, actual, testName, filename, line, allocator);
	}

	void checkEqual(BenchMarkResults& results, char* const expected, char* const actual, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
	{
		gCheckStringsEqual(results, expected, actual, testName, filename, line, allocator);
	}

	void checkEqual(BenchMarkResults& results, char* const expected, char const* const actual, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
	{
		gCheckStringsEqual(results, expected, actual, testName, filename, line, allocator);
	}

	void checkEqual(BenchMarkResults& results, char const* const expected, char* const actual, char const* const testName, char const* const filename, int const line, BenchMarkAllocator* allocator)
	{
		gCheckStringsEqual(results, expected, actual, testName, filename, line, allocator);
	}
}
