#include "IpcDeviceClient.h"
#include <guid.h>
#include "ILocalSetting.h"
#include <QDateTime>
#include <QHostAddress>
#include <QUrl>

IpcDeviceClient::IpcDeviceClient(void):m_nRef(0),
	m_CurStatus(IDeviceClient::STATUS_DISCONNECTED),
	m_CurStream(0),
	m_csRefDelete(QMutex::Recursive),
	bHadCallCloseAll(false),
	bCloseingFlags(false),
	m_bIsSycTime(false),
	m_tcpSocket(NULL),
	m_pProtocolPTZ(NULL),
	m_steps(0),
	m_IfSwithStream(0)
{
	//设置本组件支持的回调函数事件名称
	m_EventList<<"LiveStream"<<"SocketError"<<"StateChangeed"<<"CurrentStatus"<<"foundFile"<<"recFileSearchFinished"<<"ForRecord"<<"SyncTimeMsg";
	//设置主次码流的初始连接状态
	CurStatusInfo m_statusInfo;
	m_statusInfo.m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
	m_StreamCurStatus.insert(0,m_statusInfo);
	m_StreamCurStatus.insert(1,m_statusInfo);
	//设置回调函数到结构体
	IpcDeviceClientToProcInfoItem m_IpcDevliInfo;
	//主码流回调函数
	m_IpcDevliInfo.proc=cbLiveStreamFrompPotocol_Primary;
	m_IpcDevliInfo.puser=this;
	m_IpcDevliInfo.Stream=0;
	m_EventMapToProc.insert("LiveStream",m_IpcDevliInfo);
	m_IpcDevliInfo.proc=cbStateChangeFrompPotocol_Primary;
	m_IpcDevliInfo.puser=this;
	m_IpcDevliInfo.Stream=0;
	m_EventMapToProc.insert("StateChangeed",m_IpcDevliInfo);
	m_IpcDevliInfo.proc=cbSocketErrorFrompPotocol_Primary;
	m_IpcDevliInfo.puser=this;
	m_IpcDevliInfo.Stream=0;
	m_EventMapToProc.insert("SocketError",m_IpcDevliInfo);

	//子码流回调函数
	m_IpcDevliInfo.proc=cbLiveStreamFrompPotocol_Minor;
	m_IpcDevliInfo.puser=this;
	m_IpcDevliInfo.Stream=1;
	m_EventMapToProc.insert("LiveStream",m_IpcDevliInfo);
	m_IpcDevliInfo.proc=cbStateChangeFrompPotocol_Minor;
	m_IpcDevliInfo.puser=this;
	m_IpcDevliInfo.Stream=1;
	m_EventMapToProc.insert("StateChangeed",m_IpcDevliInfo);
	m_IpcDevliInfo.proc=cbSocketErrorFrompPotocol_Minor;
	m_IpcDevliInfo.puser=this;
	m_IpcDevliInfo.Stream=1;
	m_EventMapToProc.insert("SocketError",m_IpcDevliInfo);
}


IpcDeviceClient::~IpcDeviceClient(void)
{
	if (false==bHadCallCloseAll)
	{
		closeAll();
	}
}

