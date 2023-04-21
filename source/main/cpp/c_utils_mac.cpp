#ifdef TARGET_MAC

#    include "cbenchmark/private/c_utils.h"

#    include <stdio.h>
#    include <cstdio>

namespace BenchMark
{
#    define DEST_S(d, n) (d)

    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, const void* p) { int a = sprintf(DEST_S(dest, n), format, p); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, char c) { int a = sprintf(DEST_S(dest, n), format, c); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, short s) { int a = sprintf(DEST_S(dest, n), format, s); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, int i) { int a = sprintf(DEST_S(dest, n), format, i); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, long l) { int a = sprintf(DEST_S(dest, n), format, l); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, long long l) { int a = sprintf(DEST_S(dest, n), format, l); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned char c) { int a = sprintf(DEST_S(dest, n), format, c); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned short s) { int a = sprintf(DEST_S(dest, n), format, s); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned int i) { int a = sprintf(DEST_S(dest, n), format, i); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned long l) { int a = sprintf(DEST_S(dest, n), format, l); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned long long l) { int a = sprintf(DEST_S(dest, n), format, l); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, float const f) { int a = sprintf(DEST_S(dest, n), format, f); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, double const d) { int a = sprintf(DEST_S(dest, n), format, d); return dest + a; }

} // namespace BenchMark

#endif
