#include "StdAfx.h"
#include "DDMutex.h"


CDDMutex::CDDMutex(void)
{
	InitializeCriticalSection(&m_cs);
}


CDDMutex::~CDDMutex(void)
{
	DeleteCriticalSection(&m_cs);
}

void CDDMutex::Lock()
{
	EnterCriticalSection(&m_cs);
}

void CDDMutex::Unlock()
{
	LeaveCriticalSection(&m_cs);
}