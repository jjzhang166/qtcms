#ifndef UNIXCOMMONTOOLS_H
#define UNIXCOMMONTOOLS_H

#include "unixcommontools_global.h"
#include <IStorage.h>
#include <QtCore/qmutex.h>

class UnixCommontools : public IStorage
{

public:
    UnixCommontools();
    virtual ~UnixCommontools();

    // IPcomBase interface
public:
    long QueryInterface(const IID &iid, void **ppv);
    unsigned long AddRef();
    unsigned long Release();

    // IStorage interface
public:
    bool GetFreeSpace(QString path, quint64 &freeByteAvailable, quint64 &totalNumberOfBytes, quint64 TotalNumberOfFreeBytes);

private:
    int m_nRef;
    QMutex m_csRef;
};

#endif // UNIXCOMMONTOOLS_H
