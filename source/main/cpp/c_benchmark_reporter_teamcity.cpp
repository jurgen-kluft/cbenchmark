#include "cbenchmark/private/c_benchmark_reporter_teamcity.h"
#include "cbenchmark/private/c_stdout.h"

namespace BenchMark
{
	// TeamCity BenchMark Format:
	// 
	// Property values need to be escaped as follows:
	//         \n ==> |n
	//         \r ==> |n
	//         \' ==> |'
	//         |  ==> ||
	//         ]  ==> |]
	// 
	// Location is expected to be in the following format:
	//         ==> Filename:LineNumber, e.g. Application.cpp:10
	// 
	// SuiteStarted(msg="testSuiteStarted", name) 
	//         ==> "\n##teamcity[msg name='_name']\n"
	// 
	// SuiteFinished(msg="testSuiteFinished", name) 
	//         ==> "\n##teamcity[msg name='_name']\n"
	// 
	// BenchMarkStarted(msg="testStarted", _name) 
	//         ==> "\n##teamcity[msg name='_name']\n"
	// 
	// BenchMarkFinished(msg="testFinished", _name) 
	//         ==> "\n##teamcity[msg name='_name']\n"
	// 
	// BenchMarkFailed(msg="testFailed", _name, _message, _details) 
	//         ==> "\n##teamcity[msg name='_name' message='_messsage' details='_details']\n"
	// 
	// BenchMarkIgnored(msg="testIgnored", _name, _message) 
	//         ==> "\n##teamcity[msg name='_name' message='_messsage']\n"

	void gHandleEscapedChars(const char* src, char* dst)
	{
		while (*src != '\0')
		{
			char c = *src++;
	        
			switch (c)
			{
				case '\n': *dst++='|'; *dst++='n'; break;
				case '\r': *dst++='|'; *dst++='r'; break;
				case '\'': *dst++='|'; break;
				case '|':  *dst++='|'; *dst++='|'; break;
				case ']':  *dst++='|'; *dst++=']'; break;
				default:   *dst++=c; break;
			}
		}
		*dst++ = '\0';
	}


	BenchMarkReporterTeamCity::BenchMarkReporterTeamCity()
	{
	}

	void BenchMarkReporterTeamCity::reportBenchMarkSuiteStart(int numBenchMarks, char const* name)
	{
		char escapedName[256];
		gHandleEscapedChars(name, escapedName);
		StringFormat("\n##teamcity[testSuiteStarted name='%s']\n", escapedName);
		Trace(mMessage);
	}

	void BenchMarkReporterTeamCity::reportBenchMarkSuiteEnd(char const* name, float secondsElapsed)
	{
		char escapedName[256];
		gHandleEscapedChars(name, escapedName);
		StringFormat("\n##teamcity[testSuiteFinished name='%s']\n", escapedName);
		Trace(mMessage);
	}

	void BenchMarkReporterTeamCity::reportBenchMarkFixtureStart(int numBenchMarks, char const* name)
	{
		char escapedName[256];
		gHandleEscapedChars(name, escapedName);
		StringFormat("\n##teamcity[testFixtureStarted name='%s']\n", escapedName);
		Trace(mMessage);
	}

	void BenchMarkReporterTeamCity::reportBenchMarkFixtureEnd(char const* name, float secondsElapsed)
	{
		char escapedName[256];
		gHandleEscapedChars(name, escapedName);
		StringFormat("\n##teamcity[testFixtureFinished name='%s']\n", escapedName);
		Trace(mMessage);
	}

	void BenchMarkReporterTeamCity::reportBenchMarkStart(char const* name)
	{
		char escapedName[256];
		gHandleEscapedChars(name, escapedName);
		StringFormat("\n##teamcity[testStarted name='%s']\n", escapedName);
		Trace(mMessage);
	}

	void BenchMarkReporterTeamCity::reportBenchMarkEnd(char const* name, float)
	{
		char escapedName[256];
		gHandleEscapedChars(name, escapedName);
		StringFormat("\n##teamcity[testFinished name='%s']\n", escapedName);
		Trace(mMessage);
	}

	void BenchMarkReporterTeamCity::reportFailure(char const* file, int const line, char const* name, char const* failure)
	{
		char details[256];
		StringFormat((char*)details, sizeof(details)-1, "test \"%s\" failed at %s:%d", failure, file, line);

		char escapedName[256];
		gHandleEscapedChars(name, escapedName);
		char escapedFailure[256];
		gHandleEscapedChars(failure, escapedFailure);
		char escapedDetails[256];
		gHandleEscapedChars(details, escapedDetails);

		StringFormat(mMessage, MAX_MESSAGE_LENGTH, "\n##teamcity[testFailed name='%s' message='%s' details='%s']\n", escapedName, escapedFailure, escapedDetails);
		Trace(mMessage);
	}

	void BenchMarkReporterTeamCity::reportSummary(float secondsElapsed, int failureCount, int testCount)
	{
	}

	void BenchMarkReporterTeamCity::StringFormat(const char* inFormatStr, const char* inName)
	{
		Stdout::StringFormat(mMessage, MAX_MESSAGE_LENGTH, inFormatStr, inName);
	}

	void BenchMarkReporterTeamCity::StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1, const char* inStr2, int inValue)
	{
		Stdout::StringFormat(outMessage, inMaxMessageLength, inFormatStr, inStr1, inStr2, inValue);
	}

	void BenchMarkReporterTeamCity::StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1, const char* inStr2, const char* inStr3)
	{
		Stdout::StringFormat(outMessage, inMaxMessageLength, inFormatStr, inStr1, inStr2, inStr3);
	}

	void BenchMarkReporterTeamCity::Trace(const char* inMessage)
	{
		Stdout::Trace(inMessage);
	}
}