long __stdcall IpcDeviceClient::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IDeviceClient==iid)
	{
		*ppv=static_cast<IDeviceClient*>(this);
	}
	else if (IID_IPcomBase==iid)
	{
		*ppv=static_cast<IPcomBase *>(this);
	}
	else if (IID_IEventRegister==iid)
	{
		*ppv=static_cast<IEventRegister*>(this);
	}
	else if (IID_ISwitchStream==iid)
	{
		*ppv=static_cast<ISwitchStream*>(this);
	}
	else if (IID_IAutoSycTime == iid)
	{
		*ppv=static_cast<IAutoSycTime*>(this);
	}
	else if (IID_IPTZControl == iid)
	{
		*ppv=static_cast<IPTZControl*>(this);
	}
	else 
	{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	static_cast <IPcomBase*>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall IpcDeviceClient::AddRef()
{
	m_csRef.lock();
	m_nRef++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall IpcDeviceClient::Release()
{
	int nRet=0;
	m_csRef.lock();
	m_nRef--;
	nRet=m_nRef;
	m_csRef.unlock();
	if (0==nRet)
	{
		delete this;
	}
	return nRet;
}

QStringList IpcDeviceClient::eventList()
{
	return m_EventList;
}

int IpcDeviceClient::queryEvent( QString eventName,QStringList& eventParams )
{
	if (!m_EventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	if ("LiveStream"==eventName)
	{
		eventParams<<"channel"<<"pts"<<"length"<<"data"<<"frametype"<<"width"<<"height"<<"vcodec"<<"samplerate"<<"samplewidth"<<"audiochannel"<<"acodec";
	}
	if ("foundFile" == eventName)
	{
		eventParams<<"channel"<<"types"<<"start"<<"end"<<"filename";
	}
	if ("recFileSearchFinished" == eventName)
	{
		eventParams<<"total";
	}
	if ("SyncTimeMsg")
	{
		eventParams<<"statusCode"<<"statusMsg";
	}
	return IEventRegister::OK;
}

int IpcDeviceClient::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_EventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	if (m_EventMap.contains(eventName))
	{
		IpcDeviceClientInfoItem ipcdeviceInfo;
		ipcdeviceInfo.proc=proc;
		ipcdeviceInfo.puser=pUser;
		m_EventMap.replace(eventName,ipcdeviceInfo);
		return IEventRegister::OK;
	}
	IpcDeviceClientInfoItem ipcdeviceInfo;
	ipcdeviceInfo.proc=proc;
	ipcdeviceInfo.puser=pUser;
	m_EventMap.insert(eventName,ipcdeviceInfo);
	return IEventRegister::OK;
}

int IpcDeviceClient::connectToDevice( const QString &sAddr,unsigned int uiPort,const QString &sEseeId )
{
	m_DeviceInfo.m_sAddr.clear();
	m_DeviceInfo.m_sAddr=sAddr;
	m_DeviceInfo.m_ports.clear();
	m_DeviceInfo.m_ports.insert("media",uiPort);
	m_DeviceInfo.m_sEseeId.clear();
	m_DeviceInfo.m_sEseeId=sEseeId;

	if (m_bIsSycTime)
	{
		connect(this, SIGNAL(sigSyncTime()), this, SLOT(SyncTime()));
		emit sigSyncTime();//同步时间
	}
	//断开上一次的连接
	if (m_CurStatus==IDeviceClient::STATUS_CONNECTED||m_CurStatus==IDeviceClient::STATUS_DISCONNECTING||m_CurStatus==IDeviceClient::STATUS_CONNECTING)
	{
		closeAll();
	}
	//make sure had been disconnect
	bCloseingFlags=false;
	int nStep=0;
	m_CurStatus=IDeviceClient::STATUS_CONNECTING;
	QVariantMap CurStatusParm;
	CurStatusParm.insert("CurrentStatus",m_CurStatus);
	eventProcCall("CurrentStatus",CurStatusParm);
	while(nStep!=5){
		switch(nStep){
			//尝试bubble
		case 0:
			{
				if (true==TryToConnectProtocol(CLSID_BubbleProtocol))
				{
					qDebug()<<"bubble connect success";
					nStep=3;
				}
				else{
					qDebug()<<"bubble connect fail";
					nStep=1;
					break;
				}
			}
			break;
			//尝试穿透协议连接
		case 1:
			{
				if (m_DeviceInfo.m_sEseeId!=""&&m_DeviceInfo.m_sEseeId!="0"&&true==TryToConnectProtocol(CLSID_Hole))
				{
					qDebug()<<"Hole connect success";
					nStep=3;
				}
				else{
					qDebug()<<"Hole connect fail";
					nStep=2;
					break;
				}
			}
			break;
			//尝试转发协议连接
		case 2:
			{
				if (m_DeviceInfo.m_sEseeId!=""&&m_DeviceInfo.m_sEseeId!="0"&&true==TryToConnectProtocol(CLSID_Turn))
				{
					qDebug()<<"Turn connect success";
					nStep=3;
				}
				else{
					qDebug()<<"Turn connect fail";
					nStep=4;
					break;
				}
			}
			break;
			//连接成功
		case 3:
			{
			//fixme
				bCloseingFlags=false;
				nStep=5;
			}
			break;
			//连接失败
		case 4:
			{
				qDebug()<<"connect fail";
				//fixme
				nStep =5;
				if (m_CurStatus!=IDeviceClient::STATUS_DISCONNECTED)
				{
					m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
					QVariantMap CurStatusParm;
					CurStatusParm.insert("CurrentStatus",m_CurStatus);
					eventProcCall("CurrentStatus",CurStatusParm);
				}
				return 1;
			}
			break;
		case 5:
			{
				//m_CurStatus=IDeviceClient::STATUS_CONNECTED;
				//QVariantMap CurStatusParm;
				//CurStatusParm.insert("CurrentStatus",m_CurStatus);
				//eventProcCall("CurrentStatus",CurStatusParm);
			}
			break;
		default:
			break;
			}
		}
	return 0;
}

int IpcDeviceClient::checkUser( const QString & sUsername,const QString &sPassword )
{
	m_DeviceInfo.m_sUserName.clear();
	m_DeviceInfo.m_sUserName=sUsername;
	m_DeviceInfo.m_sPassword.clear();
	m_DeviceInfo.m_sPassword=sPassword;
	return 0;
}

int IpcDeviceClient::setChannelName( const QString & sChannelName )
{
	m_DeviceInfo.m_sChannelName.clear();
	m_DeviceInfo.m_sChannelName=sChannelName;
	return 0;
}

int IpcDeviceClient::liveStreamRequire( int nChannel,int nStream,bool bOpen )
{
	//申请主码流
	if (m_DeviceClentMap.value(0).m_DeviceConnecton!=NULL)
	{
		IRemotePreview *m_LiveStreamRequire=NULL;
		m_DeviceClentMap.value(0).m_DeviceConnecton->QueryInterface(IID_IRemotePreview,(void**)&m_LiveStreamRequire);
		if (true==bOpen)
		{
			//默认主码流为0
			if (1==m_LiveStreamRequire->getLiveStream(nChannel,0))
			{
				m_LiveStreamRequire->Release();
				return 1;
			}
		}
		else if (false==bOpen)
		{
			m_LiveStreamRequire->pauseStream(true);
		}
		m_LiveStreamRequire->Release();
	}
	
	//申请次码流
	if (m_DeviceClentMap.value(1).m_DeviceConnecton!=NULL)
	{
		IRemotePreview *m_LiveStreamRequire=NULL;
		m_DeviceClentMap.value(1).m_DeviceConnecton->QueryInterface(IID_IRemotePreview,(void**)&m_LiveStreamRequire);
		if (true==bOpen)
		{
			//默认次码流为1
			if (1==m_LiveStreamRequire->getLiveStream(nChannel,1))
			{
				m_LiveStreamRequire->Release();
				return 1;
			}
		}
		else if (false==bOpen)
		{
			m_LiveStreamRequire->pauseStream(true);
		}
		m_LiveStreamRequire->Release();
	}
	return 0;
}

int IpcDeviceClient::closeAll()
{

	//中断正在连接的状态
	bHadCallCloseAll=true;
	m_csRefDelete.lock();
	//如果已经处于断开的状态，则释放资源
	if (m_CurStatus==IDeviceClient::STATUS_DISCONNECTED)
	{
		DeInitProtocl();
		m_csRefDelete.unlock();
		bHadCallCloseAll=false;
		return 0;
	}
	bCloseingFlags=true;
	m_CurStatus=IDeviceClient::STATUS_DISCONNECTING;
	QVariantMap CurStatusParm;
	CurStatusParm.insert("CurrentStatus",IDeviceClient::STATUS_DISCONNECTING);
	eventProcCall("CurrentStatus",CurStatusParm);
	//释放云台控制相关资源
	if (NULL != m_pProtocolPTZ)
	{
		m_pProtocolPTZ->Release();
		m_pProtocolPTZ = NULL;
	}

	QMultiMap<int,SingleConnect>::iterator it;
	for (it=m_DeviceClentMap.begin();it!=m_DeviceClentMap.end();it++)
	{
		
		if (NULL!=it->m_DeviceConnecton)
		{
			//断开连接
			IDeviceConnection *m_CloseAllConnect=NULL;
			it->m_DeviceConnecton->QueryInterface(IID_IDeviceConnection,(void**)&m_CloseAllConnect);
			m_CloseAllConnect->disconnect();
			m_CloseAllConnect->Release();
		}
	}
	//释放资源
	DeInitProtocl();

	m_CurStream=0;
	m_csRefDelete.unlock();
	bHadCallCloseAll=false;
	return 0;
}

QString IpcDeviceClient::getVendor()
{
	QString sReturn;
	return sReturn;
}

int IpcDeviceClient::getConnectStatus()
{
	return m_CurStatus;
}

int IpcDeviceClient::cbLiveStream( QVariantMap &evmap )
{
	m_cscbLiveStream.lock();
	//录像码流
	if ("Primary"==evmap.value("Stream"))
	{
		evmap.remove("Stream");
		eventProcCall("ForRecord",evmap);
		evmap.insert("Stream","Primary");
	}

	// 切换码流
	if (m_IfSwithStream != m_CurStream)
	{
		if (1 == evmap.value("frametype"))
		{
			if ("Primary" == evmap.value("Stream") && 0 == m_IfSwithStream)
			{
				m_CurStream = m_IfSwithStream;
			}
			else if ("Minor" == evmap.value("Stream") && 1 == m_IfSwithStream)
			{
				m_CurStream = m_IfSwithStream;
			}
		}
	}

	//预览码流
	if (m_IfSwithStream==m_CurStream)
	{
		if (0==m_CurStream)
		{
			if ("Primary"==evmap.value("Stream"))
			{
				evmap.remove("Stream");
				eventProcCall("LiveStream",evmap);
				m_cscbLiveStream.unlock();
				return 0;
			}
		}
		if (1==m_CurStream)
		{
			if ("Minor"==evmap.value("Stream"))
			{
				evmap.remove("Stream");
				eventProcCall("LiveStream",evmap);
			}
		}
		m_cscbLiveStream.unlock();
		return 0;
	}
	m_cscbLiveStream.unlock();
	return 0;
}

void IpcDeviceClient::eventProcCall( QString sEvent,QVariantMap param )
{
	if (m_EventList.contains(sEvent))
	{
		IpcDeviceClientInfoItem eventDes=m_EventMap.value(sEvent);
		if (NULL!=eventDes.proc)
		{
			eventDes.proc(sEvent,param,eventDes.puser);
		}
	}
}

int IpcDeviceClient::cbSocketError( QVariantMap &evmap )
{
	if (0==m_CurStatus)
	{
		if ("Primary"==evmap.value("Stream"))
		{
			evmap.remove("Stream");
			eventProcCall("SocketError",evmap);
			return 0;
		}
	}
	if (1==m_CurStatus)
	{
		if ("Minor"==evmap.value("Stream"))
		{
			evmap.remove("Stream");
			eventProcCall("SocketError",evmap);
			return 0;
		}
	}
	return 0;
}

int IpcDeviceClient::cbConnectStatusProc( QVariantMap evMap )
{
	//处理主码流的状态，只保存连接和不连接两种状态
		if ("Primary"==evMap.value("Stream"))
		{
			evMap.remove("Stream");
			QVariantMap::const_iterator it;
			for (it=evMap.begin();it!=evMap.end();++it)
			{
				QString sKey=it.key();
				if (IDeviceConnection::CS_Connected==it.value().toInt())
				{
					CurStatusInfo m_statusInfo;
					m_statusInfo.m_CurStatus=IDeviceClient::STATUS_CONNECTED;
					m_StreamCurStatus.insert(0,m_statusInfo);
				}
				else if (IDeviceConnection::CS_Disconnected==it.value().toInt())
				{
					CurStatusInfo m_statusInfo;
					m_statusInfo.m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
					m_StreamCurStatus.insert(0,m_statusInfo);
				}
				else if (IDeviceConnection::CS_Disconnecting==it.value().toInt())
				{
					CurStatusInfo m_statusInfo;
					m_statusInfo.m_CurStatus=IDeviceClient::STATUS_DISCONNECTING;
					m_StreamCurStatus.insert(0,m_statusInfo);
				}
				else if (IDeviceConnection::CS_Connectting==it.value().toInt())
				{
					CurStatusInfo m_statusInfo;
					m_statusInfo.m_CurStatus=IDeviceClient::STATUS_CONNECTING;
					m_StreamCurStatus.insert(0,m_statusInfo);
				}
			}
		}
		//处理子码流的状态，只保存连接和不连接两种状态
		if ("Minor"==evMap.value("Stream"))
		{
			evMap.remove("Stream");
			QVariantMap::const_iterator it;
			for (it=evMap.begin();it!=evMap.end();++it)
			{
				QString sKey=it.key();
				if (IDeviceConnection::CS_Connected==it.value().toInt())
				{
					CurStatusInfo m_statusInfo;
					m_statusInfo.m_CurStatus=IDeviceClient::STATUS_CONNECTED;
					m_StreamCurStatus.insert(1,m_statusInfo);
				}
				else if (IDeviceConnection::CS_Disconnected==it.value().toInt())
				{
					CurStatusInfo m_statusInfo;
					m_statusInfo.m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
					m_StreamCurStatus.insert(1,m_statusInfo);
				}
				else if (IDeviceConnection::CS_Disconnecting==it.value().toInt())
				{
					CurStatusInfo m_statusInfo;
					m_statusInfo.m_CurStatus=IDeviceClient::STATUS_DISCONNECTING;
					m_StreamCurStatus.insert(1,m_statusInfo);
				}
				else if (IDeviceConnection::CS_Connectting==it.value().toInt())
				{
					CurStatusInfo m_statusInfo;
					m_statusInfo.m_CurStatus=IDeviceClient::STATUS_CONNECTING;
					m_StreamCurStatus.insert(1,m_statusInfo);
				}
			}
		}

	//处理IpcDeviceClient的状态
	//连接状态，两路都连接
	if (IDeviceClient::STATUS_CONNECTED==m_StreamCurStatus.value(0).m_CurStatus&&IDeviceClient::STATUS_CONNECTED==m_StreamCurStatus.value(1).m_CurStatus)
	{
		//如果原来状态不是连接状态，测抛出信号
		if (m_CurStatus!=IDeviceClient::STATUS_CONNECTED)
		{
			QVariantMap CurStatusParm;
			CurStatusParm.insert("CurrentStatus",IDeviceClient::STATUS_CONNECTED);
			eventProcCall("CurrentStatus",CurStatusParm);
		}
			m_CurStatus=IDeviceClient::STATUS_CONNECTED;
	}

	//断开状态，两路都断开
	if (IDeviceClient::STATUS_DISCONNECTED==m_StreamCurStatus.value(0).m_CurStatus&&IDeviceClient::STATUS_DISCONNECTED==m_StreamCurStatus.value(1).m_CurStatus)
	{
		//如果是正在连接过程，都没有连接，信号，由connect函数抛出，此处不做处理
		if (IDeviceClient::STATUS_CONNECTING==m_CurStatus)
		{
			return 0;
		}
		//如果原来不是断开状态，测抛出断开的信号
		if (IDeviceClient::STATUS_DISCONNECTED!=m_CurStatus)
		{
			QVariantMap CurStatusParm;
			CurStatusParm.insert("CurrentStatus",IDeviceClient::STATUS_DISCONNECTED);
			eventProcCall("CurrentStatus",CurStatusParm);
		}
		m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
	}
	//原来状态为连接，一路断开，即断开所有的连接
	if (IDeviceClient::STATUS_CONNECTED==m_CurStatus)
	{
		if (m_StreamCurStatus.value(0).m_CurStatus==IDeviceClient::STATUS_DISCONNECTED||m_StreamCurStatus.value(1).m_CurStatus==IDeviceClient::STATUS_DISCONNECTED)
		{
			if (false==bHadCallCloseAll)
			{
				//closeAll();
			}
		}
	}

	//原来状态为未连接，一路连接，do nothing
	return 0;
}

int IpcDeviceClient::SwitchStream( int StreamNum )
{
	m_IfSwithStream=StreamNum;
	return 0;
}
int IpcDeviceClient::SetAutoSycTime(bool bEnabled)
{
	m_bIsSycTime = bEnabled;
	return 0;
}
bool IpcDeviceClient::TryToConnectProtocol( CLSID clsid )
{
	int mount=0;
	IDeviceConnection *m_DeviceConnectProtocol=NULL;
	if (true==bCloseingFlags)
	{
		return false;
	}
	while(mount<2){
		pcomCreateInstance(clsid,NULL,IID_IDeviceConnection,(void**)&m_DeviceConnectProtocol);
		if (NULL==m_DeviceConnectProtocol)
		{
			//释放资源
			DeInitProtocl();
			return false;
		}
		//save to struct 
		SingleConnect m_SingleConnect;
		m_SingleConnect.m_DeviceConnecton=NULL;
		m_DeviceConnectProtocol->QueryInterface(IID_IDeviceConnection,(void**)&m_SingleConnect.m_DeviceConnecton);
		m_DeviceClentMap.insert(mount,m_SingleConnect);
		//注册事件
		IEventRegister *m_RegisterProc=NULL;
		m_DeviceConnectProtocol->QueryInterface(IID_IEventRegister,(void**)&m_RegisterProc);
		if (NULL==m_RegisterProc)
		{
			DeInitProtocl();
			m_DeviceConnectProtocol->Release();
			m_DeviceConnectProtocol=NULL;
			return false;
		}
		RegisterProc(m_RegisterProc,mount);
		m_RegisterProc->Release();

		//设置设备参数
		if (1==m_DeviceConnectProtocol->setDeviceHost(m_DeviceInfo.m_sAddr)||1==m_DeviceConnectProtocol->setDevicePorts(m_DeviceInfo.m_ports)||1==m_DeviceConnectProtocol->setDeviceId(m_DeviceInfo.m_sEseeId))
		{
			//设置失败，释放资源，直接返回失败
			DeInitProtocl();
			m_DeviceConnectProtocol->Release();
			m_DeviceConnectProtocol=NULL;
			return false;
		}
		if ("0"==m_DeviceInfo.m_sEseeId&&(clsid==CLSID_Hole||clsid==CLSID_Turn))
		{
			DeInitProtocl();
			m_DeviceConnectProtocol->Release();
			m_DeviceConnectProtocol=NULL;
			return false;
		}
		if (true==bCloseingFlags)
		{
			//要求停止连接
			//释放资源
			DeInitProtocl();
			m_DeviceConnectProtocol->Release();
			m_DeviceConnectProtocol=NULL;
			return false;
		}
			//连接设备
		if (1==m_DeviceConnectProtocol->connectToDevice())
		{
			//连接失败，释放资源，直接返回
			DeInitProtocl();
			m_DeviceConnectProtocol->Release();
			m_DeviceConnectProtocol=NULL;
			return false;
		}
		//使用次码流的连接发送云台控制命令
		if (1 == mount)
		{
			m_DeviceConnectProtocol->QueryInterface(IID_IProtocolPTZ, (void**)&m_pProtocolPTZ);
		}

		//连接成功
		m_DeviceConnectProtocol->Release();
		m_DeviceConnectProtocol=NULL;
		mount++;
	}
	return true;
}

int IpcDeviceClient::RegisterProc(IEventRegister *m_RegisterProc,int m_Stream )
{
	QMultiMap<QString ,IpcDeviceClientToProcInfoItem>::const_iterator it;
	for(it=m_EventMapToProc.begin();it!=m_EventMapToProc.end();++it){
		QString sKey=it.key();
		IpcDeviceClientToProcInfoItem sValue=it.value();
		if (sValue.Stream==m_Stream)
		{
			m_RegisterProc->registerEvent(sKey,sValue.proc,sValue.puser);
		}
	}
	return 0;
}

void IpcDeviceClient::DeInitProtocl()
{
	m_csDeInit.lock();
	QMultiMap<int,SingleConnect>::iterator it;
	for (it=m_DeviceClentMap.begin();it!=m_DeviceClentMap.end();it++)
	{
		if (NULL!=it->m_DeviceConnecton)
		{
			qDebug()<<"release======================"<<it->m_DeviceConnecton;
			it->m_DeviceConnecton->Release();
			it->m_DeviceConnecton=NULL;
		}
	}
	m_csDeInit.unlock();
}

int IpcDeviceClient::sndGetVesionInfo()
{
	QByteArray block = "GET /cgi-bin/gw2.cgi?f=j&xml=%3Cjuan%20ver%3D%22%22%20seq%3D%22%22%3E%3Cconf%20type%3D%22read%22%20user%3D%22admin%22%20password%3D%22%22%3E%3Cspec%20vin%3D%22%22%20ain%3D%22%22%20io_sensor%3D%22%22%20io_alarm%3D%22%22%20hdd%3D%22%22%20sd_card%3D%22%22%20%2F%3E%3Cinfo%20device_name%3D%22%22%20device_model%3D%22%22%20device_sn%3D%22%22%20hardware_version%3D%22%22%20software_version%3D%22%22%20build_date%3D%22%22%20build_time%3D%22%22%20%2F%3E%3C%2Fconf%3E%3C%2Fjuan%3E HTTP/1.1\r\n";
	block += "Connection: Keep-Alive\r\n";
	block += "\r\n";
	m_tcpSocket->write(block);
	if (m_tcpSocket->waitForBytesWritten(500))
	{
		return 0;
	}
	else
		return 1;
}
int IpcDeviceClient::sndSyncTimeForPreVersion()
{
	QByteArray block = "GET /cgi-bin/gw2.cgi?f=j&xml=";
	QDateTime time = QDateTime::currentDateTime();

	QString xmlStr = "<juan ver=\"\" seq=\"\"><setup type=\"write\" user=\"";
	xmlStr += m_DeviceInfo.m_sUserName + "\"" + " password=\"" + m_DeviceInfo.m_sPassword + "\">";
	xmlStr += "<time value=\"" + QString::number(time.toTime_t()) + "\" /></setup></juan>";
	
	block += QUrl::toPercentEncoding(xmlStr, "", "");
	block += " HTTP/1.1\r\n";
	block += "Connection: keep-alive\r\n";
	block += "\r\n";

	m_tcpSocket->write(block);
	if (m_tcpSocket->waitForBytesWritten(500))
	{
		return 0;
	}
	else
		return 1;
}
int IpcDeviceClient::sndGetLocalSystemTime()
{
	QByteArray block;
	block += "GET /netsdk/system/time/localtime HTTP/1.1\r\n";
	block += "Authorization: Basic YWRtaW46\r\n";
	block += "Accept: application/json, text/javascript, */*; q=0.01\r\n";
	block += "X-Requested-With: XMLHttpRequest\r\n";
	block += "Referer: http://" + m_DeviceInfo.m_sAddr.toLatin1() + "/view.html\r\n";
	block += "Host: " + m_DeviceInfo.m_sAddr.toLatin1() + "\r\n";
	block += "DNT: 1\r\n";
	block += "Connection: Keep-Alive\r\n";
	block += "Cookie: juanipcam_lang=zh-cn; login=admin%2C; sync_time=true; usr=" + m_DeviceInfo.m_sUserName + "; pwd=" + m_DeviceInfo.m_sPassword + "\r\n";
	block += "\r\n";
	m_tcpSocket->write(block);
	if (m_tcpSocket->waitForBytesWritten(500))
	{
		return 0;
	}
	else
		return 1;
}
int IpcDeviceClient::sndSyncTimeCmd()
{
	QByteArray block;
	block += "PUT /netsdk/system/time/localtime HTTP/1.1\r\n";
	block += "Authorization: Basic YWRtaW46\r\n";
	block += "Accept: application/json, text/javascript, */*; q=0.01\r\n";
	block += "X-Requested-With: XMLHttpRequest\r\n";
	block += "Referer: http://" + m_DeviceInfo.m_sAddr.toLatin1() + "/view.html\r\n";
	block += "Host: " + m_DeviceInfo.m_sAddr.toLatin1() + "\r\n";
	block += "DNT: 1\r\n";
	block += "Content-Length: 27Connection: Keep-Alive\r\n";
	block += "Cookie: juanipcam_lang=zh-cn; login=admin%2C; sync_time=true; usr=" + m_DeviceInfo.m_sUserName + "; pwd=" + m_DeviceInfo.m_sPassword + "\r\n";
	block += "\r\n";

	QDateTime curTime = QDateTime::currentDateTime();
	QString timeStr = curTime.toString("yyyy-MM-ddThh:mm:ss") + m_timeZone;
	block += "\"" + timeStr.toLatin1() + "\"";
	m_tcpSocket->write(block);
	if (m_tcpSocket->waitForBytesWritten(500))
	{
		return 0;
	}
	else
		return 1;
}
void IpcDeviceClient::SyncTime()
{
	if (m_DeviceInfo.m_sAddr.isEmpty() || m_DeviceInfo.m_ports.isEmpty())
	{
		return;
	}

	m_tcpSocket = new QTcpSocket;
	connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(Reveived()));

	m_tcpSocket->connectToHost(QHostAddress(m_DeviceInfo.m_sAddr), (quint16)m_DeviceInfo.m_ports["media"].toUInt());
	if (m_tcpSocket->waitForConnected(1000))
	{
		//get version info
		sndGetVesionInfo();
		m_steps = 1;
	}
}
void IpcDeviceClient::Reveived()
{
	QVariantMap item;
	if (m_tcpSocket->bytesAvailable() > 0)
	{
		QByteArray buffer = m_tcpSocket->readAll();
		if (buffer.contains("HTTP/1.1 200 OK"))
		{
			if (1 == m_steps)
			{
				int posStart = buffer.indexOf("software_version") + qstrlen("software_version=\\\"");
				int posEnd = buffer.indexOf("\\\"", posStart);

				QByteArray ms = buffer.mid(posStart);

				m_softwareVersion = buffer.mid(posStart, posEnd - posStart);
				if (m_softwareVersion.left(5) <= "1.1.3")
				{
					sndSyncTimeForPreVersion();
					m_steps = 2;
				}
				else
				{
					sndGetLocalSystemTime();
					m_steps = 3;
				}
			}
			else if (2 == m_steps)
			{
				//sync time over
				item.insert("statusCode", 0);
				item.insert("statusMsg", "OK");
				eventProcCall("SyncTimeMsg", item);

				m_steps = 0;
				m_tcpSocket->deleteLater();
				return;
			}
			else if (3 == m_steps)
			{
				int posStart = buffer.indexOf("\"") + 1;
				int posEnd = buffer.lastIndexOf("\"");
				QByteArray localSysTime = buffer.mid(posStart, posEnd - posStart);
				m_timeZone = localSysTime.right(6);
				sndSyncTimeCmd();
				m_steps = 4;
			}
			else if (4 == m_steps)
			{
				QString status;
				int code;
				QRegExp rx("[a-zA-Z]+");
				if (-1 != rx.indexIn(buffer, buffer.indexOf("statusMessage") + qstrlen("statusMessage")))
				{
					status = rx.cap(0);
				}
				rx = QRegExp("\\d+");
				if (-1 != rx.indexIn(buffer, buffer.indexOf("statusCode") + qstrlen("statusCode")))
				{
					code = rx.cap(0).toUInt();
				}

				item.insert("statusCode", code);
				item.insert("statusMsg", status);
				eventProcCall("SyncTimeMsg", item);

				m_steps = 0;
				m_tcpSocket->deleteLater();
			}
		}
		else
		{
			item.insert("statusCode", -1);
			item.insert("statusMsg", "failure");
			eventProcCall("SyncTimeMsg", item);

			m_steps = 0;
			m_tcpSocket->deleteLater();
		}
	}
}

int IpcDeviceClient::ControlPTZUp( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZUp(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZDown( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZDown(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZLeft( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZLeft(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZRight( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZRight(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZIrisOpen( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZIrisOpen(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZIrisClose( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZIrisClose(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZFocusFar( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZFocusFar(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZFocusNear( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZFocusNear(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZZoomIn( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZZoomIn(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZZoomOut( const int &nChl, const int &nSpeed )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nSpeed < 0 || nSpeed > 7)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZZoomOut(nChl, nSpeed);
}

int IpcDeviceClient::ControlPTZAuto( const int &nChl, bool bOpend )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZAuto(nChl, bOpend);
}

int IpcDeviceClient::ControlPTZStop( const int &nChl, const int &nCmd )
{
	if (NULL == m_pProtocolPTZ || nChl < 0 || nChl > 64 || nCmd < 0 || nCmd > 10)
	{
		return 1;
	}
	return m_pProtocolPTZ->PTZStop(nChl, nCmd);
}


int cbLiveStreamFrompPotocol_Primary( QString evName,QVariantMap evMap,void*pUser )
{
	if ("LiveStream"==evName)
	{
		evMap.insert("Stream","Primary");
		((IpcDeviceClient*)pUser)->cbLiveStream(evMap);
		return 0;
	}
	return 1;
}

int cbSocketErrorFrompPotocol_Primary( QString evName,QVariantMap evMap,void*pUser )
{
	if ("SocketError"==evName)
	{
		evMap.insert("Stream","Primary");
		((IpcDeviceClient*)pUser)->cbSocketError(evMap);
		return 0;
	}
	return 1;
}

int cbStateChangeFrompPotocol_Primary( QString evName,QVariantMap evMap,void*pUser )
{
	if ("StateChangeed"==evName)
	{
		evMap.insert("Stream","Primary");
		((IpcDeviceClient*)pUser)->cbConnectStatusProc(evMap);
		return 0;
	}
	return 1;
}

int cbLiveStreamFrompPotocol_Minor( QString evName,QVariantMap evMap,void*pUser )
{
	if ("LiveStream"==evName)
	{
		evMap.insert("Stream","Minor");
		((IpcDeviceClient*)pUser)->cbLiveStream(evMap);
		return 0;
	}
	return 1;
}

int cbSocketErrorFrompPotocol_Minor( QString evName,QVariantMap evMap,void*pUser )
{
	if ("SocketError"==evName)
	{
		evMap.insert("Stream","Minor");
		((IpcDeviceClient*)pUser)->cbSocketError(evMap);
		return 0;
	}
	return 1;
}

int cbStateChangeFrompPotocol_Minor( QString evName,QVariantMap evMap,void*pUser )
{
	if ("StateChangeed"==evName)
	{
		evMap.insert("Stream","Minor");
		((IpcDeviceClient*)pUser)->cbConnectStatusProc(evMap);
		return 0;
	}
	return 1;
}

