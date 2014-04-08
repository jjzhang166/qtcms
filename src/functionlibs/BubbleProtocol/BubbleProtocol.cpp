#include "BubbleProtocol.h"
#include <guid.h>
#include <QtGui/QMessageBox>
#include <QtCore/QEventLoop>
#include <QDateTime>
#include <QtEndian>
#include <QTimer>
#include "h264wh.h"



BubbleProtocol::BubbleProtocol(void):
m_nRef(0),
m_bIsPreviewPaused(false),
m_bIsPreviewStopped(true),
m_bIsPlaybackPaused(false),
m_bIsPlaybackStopped(true),
m_bIsResearch(false),
m_nRecordNum(0),
m_bIsPostSuccessed(false),
m_channelNum(0),
m_streamNum(0),
m_streanCount(0)
{
	m_eventList<<"LiveStream"           <<"SocketError"<<"StateChangeed"<<"foundFile"
               <<"recFileSearchFinished"<<"RecordStream"  <<"SocketError"  <<"StateChanged";

// 	m_timer.singleShot(10000, this, SLOT(sendHeartBeat()));

    m_manager = new QNetworkAccessManager(this);
	m_pStreamProcess = new StreamProcess();
	m_pStreamProcess->moveToThread(&m_workerThread);
	connect(&m_workerThread, SIGNAL(finished()), m_pStreamProcess, SLOT(deleteLater()));
    connect(this, SIGNAL(sigChildThreadToConn(QString , quint16 )),m_pStreamProcess,SLOT(conToHost(QString , quint16 )),Qt::BlockingQueuedConnection);
    connect(this, SIGNAL(sigEndStream()), m_pStreamProcess, SLOT(stopStream())/*, Qt::BlockingQueuedConnection*/);
	connect(this, SIGNAL(sigWriteSocket(QByteArray)), m_pStreamProcess,SLOT(socketWrites(QByteArray)),Qt::BlockingQueuedConnection);

	m_workerThread.start();
}

BubbleProtocol::~BubbleProtocol(void)
{
//	m_pStreamProcess->stopStream();
	emit sigEndStream();

	m_workerThread.quit();
	m_workerThread.wait();
	//delete m_pStreamProcess;
}
// inherited from IDeviceConnection  >>>>>
int BubbleProtocol::setDeviceHost(const QString & sAddr)
{
	if (!m_hostAddress.setAddress(sAddr))
	{
		return 1;
	}
	else
		return 0;
}

int BubbleProtocol::setDevicePorts(const QVariantMap & ports)
{
	if (ports["media"].toInt() <= 0 || ports["media"].toInt() >= 65535)
	{
		return 1;
	}
	m_ports = ports;
	return 0;
}

int BubbleProtocol::setDeviceId(const QString & sAddress)
{
	m_deviceId = sAddress;
	return 0;
}

int BubbleProtocol::setDeviceAuthorityInfomation(QString username,QString password)
{
	m_deviceUsername = username;
	m_devicePassword = password;

	return 0;
}

int BubbleProtocol::connectToDevice()
{
	int result = 1;

	m_pStreamProcess->setAddressInfo(m_hostAddress, m_ports["media"].toInt());
	emit sigChildThreadToConn(m_hostAddress.toString(), m_ports["media"].toInt());

 	QEventLoop loop;
 	QTimer::singleShot(1000, &loop, SLOT(quit()));
 	connect(this, SIGNAL(sigQuitThread()), &loop, SLOT(quit()));
 	loop.exec();
 
  	QString block = "GET /bubble/live?ch=0&stream=0 HTTP/1.1\r\n\r\n";
  	emit sigWriteSocket(block.toAscii());
  
  	g_mutex.lock();
  	g_verify.wait(&g_mutex, 100);
  	g_mutex.unlock();
  
	if (CS_Connected == m_pStreamProcess->getSocketState())
	{
		result = 0;
	}

	return result;
}

