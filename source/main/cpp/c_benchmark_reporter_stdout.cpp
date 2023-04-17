#include "cbenchmark/private/c_benchmark_reporter_stdout.h"
#include "cbenchmark/private/c_stdout.h"

namespace BenchMark
{
	BenchMarkReporterStdout::BenchMarkReporterStdout()
	{
	}

	void BenchMarkReporterStdout::reportBenchMarkSuiteStart(int numBenchMarks, char const* name)
	{
		StringFormat("BenchMarkSuite(%d): %s.\n", numBenchMarks, name);
		Trace(mMessage);
	}

	void BenchMarkReporterStdout::reportBenchMarkSuiteEnd(char const* name, float secondsElapsed)
	{
	}

	void BenchMarkReporterStdout::reportBenchMarkFixtureStart(int numBenchMarks, char const* name)
	{
		StringFormat("\tBenchMarkFixture(%d): %s.\n", numBenchMarks, name);
		Trace(mMessage);
	}

	void BenchMarkReporterStdout::reportBenchMarkFixtureEnd(char const* name, float secondsElapsed)
	{
	}

	void BenchMarkReporterStdout::reportBenchMarkStart(char const* name)
	{
	}

	void BenchMarkReporterStdout::reportBenchMarkEnd(char const* name, float secondsElapsed)
	{
	}

	void BenchMarkReporterStdout::reportFailure(char const* file, int const line, char const* testName, char const* failure)
	{
#ifdef TARGET_MAC
		StringFormat("%s:%d: error: Failure in %s:%s.\n", file, line, testName, failure);
#else
		StringFormat("%s(%d): error: Failure in %s:%s.\n", file, line, testName, failure);
#endif
		Trace(mMessage);
	}

	void BenchMarkReporterStdout::reportSummary(float secondsElapsed,int failureCount,int testCount)
	{
		if (failureCount > 0)
			StringFormat("FAILURE: %d out of %d tests failed.\n", failureCount, testCount);
		else
			StringFormat("Success: %d  tests passed.\n", testCount);
		Trace(mMessage);

		StringFormat("BenchMark time: %.2f seconds.\n", secondsElapsed);
		Trace(mMessage);
	}

	void BenchMarkReporterStdout::StringFormat(const char* inFormatStr, float inValue)
	{
		Stdout::StringFormat(mMessage, MAX_MESSAGE_LENGTH, inFormatStr, inValue);
	}

	void BenchMarkReporterStdout::StringFormat(const char* inFormatStr, int inValue)
	{
		Stdout::StringFormat(mMessage, MAX_MESSAGE_LENGTH, inFormatStr, inValue);
	}

	void BenchMarkReporterStdout::StringFormat(const char* inFormatStr, int inValue, int inValue2)
	{
		Stdout::StringFormat(mMessage, MAX_MESSAGE_LENGTH, inFormatStr, inValue, inValue2);
	}

	void BenchMarkReporterStdout::StringFormat(const char* inFormatStr, int inValue, const char* inName)
	{
		Stdout::StringFormat(mMessage, MAX_MESSAGE_LENGTH, inFormatStr, inValue, inName);
	}

	void BenchMarkReporterStdout::StringFormat(const char* inFormatStr, const char* inFile, int inLine, const char* inBenchMarkName, const char* inFailure)
	{
		Stdout::StringFormat(mMessage, MAX_MESSAGE_LENGTH, inFormatStr, inFile, inLine, inBenchMarkName, inFailure);
	}

	void BenchMarkReporterStdout::Trace(const char* inMessage)
	{
		Stdout::Trace(inMessage);
	}
}
