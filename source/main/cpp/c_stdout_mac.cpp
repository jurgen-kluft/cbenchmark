#if defined(TARGET_MAC)

#include "cbenchmark/private/c_stdout.h"

#include <time.h>
#include <stdio.h>

#define MESSAGE_S(str, size)	str
#define STRINGFORMAT			sprintf				// Here you can divert to a printf/string-formatting implementation

namespace BenchMark
{
	void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, float inValue)
	{
		STRINGFORMAT(MESSAGE_S(inMessage, inMessageSizeInBytes), inFormatStr, inValue);
	}

	void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, int inValue)
	{
		STRINGFORMAT(MESSAGE_S(inMessage, inMessageSizeInBytes), inFormatStr, inValue);
	}

	void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, int inValue, int inValue2)
	{
		STRINGFORMAT(MESSAGE_S(inMessage, inMessageSizeInBytes), inFormatStr, inValue, inValue2);
	}

	void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, int inValue, const char* inName)
	{
		STRINGFORMAT(MESSAGE_S(inMessage, inMessageSizeInBytes), inFormatStr, inValue, inName);
	}

	void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, const char* inFile, int inLine, const char* inBenchMarkName, const char* inFailure)
	{
		STRINGFORMAT(MESSAGE_S(inMessage, inMessageSizeInBytes), inFormatStr, inFile, inLine, inBenchMarkName, inFailure);
	}

	void			Stdout::StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1)
	{
		STRINGFORMAT(MESSAGE_S(outMessage, inMessageSizeInBytes), inFormatStr, inStr1);
	}

	void			Stdout::StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1, const char* inStr2, int inValue)
	{
		STRINGFORMAT(MESSAGE_S(outMessage, inMessageSizeInBytes), inFormatStr, inStr1, inStr2, inValue);
	}

	void			Stdout::StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1, const char* inStr2, const char* inStr3)
	{
		STRINGFORMAT(MESSAGE_S(outMessage, inMessageSizeInBytes), inFormatStr, inStr1, inStr2, inStr3);
	}


	void Stdout::Trace(const char* inMessage)
	{
		printf("%s", inMessage);
	}
}


#endif
