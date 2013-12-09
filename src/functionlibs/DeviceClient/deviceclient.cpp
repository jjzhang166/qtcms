#include "deviceclient.h"
#include <guid.h>

DeviceClient::DeviceClient():m_nRef(0),
	m_DeviceConnecton(NULL),
	bIsInitFlags(false)
{
	pcomCreateInstance(CLSID_RemotePreview,NULL,IID_IDeviceConnection,(void**)&m_DeviceConnecton);
	m_EventList<<"LiveStream"<<"SocketError"<<"StateChangeed";
}

DeviceClient::~DeviceClient()
{
	m_DeviceConnecton->Release();
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
	//把前一次的连接释放掉
	qDebug()<<"connectToDevice";
	qDebug()<<this;
	if (NULL==m_DeviceConnecton)
	{
		return 1;
	}
	if (IDeviceConnection::CS_Connected==m_DeviceConnecton->getCurrentStatus()||IDeviceConnection::CS_Connectting==m_DeviceConnecton->getCurrentStatus())
	{
		m_DeviceConnecton->disconnect();
		while(IDeviceConnection::CS_Disconnected!=m_DeviceConnecton->getCurrentStatus()){
			sleep(10);
		}
	}

	m_ports.insert("media",uiPort);

	if (1==m_DeviceConnecton->setDeviceHost(sAddr)||1==m_DeviceConnecton->setDevicePorts(m_ports)||1==m_DeviceConnecton->setDeviceId(sEseeId))
	{
		return 1;
	}

	if (false==bIsInitFlags)
	{
		if (1==cbInit())
		{
			return 1;
		}
	}
	int nRet=1;
	nRet=m_DeviceConnecton->connectToDevice();
	if (1==nRet)
	{
		qDebug()<<"can't connect";
		return 1;
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
		n_IRemotePreview->pauseStream(false);
	}
	n_IRemotePreview->Release();
	return 0;
}

int DeviceClient::closeAll()
{
	qDebug()<<"closeAll";
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
	n_IRemotePreview->stopStream();
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
	if (NULL==m_DeviceConnecton)
	{
		return 1;
	}
	int nRet=m_DeviceConnecton->getCurrentStatus();
	return nRet;
}
int DeviceClient::cbInit()
{
	//注册回调函数
	if (NULL==m_DeviceConnecton)
	{
		return 1;
	}
	IEventRegister *IEventReg=NULL;
	m_DeviceConnecton->QueryInterface(IID_IEventRegister,(void**)&IEventReg);
	if (NULL==IEventReg)
	{
		return 1;
	}
	QMultiMap<QString,DeviceClientInfoItem>::const_iterator it;
	for (it=m_EventMap.begin();it!=m_EventMap.end();++it)
	{
		QString sKey=it.key();
		DeviceClientInfoItem sValue=it.value();
		IEventReg->registerEvent(sKey,sValue.proc,sValue.puser);
	}
	IEventReg->Release();
	bIsInitFlags=true;
	return 0;
}