#include "onvifdevice.h"
#include <guid.h>
int cbXConnectStatusChange(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXLiveStream(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXAuthority(QString sEvName,QVariantMap tInfo,void *pUser);
onvifDevice::onvifDevice():m_nRef(0),
	m_pOnvifProctol(NULL),
	m_tConnectStatus(IDeviceClient::STATUS_DISCONNECTED)
{
	m_sEventList<<"Authority"<<"CurrentStatus"<<"LiveStream";
	m_hMainThread=QThread::currentThreadId();
	connect(this,SIGNAL(sgbackToMainThread(QString,QVariantMap)),this,SLOT(slbackToMainThread(QString,QVariantMap)));
}

onvifDevice::~onvifDevice()
{

}

long __stdcall onvifDevice::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IEventRegister==iid)
	{
		*ppv = static_cast<IEventRegister *>(this);
	}else if (IID_IPcomBase==iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}else if (IID_IDeviceClient==iid)
	{
		*ppv=static_cast<IDeviceClient*>(this);
	}
	else
	{
		qDebug ()<<__FUNCTION__<<__LINE__<<"it do not support ::that interface";
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall onvifDevice::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall onvifDevice::Release()
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

QStringList onvifDevice::eventList()
{
	return m_sEventList;
}

int onvifDevice::queryEvent( QString eventName,QStringList &eventParams )
{
	if (m_sEventList.contains(eventName))
	{
		return IEventRegister::OK;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined"<<eventName;;
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
}

int onvifDevice::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		tagOnvifDeviceProcInfo tProInfo;
		tProInfo.proc=proc;
		tProInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProInfo);
		return IEventRegister::OK;
	}
}

