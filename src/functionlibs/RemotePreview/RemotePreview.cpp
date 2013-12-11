#include "RemotePreview.h"
#include <guid.h>
#include <QtGui/QMessageBox>
#include <QtCore/QEventLoop>
#include <QDateTime>
#include <QtEndian>
#include <QTimer>
#include "h264wh.h"



RemotePreview::RemotePreview(void):
m_nRef(0),
m_streanCount(0),
m_bPaused(false)
{
	m_eventList<<"LiveStream"<<"SocketError"<<"StateChangeed";

	m_pStreamProcess = new StreamProcess();
	m_pStreamProcess->moveToThread(&m_workerThread);
	connect(&m_workerThread, SIGNAL(finished()), m_pStreamProcess, SLOT(deleteLater()));
    connect(this, SIGNAL(childThreadToConn(QString , quint16 )),m_pStreamProcess,SLOT(conToHost(QString , quint16 )),Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(EndStream()), m_pStreamProcess, SLOT(stopStream()), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(writeSocket(QByteArray)), m_pStreamProcess,SLOT(socketWrites(QByteArray)),Qt::BlockingQueuedConnection);

	m_workerThread.start();
}

RemotePreview::~RemotePreview(void)
{
	m_workerThread.quit();
	m_workerThread.wait();
}

int RemotePreview::setDeviceHost(const QString & sAddr)
{
	if (m_hostAddress.setAddress(sAddr) == false)
	{
		return 1;
	}
	else
		return 0;
}

int RemotePreview::setDevicePorts(const QVariantMap & ports)
{
	if (ports["media"].toInt() <= 0 || ports["media"].toInt() >= 65535)
	{
		return 1;
	}

	m_ports = ports;

	return 0;
}

int RemotePreview::setDeviceId(const QString & sAddress)
{
	m_deviceId = sAddress;

	return 0;
}

int RemotePreview::setDeviceAuthorityInfomation(QString username,QString password)
{
	m_deviceUsername = username;
	m_devicePassword = password;

	return 0;
}

int RemotePreview::connectToDevice()
{
	int result = 1;

	m_pStreamProcess->setAddressInfo(m_hostAddress, m_ports["media"].toInt());
	emit childThreadToConn(m_hostAddress.toString(), m_ports["media"].toInt());


	QString block = "GET /bubble/live?ch=0&stream=0 HTTP/1.1\r\n\r\n";
	emit writeSocket(block.toAscii());

	g_mutex.lock();
	g_verify.wait(&g_mutex, 100);
	g_mutex.unlock();

	m_pStreamProcess->getStreamListInfo(m_lstStreamList);

	if (CS_Connected == m_pStreamProcess->getSocketState())
	{
		result = 0;
	}

	return result;
}


int RemotePreview::authority()
{
	char buff[100];
	QByteArray block;
	qint64 nLen = 0;
	Message *msgParam = NULL;
	Authority *auth = NULL;

	memset(buff, 0, 100);

	Bubble * bubble = (Bubble *)buff;
	bubble->cHead = (char)0xaa;

	QDateTime time = QDateTime::currentDateTime();
	bubble->uiTicket = qToBigEndian((unsigned int)(time.toMSecsSinceEpoch() * 1000));
	bubble->cCmd = 0x00;

	msgParam = (Message *)bubble->pLoad;
	msgParam->cMessage = 0x00;
	memset(msgParam->cReverse, 0x00, 3);
	msgParam->uiLength = sizeof(Message) + sizeof(Authority) - sizeof(msgParam->uiLength) - sizeof(msgParam->pParameters);

	auth = (Authority *)msgParam->pParameters;
	qstrcpy(auth->sUserName, m_deviceUsername.toUtf8().data());
	qstrcpy(auth->sPassWord, m_devicePassword.toUtf8().data());

	bubble->uiLength = sizeof(Bubble) + sizeof(Message) + sizeof(Authority) - sizeof(msgParam->pParameters)\
		- sizeof(bubble->cHead) - sizeof(bubble->uiLength) - sizeof(bubble->pLoad);


	nLen = (qint64)(bubble->uiLength + sizeof(bubble->cHead) + sizeof(bubble->uiLength));
	bubble->uiLength =  qToBigEndian(bubble->uiLength);
	msgParam->uiLength = qToBigEndian(msgParam->uiLength);
	
	block.append(buff, nLen);

	emit writeSocket(block);

	g_mutex.lock();
	g_verify.wait(&g_mutex, 100);
	g_mutex.unlock();
	if (1 == m_pStreamProcess->getVerifyResult())
	{
		return 0;
	}
	else
		return 1;
}

