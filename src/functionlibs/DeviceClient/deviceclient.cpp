#include "deviceclient.h"
#include <guid.h>

DeviceClient::DeviceClient():m_nRef(0),
	m_DeviceConnecton(NULL),
	m_pRemotePlayback(NULL),
	m_nChannels(0),
	m_nSpeedRate(0),
	m_DeviceConnectonBubble(NULL),
	m_DeviceConnectonHole(NULL),
	m_DeviceConnectonTurn(NULL),
	bIsInitFlags(false),
	bCloseingFlags(false),
	m_CurStatus(IDeviceClient::STATUS_DISCONNECTED)
{
	pcomCreateInstance(CLSID_BubbleProtocol,NULL,IID_IDeviceConnection,(void**)&m_DeviceConnectonBubble);
	pcomCreateInstance(CLSID_Hole,NULL,IID_IDeviceConnection,(void**)&m_DeviceConnectonHole);
	pcomCreateInstance(CLSID_Turn,NULL,IID_IDeviceConnection,(void**)&m_DeviceConnectonTurn);
	m_EventList<<"LiveStream"<<"SocketError"<<"StateChangeed"<<"CurrentStatus"<<"foundFile"<<"recFileSearchFinished";

	if (NULL != m_DeviceConnectonBubble)
	{
		m_DeviceConnectonBubble->QueryInterface(IID_IRemotePlayback, (void**)&m_pRemotePlayback);
	}
}

