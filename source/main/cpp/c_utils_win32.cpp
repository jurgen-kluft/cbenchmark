#ifdef TARGET_PC

#include "cbenchmark/private/c_utils.h"

#include <stdio.h>
#include <cstdio>

namespace BenchMark
{
	void	gStringPrint(char* dest, int n, const char* format, const void* p)
	{
		sprintf_s(dest, n, format, p);
	}

	void	gStringPrint(char* dest, int n, const char* format, char c)
	{
		sprintf_s(dest, n, format, c);
	}

	void	gStringPrint(char* dest, int n, const char* format, short s)
	{
		sprintf_s(dest, n, format, s);
	}

	void	gStringPrint(char* dest, int n, const char* format, int i)
	{
		sprintf_s(dest, n, format, i);
	}

	void	gStringPrint(char* dest, int n, const char* format, long l)
	{
		sprintf_s(dest, n, format, l);
	}

	void	gStringPrint(char* dest, int n, const char* format, long long l)
	{
		sprintf_s(dest, n, format, l);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned char c)
	{
		sprintf_s(dest, n, format, c);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned short s)
	{
		sprintf_s(dest, n, format, s);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned int i)
	{
		sprintf_s(dest, n, format, i);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned long l)
	{
		sprintf_s(dest, n, format, l);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned long long l)
	{
		sprintf_s(dest, n, format, l);
	}

	void	gStringPrint(char* dest, int n, const char* format, float const f)
	{
		sprintf_s(dest, n, format, f);
	}

}

#endif
