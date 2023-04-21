#include "cbenchmark/private/c_utils.h"

namespace BenchMark
{
    void gStringCopy(char* dst, const char* src, int max)
    {
        int i = 0;
        while (i < max && src[i] != 0)
        {
            dst[i] = src[i];
            ++i;
        }

        if (i < max)
            dst[i] = '\0';
    }

    const char* gStringFind(const char* src, const char* findstr)
    {
        if (*findstr == '\0')
            return src;

        for (; *src != '\0'; ++src)
        {
            if (*src == *findstr)
            {
                /// Matched starting char -- loop through remaining chars.
                const char *h, *n;
                for (h = src, n = findstr; *h != '\0' && *n != '\0'; ++h, ++n)
                {
                    if (*h != *n)
                        break;
                }

                if (*n == '\0') ///< matched all of 'needle' to null termination
                    return src;
            }
        }
        return 0;
    }

    char* gStringFind(char* src, const char* findstr)
    {
        if (*findstr == '\0')
            return src;

        for (; *src != '\0'; ++src)
        {
            if (*src == *findstr)
            {
                /// Matched starting char -- loop through remaining chars.
                const char *h, *n;
                for (h = src, n = findstr; *h != '\0' && *n != '\0'; ++h, ++n)
                {
                    if (*h != *n)
                        break;
                }

                if (*n == '\0') ///< matched all of 'needle' to null termination
                    return src;
            }
        }
        return 0;
    }

    int gStringLength(const char* str)
    {
        int i = 0;
        while (str[i] != 0)
            i++;
        return i;
    }

    s32 gCompareStrings(char const* expected, char const* actual)
    {
        if (expected == actual)
            return 0;
        if (expected == nullptr && actual != nullptr)
            return -1;
        if (expected != nullptr && actual == nullptr)
            return 1;

        char e, a;
        do
        {
            e = *expected++;
            a = *actual++;
            if (e < a)
                return -1;
            else if (e > a)
                return 1;
        } while (e != '\0' && a != '\0');
        return 0;
    }

    bool gAreStringsEqual(char const* expected, char const* actual) { return gCompareStrings(expected, actual) == 0; }

} // namespace BenchMark
