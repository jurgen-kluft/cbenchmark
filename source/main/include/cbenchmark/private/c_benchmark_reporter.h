#ifndef __CBENCHMARK_TESTREPORTER_H__
#define __CBENCHMARK_TESTREPORTER_H__

namespace BenchMark
{
	class BenchMarkReporter
	{
	public:
		virtual			~BenchMarkReporter() {}

		virtual void	reportBenchMarkSuiteStart(int numBenchMarks, char const* name) = 0;
		virtual void	reportBenchMarkSuiteEnd(char const* name, float secondsElapsed) = 0;
		virtual void	reportBenchMarkFixtureStart(int numBenchMarks, char const* name) = 0;
		virtual void	reportBenchMarkFixtureEnd(char const* name, float secondsElapsed) = 0;
		virtual void	reportBenchMarkStart(char const* name) = 0;
		virtual void	reportBenchMarkEnd(char const* name, float secondsElapsed) = 0;
		virtual void	reportFailure(char const* file, int const line, char const* name, char const* failure)= 0;
		virtual void	reportSummary(float secondsElapsed, int failureCount, int testCount) = 0;
	};
}

#endif	///<__XUNITTEST_TESTREPORTER_H__