void BubbleProtocol::finishReply()
{
    int total = 0;
    if (m_reply->hasRawHeader("Content-Length"))
    {
        total = m_reply->header(QNetworkRequest::ContentLengthHeader).toUInt();
    }

    m_block += m_reply->readAll();
    if(m_block.size() < total)
    {
        return;
    }

    QVariant statusCode = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (200 != statusCode.toInt())
    {
		QVariantMap recordTotal;
		recordTotal.insert("total", QString("%1").arg(0));
		eventProcCall(QString("recFileSearchFinished"), recordTotal); 
		m_block.clear();
        return;
    }
    int pos = m_block.indexOf("<");
    QString xml = m_block.mid(pos, m_block.lastIndexOf(">") + 1 - pos);


    QDomDocument *dom = new QDomDocument();
    if(!dom->setContent(xml))
    {
        delete dom;
        dom = NULL;
        return;
    }

    m_bIsPostSuccessed = true;
    extractRecordInfo(dom);
    

    m_block.clear();
    m_reply->disconnect(m_reply, SIGNAL(readyRead()), this, SLOT(finishReply()));
    m_reply->deleteLater();
    delete dom;
    dom = NULL;

    emit sigQuitThread();
}

void BubbleProtocol::extractRecordInfo(QDomDocument* dom)
{
    if (NULL == dom)
    {
        return;
    }

    QDomNodeList searchNodeList = dom->elementsByTagName("recsearch");
    QDomNode subnode = searchNodeList.at(0);
	int toalnum=subnode.toElement().attribute("session_total").toInt();
    QDomNodeList recordList = dom->elementsByTagName("s");
	m_nRecordNum = recordList.size();

    QVariantMap recordTotal;
   /* recordTotal.insert("total", QString("%1").arg(m_nRecordNum));*/
	 recordTotal.insert("total", QString("%1").arg(toalnum));
	if (m_ReSearchInfo.session_index==0)
	{
		 eventProcCall(QString("recFileSearchFinished"), recordTotal); 
	}
  
    for (int i = 0; i < m_nRecordNum; i++)
    {
        Record record;
        QDomNode reNode = recordList.at(i);
        QDomElement element = reNode.toElement();
        QString recordValue = element.text();
        QStringList strList = recordValue.split("|");

        setRecordInfo(record, strList);
        m_lstRecordList.append(record);

        QVariantMap recordInfo;
        recordInfo.insert("channel", record.cChannel);
        recordInfo.insert("types", record.cTypes);
        recordInfo.insert("start", record.StartTime.toString("yyyy-MM-dd hh:mm:ss"));
        recordInfo.insert("end", record.EndTime.toString("yyyy-MM-dd hh:mm:ss"));
        recordInfo.insert("filename", record.sFileName);
		recordInfo.insert("index",m_ReSearchInfo.session_index+i);

        eventProcCall(QString("foundFile"), recordInfo);
    }
	//save reserchInfo
	m_ReSearchInfo.session_total=toalnum;
}

void BubbleProtocol::setRecordInfo(Record& record, QStringList strList)
{
	if (strList.isEmpty())
	{
		return;
	}
	QDateTime time1 = QDateTime::currentDateTime();
	QDateTime time2 = QDateTime::currentDateTimeUtc();
	int timeDifference = qAbs(time1.time().hour() - time2.time().hour())*3600;

	int typeNum = strList.at(3).toInt();
	record.cChannel = strList.at(2).toInt();
	record.cTypes = typeNum;
	record.StartTime = QDateTime::fromTime_t(strList.at(4).toInt());
	record.StartTime = record.StartTime.addSecs(0 - timeDifference);
	record.EndTime = QDateTime::fromTime_t(strList.at(5).toInt());
	record.EndTime = record.EndTime.addSecs(0 - timeDifference);
	record.sFileName = record.StartTime.toString("yyyyMMddhhmmss") + "_" + record.EndTime.toString("yyyyMMddhhmmss");

	QString sType;
	if (1 == (int)(typeNum & 1) )
	{
		sType += "T";
	}
	if (2 == (int)(typeNum & 2))
	{
		sType += "M";
	}
	if (4 == (int)(typeNum & 4))
	{
		sType += "S";
	}
	if (8 == (int)(typeNum & 8))
	{
		sType += "H";
	}

	record.sFileName += "_" + sType;
}

