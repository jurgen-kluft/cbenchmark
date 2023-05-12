#ifdef TARGET_PC

#include "cbenchmark/private/c_utils.h"

#include <stdio.h>
#include <cstdio>

namespace BenchMark
{
#    define DEST_S(dest, destEnd) dest, (int)(dstEnd - dest)

	char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, const char* p) { int a = sprintf_s(DEST_S(dest, dstEnd), format, p); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, char c) { int a = sprintf_s(DEST_S(dest, dstEnd), format, c); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, short s) { int a = sprintf_s(DEST_S(dest, dstEnd), format, s); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, int i) { int a = sprintf_s(DEST_S(dest, dstEnd), format, i); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, long l) { int a = sprintf_s(DEST_S(dest, dstEnd), format, l); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, long long l) { int a = sprintf_s(DEST_S(dest, dstEnd), format, l); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned char c) { int a = sprintf_s(DEST_S(dest, dstEnd), format, c); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned short s) { int a = sprintf_s(DEST_S(dest, dstEnd), format, s); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned int i) { int a = sprintf_s(DEST_S(dest, dstEnd), format, i); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned long l) { int a = sprintf_s(DEST_S(dest, dstEnd), format, l); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, unsigned long long l) { int a = sprintf_s(DEST_S(dest, dstEnd), format, l); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, float const f) { int a = sprintf_s(DEST_S(dest, dstEnd), format, f); return dest + a; }
    char* gStringFormatAppend(char* dest, const char* dstEnd, const char* format, double const d) { int a = sprintf_s(DEST_S(dest, dstEnd), format, d); return dest + a; }


}

#endif
