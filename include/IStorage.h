#ifndef __INTERFACE_STORAGE_HEAD_FILE__
#define __INTERFACE_STORAGE_HEAD_FILE__

#include "libpcom.h"
#include <QtCore/qstring.h>

interface IStorage : public IPComBase 
{
    virtual bool GetFreeSpace(QString path, quint64 &freeByteAvailable, quint64 & totalNumberOfBytes, quint64 TotalNumberOfFreeBytes) = 0;
};

#endif
