#ifndef __CBENCHMARK_REPORTERSTDOUT_H__
#define __CBENCHMARK_REPORTERSTDOUT_H__

#include "cbenchmark/private/c_benchmark_reporter.h"

namespace BenchMark
{
	class BenchMarkReporterStdout : public BenchMarkReporter
	{
		enum EConfig
		{
			MAX_MESSAGE_LENGTH = 1023
		};

	public:
						BenchMarkReporterStdout();

	private:
		virtual void	reportBenchMarkSuiteStart(int numBenchMarks, char const* name);
		virtual void	reportBenchMarkSuiteEnd(char const* name, float secondsElapsed);
		virtual void	reportBenchMarkFixtureStart(int numBenchMarks, char const* name);
		virtual void	reportBenchMarkFixtureEnd(char const* name, float secondsElapsed);
		virtual void	reportBenchMarkStart(char const* name);
		virtual void	reportBenchMarkEnd(char const* name, float secondsElapsed);
		virtual void	reportFailure(char const* file, int const line, char const* name, char const* failure);
		virtual void	reportSummary(float secondsElapsed, int failureCount, int testCount);

		void			StringFormat(const char* inFormatStr, float inValue);
		void			StringFormat(const char* inFormatStr, int inValue);
		void			StringFormat(const char* inFormatStr, int inValue, int inValue2);
		void			StringFormat(const char* inFormatStr, int inValue, const char* inName);
		void			StringFormat(const char* inFormatStr, const char* inFile, int inLine, const char* inBenchMarkName, const char* inFailure);

		void			Trace(const char* inMessage);

		char			mMessage[MAX_MESSAGE_LENGTH+1];
	};

}


#endif	// __CBENCHMARK_REPORTERSTDOUT_H__