void onvifDevice::eventProcCall( QString sEvent,QVariantMap tInfo )
{
	if (m_sEventList.contains(sEvent))
	{
		tagOnvifDeviceProcInfo tProInfo=m_tEventMap.value(sEvent);
		if (NULL!=tProInfo.proc)
		{
			tProInfo.proc(sEvent,tInfo,tProInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is not register";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is  undefined";
	}
}

int onvifDevice::setDeviceHost( const QString & sAddr )
{
	m_tDeviceParamInfo.sAddress=sAddr;
	return 0;
}

int onvifDevice::setDevicePorts( unsigned int nPorts )
{
	m_tDeviceParamInfo.nPorts=nPorts;
	return 0;
}

int onvifDevice::setDeviceId( const QString & sEsee )
{
	m_tDeviceParamInfo.sEsee=sEsee;
	return 0;
}

int onvifDevice::connectToDevice()
{
	if (m_tConnectStatus!=IDeviceClient::STATUS_DISCONNECTED)
	{
		if (NULL!=m_pOnvifProctol)
		{
			m_tpOnvifProtocolLock.lock();
			m_pOnvifProctol->Release();
			m_pOnvifProctol=NULL;
			m_tpOnvifProtocolLock.unlock();
		}else{
			//do nothing
		}
		//生成 协议组件 先用 bubble 组件 来 代替
		//pcomCreateInstance(CLSID_Bubble,NULL,IID_IDeviceConnection,(void**)&m_DeviceConnectonBubble);
		pcomCreateInstance(CLSID_Bubble,NULL,IID_IDeviceConnection,(void**)&m_pOnvifProctol);
		if (NULL!=m_pOnvifProctol)
		{
			IEventRegister *pRegister=NULL;
			m_tpOnvifProtocolLock.lock();
			m_pOnvifProctol->QueryInterface(IID_IEventRegister,(void**)&pRegister);
			m_tpOnvifProtocolLock.unlock();
			if (NULL==pRegister)
			{
				m_tpOnvifProtocolLock.lock();
				m_pOnvifProctol->Release();
				m_pOnvifProctol=NULL;
				m_tpOnvifProtocolLock.unlock();
				return 1;
			}else{
				pRegister->registerEvent("Authority",cbXAuthority,this);
				pRegister->registerEvent("LiveStream",cbXLiveStream,this);
				pRegister->registerEvent("StateChangeed",cbXConnectStatusChange,this);
				pRegister->Release();
				pRegister=NULL;
			}
			QVariantMap tStatusInfo;
			tStatusInfo.insert("CurrentStatus",IDeviceClient::STATUS_CONNECTING);
			backToMainThread("CurrentStatus",tStatusInfo);
			IDeviceConnection *pProctolConnect=NULL;
			m_tpOnvifProtocolLock.lock();
			m_pOnvifProctol->QueryInterface(IID_IDeviceConnection,(void**)&pProctolConnect);
			m_tpOnvifProtocolLock.unlock();
			if (NULL!=pProctolConnect)
			{
				QVariantMap tPorts;
				tPorts.insert("media",m_tDeviceParamInfo.nPorts);
				if (1==pProctolConnect->setDeviceHost(m_tDeviceParamInfo.sAddress)||1==pProctolConnect->setDevicePorts(tPorts)||1==pProctolConnect->setDeviceId(m_tDeviceParamInfo.sEsee)||1==pProctolConnect->setDeviceAuthorityInfomation(m_tDeviceParamInfo.sUserName,m_tDeviceParamInfo.sPassword))
				{
					if (1==pProctolConnect->connectToDevice())
					{
						//tStatusInfo.clear();
						//tStatusInfo.insert("CurrentStatus",IDeviceClient::STATUS_CONNECTED);
						//backToMainThread("CurrentStatus",tStatusInfo);
						pProctolConnect->Release();
						pProctolConnect=NULL;
						return 0;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"onvif connect to device fail";
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail as set protocol param fail";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail as QueryInterface IID_IDeviceConnection interface fail";
			}
			if (NULL!=pProctolConnect)
			{
				pProctolConnect->Release();
				pProctolConnect=NULL;
			}
			tStatusInfo.clear();
			tStatusInfo.insert("CurrentStatus",IDeviceClient::STATUS_DISCONNECTED);
			backToMainThread("CurrentStatus",tStatusInfo);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail as pcomCreateInstance m_pOnvifProctol fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"current status ::"<<m_tConnectStatus<<"please make sure current status is disconnect";
	}
	if (NULL!=m_pOnvifProctol)
	{
		m_tpOnvifProtocolLock.lock();
		m_pOnvifProctol->Release();
		m_pOnvifProctol=NULL;
		m_tpOnvifProtocolLock.unlock();
	}else{
		//do nothing
	}
	return 1;
}

int onvifDevice::checkUser( const QString & sUsername,const QString &sPassword )
{
	m_tDeviceParamInfo.sUserName=sUsername;
	m_tDeviceParamInfo.sPassword=sPassword;
	return 0;
}

int onvifDevice::setChannelName( const QString & sChannelName )
{
	m_tDeviceParamInfo.sChannelName=sChannelName;
	return 0;
}

int onvifDevice::liveStreamRequire( int nChannel,int nStream,bool bOpen )
{
	if (m_tConnectStatus!=IDeviceClient::STATUS_CONNECTED)
	{
		if (NULL!=m_pOnvifProctol)
		{
			IRemotePreview *pRemotePreview=NULL;
			m_tpOnvifProtocolLock.lock();
			m_pOnvifProctol->QueryInterface(IID_IRemotePreview,(void**)&pRemotePreview);
			m_tpOnvifProtocolLock.unlock();
			if (NULL!=pRemotePreview)
			{
				int nFlags=1;
				if (bOpen)
				{
					nFlags=pRemotePreview->getLiveStream(nChannel,nStream);
				}else{
					nFlags=pRemotePreview->pauseStream(true);
				}
				pRemotePreview->Release();
				pRemotePreview=NULL;
				return nFlags;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"liveStreamRequire fail as onvif protocol do not support IID_IRemotePreview interface";
				return 1;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"liveStreamRequire fail as m_pOnvifProctol is null";
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"liveStreamRequire fail as current connect status is not in connected";
		return 1;
	}
}

int onvifDevice::closeAll()
{
	if (NULL!=m_pOnvifProctol)
	{
		IDeviceConnection *pProcotolConnect=NULL;
		m_tpOnvifProtocolLock.lock();
		m_pOnvifProctol->QueryInterface(IID_IDeviceConnection,(void**)&pProcotolConnect);
		m_tpOnvifProtocolLock.unlock();
		if (NULL!=pProcotolConnect)
		{
			pProcotolConnect->disconnect();
			pProcotolConnect->Release();
			pProcotolConnect=NULL;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"onvif protocol do not support IID_IDeviceConnection interface";
			abort();
		}
		m_tpOnvifProtocolLock.lock();
		m_pOnvifProctol->Release();
		m_pOnvifProctol=NULL;
		m_tpOnvifProtocolLock.unlock();
	}else{
		//do nothing
	}
	QVariantMap tStatusInfo;
	tStatusInfo.insert("CurrentStatus",IDeviceClient::STATUS_DISCONNECTED);
	backToMainThread("CurrentStatus",tStatusInfo);
	return 0;
}

QString onvifDevice::getVendor()
{
	return "";
}

int onvifDevice::getConnectStatus()
{
	return m_tConnectStatus;
}

int onvifDevice::cbConnectStatusChange( QVariantMap &tInfo )
{
	backToMainThread("CurrentStatus",tInfo);
	return 0;
}

int onvifDevice::cbLiveStream( QVariantMap &tInfo )
{
	eventProcCall("LiveStream",tInfo);
	return 0;
}

int onvifDevice::cbAuthority( QVariantMap &tInfo )
{
	eventProcCall("Authority",tInfo);
	return 0;
}

void onvifDevice::backToMainThread( QString sEvName,QVariantMap tInfo )
{
	if (QThread::currentThreadId()==m_hMainThread)
	{
		slbackToMainThread(sEvName,tInfo);
	}else{
		emit sgbackToMainThread(sEvName,tInfo);
	}
}

void onvifDevice::slbackToMainThread( QString sEvName,QVariantMap evMap )
{
	if (sEvName=="CurrentStatus")
	{
		IDeviceClient::ConnectStatus tCurrentStatus=(IDeviceClient::ConnectStatus)evMap.value(sEvName).toInt();
		if (tCurrentStatus!=m_tConnectStatus)
		{
			m_tConnectStatus=tCurrentStatus;
			eventProcCall(sEvName,evMap);
		}else{
			//do nothing
		}
	}else{
		//do nothing
	}
}

int cbXConnectStatusChange( QString sEvName,QVariantMap tInfo,void *pUser )
{
	return ((onvifDevice*)pUser)->cbConnectStatusChange(tInfo);
}

int cbXLiveStream( QString sEvName,QVariantMap tInfo,void *pUser )
{
	return ((onvifDevice*)pUser)->cbLiveStream(tInfo);
}
int cbXAuthority(QString sEvName,QVariantMap tInfo,void *pUser)
{
	return ((onvifDevice*)pUser)->cbAuthority(tInfo);
}