#include "cross.h"

int sleep_c(unsigned long ms)
{
#ifdef _COS_WIN
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
	return 0;
}

int Sleep_c( unsigned long s )
{
#ifdef _COS_WIN
	Sleep(s * 1000);
#else
	sleep(s);
#endif
	return 0;
}

struct tm * localtime_c(time_t * tt,struct tm *t)
{
#ifdef _COS_WIN
	struct tm * ret = localtime(tt);
	*t = *ret;
	return ret;
#else
	return localtime_r(tt,t);
#endif
}

EXTERN_C struct tm * gmtime_c( time_t * tt,struct tm * t )
{
#ifdef _COS_WIN
	struct tm * ret = gmtime(tt);
	*t = *ret;
	return ret;
#else
	return gmtime_r(tt,t);
#endif
}

char * ctime_c( time_t * t,char * str )
{
	char *ret;
#ifdef WIN32
	ret = ctime(t);
	strcpy(str,ret);
#else
	ret = ctime_r(t,str);
#endif
	return ret;
}

char * basename_c( char *path )
{
#ifdef WIN32
	int i;
	for (i = strlen(path) -1;i >= 0; i--)
	{
		if ('\\' == path[i])
		{
			break;
		}
	}

	return (path + i + 1);

#else
	return basename(path);
#endif
}

EXTERN_C char * strdupa_c( char * s )
{
#ifdef WIN32
	char * newStr = (char *)_alloca(strlen(s) + 1);
	strcpy(newStr,s);
	return newStr;
#else
	return strdupa(s);
#endif
}

EXTERN_C void bcopy_c( void * src, void * dest,size_t n )
{
#ifdef WIN32
	memcpy(dest,src,n);
#else
	bcopy(src,dest,n);
#endif
}
