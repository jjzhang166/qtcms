#include "stdinc.h"
#include "cross_thread.h"


extern THREAD_ID_t currentThreadId_c()
{
	THREAD_ID_t ret;
#ifdef WIN32
	ret = GetCurrentThreadId();
#else
	ret = pthread_self();
#endif
	return ret;
}

extern int initThread_c( THREAD_HANDLE *hThread,THREAD_PROC proc,void *ThreadParam)
{
#ifdef WIN32
	THREAD_HANDLE hThreadTemp;
	DWORD dwThreadId;
	hThreadTemp = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)proc,ThreadParam,0,&dwThreadId);
	if (NULL == hThreadTemp)
	{
		*hThread = NULL;
		return -1;
	}
	*hThread = hThreadTemp;
#else
	int nRet = pthread_create(hThread,NULL,proc,ThreadParam);
	if (0 != nRet)
	{
		*hThread = 0;
		return -1;
	}
#endif

	return 0;
}

extern int joinThread_c( THREAD_HANDLE hThread )
{
#ifdef WIN32
	WaitForSingleObject(hThread,INFINITE);
#else
	pthread_join(hThread,NULL);
#endif
	return 0;
}
