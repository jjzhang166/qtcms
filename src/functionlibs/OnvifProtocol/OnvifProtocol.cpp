#include "OnvifProtocol.h"
#include <guid.h>
#include <QEventLoop>
#include <QTimer>

#include <QDebug>



OnvifProtocol::OnvifProtocol():
    m_nRef(0),
	m_pDeviceSeach(NULL),
	m_pWorkThread(NULL),
	m_bSearchStoping(false),
	m_emStatus(CONNECT_STATUS_DISCONNECTED)
{
	m_sEventList << "LiveStream"<<"SocketError"<<"StateChangeed"<<"ConnectRefuse"<<"SearchDeviceSuccess"<<"Authority"<<"CurrentStatus";
// 	typedef QMultiMap<QString,tagOnvifProInfo> EventInfo;
	qRegisterMetaType<QMultiMap<QString,tagOnvifProInfo> >("QMultiMap<QString,tagOnvifProInfo>");
	qRegisterMetaType<ConnectStatus>("ConnectStatus");

	m_pWorkThread = new WorkerThread();
	m_pWorkThread->moveToThread(&m_workThread);
	connect(this, SIGNAL(sigConnectToDevice(int*)), m_pWorkThread, SLOT(ConnectToDevice(int*)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(sigAuthority(int*)), m_pWorkThread, SLOT(Authority(int*)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(sigDisconnect(int*)), m_pWorkThread, SLOT(Disconnect(int*)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(sigGetLiveStream(int, int, int*)), m_pWorkThread, SLOT(GetLiveStream(int, int, int*)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(sigPauseStream(bool, int*)), m_pWorkThread, SLOT(PauseStream(bool, int*)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(sigStopStream(int*)), m_pWorkThread, SLOT(StopStream(int*)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(sigGetStreamCount(int*)), m_pWorkThread, SLOT(GetStreamCount(int*)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(sigGetStreamInfo(int, QVariantMap&, int*)), m_pWorkThread, SLOT(GetStreamInfo(int, QVariantMap&, int*)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(sigAddEvent(const QMultiMap<QString,tagOnvifProInfo>&)), m_pWorkThread, SLOT(setEventMap(const QMultiMap<QString,tagOnvifProInfo>&)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(sigPtzCtrl(NVP_PTZ_CMD, int, int, bool, int*)), m_pWorkThread, SLOT(PtzCtrl(NVP_PTZ_CMD, int, int, bool, int*)), Qt::BlockingQueuedConnection);

	m_workThread.start();
}

OnvifProtocol::~OnvifProtocol()
{
	m_workThread.quit();
	m_workThread.wait();

	if (m_pWorkThread)
	{
		delete m_pWorkThread;
		m_pWorkThread = NULL;
	}
}

long __stdcall OnvifProtocol::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase == iid)
	{
		*ppv=static_cast<IPcomBase *>(this);
	}
	else if (IID_IEventRegister == iid)
	{
		*ppv=static_cast<IEventRegister *>(this);
	}
	else if (IID_IDeviceSearch == iid)
	{
		*ppv = static_cast<IDeviceSearch *>(this);
	}
	else if (IID_IRemotePreview == iid)
	{
		*ppv=static_cast<IRemotePreview*>(this);
	}
	else if (IID_IDeviceConnection == iid)
	{
		*ppv = static_cast<IDeviceConnection *>(this);
	}
	else if (IID_IProtocolPTZ == iid)
	{
		*ppv = static_cast<IProtocolPTZ *>(this);
	}
	else{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall OnvifProtocol::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall OnvifProtocol::Release()
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

QStringList OnvifProtocol::eventList()
{
	return m_sEventList;
}

int OnvifProtocol::queryEvent( QString eventName,QStringList& eventParams )
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

int OnvifProtocol::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		tagOnvifProInfo tProInfo;
		tProInfo.proc=proc;
		tProInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProInfo);
		emit sigAddEvent(m_tEventMap);
		return IEventRegister::OK;
	}
}

void OnvifProtocol::eventProcCall( QString sEvent,QVariantMap tInfo )
{
	if (m_sEventList.contains(sEvent))
	{
		tagOnvifProInfo tProInfo=m_tEventMap.value(sEvent);
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

// void OnvifProtocol::analyzeDeviceInfo( unsigned char *ip,unsigned short port, char *name, char *location, char *firmware )
// {
// 	QString ipStr;
// 	ipStr = ipStr.sprintf("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
// 	QVariantMap deviceInfo;
// 	deviceInfo.insert("SearchDeviceName_ID"    ,QVariant(QString(name)));
// 	deviceInfo.insert("SearchDeviceId_ID"      ,QVariant(""));
// 	deviceInfo.insert("SearchDeviceModelId_ID" ,QVariant(QString(firmware)));
// 	deviceInfo.insert("SearchSeeId_ID"         ,QVariant(""));
// 	deviceInfo.insert("SearchChannelCount_ID"  ,QVariant(""));
// 	deviceInfo.insert("SearchIP_ID"            ,QVariant(ipStr));
// 	deviceInfo.insert("SearchMask_ID"          ,QVariant(""));
// 	deviceInfo.insert("SearchMac_ID"           ,QVariant(""));
// 	deviceInfo.insert("SearchGateway_ID"       ,QVariant(""));     
// 	deviceInfo.insert("SearchHttpport_ID"      ,QVariant(port));
// 	deviceInfo.insert("SearchMediaPort_ID"     ,QVariant(""));
// 	eventProcCall(QString("SearchDeviceSuccess"),deviceInfo);
// }

int OnvifProtocol::Start()
{
	if (m_pDeviceSeach!=NULL)
	{
		return 0;
	}else{
		//keep going
	}
	m_pDeviceSeach = new DeviceSearch;
	m_pDeviceSeach->setHook(QString("SearchDeviceSuccess"), m_tEventMap.value("SearchDeviceSuccess"));
	int ret = m_pDeviceSeach->Start();
	return ret;
}

int OnvifProtocol::Flush()
{
	if (m_pDeviceSeach)
	{
		m_pDeviceSeach->Flush();
		return 0;
	}
	else
		return 1;
}

int OnvifProtocol::Stop()
{
	if (m_bSearchStoping==true)
	{
		return 0;
	}
	m_bSearchStoping=true;
	if (m_pDeviceSeach)
	{
		m_pDeviceSeach->Stop();
		delete m_pDeviceSeach;
		m_pDeviceSeach = NULL;
		m_bSearchStoping=false;
		return 0;
	}
	else
		m_bSearchStoping=false;
		return 1;
}

int OnvifProtocol::setInterval( int nInterval )
{
	if (m_pDeviceSeach)
	{
		return m_pDeviceSeach->setInterval(nInterval);
	}
	else
		return 1;
}

IEventRegister * OnvifProtocol::QueryEventRegister()
{
	IEventRegister *pEvRegister = NULL;
	QueryInterface(IID_IEventRegister, (void**)&pEvRegister);
	return pEvRegister;
}

int OnvifProtocol::setDeviceHost( const QString &sIpAddr )
{
	m_tDeviceInfo.sIpAddr = sIpAddr;
	return 0;
}

int OnvifProtocol::setDevicePorts( const QVariantMap &tPorts )
{
	m_tDeviceInfo.vPorts = tPorts;
	return 0;
}

int OnvifProtocol::setDeviceId( const QString &sEseeId )
{
	m_tDeviceInfo.sEseeId = sEseeId;
	return 0;
}

int OnvifProtocol::setDeviceAuthorityInfomation( QString sUserName,QString sPassword )
{
	m_tDeviceInfo.sUsername = sUserName;
	m_tDeviceInfo.sPassword = sPassword;
	return 0;
}

int OnvifProtocol::connectToDevice()
{
	int ret = -1;
	m_pWorkThread->setDeviceInfo(m_tDeviceInfo);
	emit sigConnectToDevice(&ret);
	StatusChanged(m_pWorkThread->getCurrentStatus());
	return ret ? 1 : 0;
}

int OnvifProtocol::authority()
{
	int ret = -1;
	emit sigAuthority(&ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::disconnect()
{
	int ret = -1;
	emit sigDisconnect(&ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::getCurrentStatus()
{
	return m_pWorkThread->getCurrentStatus();
}

QString OnvifProtocol::getDeviceHost()
{
	return m_tDeviceInfo.sIpAddr;
}

QString OnvifProtocol::getDeviceid()
{
	return m_tDeviceInfo.sEseeId;
}

QVariantMap OnvifProtocol::getDevicePorts()
{
	return m_tDeviceInfo.vPorts;
}

void OnvifProtocol::sleepEx( uint millisecond )
{
	QEventLoop evLoop;
	QTimer::singleShot(millisecond, &evLoop, SLOT(quit()));
	evLoop.exec();
}

int OnvifProtocol::getLiveStream( int nChannel,int nStream )
{
	int ret = -1;
	emit sigGetLiveStream(nChannel, nStream, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::stopStream()
{
	int ret = -1;
	emit sigStopStream(&ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::pauseStream( bool bPause )
{
	int ret = -1;
	emit sigPauseStream(bPause, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::getStreamCount()
{
	int count = 0;
	emit sigGetStreamCount(&count);
	return count;
}

int OnvifProtocol::getStreamInfo( int nStreamId,QVariantMap &tStreamInfo )
{
	int ret = -1;
	emit sigGetStreamInfo(nStreamId, tStreamInfo, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZUp( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_UP, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZDown( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_DOWN, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZLeft( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_LEFT, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZRight( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_RIGHT, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZIrisOpen( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_IRIS_OPEN, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZIrisClose( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_IRIS_CLOSE, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZFocusFar( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_FOCUS_FAR, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZFocusNear( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_FOCUS_NEAR, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZZoomIn( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_ZOOM_IN, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZZoomOut( const int &nChl, const int &nSpeed )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_ZOOM_OUT, nChl, nSpeed, true, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZAuto( const int &nChl, bool bOpend )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_AUTOPAN, nChl, 0, bOpend, &ret);
	return ret ? 1 : 0;
}

int OnvifProtocol::PTZStop( const int &nChl, const int &nCmd )
{
	int ret = -1;
	emit sigPtzCtrl(NVP_PTZ_CMD_STOP, nChl, 0, false, &ret);
	return ret ? 1 : 0;
}

void OnvifProtocol::StatusChanged( ConnectStatus emStatus )
{
	if (m_emStatus != emStatus)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"status changed from "<<m_emStatus<<" to "<<emStatus;
		QVariantMap item;
		item.insert("status", emStatus);
		eventProcCall("StateChangeed",item);
		m_emStatus = emStatus;
	}
	else
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"status didn't changed";
	}
}

// void cbSearchHook( const char *bind_host, unsigned char *ip,unsigned short port, char *name, char *location, char *firmware, void *customCtx )
// {
// 	((OnvifProtocol*)customCtx)->analyzeDeviceInfo(ip, port, name, location, firmware);
// }
