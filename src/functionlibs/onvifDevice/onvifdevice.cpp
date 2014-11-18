#include "onvifdevice.h"
#include <guid.h>
int cbXMainConnectStatusChange(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXMainLiveStream(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXMainAuthority(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubConnectStatusChange(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubLiveStream(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubAuthority(QString sEvName,QVariantMap tInfo,void *pUser);
onvifDevice::onvifDevice():m_nRef(0),
	m_nSwithStream(0),
	m_nCurrentStream(0),
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
	else if (IID_ISwitchStream==iid)
	{
		*ppv=static_cast<ISwitchStream*>(this);
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
		//确保上一次的对象 已经被释放 
		clearProtocol();
		int nStreamNum=0;
		while(nStreamNum<2){
			m_tpOnvifProtocolLock.lock();
			pcomCreateInstance(CLSID_OnvifProtocol,NULL,IID_IDeviceConnection,(void**)&m_tOnvifProtocolInfo[nStreamNum].pOnvifProctol);
			if (NULL==m_tOnvifProtocolInfo[nStreamNum].pOnvifProctol)
			{
				m_tpOnvifProtocolLock.unlock();
				clearProtocol();
			}else{
				m_tpOnvifProtocolLock.unlock();
				//do nothing
			}
			//注册事件
			IEventRegister *pEventRegister=NULL;
			m_tpOnvifProtocolLock.lock();
			m_tOnvifProtocolInfo[nStreamNum].pOnvifProctol->QueryInterface(IID_IEventRegister,(void**)&pEventRegister);
			if (pEventRegister==NULL)
			{
				m_tpOnvifProtocolLock.unlock();
				clearProtocol();
				qDebug()<<__FUNCTION__<<__LINE__<<"connect fail as onvifProtocal should support IID_IEventRegister interface";
				return 1;
			}else{
				m_tpOnvifProtocolLock.unlock();
				registerEventCb(nStreamNum,pEventRegister);
				pEventRegister->Release();
				pEventRegister=NULL;
			}
			IDeviceConnection *pDeviceConnection=NULL;
			QVariantMap tPorts;
			tPorts.insert("media",m_tDeviceParamInfo.nPorts);
			m_tpOnvifProtocolLock.lock();
			m_tOnvifProtocolInfo[nStreamNum].pOnvifProctol->QueryInterface(IID_IDeviceConnection,(void**)&pDeviceConnection);
			if (pDeviceConnection->setDeviceAuthorityInfomation(m_tDeviceParamInfo.sUserName,m_tDeviceParamInfo.sPassword)==1||pDeviceConnection->setDeviceHost(m_tDeviceParamInfo.sAddress)==1||pDeviceConnection->setDeviceId(m_tDeviceParamInfo.sEsee)==1||pDeviceConnection->setDevicePorts(tPorts)==1)
			{
				m_tpOnvifProtocolLock.unlock();
				clearProtocol();
				qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail as set param to device fail";
				return 1;
			}else{
				m_tpOnvifProtocolLock.unlock();
				//keep going
			}
			if (pDeviceConnection->connectToDevice()==1)
			{
				clearProtocol();
				qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail";
				return 1;
			}else{
				//keep going
			}
			nStreamNum++;
		}
		return 0;
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
	if (IDeviceClient::STATUS_CONNECTED==m_tConnectStatus)
	{
		//申请码流
		int nStreamNum=0;
		while(nStreamNum<2){
			if (NULL!=m_tOnvifProtocolInfo[nStreamNum].pOnvifProctol)
			{
				//用户验证
				IDeviceConnection *pDeviceConnection=NULL;
				m_tpOnvifProtocolLock.lock();
				m_tOnvifProtocolInfo[nStreamNum].pOnvifProctol->QueryInterface(IID_IDeviceConnection,(void**)&pDeviceConnection);
				m_tpOnvifProtocolLock.unlock();
				if (NULL!=pDeviceConnection)
				{
					if (1==pDeviceConnection->authority())
					{
						pDeviceConnection->Release();
						pDeviceConnection=NULL;
						qDebug()<<__FUNCTION__<<__LINE__<<"liveStreamRequire fail as authority fail";
						return 1;
					}else{
						pDeviceConnection->Release();
						pDeviceConnection=NULL;
						//keep going
					}
				}else{
					return 1;
				}
				//码流申请
				IRemotePreview *pRemovePreview=NULL;
				m_tpOnvifProtocolLock.lock();
				m_tOnvifProtocolInfo[nStreamNum].pOnvifProctol->QueryInterface(IID_IRemotePreview,(void**)&pRemovePreview);
				m_tpOnvifProtocolLock.unlock();
				if (NULL!=pRemovePreview)
				{
					if (1!=pRemovePreview->getLiveStream(nChannel,nStream))
					{
						pRemovePreview->Release();
						pRemovePreview=NULL;
					}else{
						pRemovePreview->Release();
						pRemovePreview=NULL;
						return 1;
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"liveStreamRequire fail as onvifProtocol do not support IID_IRemotePreview interface";
					return 1;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"liveStreamRequire fail as pOnvifProctol is null";
				return 1;
			}
			nStreamNum++;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"liveStreamRequire fail as current status is not in connected";
		return 1;
	}
	return 0;
}

int onvifDevice::closeAll()
{
	clearProtocol();
	m_nCurrentStream=0;
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
	m_tLiveStreamLock.lock();
	if ("Main"==tInfo.value("streamNum"))
	{
		tInfo.remove("streamNum");
		eventProcCall("ForRecord",tInfo);
		tInfo.insert("streamNum","Main");
	}else{
		//do nothing
	}
	if (m_nCurrentStream!=m_nSwithStream)
	{
		if (1==tInfo.value("frametype"))
		{
			if ("Main"==tInfo.value("streamNum")&&0==m_nSwithStream)
			{
				m_nCurrentStream=m_nSwithStream;
			}
			if ("Sub"==tInfo.value("streamNum")&&1==m_nSwithStream)
			{
				m_nCurrentStream=m_nSwithStream;
			}
		}
	}else{
		if (0==m_nCurrentStream)
		{
			if ("Main"==tInfo.value("streamNum"))
			{
				tInfo.remove("streamNum");
				eventProcCall("LiveStream",tInfo);
			}
		}else{
			if ("Sub"==tInfo.value("streamNum"))
			{
				tInfo.remove("streamNum");
				eventProcCall("LiveStream",tInfo);
			}
		}
	}
	m_tLiveStreamLock.unlock();
	return 0;
}

int onvifDevice::cbAuthority( QVariantMap &tInfo )
{
	backToMainThread("Authority",tInfo);
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
		int nStreamNum=0;
		if (evMap.value("streamNum")=="Main")
		{
			nStreamNum=0;
		}else{
			nStreamNum=1;
		}
		evMap.remove("streamNum");
		IDeviceConnection::_enConnectionStatus tCurrentStatus=(IDeviceConnection::_enConnectionStatus)evMap.value("status").toInt();
		if (IDeviceConnection::CS_Connected==tCurrentStatus)
		{
			m_tOnvifProtocolInfo[nStreamNum].tConnectStatus=IDeviceClient::STATUS_CONNECTED;
		}else if (IDeviceConnection::CS_Disconnected==tCurrentStatus)
		{
			m_tOnvifProtocolInfo[nStreamNum].tConnectStatus=IDeviceClient::STATUS_DISCONNECTED;
		}else if (IDeviceConnection::CS_Disconnecting==tCurrentStatus)
		{
			m_tOnvifProtocolInfo[nStreamNum].tConnectStatus=IDeviceClient::STATUS_DISCONNECTING;
		}else if (IDeviceConnection::CS_Connectting==tCurrentStatus)
		{
			m_tOnvifProtocolInfo[nStreamNum].tConnectStatus=IDeviceClient::STATUS_CONNECTING;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"undefined connect status";
			abort();
		}
		//连接状态，两路都连接
		if (m_tOnvifProtocolInfo[0].tConnectStatus==m_tOnvifProtocolInfo[1].tConnectStatus&&m_tOnvifProtocolInfo[0].tConnectStatus==IDeviceClient::STATUS_CONNECTED)
		{
			m_tConnectStatus=IDeviceClient::STATUS_CONNECTED;
			QVariantMap tStatusParm;
			tStatusParm.insert("CurrentStatus",m_tConnectStatus);
			eventProcCall("CurrentStatus",tStatusParm);
		}else{
			//do nothing
		}
		//断开状态，两路都断开
		if (m_tOnvifProtocolInfo[0].tConnectStatus==m_tOnvifProtocolInfo[1].tConnectStatus&&m_tOnvifProtocolInfo[0].tConnectStatus==IDeviceClient::STATUS_DISCONNECTED)
		{
			if (m_tConnectStatus!=IDeviceClient::STATUS_DISCONNECTED)
			{
				m_tConnectStatus=IDeviceClient::STATUS_DISCONNECTED;
				QVariantMap tStatusParm;
				tStatusParm.insert("CurrentStatus",m_tConnectStatus);
				eventProcCall("CurrentStatus",tStatusParm);
			}else{
				//do nothing
			}
		}else{
			//do nothing
		}
		//原来状态为连接，一路断开，即断开所有的连接
		if (m_tConnectStatus==IDeviceClient::STATUS_CONNECTED)
		{
			if (m_tOnvifProtocolInfo[0].tConnectStatus==IDeviceClient::STATUS_DISCONNECTED||m_tOnvifProtocolInfo[1].tConnectStatus==IDeviceClient::STATUS_DISCONNECTED)
			{
				m_tConnectStatus=IDeviceClient::STATUS_DISCONNECTED;
				QVariantMap tStatusParm;
				tStatusParm.insert("CurrentStatus",m_tConnectStatus);
				eventProcCall("CurrentStatus",tStatusParm);
			}else{
				//do nothing
			}
		}else{
			//do nothing
		}
		//原来状态为未连接，一路连接，do nothing
	}else if(sEvName=="Authority"){
		evMap.remove("streamNum");
		eventProcCall(sEvName,evMap);
	}
	else{
		//do nothing
	}
}

void onvifDevice::clearProtocol()
{
	m_tpOnvifProtocolLock.lock();
	QMap<int ,tagOnvifProtocolInfo>::Iterator it=m_tOnvifProtocolInfo.constBegin();
	while(it!=m_tOnvifProtocolInfo.constEnd()){
		if (NULL!=it->pOnvifProctol)
		{
			IDeviceConnection *pDeviceConnection=NULL;
			it->pOnvifProctol->QueryInterface(IID_IDeviceConnection,(void**)&pDeviceConnection);
			if (NULL!=pDeviceConnection)
			{
				pDeviceConnection->disconnect();
				pDeviceConnection->Release();
				pDeviceConnection=NULL;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"onvifProtocol should support IID_IDeviceConnection interface";
				abort();
			}
			it.value().pOnvifProctol->Release();
			m_tOnvifProtocolInfo[it.key()].pOnvifProctol=NULL;
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
	m_tpOnvifProtocolLock.unlock();
}

void onvifDevice::registerEventCb( int nStreamNum,IEventRegister *pRegister )
{
	if (nStreamNum==0)
	{
		pRegister->registerEvent("StateChangeed",cbXMainConnectStatusChange,this);
		pRegister->registerEvent("Authority",cbXMainAuthority,this);
		pRegister->registerEvent("LiveStream",cbXMainLiveStream,this);
	}else{
		pRegister->registerEvent("StateChangeed",cbXSubConnectStatusChange,this);
		pRegister->registerEvent("Authority",cbXSubAuthority,this);
		pRegister->registerEvent("LiveStream",cbXSubLiveStream,this);
	}
}

int onvifDevice::SwitchStream( int nStreamNum )
{
	m_nSwithStream=nStreamNum;
	return 0;
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
