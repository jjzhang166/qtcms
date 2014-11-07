#include "stringfunction.h"
#include <qbytearray.h>

char * strcasestr_c( char * haystack,char *needle )
{
	uint sizeHay = qstrlen(haystack);
	uint sizeNeedle = qstrlen(needle);
	int nloop = sizeHay - sizeNeedle + 1;
	int nomatch = 1;
	if (nloop > 0)
	{
		while (nomatch = qstrnicmp(haystack,needle,sizeNeedle) && nloop --)
		{
			haystack ++;
		}
	}

	if (nomatch)
	{
		return NULL;
	}
	else
	{
		return (char *)haystack;
	}
}