DeviceClient::~DeviceClient()
{
	closeAll();
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
	if (NULL != m_pRemotePlayback)
	{
		m_pRemotePlayback->Release();
	}

	QMap<int, WndPlay>::iterator it;
	for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
	{
		if (NULL != it->playManager)
		{
			delete it->playManager;
			it->playManager = NULL;
		}

		msleep(10);
		if (NULL != it->bufferManager)
		{
			delete it->bufferManager;
			it->bufferManager = NULL;
		}
	}
	m_groupMap.clear();
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
	else if (IID_IDeviceSearchRecord==iid)
	{
		*ppv=static_cast<IDeviceSearchRecord*>(this);
	}
	else if (IID_IDeviceGroupRemotePlayback==iid)
	{
		*ppv=static_cast<IDeviceGroupRemotePlayback*>(this);
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
 	if ("foundFile" == eventName)
 	{
 		eventParams<<"channel"<<"types"<<"start"<<"end"<<"filename";
 	}
	if ("recFileSearchFinished" == eventName)
	{
		eventParams<<"total";
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
	// 检测状态
	// 已连接则返回错误
	Device_Debug("test");
	bCloseingFlags=false;
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
			//m_DeviceConnecton->disconnect();
			//while(IDeviceConnection::CS_Disconnected!=m_DeviceConnecton->getCurrentStatus()){
			//	m_CurStatus=IDeviceClient::STATUS_DISCONNECTING;
			//	sleep(10);
			//}
			//m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
			return 1;
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
				m_DeviceConnecton=m_DeviceConnectonBubble;
				if (true==bCloseingFlags)
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
					qDebug()<<"bubble can't connect";
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
				m_DeviceConnecton=m_DeviceConnectonHole;
				if (true==bCloseingFlags)
				{
					nStep=2;
					break;
				}
				nRet=m_DeviceConnectonHole->connectToDevice();
				if (1==nRet)
				{
					qDebug()<<"Hole can't connect";
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
				m_DeviceConnecton=m_DeviceConnectonTurn;
				if (true==bCloseingFlags)
				{
					nStep=4;
					break;
				}
				nRet=m_DeviceConnectonTurn->connectToDevice();
				if (1==nRet)
				{
					qDebug()<<"turn can't connect";
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
				bCloseingFlags=false;
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
				qDebug("%s,%d,%s", __FUNCTION__,__LINE__,__FILE__);
				qDebug()<<"connect fail";
				bCloseingFlags=false;
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
	m_sUserName = sUsername;
	m_sPassWord = sPassword;
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
		return 0;
	}
	m_DeviceConnecton->QueryInterface(IID_IRemotePreview,(void**)&n_IRemotePreview);
	if (NULL==n_IRemotePreview)
	{
		return 0;
	}
	m_CurStatus=IDeviceClient::STATUS_DISCONNECTING;
	QVariantMap CurStatusParm;
	CurStatusParm.insert("CurrentStatus",m_CurStatus);
	eventProcCall("CurrentStatus",CurStatusParm);
	bCloseingFlags=true;
	//n_IRemotePreview->stopStream();
	m_DeviceConnecton->disconnect();
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
	QVariantMap::const_iterator it;
	for (it=evMap.begin();it!=evMap.end();++it)
	{
		QString sKey=it.key();
		QString sValue=it.value().toString();
		qDebug()<<it.value().toInt();
		qDebug()<<m_CurStatus;
		if (IDeviceConnection::CS_Disconnected==it.value().toInt()&&m_CurStatus==IDeviceClient::STATUS_CONNECTED)
		{
			//m_CurStatus=IDeviceClient::STATUS_DISCONNECTED;
			//QVariantMap CurStatusParm;
			//CurStatusParm.insert("CurrentStatus",IDeviceClient::STATUS_DISCONNECTED);
			//eventProcCall("CurrentStatus",CurStatusParm);
		}
	}
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
			QString evName = QString("RecordStream");
			IEventReg->registerEvent(evName, cbRecordStream, &m_groupMap);

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

void DeviceClient::action(QString options, BufferManager *pBuffer)
{
	if ("StartPlay" == options)
	{
		QMap<int, WndPlay>::iterator it;
		for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
		{
			if (it->bufferManager == pBuffer)
			{
				it->playManager->setParamter(pBuffer, it->wnd);

				if (it->playManager->isRunning())
				{
					it->playManager->quit();
				}

				it->playManager->start();
			}
		}
	}
	else if ("Pause" == options)
	{
		m_pRemotePlayback->pausePlaybackStream(true);
	}
	else if ("Continue" == options)
	{
		m_pRemotePlayback->pausePlaybackStream(false);
	}
	else
	{
		//nothing
	}
}

bool DeviceClient::removeRepeatWnd(QWidget *wndID)
{
	QMap<int, WndPlay>::iterator it;
	for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
	{
		if (it->wnd == wndID)
		{
			int temp = ~m_nChannels;
			temp |= 1<<(it.key() - 1);
			m_nChannels = ~temp;
			m_groupMap.remove(it.key());

			return true;
		}
	}

	return false;
}

int DeviceClient::startSearchRecFile(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime)
{
	if (nChannel < 0 || nChannel > 31 || nTypes < 0 || nTypes > 15 || startTime >= endTime)
	{
		return 2;
	}

	int ret = 1;
	if (NULL == m_pRemotePlayback)
	{
		return 1;
	}

	IDeviceConnection *pDeviceConnection = NULL;
	m_pRemotePlayback->QueryInterface(IID_IDeviceConnection, (void**)&pDeviceConnection);
	if (NULL == pDeviceConnection)
	{
		return 1;
	}

	pDeviceConnection->setDeviceAuthorityInfomation(m_sUserName, m_sPassWord);

	ret = m_pRemotePlayback->startSearchRecFile(nChannel,nTypes, startTime, endTime);

	return ret;
}
//nChannel start from 0 to 31
int DeviceClient::AddChannelIntoPlayGroup(int nChannel,QWidget * wnd)
{
	if (4 <= m_groupMap.size() || nChannel < 0 || NULL == wnd)
	{
		return 1;
	}

	//insert new item into the map
	WndPlay wndPlay;
	if (1 != (m_nChannels>>(nChannel))&1)
	{
		m_nChannels |= 1<<(nChannel);

		wndPlay.bufferManager = new BufferManager();
		wndPlay.playManager = new PlayManager();

		removeRepeatWnd(wnd);
		wndPlay.wnd = wnd;

		QObject::connect(wndPlay.bufferManager, SIGNAL(action(QString, BufferManager*)), this, SLOT(action(QString, BufferManager*)));
		QObject::connect(wndPlay.playManager, SIGNAL(action(QString, BufferManager*)), this, SLOT(action(QString, BufferManager*)));

		m_groupMap.insert(nChannel, wndPlay);
	}
	else
	{
		m_groupMap[nChannel].wnd = wnd;
	}
	
	return 0;
}

int DeviceClient::GroupPlay(int nTypes,const QDateTime & start,const QDateTime & end)
{
	if (nTypes < 0 || nTypes > 15 || start >= end)
	{
		return 2;
	}
	if (NULL == m_pRemotePlayback || 0 != getConnectStatus())
	{
		return 1;
	}

	int nRet = m_pRemotePlayback->getPlaybackStreamByTime(m_nChannels, nTypes, start, end);

	return nRet;
}
QDateTime DeviceClient::GroupGetPlayedTime()
{
	QDateTime time;
	QTime secTime;

	if (m_groupMap.isEmpty())
	{
		return time;
	}

	QMap<int, WndPlay>::iterator it;
	it = m_groupMap.begin();

	if (NULL == it->playManager)
	{
		return time;
	}

	int seconds = 0;
	seconds = it->playManager->getPlayTime();

	time.setTime(secTime.addSecs(seconds));

	return time;
}
int DeviceClient::GroupPause()
{
	if (m_groupMap.isEmpty())
	{
		return 1;
	}

	QMap<int, WndPlay>::iterator it;
	for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
	{
		if (NULL == it->playManager)
		{
			continue;
		}

		it->playManager->pause(true);
	}
	return 0;
}
int DeviceClient::GroupContinue()
{
	if (m_groupMap.isEmpty())
	{
		return 1;
	}

	QMap<int, WndPlay>::iterator it;
	for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
	{
		if (NULL == it->playManager)
		{
			continue;
		}

		it->playManager->pause(false);
		g_pause.wakeOne();
	}
	return 0;
}
int DeviceClient::GroupStop()
{
	if (m_groupMap.isEmpty())
	{
		return 1;
	}

	if (NULL == m_pRemotePlayback)
	{
		return 1;
	}

	int nRet = m_pRemotePlayback->stopPlaybackStream();

	QMap<int, WndPlay>::iterator it;
	for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
	{
		if (NULL == it->bufferManager)
		{
			continue;
		}
		if (NULL == it->playManager)
		{
			continue;
		}
		g_pause.wakeOne();
		it->playManager->stop();
		it->bufferManager->emptyBuff();
	}

	m_groupMap.clear();

	return nRet;
}
bool DeviceClient::GroupEnableAudio(bool bEnable)
{
	if (m_groupMap.isEmpty())
	{
		return 1;
	}

	bool preStatus = false;
	QMap<int, WndPlay>::iterator it;
	for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
	{
		BufferManager *pBuffer = it->bufferManager;
		if (NULL == pBuffer)
		{
			continue;
		}
		preStatus = pBuffer->getAudioStatus();
		pBuffer->audioSwitch(bEnable);
	}

	return preStatus;
}
int DeviceClient::GroupSpeedFast()
{
	if (m_groupMap.isEmpty())
	{
		return 1;
	}

	QMap<int, WndPlay>::iterator it;
	for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
	{
		if (NULL == it->playManager)
		{
			continue;
		}
		m_nSpeedRate = 0;
		it->playManager->setPlaySpeed(1, 1);
	}
	return 0;
}
int DeviceClient::GroupSpeedSlow()
{
	if (m_groupMap.isEmpty())
	{
		return 1;
	}

	m_nSpeedRate++;

	QMap<int, WndPlay>::iterator it;
	for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
	{
		if (NULL == it->playManager)
		{
			continue;
		}

		it->playManager->setPlaySpeed(2, m_nSpeedRate);
	}
	return 0;
}
int DeviceClient::GroupSpeedNormal()
{
	if (m_groupMap.isEmpty())
	{
		return 1;
	}

	QMap<int, WndPlay>::iterator it;
	for (it = m_groupMap.begin(); it != m_groupMap.end(); it++)
	{
		if (NULL == it->playManager)
		{
			continue;
		}
		m_nSpeedRate = 0;
		it->playManager->setPlaySpeed(0, 1);
	}
	return 0;
}

