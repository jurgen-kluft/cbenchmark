#ifdef TARGET_MAC

#include "cbenchmark/private/c_utils.h"

#include <stdio.h>
#include <cstdio>

namespace BenchMark
{
	#define DEST_S(d, n)	(d)

	void	gStringPrint(char* dest, int n, const char* format, const void* p)
	{
		sprintf(DEST_S(dest, n), format, p);
	}

	void	gStringPrint(char* dest, int n, const char* format, char c)
	{
		sprintf(DEST_S(dest, n), format, c);
	}

	void	gStringPrint(char* dest, int n, const char* format, short s)
	{
		sprintf(DEST_S(dest, n), format, s);
	}

	void	gStringPrint(char* dest, int n, const char* format, int i)
	{
		sprintf(DEST_S(dest, n), format, i);
	}

	void	gStringPrint(char* dest, int n, const char* format, long l)
	{
		sprintf(DEST_S(dest, n), format, l);
	}

	void	gStringPrint(char* dest, int n, const char* format, long long l)
	{
		sprintf(DEST_S(dest, n), format, l);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned char c)
	{
		sprintf(DEST_S(dest, n), format, c);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned short s)
	{
		sprintf(DEST_S(dest, n), format, s);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned int i)
	{
		sprintf(DEST_S(dest, n), format, i);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned long l)
	{
		sprintf(DEST_S(dest, n), format, l);
	}

	void	gStringPrint(char* dest, int n, const char* format, unsigned long long l)
	{
		sprintf(DEST_S(dest, n), format, l);
	}

	void	gStringPrint(char* dest, int n, const char* format, float const f)
	{
		sprintf(DEST_S(dest, n), format, f);
	}

}

#endif
