#include "unixcommontools.h"
#include <sys/param.h>
#include <sys/mount.h>
#include <guid.h>

UnixCommontools::UnixCommontools() :
    m_nRef(0)
{
}

UnixCommontools::~UnixCommontools()
{

}

long UnixCommontools::QueryInterface(const IID &iid, void **ppv)
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

unsigned long UnixCommontools::AddRef()
{
    m_csRef.lock();
    m_nRef ++;
    m_csRef.unlock();
    return m_nRef;
}

unsigned long UnixCommontools::Release()
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

bool UnixCommontools::GetFreeSpace(QString path, quint64 &freeByteAvailable, quint64 &totalNumberOfBytes, quint64 TotalNumberOfFreeBytes)
{
    struct statfs buf;
    int nRet = statfs(path.toAscii().data(),&buf);
    if (-1 == nRet)
    {
        return false;
    }

    TotalNumberOfFreeBytes = freeByteAvailable = (quint64)buf.f_bavail * (quint64)buf.f_bsize;
    totalNumberOfBytes = (quint64)buf.f_blocks * (quint64)buf.f_bsize;

    return true;
}
