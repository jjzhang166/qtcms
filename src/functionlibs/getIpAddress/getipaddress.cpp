#include "getipaddress.h"
#include <QDebug>


getIpAddress::getIpAddress():m_nRef(0)
{

}

bool getIpAddress::getIpAddressEx( const QString sId,QString &sIp,QString &sPort,QString &sHttp )
{
	if (getIpAddressInLan(sId,sIp,sPort,sHttp))
	{
		return true;
	}else if (getIpAddressInWan(sId,sIp,sPort,sHttp))
	{
		return true;
	}else{
		return false;
	}
}

getIpAddress::~getIpAddress()
{

}

long __stdcall getIpAddress::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase==iid)
	{
		*ppv=static_cast<IPcomBase *>(this);
	}else if (IID_IEventRegister==iid)
	{
		*ppv=static_cast<IEventRegister *>(this);
	}else if (IID_IGetIpAddress==iid)
	{
		*ppv=static_cast<IGetIpAddress *>(this);
	}
	else{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall getIpAddress::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall getIpAddress::Release()
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

QStringList getIpAddress::eventList()
{
	return m_sEventList;
}

int getIpAddress::queryEvent( QString eventName,QStringList& eventParams )
{
	Q_UNUSED(eventParams);
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		//fix eventParams
		return IEventRegister::OK;
	}
}

int getIpAddress::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		tagGetIpAddressProInfo tProInfo;
		tProInfo.proc=proc;
		tProInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProInfo);
		return IEventRegister::OK;
	}
}

bool getIpAddress::getIpAddressInLan( const QString sId,QString &sIp,QString &sPort,QString &sHttp )
{
	return m_tGetIpWinSock.getIpAddressInLan(sId,sIp,sPort,sHttp);
}

bool getIpAddress::getIpAddressInWan( const QString sId,QString &sIp,QString &sPort,QString &sHttp )
{
	return m_tGetIpWinSock.getIpAddressInWan(sId,sIp,sPort,sHttp);
}