void BubbleProtocol::sendHeartBeat()
{
    char buff[100];
    qint64 nLen = 0;

    Bubble * bubble = (Bubble *)buff;
    if(m_bIsPreviewStopped)
    {
        bubble->cHead = (char)0xab;
        bubble->cCmd = 0x07;
    }
    else
    {
            bubble->cHead = (char)0xaa;
            bubble->cCmd = 0x02;
    }

    QDateTime time = QDateTime::currentDateTime();
    bubble->uiTicket = qToBigEndian((unsigned int)(time.toMSecsSinceEpoch() * 1000));
    
    bubble->pLoad[0] = 0x02;
    bubble->uiLength = sizeof(Bubble) - sizeof(bubble->cHead) - sizeof(bubble->uiLength);

    nLen = (qint64)(bubble->uiLength + sizeof(bubble->cHead) + sizeof(bubble->uiLength));
    bubble->uiLength = qToBigEndian(bubble->uiLength);

    QByteArray block;
    block.append(buff, nLen);

    emit sigWriteSocket(block);

}


int BubbleProtocol::authority()
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

	emit sigWriteSocket(block);

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

int BubbleProtocol::disconnect()
{
	//if has socket connecting, stop it
	emit sigQuitThread();
	emit sigEndStream();
	//m_pStreamProcess->stopStream();

	QTime time;
	time.start();
	while (time.elapsed() <= 100&&CS_Connected==getCurrentStatus())
	{
	}

    int nTmp = getCurrentStatus();
	if (CS_Connected == nTmp)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int BubbleProtocol::getCurrentStatus()
{
	return m_pStreamProcess->getSocketState();
}

QString BubbleProtocol::getDeviceHost()
{
	return m_hostAddress.toString();
}

QString BubbleProtocol::getDeviceid()
{
	return m_deviceId;
}

QVariantMap BubbleProtocol::getDevicePorts()
{
	return m_ports;
}
// inherited from IDeviceConnection <<<< 
// inherited from IRemotePlayback  >>>>>
int BubbleProtocol::startSearchRecFile(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime)
{
    m_bIsPreviewStopped  = true;
    m_bIsPlaybackStopped = false;
    if (nChannel < 0 || nTypes < 0 || startTime > endTime||m_bIsResearch==true)
    {
        return 2;
    }
	//save researchinfo
	m_ReSearchInfo.session_count=99;
	m_ReSearchInfo.session_index=0;
	m_ReSearchInfo.session_total=0;
	// research
	bool bSearch=true;
	while(bSearch){
		m_bIsResearch=true;
		QUrl url;
		url.setScheme(QLatin1String("http"));
		url.setHost(m_hostAddress.toString());
		url.setPath("cgi-bin/gw.cgi");
		url.setPort(m_ports["media"].toInt());

		QString sndData(QString("<juan ver=\"%1\" squ=\"%2\" dir=\"%3\">\n    <recsearch usr=\"%4\" pwd=\"%5\" channels=\"%6\" types=\"%7\" date=\"%8\" begin=\"%9\" end=\"%10\" session_index=\"%11\" session_count=\"%12\" />\n</juan>\n").arg("").arg(1).arg("").arg(m_deviceUsername).arg(m_devicePassword).arg(nChannel).arg(nTypes).arg(startTime.date().toString("yyyy-MM-dd")).arg(startTime.time().toString("hh:mm:ss")).arg(endTime.time().toString("hh:mm:ss")).arg(m_ReSearchInfo.session_index).arg(m_ReSearchInfo.session_count));

		QNetworkRequest request;
		request.setUrl(url);
		request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		m_reply = m_manager->post(request, sndData.toUtf8());
		connect(m_reply, SIGNAL(readyRead()), this, SLOT(finishReply()));

		QEventLoop loop;
		connect(this, SIGNAL(sigQuitThread()), &loop, SLOT(quit()));
		QTimer::singleShot(5000,&loop, SLOT(quit()));
		loop.exec();
		//reSearch
		if (m_ReSearchInfo.session_total-1>m_ReSearchInfo.session_index+100)
		{
			m_ReSearchInfo.session_index+=100;
		}else{
			m_bIsResearch=false;
			bSearch=false;
		}
	}


	if (!m_lstRecordList.isEmpty())
	{
		return 0;
	}
	else
	    return 1;
}

int BubbleProtocol::writeBuff(QByteArray &block, int nChannel, int nTypes, uint nStartTime, uint nEndTime)
{
    //if (nChannel<0 || nChannel > 32)
    //{
    //    return 2;
    //}
	if (nChannel<0)
	{
		return 2;
	}
    char cBuff[50];
    uint nLength = 0;
    Bubble *bubble = NULL;
    memset(cBuff, 0, sizeof(cBuff));
    bubble = (Bubble *)cBuff;
    bubble->cHead = (char)0xab;
    bubble->cCmd  = (char)0x05; 
    nLength = sizeof(Bubble) - sizeof(bubble->pLoad) + sizeof(RecordRequireV2);
    bubble->uiLength = qToBigEndian(nLength - sizeof(bubble->cHead) -sizeof(bubble->uiLength));
    QDateTime time = QDateTime::currentDateTime();
    bubble->uiTicket = qToBigEndian(unsigned int (time.toMSecsSinceEpoch() * 1000));

    RecordRequireV2 *pRecordRequire = (RecordRequireV2 *)bubble->pLoad;
    pRecordRequire->nChannels = nChannel;
    pRecordRequire->nTypes = nTypes;
    pRecordRequire->nStart = nStartTime;
    pRecordRequire->nEnd   = nEndTime;

    block.clear();
    block.append(cBuff, nLength);
    return 0;
}

int BubbleProtocol::isFileExist(QString fileName)
{
    Record record;
    for (int i = 0; i < m_lstRecordList.size(); i++)
    {
        record = m_lstRecordList.at(i);
        if (record.sFileName == fileName)
        {
            return 0;
        }
    }
    return 1;
}

int BubbleProtocol::getPlaybackStreamByTime(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime)
{
    m_bIsPreviewStopped  = true;
    m_bIsPlaybackStopped = false;
    if (nChannel < 0 || nTypes < 0 || startTime > endTime )
    {
        return 2;
    }

	QDateTime time1 = QDateTime::currentDateTime();
	QDateTime time2 = QDateTime::currentDateTimeUtc();
	int timeDifference = qAbs(time1.time().hour() - time2.time().hour())*3600;

    uint nStartTime = startTime.toTime_t();
    uint nEndTime   = endTime.toTime_t();

    QString sendData = "GET /bubble/live?ch=0&stream=0 HTTP/1.1\r\n\r\n";
    emit sigWriteSocket(sendData.toAscii());

    QByteArray block;
    if (0 == writeBuff(block, nChannel,nTypes, nStartTime + timeDifference, nEndTime + timeDifference))
    {
        emit sigWriteSocket(block);
        return 0;
    }
    else
        return 1;
}

int BubbleProtocol::getPlaybackStreamByFileName(int nChannel,const QString &sFileName)
{
    m_bIsPreviewStopped  = true;
    m_bIsPlaybackStopped = false;
    if (nChannel < 0 || sFileName.isEmpty() || 1 == isFileExist(sFileName))
    {
        return 2;
    }
    QString sendData = "GET /bubble/live?ch=0&stream=0 HTTP/1.1\r\n\r\n";
    emit sigWriteSocket(sendData.toAscii());

    QStringList list1 = sFileName.split("_",QString::SkipEmptyParts);
    if (list1.size() != 3)
    {
        return 1;
    }
    unsigned int nStartTime = QDateTime::fromString(list1.at(0),"yyyyMMddhhmmss").toTime_t();
    unsigned int nEndTime   = QDateTime::fromString(list1.at(1),"yyyyMMddhhmmss").toTime_t();

	QDateTime time1 = QDateTime::currentDateTime();
	QDateTime time2 = QDateTime::currentDateTimeUtc();
	int timeDifference = qAbs(time1.time().hour() - time2.time().hour())*3600;

	nStartTime += timeDifference;
	nEndTime += timeDifference;

    QString sPlayBackType = list1.at(2);
    int nTmp = 0;

    if (sPlayBackType.contains("T"))
    {
        nTmp += 1;
    }
    if (sPlayBackType.contains("M"))
    {
        nTmp += 2;
    }
    if (sPlayBackType.contains("S"))
    {
        nTmp += 4;
    }
    if (sPlayBackType.contains("H"))
    {
        nTmp += 8;
    }
    if (0 == nTmp)
    {
        nTmp = 15;
    }
    QByteArray block;
    if (0 == writeBuff(block, nChannel,nTmp, nStartTime, nEndTime))
    {
        emit sigWriteSocket(block);
        return 0;
    }
    else
        return 1;
}

int BubbleProtocol::pausePlaybackStream(bool bPause)
{
    if (bPause && !m_bIsPlaybackPaused && (2 == getCurrentStatus()) && m_bIsPreviewStopped)
    {
        m_bIsPlaybackPaused = true;
        char cBuff[20];
        Bubble *bubble = NULL;
        memset(cBuff, 0, sizeof(cBuff));
        bubble = (Bubble *)cBuff;
        bubble->cHead = (char)0xab;
        bubble->cCmd  = (char)0x02; //pause
        bubble->uiLength = qToBigEndian(sizeof(bubble->cCmd) + sizeof(bubble->uiTicket));
        QDateTime time = QDateTime::currentDateTime();
        bubble->uiTicket = qToBigEndian(unsigned int (time.toMSecsSinceEpoch() * 1000));
        QByteArray block;
        block.append(cBuff, (qint64)(sizeof(Bubble) - 1));
        emit sigWriteSocket(block);
        return 0;
    }
    else if (!bPause && m_bIsPlaybackPaused && (2 == getCurrentStatus())&& m_bIsPreviewStopped)
    {
        m_bIsPlaybackPaused = false;
        char cBuff[20];
        Bubble *bubble = NULL;
        memset(cBuff, 0, sizeof(cBuff));
        bubble = (Bubble *)cBuff;
        bubble->cHead = (char)0xab;
        bubble->cCmd  = (char)0x03; //continue
        bubble->uiLength = qToBigEndian(sizeof(bubble->cCmd) + sizeof(bubble->uiTicket));
        QDateTime time = QDateTime::currentDateTime();
        bubble->uiTicket = qToBigEndian(unsigned int (time.toMSecsSinceEpoch() * 1000));
        QByteArray block;
        block.append(cBuff, (qint64)(sizeof(Bubble) - 1));
        emit sigWriteSocket(block);
        return 0;
    }
    else
        return 1;
}

int BubbleProtocol::stopPlaybackStream()
{
    char cBuff[20];
    Bubble *bubble = NULL;
    memset(cBuff, 0, sizeof(cBuff));

    m_bIsPreviewStopped  = true;
    m_bIsPlaybackStopped = true;
    bubble = (Bubble *)cBuff;
    bubble->cHead = (char)0xab;
    bubble->cCmd  = (char)0x04; //stop
    bubble->uiLength = qToBigEndian(sizeof(bubble->cCmd) + sizeof(bubble->uiTicket));
    QDateTime time = QDateTime::currentDateTime();
    bubble->uiTicket = qToBigEndian(unsigned int (time.toMSecsSinceEpoch() * 1000));
    QByteArray block;
    block.append(cBuff, (qint64)(sizeof(Bubble) - 1));
    emit sigWriteSocket(block);

	disconnect();

    return 0;

}
// inherited from IRemotePlayback <<<< 

// inherited from IRemotePreview  >>>>>
int BubbleProtocol::getLiveStream(int nChannel, int nStream)
{
	if (nChannel< 0 || nStream <0)
	{
		return 1;
	}

    m_bIsPreviewStopped  = false;
    m_bIsPlaybackStopped = true;

	m_channelNum = nChannel;
	m_streamNum = nStream;

	if (IDeviceConnection::CS_Connected == getCurrentStatus())
	{
		sendRequire(true);
	}

	return 0;
}

void BubbleProtocol::sendRequire(bool bSwitch)
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

void BubbleProtocol::sendLiveStreamRequire(bool option)
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

	emit sigWriteSocket(block);

}

