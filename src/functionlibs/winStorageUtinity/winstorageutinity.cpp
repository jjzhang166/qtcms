#include "winstorageutinity.h"
#include <guid.h>
#include "winfunctions.h"

winStorageUtinity::winStorageUtinity() :
m_nRef(0)
{

}

winStorageUtinity::~winStorageUtinity()
{

}

bool winStorageUtinity::GetFreeSpace( QString path, quint64 &freeByteAvailable, quint64 & totalNumberOfBytes, quint64 TotalNumberOfFreeBytes )
{
	return wfGetDiskFreeSpaceEx(path.toAscii().data(),freeByteAvailable,TotalNumberOfFreeBytes,TotalNumberOfFreeBytes);
}

long __stdcall winStorageUtinity::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IStorage == iid)
	{
		*ppv = static_cast<IStorage *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall winStorageUtinity::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall winStorageUtinity::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}
