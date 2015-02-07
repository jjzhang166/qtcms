#include "onvifdevice.h"
#include <guid.h>
int cbXMainConnectStatusChange(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXMainLiveStream(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXMainAuthority(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubConnectStatusChange(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubLiveStream(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubAuthority(QString sEvName,QVariantMap tInfo,void *pUser);
int cbXSubMotionDetion(QString sEvName,QVariantMap tInfo,void *pUser);
onvifDevice::onvifDevice():m_nRef(0),
	m_nSwithStream(0),
	m_nCurrentStream(0),
	m_tConnectStatus(IDeviceClient::STATUS_DISCONNECTED),
	m_pIAutoSycTime(NULL)
{
	m_sEventList<<"Authority"<<"CurrentStatus"<<"LiveStream"<<"ForRecord"<<"MDSignal";
	m_hMainThread=QThread::currentThreadId();
	connect(this,SIGNAL(sgbackToMainThread(QString,QVariantMap)),this,SLOT(slbackToMainThread(QString,QVariantMap)));
	for (int i=0;i<2;i++)
	{
		tagOnvifProtocolInfo tProtocolInfo;
		tProtocolInfo.pOnvifProctol=NULL;
		tProtocolInfo.tConnectStatus=IDeviceClient::STATUS_DISCONNECTED;
		m_tOnvifProtocolInfo.insert(i,tProtocolInfo);
	}
	pcomCreateInstance(CLSID_OnvifNetwork,NULL,IID_IAutoSycTime,(void**)&m_pIAutoSycTime);
}

onvifDevice::~onvifDevice()
{
	if (m_pIAutoSycTime!=NULL)
	{
		m_pIAutoSycTime->Release();
		m_pIAutoSycTime=NULL;
	}
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
	else if (IID_IPTZControl == iid)
	{
		*ppv = static_cast<IPTZControl*>(this);
	}
	else if (IID_IAutoSycTime==iid)
	{
		*ppv=static_cast<IAutoSycTime*>(this);
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
				pDeviceConnection->Release();
				pDeviceConnection=NULL;
				clearProtocol();
				qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail as set param to device fail";
				return 1;
			}else{
				m_tpOnvifProtocolLock.unlock();
				//keep going
			}
			if (pDeviceConnection->connectToDevice()==1)
			{
				pDeviceConnection->Release();
				pDeviceConnection=NULL;
				clearProtocol();
				qDebug()<<__FUNCTION__<<__LINE__<<"connect to device fail";
				return 1;
			}else{
				//keep going
			}
			pDeviceConnection->Release();
			pDeviceConnection=NULL;
			nStreamNum++;
		}
		//启动移动侦测
		IRemoteMotionDetection *pRemoteMotionDetection=NULL;
		m_tpOnvifProtocolLock.lock();
		m_tOnvifProtocolInfo.value(0).pOnvifProctol->QueryInterface(IID_IRemoteMotionDetection,(void**)&pRemoteMotionDetection);
		m_tpOnvifProtocolLock.unlock();
		if (NULL!=pRemoteMotionDetection)
		{
			//注册移动侦测事件
			//注册事件
			IEventRegister *pEventRegister=NULL;
			pRemoteMotionDetection->QueryInterface(IID_IEventRegister,(void**)&pEventRegister);
			if (NULL!=pEventRegister)
			{
				pEventRegister->registerEvent("MDSignal",cbXSubMotionDetion,this);
				pEventRegister->Release();
				pEventRegister=NULL;
			}else{
				//do nothing
			}
			//启动移动侦测
			pRemoteMotionDetection->startMotionDetection();
			pRemoteMotionDetection->Release();
			pRemoteMotionDetection=NULL;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"start remoteMotion detect fail as onvifProctol do not support IRemoteMotionDetection interface";
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
			m_tpOnvifProtocolLock.lock();
			if (NULL!=m_tOnvifProtocolInfo[nStreamNum].pOnvifProctol)
			{
				//用户验证
				IDeviceConnection *pDeviceConnection=NULL;
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
					if (1!=pRemovePreview->getLiveStream(nChannel,nStreamNum))
					{
						pRemovePreview->Release();
						pRemovePreview=NULL;
					}else{
						pRemovePreview->Release();
						pRemovePreview=NULL;
						qDebug()<<__FUNCTION__<<__LINE__<<"getLiveStream fail"<<nChannel<<nStreamNum;
						return 1;
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"liveStreamRequire fail as onvifProtocol do not support IID_IRemotePreview interface";
					return 1;
				}
			}else{
				m_tpOnvifProtocolLock.unlock();
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
			else if ("Sub"==tInfo.value("streamNum")&&1==m_nSwithStream)
			{
				m_nCurrentStream=m_nSwithStream;
			}
		}
	}
	if (m_nCurrentStream==m_nSwithStream)
	{
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
		/*emit sgbackToMainThread(sEvName,tInfo);*/
		slbackToMainThread(sEvName,tInfo);
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
		IDeviceClient::ConnectStatus tCurrentStatus=(IDeviceClient::ConnectStatus)evMap.value("status").toInt();
		if (IDeviceClient::STATUS_CONNECTED==tCurrentStatus)
		{
			m_tOnvifProtocolInfo[nStreamNum].tConnectStatus=IDeviceClient::STATUS_CONNECTED;
		}else if (IDeviceClient::STATUS_DISCONNECTED==tCurrentStatus)
		{
			m_tOnvifProtocolInfo[nStreamNum].tConnectStatus=IDeviceClient::STATUS_DISCONNECTED;
		}else if (IDeviceClient::STATUS_DISCONNECTING==tCurrentStatus)
		{
			m_tOnvifProtocolInfo[nStreamNum].tConnectStatus=IDeviceClient::STATUS_DISCONNECTING;
		}else if (IDeviceClient::STATUS_CONNECTING==tCurrentStatus)
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
	//停止移动侦测
	IRemoteMotionDetection *pRemoteMotionDetection=NULL;
	m_tpOnvifProtocolLock.lock();
	if (NULL!=m_tOnvifProtocolInfo.value(0).pOnvifProctol)
	{
		m_tOnvifProtocolInfo.value(0).pOnvifProctol->QueryInterface(IID_IRemoteMotionDetection,(void**)&pRemoteMotionDetection);
		m_tpOnvifProtocolLock.unlock();
		if (NULL!=pRemoteMotionDetection)
		{
			//停止移动侦测
			pRemoteMotionDetection->stopMotionDetection();
			pRemoteMotionDetection->Release();
			pRemoteMotionDetection=NULL;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"start remoteMotion detect fail as onvifProctol do not support IRemoteMotionDetection interface";
		}
	}else{
		m_tpOnvifProtocolLock.unlock();
		//do nothing
	}

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
		tCurrentConnectStatus.insert("status",IDeviceClient::STATUS_DISCONNECTED);
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

IProtocolPTZ * onvifDevice::getPTZInterface( int streamId )
{
	IProtocolPTZ *pPtzInterface = NULL;
	IDeviceConnection *pCon = m_tOnvifProtocolInfo[streamId].pOnvifProctol;
	if (pCon)
	{
		pCon->QueryInterface(IID_IProtocolPTZ, (void**)&pPtzInterface);
		if (pPtzInterface)
		{
			return pPtzInterface;
		}
	}
	return NULL;
}

int onvifDevice::ControlPTZUp( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZUp(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZDown( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZDown(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZLeft( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZLeft(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZRight( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZRight(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZIrisOpen( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZIrisOpen(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZIrisClose( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZIrisClose(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZFocusFar( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZFocusFar(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZFocusNear( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZFocusNear(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZZoomIn( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZZoomIn(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZZoomOut( const int &nChl, const int &nSpeed )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZZoomOut(nChl, nSpeed);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZAuto( const int &nChl, bool bOpend )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZAuto(nChl, bOpend);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::ControlPTZStop( const int &nChl, const int &nCmd )
{
	IProtocolPTZ *pPtz = getPTZInterface(0);	//get main stream connection
	if (pPtz)
	{
		int ret = pPtz->PTZStop(nChl, nCmd);
		pPtz->Release();
		return ret;
	}
	return 1;
}

int onvifDevice::cbMotionDetion( QVariantMap &tInfo )
{
	eventProcCall("MDSignal",tInfo);
	return 0;
}

int onvifDevice::setAutoSycTime( bool bEnabled )
{
	if (NULL!=m_pIAutoSycTime)
	{
		IOnvifRemoteInfo *pOnvifRemoteInfo=NULL;
		m_pIAutoSycTime->QueryInterface(IID_IOnvifRemoteInfo,(void**)&pOnvifRemoteInfo);
		if (NULL!=pOnvifRemoteInfo)
		{
			pOnvifRemoteInfo->setOnvifDeviceInfo(m_tDeviceParamInfo.sAddress,QString::number(m_tDeviceParamInfo.nPorts),m_tDeviceParamInfo.sUserName,m_tDeviceParamInfo.sPassword);
			m_pIAutoSycTime->setAutoSycTime(bEnabled);
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"setAutoSycTime fail as pOnvifRemoteInfo is null";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setAutoSycTime fail as m_pIAutoSycTime is null";
	}
	return 1;
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

int cbXSubMotionDetion( QString sEvName,QVariantMap tInfo,void *pUser )
{
	return ((onvifDevice*)pUser)->cbMotionDetion(tInfo);;
}
