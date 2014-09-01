#ifndef WINSTORAGEUTINITY_H
#define WINSTORAGEUTINITY_H

#include "winstorageutinity_global.h"
#include <IStorage.h>
#include <QtCore/QMutex>

class winStorageUtinity : public IStorage
{
public:
	winStorageUtinity();
	~winStorageUtinity();

	virtual bool GetFreeSpace( QString path, quint64 &freeByteAvailable, quint64 & totalNumberOfBytes, quint64 TotalNumberOfFreeBytes );

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();

private:
	int m_nRef;
	QMutex m_csRef;

};

#endif // WINSTORAGEUTINITY_H
