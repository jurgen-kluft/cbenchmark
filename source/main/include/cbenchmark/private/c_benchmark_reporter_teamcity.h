#ifndef __CBENCHMARK_REPORTER_TEAMCITY_H__
#define __CBENCHMARK_REPORTER_TEAMCITY_H__

#include "cbenchmark/private/c_benchMark_reporter.h"

namespace BenchMark
{
	class BenchMarkReporterTeamCity : public BenchMarkReporter
	{
		enum EConfig
		{
			MAX_MESSAGE_LENGTH = 1023
		};

	public:
						BenchMarkReporterTeamCity();

	private:
		virtual void	reportBenchMarkSuiteStart(int numBenchMarks, char const* name);
		virtual void	reportBenchMarkSuiteEnd(char const* name, float secondsElapsed);
		virtual void	reportBenchMarkFixtureStart(int numBenchMarks, char const* name);
		virtual void	reportBenchMarkFixtureEnd(char const* name, float secondsElapsed);
		virtual void	reportBenchMarkStart(char const* name);
		virtual void	reportBenchMarkEnd(char const* name, float secondsElapsed);
		virtual void	reportFailure(char const* file, int const line, char const* name, char const* failure);
		virtual void	reportSummary(float secondsElapsed, int failureCount, int testCount);

		void			StringFormat(const char* inFormatStr, const char* inName);
		void			StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1, const char* inStr2, int inValue);
		void			StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1, const char* inStr2, const char* inStr3);

		void			Trace(const char* inMessage);

		char			mMessage[MAX_MESSAGE_LENGTH+1];
	};

}


#endif	///< __CBENCHMARK_TESTREPORTER_TEAMCITY_H__
