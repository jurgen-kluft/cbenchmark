#if defined(TARGET_MAC)

#    include "cbenchmark/private/c_stdout.h"

#    include <time.h>
#    include <stdio.h>

#    define STRINGFORMAT snprintf // Here you can divert to a printf/string-formatting implementation

namespace BenchMark
{
    void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, float inValue) { STRINGFORMAT(inMessage, inMessageSizeInBytes, inFormatStr, inValue); }
    void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, int inValue) { STRINGFORMAT(inMessage, inMessageSizeInBytes, inFormatStr, inValue); }
    void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, int inValue, int inValue2) { STRINGFORMAT(inMessage, inMessageSizeInBytes, inFormatStr, inValue, inValue2); }
    void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, int inValue, const char* inName) { STRINGFORMAT(inMessage, inMessageSizeInBytes, inFormatStr, inValue, inName); }
    void Stdout::StringFormat(char* inMessage, int inMessageSizeInBytes, const char* inFormatStr, const char* inFile, int inLine, const char* inBenchMarkName, const char* inFailure)
    {
        STRINGFORMAT(inMessage, inMessageSizeInBytes, inFormatStr, inFile, inLine, inBenchMarkName, inFailure);
    }

    void Stdout::StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1) { STRINGFORMAT(outMessage, inMaxMessageLength, inFormatStr, inStr1); }
    void Stdout::StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1, const char* inStr2, int inValue) { STRINGFORMAT(outMessage, inMaxMessageLength, inFormatStr, inStr1, inStr2, inValue); }
    void Stdout::StringFormat(char* outMessage, int inMaxMessageLength, const char* inFormatStr, const char* inStr1, const char* inStr2, const char* inStr3) { STRINGFORMAT(outMessage, inMaxMessageLength, inFormatStr, inStr1, inStr2, inStr3); }
    void Stdout::Trace(const char* inMessage) { printf("%s", inMessage); }
} // namespace BenchMark

#endif
