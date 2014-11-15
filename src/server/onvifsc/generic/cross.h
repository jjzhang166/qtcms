#ifndef __CROSS_HEAD_FILE__
#define __CROSS_HEAD_FILE__
#include "stdinc.h"

#ifdef WIN32
#define _COS_WIN
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C    extern
#endif
#endif

#if defined(WIN32) && !defined(__cplusplus)
#define inline __inline

typedef enum {
	false,
	true
}bool;
#endif

#ifdef WIN32
#define STRDUP _strdup
#define SNPRINTF _sntprintf
#define STRCASECMP _stricmp
#define RANDOM rand
typedef HANDLE pthread_t;
typedef unsigned char uint8_t;
typedef INT64 int64_t;
typedef unsigned int uint32_t;
typedef unsigned char u_char;
typedef unsigned short u_short;
#else // _COS_WIN
#define STRDUP strdup
#define SNPRINTF snprintf
#define STRCASECMP strcasecmp
#define RANDOM random
#endif //!defined(_COS_WIN)

EXTERN_C int sleep_c(unsigned long ms);

EXTERN_C int Sleep_c(unsigned long s);

EXTERN_C struct tm * localtime_c(time_t * tt,struct tm *t);

EXTERN_C struct tm * gmtime_c(time_t * tt,struct tm * t);

EXTERN_C char * ctime_c(time_t * t,char * str);

EXTERN_C char * basename_c(char *path);

EXTERN_C char * strdupa_c(char * s);

EXTERN_C void bcopy_c(void * src, void * dest,size_t n);
#endif