void BubbleProtocol::sendLiveStreamRequireEx(bool option)
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

	emit sigWriteSocket(block);
}

int BubbleProtocol::stopStream()
{
    m_bIsPreviewStopped  = true;
    m_bIsPlaybackStopped = true;
	return disconnect();
}
int BubbleProtocol::pauseStream(bool bPaused)
{
	if (bPaused && m_bIsPlaybackStopped)//pause stream
	{
		m_bIsPreviewPaused = bPaused;
		if (CS_Connected == m_pStreamProcess->getSocketState())
		{
			sendRequire(false);
		}
	}
	//if push pause before
	if (!bPaused && m_bIsPreviewPaused && m_bIsPlaybackStopped)
	{
		m_bIsPreviewPaused = bPaused;
		if (CS_Connected == m_pStreamProcess->getSocketState())
		{
			sendRequire(true);
		}
	}

	return 0;
}
int BubbleProtocol::getStreamCount()
{
	if (m_lstStreamList.isEmpty())
	{
		m_pStreamProcess->getStreamListInfo(m_lstStreamList);
	}

	QList<Stream> lstStreamList = m_lstStreamList.at(m_channelNum);
	return lstStreamList.size();
}
int BubbleProtocol::getStreamInfo(int nStreamId,QVariantMap &streamInfo)
{
	if (m_lstStreamList.isEmpty())
	{
		m_pStreamProcess->getStreamListInfo(m_lstStreamList);
	}

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
 // inherited from IRemotePreview <<<< 

// inherited from IEventRegister  >>>>>
 QStringList BubbleProtocol::eventList()
 {
 	return m_eventList;
 }
 int BubbleProtocol::queryEvent(QString eventName,QStringList& eventParams)
 {
 	if (!m_eventList.contains(eventName))
 	{
 		return IEventRegister::E_EVENT_NOT_SUPPORT;
 	}
 	if ("LiveStream" == eventName)
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
    if ("RecordStream" == eventName)
    {
        eventParams<<"length"<<"frametype"<<"channel"<<"width"<<"height"<<"framerate"
            <<"audioSampleRate"<<"audioFormat"<<"audioDataWidth"<<"pts"<<"gentime"<<"data";
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
 int BubbleProtocol::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
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
 // inherited from IEventRegister <<<< 
long __stdcall BubbleProtocol::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IRemotePreview == iid)
	{
		*ppv = static_cast<IRemotePreview *>(this);
	}
    else if (IID_IRemotePlayback == iid)
    {
        *ppv = static_cast<IRemotePlayback *>(this);
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

unsigned long __stdcall BubbleProtocol::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall BubbleProtocol::Release()
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

void BubbleProtocol::eventProcCall( QString sEvent,QVariantMap param )
{
    if (m_eventList.contains(sEvent))
    {
        ProcInfoItem eventDes = m_eventMap.value(sEvent);
        if (NULL != eventDes.proc)
        {
            eventDes.proc(sEvent,param,eventDes.puser);
        }
    }
}
