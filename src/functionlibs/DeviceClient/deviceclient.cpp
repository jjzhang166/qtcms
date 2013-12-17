#include "deviceclient.h"
#include <guid.h>

DeviceClient::DeviceClient():m_nRef(0),
	m_DeviceConnecton(NULL),
	m_DeviceConnectonBubble(NULL),
	m_DeviceConnectonHole(NULL),
	m_DeviceConnectonTurn(NULL),
	bIsInitFlags(false),
	m_CurStatus(IDeviceClient::STATUS_DISCONNECTED)
{
	pcomCreateInstance(CLSID_RemotePreview,NULL,IID_IDeviceConnection,(void**)&m_DeviceConnectonBubble);
	pcomCreateInstance(CLSID_Hole,NULL,IID_IDeviceConnection,(void**)&m_DeviceConnectonHole);
	pcomCreateInstance(CLSID_Turn,NULL,IID_IDeviceConnection,(void**)&m_DeviceConnectonTurn);
	m_EventList<<"LiveStream"<<"SocketError"<<"StateChangeed"<<"CurrentStatus";
}

DeviceClient::~DeviceClient()
{
	if (NULL!=m_DeviceConnectonBubble)
	{
		m_DeviceConnectonBubble->Release();
		m_DeviceConnectonBubble=NULL;
	}
	if (NULL!=m_DeviceConnectonHole)
	{
		m_DeviceConnectonHole->Release();
		m_DeviceConnectonHole=NULL;
	}
	if (NULL!=m_DeviceConnectonTurn)
	{
		m_DeviceConnectonTurn->Release();
		m_DeviceConnectonTurn=NULL;
	}
}

long _stdcall DeviceClient::QueryInterface(const IID & iid,void **ppv)
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
	else 
	{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	static_cast <IPcomBase*>(this)->AddRef();
	return S_OK;
}

