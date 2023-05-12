#ifdef TARGET_MAC

#    include "cbenchmark/private/c_utils.h"

#    include <stdio.h>
#    include <cstdio>

namespace BenchMark
{
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, const char* p) { int a = snprintf(dst, dstEnd-dst, format, p); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, char c) { int a = snprintf(dst, dstEnd-dst, format, c); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, short s) { int a = snprintf(dst, dstEnd-dst, format, s); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, int i) { int a = snprintf(dst, dstEnd-dst, format, i); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, long l) { int a = snprintf(dst, dstEnd-dst, format, l); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, long long l) { int a = snprintf(dst, dstEnd-dst, format, l); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, unsigned char c) { int a = snprintf(dst, dstEnd-dst, format, c); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, unsigned short s) { int a = snprintf(dst, dstEnd-dst, format, s); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, unsigned int i) { int a = snprintf(dst, dstEnd-dst, format, i); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, unsigned long l) { int a = snprintf(dst, dstEnd-dst, format, l); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, unsigned long long l) { int a = snprintf(dst, dstEnd-dst, format, l); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, float const f) { int a = snprintf(dst, dstEnd-dst, format, f); return dst + a; }
    char* gStringFormatAppend(char* dst, const char* dstEnd, const char* format, double const d) { int a = snprintf(dst, dstEnd-dst, format, d); return dst + a; }

} // namespace BenchMark

#endif
