#include "stdinc.h"
#include "cross_lock.h"
HMUTEX mutex_create()
{
#ifdef WIN32
	CRITICAL_SECTION *cs = malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSection(cs);
#else
	pthread_mutex_t * cs = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(cs,NULL);
#endif

	return (HMUTEX)cs;
}

void mutex_destroy( HMUTEX mutex )
{
#ifdef WIN32
	CRITICAL_SECTION * cs = (CRITICAL_SECTION *)mutex;
	DeleteCriticalSection(cs);
#else
	pthread_mutex_t * cs = (pthread_mutex_t*)mutex;
	pthread_mutex_destroy(cs);
#endif
	free(cs);
}

void mutex_lock( HMUTEX mutex )
{
#ifdef WIN32
	CRITICAL_SECTION * cs = (CRITICAL_SECTION *)mutex;
	EnterCriticalSection(cs);
#else
	pthread_mutex_t * cs = (pthread_mutex_t *)mutex;
	pthread_mutex_lock(cs);
#endif

}

void mutex_unlock( HMUTEX mutex )
{
#ifdef WIN32
	CRITICAL_SECTION * cs = (CRITICAL_SECTION *)mutex;
	LeaveCriticalSection(cs);
#else
	pthread_mutex_t * cs = (pthread_mutex_t *)mutex;
	pthread_mutex_unlock(cs);
#endif
}

