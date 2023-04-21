#ifdef TARGET_MAC

#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_exception.h"
#include "cbenchmark/private/c_time_helpers.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_stdout.h"
#include "cbenchmark/private/c_utils.h"

#include <exception>

namespace BenchMark
{
    class AssertException : public std::exception
    {
        enum ESettings
        {
            DESCRIPTION_MAX_STR_LENGTH = 512,
            FILENAME_MAX_STR_LENGTH    = 256
        };

    public:
        AssertException(char const* description, char const* filename, const int lineNumber);

        char mDescription[DESCRIPTION_MAX_STR_LENGTH];
        char mFilename[FILENAME_MAX_STR_LENGTH];
        int  mLineNumber;
    };

    AssertException::AssertException(char const* description, char const* filename, const int lineNumber)
        : mLineNumber(lineNumber)
    {
        gStringCopy(mDescription, description, FILENAME_MAX_STR_LENGTH);
        gStringCopy(mFilename, filename, DESCRIPTION_MAX_STR_LENGTH);
    }

	void ReportAssert(char const* description, char const* filename, int const lineNumber)
	{
		UT_THROW1(AssertException(description, filename, lineNumber));
	}

} // namespace BenchMark

#endif
