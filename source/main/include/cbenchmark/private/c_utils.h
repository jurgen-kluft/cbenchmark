#ifndef __CBENCHMARK_UTILS_H__
#define __CBENCHMARK_UTILS_H__

#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_types.h"

namespace BenchMark
{
    extern void        gStringCopy(char* dst, const char* src, int max);
    extern const char* gStringFind(const char* src, const char* findstr);
    extern char*       gStringFind(char* src, const char* findstr);
    extern int         gStringLength(const char* str);
    extern s32         gCompareStrings(char const* const expected, char const* const actual);
    extern bool        gAreStringsEqual(char const* const expected, char const* const actual);
    extern char*       gStringToEnd(char* src);

    extern char* gStringAppendTerminator(char* dst, const char* dstEnd);
    extern char* gStringAppend(char* dst, const char* dstEnd, const char* src);
    extern char* gStringAppend(char* dst, const char* dstEnd, char src);
    extern char* gStringAppend2(char* str, char const* strEnd, const char* str1, const char* str2);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, const char* p);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, char c);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, short s);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, int i);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, long l);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, long long l);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned char c);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned short s);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned int i);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned long i);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned long long i);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, float const f);
    extern char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, double const d);

    inline int   gRoundUpToMultipleOfPow2Number(int n, int pow2Number) { return (n + (pow2Number - 1)) & ~(pow2Number - 1); }
    extern char* gHumanReadableNumber(char* dst, const char* dstEnd, double n, double one_k);
    extern void  gSetWidthFormat(char* format, int width);
    extern char* gFormatTime(double time, char* str, const char* str_end);
    extern bool  gIsZero(double n);
} // namespace BenchMark

#endif // __CBENCHMARK_UTILS_H__