int RemotePreview::disconnect()
{
	if (IDeviceConnection::CS_Connected != getCurrentStatus())
	{
	    return 1;
	}
	else
	{
		emit EndStream();
	}
	return 0;
}

int RemotePreview::getCurrentStatus()
{
	return m_pStreamProcess->getSocketState();
}

QString RemotePreview::getDeviceHost()
{
	return m_hostAddress.toString();
}

QString RemotePreview::getDeviceid()
{
	return m_deviceId;
}

QVariantMap RemotePreview::getDevicePorts()
{
	return m_ports;
}


int RemotePreview::getLiveStream(int nChannel, int nStream)
{
	//¨¨Y¡ä¨ª
	if (m_lstStreamList.isEmpty() || nChannel< 0 || nStream <0)
	{
		return 1;
	}

	int ChannelCount = m_lstStreamList.size();
	QList<Stream> streamList;
	if ((nChannel >= ChannelCount && ChannelCount > 0))
	{
		nChannel = 0;
		nStream = 0;
	}
	
	streamList = m_lstStreamList.at(nChannel);
	if (nStream >= streamList.size())
	{
		nStream = 0;
	}


	m_channelNum = nChannel;
	m_streamNum = nStream;

	if (IDeviceConnection::CS_Connected == getCurrentStatus())
	{
		sendRequire(true);
	}

	return 0;
}

void RemotePreview::sendRequire(bool bSwitch)
{
	if (m_pStreamProcess->getSupportState())
	{
		sendLiveStreamRequireEx(bSwitch);
	}
	else
	{
		sendLiveStreamRequire(bSwitch);
	}
}

void RemotePreview::sendLiveStreamRequire(bool option)
{
	char buff[100];
	qint64 nLen = 0;
	LiveStreamRequire *msgParam;
	Bubble * bubble = (Bubble *)buff;
	bubble->cHead = (char)0xaa;

	QDateTime time = QDateTime::currentDateTime();
	bubble->uiTicket = qToBigEndian((unsigned int)(time.toMSecsSinceEpoch() * 1000));
	bubble->cCmd = 0x04;
	bubble->uiLength = sizeof(Bubble) - sizeof(bubble->pLoad) + sizeof(LiveStreamRequire)\
		- sizeof(bubble->cHead) - sizeof(bubble->uiLength);

	msgParam = (LiveStreamRequire *)bubble->pLoad;
	msgParam->cChannel = m_channelNum;
	msgParam->cOperation = option;
	nLen = (qint64)(bubble->uiLength + sizeof(bubble->cHead) + sizeof(bubble->uiLength));
	bubble->uiLength =  qToBigEndian(bubble->uiLength);

	QByteArray block;
	block.append(buff, nLen);

	emit writeSocket(block);

}

