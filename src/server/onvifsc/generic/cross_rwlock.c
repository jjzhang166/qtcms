#include "stdinc.h"
#include "cross.h"
#include "cross_rwlock.h"

HRWLOCK rwlock_create()
{
#ifdef _COS_WIN
	CRITICAL_SECTION * cs = malloc(sizeof(CRITICAL_SECTION));
	InitializeCriticalSection(cs);
#else
	pthread_rwlock_t * cs = malloc(sizeof(pthread_rwlock_t));
	pthread_rwlock_init(cs,NULL);
#endif
	return (HRWLOCK)cs;
}

void rwlock_destroy(HRWLOCK rwlock)
{
#ifdef _COS_WIN
	CRITICAL_SECTION * cs = (CRITICAL_SECTION *)rwlock;
	DeleteCriticalSection(cs);
	free(cs);
#else
	pthread_rwlock_t * cs = (pthread_rwlock_t *)rwlock;
	pthread_rwlock_destroy(cs);
	free(cs);
#endif
}

int rwlock_rdlock( HRWLOCK rwlock )
{
#ifdef WIN32
	CRITICAL_SECTION * cs = (CRITICAL_SECTION *)rwlock;
	EnterCriticalSection(cs);
	return 0;
#else
	pthread_rwlock_t * cs = (pthread_rwlock_t *)rwlock;
	return pthread_rwlock_rdlock(cs);
#endif
}

int rwlock_wrlock( HRWLOCK rwlock )
{
#ifdef WIN32
	CRITICAL_SECTION * cs = (CRITICAL_SECTION *)rwlock;
	EnterCriticalSection(cs);
	return 0;
#else
	pthread_rwlock_t * cs = (pthread_rwlock_t *)rwlock;
	return pthread_rwlock_wrlock(cs);
#endif
}

int rwlock_unlock( HRWLOCK rwlock )
{
#ifdef WIN32
	CRITICAL_SECTION * cs = (CRITICAL_SECTION *)rwlock;
	LeaveCriticalSection(cs);
	return 0;
#else
	pthread_rwlock_t * cs = (pthread_rwlock_t *)rwlock;
	return pthread_rwlock_unlock(cs);
#endif
}
