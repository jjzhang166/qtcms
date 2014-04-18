#include "winipcsearch.h"

WinIpcSearch::WinIpcSearch():m_nRef(0),
	m_nTimeInterval(10),
	m_bFlush(false),
	m_bStop(true)
{
	 m_sEventList.insert(0, QString("SearchDeviceSuccess"));
	 m_sEventList.insert(1, QString("SettingStatus"));
}

WinIpcSearch::~WinIpcSearch()
{

}

long __stdcall WinIpcSearch::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IDeviceSearch == iid)
	{
		*ppv = static_cast<IDeviceSearch*>(this);
	}
	else if (IID_IEventRegister == iid)
	{
		*ppv = static_cast<IEventRegister*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall WinIpcSearch::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall WinIpcSearch::Release()
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

int WinIpcSearch::Start()
{
	m_WinIpcSock.Start();
	return 0;
}

int WinIpcSearch::Stop()
{
	return m_WinIpcSock.Stop();
}

int WinIpcSearch::Flush()
{
	return m_WinIpcSock.Flush();
}

int WinIpcSearch::setInterval( int nInterval )
{
	return m_WinIpcSock.setInterval(nInterval);
}

IEventRegister * WinIpcSearch::QueryEventRegister()
{
	IEventRegister * ret ;
	QueryInterface(IID_IEventRegister,(void **)&ret);
	return ret; 
}

QStringList WinIpcSearch::eventList()
{
	return m_sEventList;
}

int WinIpcSearch::queryEvent( QString eventName, QStringList& eventParamList )
{
	if (!m_sEventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	if ( "SearchDeviceSuccess" == eventName)
	{
		eventParamList<<"SearchVendor_ID"  <<"SearchDeviceName_ID"  <<"SearchDeviceId_ID"     <<"SearchDeviceModelId_ID"
			<<"SearchSeeId_ID"   <<"SearchChannelCount_ID"<<"SearchIP_ID"           <<"SearchMask_ID"
			<<"SearchMac_ID"     <<"SearchGateway_ID"     <<"SearchHttpport_ID"     <<"SearchMediaPort_ID";
	}
	return IEventRegister::OK;
	return IEventRegister::OK;
}

int WinIpcSearch::registerEvent( QString eventName,EventCallBack eventCB,void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	EventCBInfo procInfo;
	procInfo.evCBName = eventCB;
	procInfo.pUser = pUser;
	m_mEventCBMap.insert(eventName, procInfo);
	m_WinIpcSock.registerEvent(eventName,eventCB,pUser);
	return IEventRegister::OK;
}
void WinIpcSearch::eventProcCall(QString sEvent,QVariantMap param)
{
	if (m_sEventList.contains(sEvent))
	{
		EventCBInfo eventDes = m_mEventCBMap.value(sEvent);
		if (NULL != eventDes.evCBName)
		{
			eventDes.evCBName(sEvent, param, eventDes.pUser);
		}
	}
}

int WinIpcSearch::SetNetworkInfo( const QString &sDeviceID, const QString &sAddress, const QString &sMask, const QString &sGateway, const QString &sMac, const QString &sPort, const QString &sUsername, const QString &sPassword )
{
	return 0;
}