unsigned long _stdcall DeviceClient::AddRef()
{
	m_csRef.lock();
	m_nRef++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long _stdcall DeviceClient::Release()
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

QStringList DeviceClient::eventList()
{
	return m_EventList;
}

int DeviceClient::queryEvent(QString eventName,QStringList& eventParams)
{
	if (!m_EventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	if ("LiveStream"==eventName)
	{
		eventParams<<"channel"<<"pts"<<"length"<<"data"<<"frametype"<<"width"<<"height"<<"vcodec"<<"samplerate"<<"samplewidth"<<"audiochannel"<<"acodec";
	}
	return IEventRegister::OK;
}

int DeviceClient::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
{
	if (!m_EventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	//需测试：是否重复注册
	if (m_EventMap.contains(eventName))
	{
		DeviceClientInfoItem devcliInfo;
		devcliInfo.proc=proc;
		devcliInfo.puser=pUser;
		m_EventMap.replace(eventName,devcliInfo);
		return IEventRegister::OK;
	}
	DeviceClientInfoItem devcliInfo;
	devcliInfo.proc=proc;
	devcliInfo.puser=pUser;

	m_EventMap.insert(eventName,devcliInfo);

	return IEventRegister::OK;
}

int DeviceClient::connectToDevice(const QString &sAddr,unsigned int uiPort,const QString &sEseeId)
{
	m_ports.insert("media",uiPort);
	//注册回调函数
	if (false==bIsInitFlags)
	{
		cbInit();
	}
	//检测上一次连接状况，如果是连接的则断开；
	if (NULL!=m_DeviceConnecton)
	{
		if (IDeviceConnection::CS_Connected==m_DeviceConnecton->getCurrentStatus()||IDeviceConnection::CS_Connectting==m_DeviceConnecton->getCurrentStatus())
		{
			m_DeviceConnecton->disconnect();
			while(IDeviceConnection::CS_Disconnected!=m_DeviceConnecton->getCurrentStatus()){
				m_CurStatus=IDeviceClient::STATUS_DISCONNECTING;
				sleep(10);
			}
			m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
		}
	}
	m_CurStatus=DeviceClient::STATUS_CONNECTING;
	QVariantMap CurStatusParm;
	CurStatusParm.insert("CurrentStatus",m_CurStatus);
	eventProcCall("CurrentStatus",CurStatusParm);
	//尝试连接
	int nStep=0;
	while(nStep!=5){
		switch(nStep){
			//尝试bubble协议连接
		case 0:
			{
				if (NULL==m_DeviceConnectonBubble)
				{
					nStep=1;
					break;
				}
				if (1==m_DeviceConnectonBubble->setDeviceHost(sAddr)||1==m_DeviceConnectonBubble->setDevicePorts(m_ports)||1==m_DeviceConnectonBubble->setDeviceId(sEseeId))
				{
					nStep=1;
					break;
				}
				int nRet=1;
				nRet=m_DeviceConnectonBubble->connectToDevice();
				if (1==nRet)
				{
					qDebug()<<"can't connect";
					nStep=1;
					break;
				}
				qDebug()<<"bubble success";
				m_DeviceConnecton=m_DeviceConnectonBubble;
				nStep=3;
			}
			break;
			//尝试穿透协议连接
		case 1:
			{
				if (NULL==m_DeviceConnectonHole)
				{
					nStep=2;
					break;
				}
				if (1==m_DeviceConnectonHole->setDeviceHost(sAddr)||1==m_DeviceConnectonHole->setDevicePorts(m_ports)||1==m_DeviceConnectonHole->setDeviceId(sEseeId))
				{
					nStep=2;
					break;
				}
				int nRet=1;
				nRet=m_DeviceConnectonHole->connectToDevice();
				if (1==nRet)
				{
					qDebug()<<"can't connect";
					nStep=2;
					break;
				}
				qDebug()<<"Hole success";
				m_DeviceConnecton=m_DeviceConnectonHole;
				nStep=3;
			}
			break;
			//尝试转发协议连接
		case 2:
			{
				if (NULL==m_DeviceConnectonTurn)
				{
					nStep=4;
					break;
				}
				if (1==m_DeviceConnectonTurn->setDeviceHost(sAddr)||1==m_DeviceConnectonTurn->setDevicePorts(m_ports)||1==m_DeviceConnectonTurn->setDeviceId(sEseeId))
				{
					nStep=4;
					break;
				}
				int nRet=1;
				nRet=m_DeviceConnectonTurn->connectToDevice();
				if (1==nRet)
				{
					qDebug()<<"can't connect";
					nStep=4;
					break;
				}
				qDebug()<<"turn success";
				m_DeviceConnecton=m_DeviceConnectonTurn;
				nStep=3;
			}
			break;
			//连接失败
		case 3:
			{
				m_CurStatus=IDeviceClient::STATUS_CONNECTED;
				QVariantMap CurStatusParm;
				CurStatusParm.insert("CurrentStatus",m_CurStatus);
				eventProcCall("CurrentStatus",CurStatusParm);
				nStep=5;
			}
			break;
			//连接成功
		case 4:
			{
				m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
				QVariantMap CurStatusParm;
				CurStatusParm.insert("CurrentStatus",m_CurStatus);
				eventProcCall("CurrentStatus",CurStatusParm);
				nStep=5;
			}
			break;
			//退出
		case 5:
			{
	
			}
			break;
		default:
			break;
		}
	}
	return 0;
}
int DeviceClient::checkUser(const QString & sUsername,const QString &sPassword)
{
	return 0;
}
int DeviceClient::setChannelName(const QString & sChannelName)
{
	m_sChannelName=sChannelName;
	return 0;
}
int DeviceClient::liveStreamRequire(int nChannel,int nStream,bool bOpen)
{
	qDebug()<<this;
	IRemotePreview *n_IRemotePreview=NULL;
	if (NULL==m_DeviceConnecton)
	{
		return 1;
	}
	m_DeviceConnecton->QueryInterface(IID_IRemotePreview,(void**)&n_IRemotePreview);
	if (NULL==n_IRemotePreview)
	{
		return 1;
	}
	//需要判定 是否已经连接
	//需要判定 是否已经请求过码流
	if (true==bOpen)
	{
		if (1==n_IRemotePreview->getLiveStream(nChannel,nStream))
		{
			n_IRemotePreview->Release();
			return 1;
		}
	}
	else if (false==bOpen)
	{
		n_IRemotePreview->pauseStream(true);
	}
	n_IRemotePreview->Release();
	return 0;
}

int DeviceClient::closeAll()
{
	IRemotePreview *n_IRemotePreview=NULL;
	if (NULL==m_DeviceConnecton)
	{
		return 1;
	}
	m_DeviceConnecton->QueryInterface(IID_IRemotePreview,(void**)&n_IRemotePreview);
	if (NULL==n_IRemotePreview)
	{
		return 1;
	}
	m_CurStatus=IDeviceClient::STATUS_DISCONNECTING;
	QVariantMap CurStatusParm;
	CurStatusParm.insert("CurrentStatus",m_CurStatus);
	eventProcCall("CurrentStatus",CurStatusParm);
	n_IRemotePreview->stopStream();
	while(IDeviceConnection::CS_Disconnected!=m_DeviceConnecton->getCurrentStatus())
	{
		msleep(100);
	}
	m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
	CurStatusParm.clear();
	CurStatusParm.insert("CurrentStatus",m_CurStatus);
	eventProcCall("CurrentStatus",CurStatusParm);
	n_IRemotePreview->Release();
	return 0;
}

QString DeviceClient::getVendor()
{
	QString sReturn;
	return sReturn;
}
int DeviceClient::getConnectStatus()
{
	return m_CurStatus;
}
int DeviceClient::ConnectStatusProc(QVariantMap evMap)
{
	//QVariantMap CurStatusParm;
	//CurStatusParm.insert("CurrentStatus",m_CurStatus);
	//eventProcCall("CurrentStatus",CurStatusParm);
	return 0;
}
void DeviceClient::eventProcCall(QString sEvent,QVariantMap param)
{
	if (m_EventList.contains(sEvent))
	{
		DeviceClientInfoItem eventDes=m_EventMap.value(sEvent);
		if (NULL!=eventDes.proc)
		{
			eventDes.proc(sEvent,param,eventDes.puser);
		}
	}
}
int DeviceClient::cbInit()
{
	DeviceClientInfoItem devcliInfo;
	devcliInfo.proc=cbStateChangeFormIprotocl;
	devcliInfo.puser=this;
	m_EventMap.insert("StateChangeed",devcliInfo);
	//注册bubb协议的回调函数
	if (NULL!=m_DeviceConnectonBubble)
	{
		IEventRegister *IEventReg=NULL;
		m_DeviceConnectonBubble->QueryInterface(IID_IEventRegister,(void**)&IEventReg);
		if (NULL!=IEventReg)
		{
			QMultiMap<QString,DeviceClientInfoItem>::const_iterator it;
			for (it=m_EventMap.begin();it!=m_EventMap.end();++it)
			{
				QString sKey=it.key();
				DeviceClientInfoItem sValue=it.value();
				IEventReg->registerEvent(sKey,sValue.proc,sValue.puser);
			}
			IEventReg->Release();
		}
	}
	//注册穿透协议的回调函数
	if (NULL!=m_DeviceConnectonHole)
	{
		IEventRegister *IEventReg=NULL;
		m_DeviceConnectonHole->QueryInterface(IID_IEventRegister,(void**)&IEventReg);
		if (NULL!=IEventReg)
		{
			QMultiMap<QString,DeviceClientInfoItem>::const_iterator it;
			for (it=m_EventMap.begin();it!=m_EventMap.end();++it)
			{
				QString sKey=it.key();
				DeviceClientInfoItem sValue=it.value();
				IEventReg->registerEvent(sKey,sValue.proc,sValue.puser);
			}
			IEventReg->Release();
		}
	}
	//注册转发协议的回调函数
	if (NULL!=m_DeviceConnectonTurn)
	{
		IEventRegister *IEventReg=NULL;
		m_DeviceConnectonTurn->QueryInterface(IID_IEventRegister,(void**)&IEventReg);
		if (NULL!=IEventReg)
		{
			QMultiMap<QString,DeviceClientInfoItem>::const_iterator it;
			for (it=m_EventMap.begin();it!=m_EventMap.end();++it)
			{
				QString sKey=it.key();
				DeviceClientInfoItem sValue=it.value();
				IEventReg->registerEvent(sKey,sValue.proc,sValue.puser);
			}
			IEventReg->Release();
		}
	}
	bIsInitFlags=true;
	return 0;
}
int cbStateChangeFormIprotocl(QString evName,QVariantMap evMap,void*pUser)
{
	qDebug("cbStateChangeFormIprotocl");
	if ("StateChangeed"==evName)
	{
		((DeviceClient*)pUser)->ConnectStatusProc(evMap);
		return 0;
	}
	return 1;
}