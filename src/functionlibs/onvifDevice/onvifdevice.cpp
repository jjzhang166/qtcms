#include "onvifdevice.h"
#include <guid.h>
int cbXMainConnectStatusChange(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXMainLiveStream(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXMainAuthority(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubConnectStatusChange(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubLiveStream(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubAuthority(QString sEvName,QVariantMap tInfo,void *pUser);
onvifDevice::onvifDevice():m_nRef(0),
	m_tConnectStatus(IDeviceClient::STATUS_DISCONNECTED)
{
	m_sEventList<<"Authority"<<"CurrentStatus"<<"LiveStream";
	m_hMainThread=QThread::currentThreadId();
	connect(this,SIGNAL(sgbackToMainThread(QString,QVariantMap)),this,SLOT(slbackToMainThread(QString,QVariantMap)));
	for (int i=0;i<2;i++)
	{
		tagOnvifProtocolInfo tProtocolInfo;
		tProtocolInfo.pOnvifProctol=NULL;
		tProtocolInfo.tConnectStatus=IDeviceClient::STATUS_DISCONNECTED;
		m_tOnvifProtocolInfo.insert(i,tProtocolInfo);
	}
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
	if (IDeviceClient::STATUS_DISCONNECTED==m_tConnectStatus)
	{
		QMap<int,tagOnvifProtocolInfo>::const_iterator it=m_tOnvifProtocolInfo.constBegin();
		while(it!=m_tOnvifProtocolInfo.constEnd()){
			if (NULL!=it.value().pOnvifProctol)
			{
				IDeviceConnection *pDeviceConnection=NULL;
				it.value().pOnvifProctol->QueryInterface(IID_IDeviceConnection,(void**)&pDeviceConnection);
				if (NULL!=pDeviceConnection)
				{
					pDeviceConnection->disconnect();
					pDeviceConnection->Release();
					pDeviceConnection=NULL;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"onvifProtocol should  support interface IID_IDeviceConnection";
					abort();
				}
				it.value().pOnvifProctol->Release();
				m_tOnvifProtocolInfo[0].pOnvifProctol=NULL;
			}else{
				//do nothing
			}
			QVariantMap tCurrentConnectStatus;
			tCurrentConnectStatus.insert("CurrentStatus",IDeviceClient::STATUS_DISCONNECTED);
			QString sStreamNum;
			if (it.key()==0)
			{
				sStreamNum=QString("Main");
			}else{
				sStreamNum=QString("Sub");
			}
			tCurrentConnectStatus.insert("streamNum",sStreamNum);
			backToMainThread("CurrentStatus",tCurrentConnectStatus);
			++it;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"current status is not disconnect,if you want to reconnect,please call closeAll() frist";
		return 1;
	}
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
	return 0;
}

int onvifDevice::closeAll()
{
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

	}else{
		//do nothing
	}
}

int cbXMainConnectStatusChange( QString sEvName,QVariantMap tInfo,void *pUser )
{
	tInfo.insert("streamNum","Main");
	return ((onvifDevice*)pUser)->cbConnectStatusChange(tInfo);
}

int cbXMainLiveStream( QString sEvName,QVariantMap tInfo,void *pUser )
{
	tInfo.insert("streamNum","Main");
	return ((onvifDevice*)pUser)->cbLiveStream(tInfo);
}
int cbXMainAuthority(QString sEvName,QVariantMap tInfo,void *pUser)
{
	tInfo.insert("streamNum","Main");
	return ((onvifDevice*)pUser)->cbAuthority(tInfo);
}

int cbXSubConnectStatusChange( QString sEvName,QVariantMap tInfo,void *pUser )
{
	tInfo.insert("streamNum","Sub");
	return ((onvifDevice*)pUser)->cbConnectStatusChange(tInfo);
}

int cbXSubLiveStream( QString sEvName,QVariantMap tInfo,void *pUser )
{
	tInfo.insert("streamNum","Sub");
	return ((onvifDevice*)pUser)->cbLiveStream(tInfo);
}

int cbXSubAuthority( QString sEvName,QVariantMap tInfo,void *pUser )
{
	tInfo.insert("streamNum","Sub");
	return ((onvifDevice*)pUser)->cbAuthority(tInfo);
}