void RemotePreview::sendLiveStreamRequireEx(bool option)
{
	char buff[100];
	qint64 nlength = 0;
	Bubble *data = NULL;

	memset(buff, 0, 100);

	//write head
	data = (Bubble *)buff;
	data->cHead = (char)0xaa;
	data->cCmd  = (char)0x0a;

	QDateTime time = QDateTime::currentDateTime();
	unsigned int ticket = (unsigned int)(time.toMSecsSinceEpoch() * 1000);
	data->uiTicket = qToBigEndian(ticket);

	LiveStreamRequireEx *plsre = (LiveStreamRequireEx*)data->pLoad;
	plsre->uiChannel = m_channelNum;
	plsre->uiStream = m_streamNum;
	plsre->uiOperation = option;
	plsre->uiReversed = 0;

	nlength = qint64(sizeof(Bubble) + sizeof(LiveStreamRequireEx) - sizeof(data->pLoad));
	data->uiLength = sizeof(Bubble) - sizeof(data->pLoad) + sizeof(LiveStreamRequireEx)\
		- sizeof(data->cHead) - sizeof(data->uiLength);

	data->uiLength = qToBigEndian(data->uiLength);

	QByteArray block;
	block.append(buff, nlength);

	emit writeSocket(block);
}

int RemotePreview::stopStream()
{
	return disconnect();
}
int RemotePreview::pauseStream(bool bPaused)
{
	if (bPaused)//pause stream
	{
		m_bPaused = bPaused;
		if (CS_Connected == m_pStreamProcess->getSocketState())
		{
			sendRequire(false);
		}
	}
	//if push pause before
	if (!bPaused && m_bPaused)
	{
		m_bPaused = bPaused;
		if (CS_Connected == m_pStreamProcess->getSocketState())
		{
			sendRequire(true);
		}
	}

	return 0;
}
int RemotePreview::getStreamCount()
{
	QList<Stream> lstStreamList = m_lstStreamList.at(m_channelNum);
	return lstStreamList.size();
}
int RemotePreview::getStreamInfo(int nStreamId,QVariantMap &streamInfo)
{
	QList<Stream> lstStreamList = m_lstStreamList.at(m_channelNum);

	if (lstStreamList.isEmpty() || lstStreamList.size() <= nStreamId)
	{
		return 1;
	}
	Stream sStream = lstStreamList.at(nStreamId);

	streamInfo.insert("name", sStream.sName);
	streamInfo.insert("size", sStream.sSize);
	streamInfo.insert("x1", sStream.sx1);
	streamInfo.insert("x2", sStream.sx2);
	streamInfo.insert("x4", sStream.sx4);

	return 0;
}

 QStringList RemotePreview::eventList()
 {
 	return m_eventList;
 }
 int RemotePreview::queryEvent(QString eventName,QStringList& eventParams)
 {
 	if (!m_eventList.contains(eventName))
 	{
 		return IEventRegister::E_EVENT_NOT_SUPPORT;
 	}
 	if ("LiveStream" == eventName)
 	{
 		eventParams<<"channel"<<"pts"<<"length"<<"data"<<"frametype"<<"width"<<"height"<<"vcodec"<<"samplerate"<<"samplewidth"<<"audiochannel"<<"acodec";
 	}

	if ("SocketError" == eventName)
	{
		eventParams<<"connectionStatus"<<"errorValue"<<"errorDescription";
	}

	if ("StateChangeed" == eventName)
	{
		eventParams<<"status";
	}
 
 	return IEventRegister::OK;
 }
 int RemotePreview::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
 {
 	if (!m_eventList.contains(eventName))
 	{
 		return IEventRegister::E_EVENT_NOT_SUPPORT;
 	}
 
 	ProcInfoItem proInfo;
 	proInfo.proc = proc;
 	proInfo.puser = pUser;
 
 	m_eventMap.insert(eventName, proInfo);

	m_pStreamProcess->setEventMap(m_eventList, m_eventMap);
 	return IEventRegister::OK;
 }

long __stdcall RemotePreview::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IRemotePreview == iid)
	{
		*ppv = static_cast<IRemotePreview *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IEventRegister == iid)
	{
		*ppv = static_cast<IEventRegister *>(this);
	}
	else if (IID_IDeviceConnection == iid)
	{
		*ppv = static_cast<IDeviceConnection *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall RemotePreview::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall RemotePreview::Release()
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

