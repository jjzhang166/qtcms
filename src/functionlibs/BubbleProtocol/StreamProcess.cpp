#include "StreamProcess.h"
#include <QDateTime>
#include <QtEndian>
#include <QDomDocument>
#include "h264wh.h"
#include <QtCore/QTime>


StreamProcess::StreamProcess():
m_nRemainBytes(0),
m_nTotalBytes(0),
m_bIsHead(true),
m_nPort(80),
m_bIsSupportBubble(true),
m_bStop(false),
m_nVerifyResult(0),
m_hearttimer(NULL),
m_tcpSocket(NULL)
{
	m_curstate=IDeviceConnection::CS_Disconnecting;
}

StreamProcess::~StreamProcess(void)
{
	if ( NULL != m_tcpSocket)
	{
		delete m_tcpSocket;
        m_tcpSocket = NULL;
	}
}

int  StreamProcess::getSocketState()
{
	return m_curstate;

}

void StreamProcess::setAddressInfo(QHostAddress hostAddress, int port)
{
	m_hostAddress = hostAddress;
	m_nPort = port;
}

void StreamProcess::setEventMap(QStringList eventList, QMultiMap<QString, ProcInfoItem> eventMap)
{
	m_eventList = eventList;
	m_eventMap = eventMap;
}

void StreamProcess::conToHost(QString hostAddr, quint16 ui16Port )
{
	if ( NULL == m_tcpSocket )
	{
		m_tcpSocket = new QTcpSocket(this);
	}

    m_tcpSocket->abort();
	
	connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveStream()));
	connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(showError(QAbstractSocket::SocketError)));
	connect(m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    m_tcpSocket->connectToHost(hostAddr, ui16Port);
    if (m_tcpSocket->waitForConnected(2000))
    {
		m_nRemainBytes = 0;
		m_nTotalBytes = 0;
		m_bIsHead = true;
		m_bStop = false;
		m_nPort = 80;
		m_nVerifyResult = 0;
    }
}

void StreamProcess::stopStream()
{
	if (NULL != m_tcpSocket)
	{
		m_bStop = true;

		disconnect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveStream()));
		m_tcpSocket->disconnectFromHost();

		disconnect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(showError(QAbstractSocket::SocketError)));
		disconnect(m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));	
	}
}

void StreamProcess::socketWrites(QByteArray block)
{
	if (NULL != m_tcpSocket && QAbstractSocket::ConnectedState == m_tcpSocket->state())
	{
        int size=m_tcpSocket->write(block);
        if (!m_tcpSocket->waitForBytesWritten(300))
        {
            qDebug()<<__FUNCTION__<<__LINE__<<"socket write failure:\t"<<"time out";
		}else{
			
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"socket write failure:\t"<<"connnet fail ";
	}
	//else if (NULL != m_tcpSocket && QAbstractSocket::ConnectedState != m_tcpSocket->state())
	//{
	//	m_tcpSocket->abort();
 //       int times = 0;
 //       while (m_tcpSocket->state() != QAbstractSocket::ConnectedState && times < 3)
 //       {
 //           ++times;
 //           m_tcpSocket->connectToHost(m_hostAddress, m_nPort);
 //           if (m_tcpSocket->waitForConnected(300))
 //           {
 //               m_tcpSocket->write(block);
 //               if (!m_tcpSocket->waitForBytesWritten(300))
 //               {
 //                   qDebug()<<"socket write failure:\t"<<__FUNCTION__<<__LINE__;
 //               }
 //           }
 //       }
	//	
 //   }
}
QString StreamProcess::checkXML(QString source)
{
	int i = 0;
	int nPos = source.lastIndexOf(QString("vin%1").arg(i));
	while(nPos != -1)
	{
		if (! source.mid(--nPos,1).contains('/'))
		{
			source.insert(++nPos, '/');
		} 
		++i;
		nPos = source.lastIndexOf(QString("vin%1").arg(i));    
	}
	return source;
}

void StreamProcess::analyzeBubbleInfo()
{
	int pos = m_buffer.indexOf("<");
	QString xml = m_buffer.mid(pos, m_buffer.indexOf("#") - pos);


	QString temp = xml;
	xml = checkXML(temp);

	QDomDocument *dom = new QDomDocument();
	if(NULL == dom || !dom->setContent(xml))
	{
		delete dom;
		dom = NULL;
		return;
	}

	QDomNode root = dom->firstChild();
	int uiChannelCount = root.toElement().attribute("vin").toInt();

	for (int i = 0; i < uiChannelCount; i++)
	{
		QList<Stream> list;
		QString tagName = QString("vin%1").arg(i);
		QDomNodeList nodelist = dom->elementsByTagName(tagName);
		QDomNode subnode = nodelist.at(0);
		int StreamNum = subnode.toElement().attribute("stream").toInt();

		for (int j = 0; j < StreamNum; j++)
		{
			QString name = QString("stream%1").arg(j);
			QDomNodeList nodelist1 = dom->elementsByTagName(name);
			QDomNode node =  nodelist1.at(0);
			Stream stem;
			stem.sName = node.toElement().attribute("name");
			stem.sSize = node.toElement().attribute("size");
			stem.sx1 = node.toElement().attribute("x1");
			stem.sx2 = node.toElement().attribute("x2");
			stem.sx4 = node.toElement().attribute("x4");

			list.append(stem);
		}
		m_lstStreamList.append(list);
	}
	if (NULL!=dom)
	{
		delete dom;
		dom=NULL;
	}
	return ;
}



void StreamProcess::receiveStream()
{
	Bubble *pBubble = NULL;
	if (m_bStop)
	{
		m_buffer.clear();
// 		m_bIsResethead=false;
		return;
	}

	if (m_tcpSocket->bytesAvailable() > 0)
	{
		m_buffer += m_tcpSocket->readAll();
		if (m_buffer.contains("HTTP/1.1 200") && m_buffer.size() >= 1024)
		{
			analyzeBubbleInfo();
			g_verify.wakeOne();
			int pos = m_buffer.indexOf("HTTP/1.1 200");

			m_buffer.remove(0, 1024 + pos);
			if (m_buffer.size() > 0)
			{
				qDebug("-------------------->size:%d %d",m_buffer.size(),m_buffer.at(0));
			}
		}
		else if(m_buffer.contains("HTTP/1.1 404"))
		{
			m_tcpSocket->disconnectFromHost();
			m_bIsSupportBubble = false;
			m_buffer.clear();
		}
		else if (m_buffer.startsWith("\xab") && m_buffer.size() > 5)
		{
			m_curHead = '\xab';
			pBubble = (Bubble *)m_buffer.data();
			m_nTotalBytes = qToBigEndian(pBubble->uiLength) + sizeof(pBubble->cHead) + sizeof(pBubble->uiLength);
			if (m_buffer.size() >= m_nTotalBytes)
			{
				analyzeRecordStream();
			}
		}
		else if (m_buffer.startsWith("\xaa") && m_buffer.size() > 5)
		{
			m_curHead = '\xaa';
			pBubble = (Bubble *)m_buffer.data();
			m_nTotalBytes = qToBigEndian(pBubble->uiLength) + sizeof(pBubble->cHead) + sizeof(pBubble->uiLength);
			if (m_buffer.size() >= m_nTotalBytes)
			{
				analyzePreviewStream();
			}
		}
		//Clear information which does not satisfy the condition
		if (m_buffer.size() > 6 && !m_buffer.startsWith("\xaa") && !m_buffer.startsWith("\xab"))
		{
			int pos = 0;
			while(pos >= 0)
			{
				pos = m_buffer.indexOf(m_curHead);
				//not find frame head
				if (-1 == pos)
				{
					m_buffer.clear();
					return;
				}
				
				char *pStr = m_buffer.data() + pos;
				pBubble = (Bubble *)pStr;
				if ((pBubble->cHead == '\xaa' || pBubble->cHead == '\xab') 
					&& (pBubble->cCmd == '\x00' || pBubble->cCmd == '\x01' || pBubble->cCmd == '\x02' || pBubble->cCmd == '\x08' || pBubble->cCmd == '\x09')
					&& (qToBigEndian(pBubble->uiLength) < 200000))
				{
					m_nTotalBytes = qToBigEndian(pBubble->uiLength) + sizeof(pBubble->cHead) + sizeof(pBubble->uiLength);
					m_buffer.remove(0, pos);
					if ('\xaa' == pBubble->cHead)
					{
						analyzePreviewStream();
					}
					else
					{
						analyzeRecordStream();
					}
					return;
				}
				else
				{
					m_buffer.remove(0, pos + 1);
					if (m_buffer.size() < 6)
					{
						return;
					}
				}
			}
		}
	}
}



void StreamProcess::analyzePreviewStream()
{
	Bubble *pBubble = NULL;
	Message *pMsg = NULL;
	AuthorityBack *pAutoBack = NULL;
	LiveStream *liveStreamInfo = NULL;

	while(m_buffer.size() >= m_nTotalBytes)
	{
		char *dataArr = new char[m_nTotalBytes];

		memset(dataArr, 0, m_nTotalBytes);
		memcpy(dataArr, m_buffer.data(), m_nTotalBytes);

		pBubble = (Bubble *)dataArr;

		//verify reback
		if ('\x00' == pBubble->cCmd)
		{
			pMsg = (Message *)(m_buffer.data() + 10);
			if ('\x03' == pMsg->cMessage)
			{
				pAutoBack = (AuthorityBack *)pMsg->pParameters;
				m_nVerifyResult = (int)pAutoBack->cVerified;
				g_verify.wakeOne();
			}
			m_buffer.remove(0, m_nTotalBytes);
		}
		else if ('\x01' == pBubble->cCmd)//preview stream
		{
			liveStreamInfo = (LiveStream *)pBubble->pLoad;

			QVariantMap mStreamInfo;

			//ÒôÆµ
			if (0 == liveStreamInfo->cType)
			{
				LiveStreamAudio *audioStream = (LiveStreamAudio*)(liveStreamInfo->pData);

				mStreamInfo.insert("channel", liveStreamInfo->cChannel);
				mStreamInfo.insert("pts", audioStream->ui64Pts);
				mStreamInfo.insert("length", audioStream->uiEntries * audioStream->uiPacksize);
				mStreamInfo.insert("data", (int)((char*)audioStream + sizeof(LiveStreamAudio)));
				mStreamInfo.insert("frametype", liveStreamInfo->cType);
				mStreamInfo.insert("samplerate", audioStream->uiSampleRate);
				mStreamInfo.insert("samplewidth", audioStream->uiSampleWidth);
				mStreamInfo.insert("audiochannel", liveStreamInfo->cChannel);
				mStreamInfo.insert("acodec", audioStream->sEncode);
				mStreamInfo.insert("gentime", audioStream->uiGtime);

				eventProcCall(QString("LiveStream")  , mStreamInfo);
			}

			//ÊÓÆµ
			if (1 == liveStreamInfo->cType || 2 == liveStreamInfo->cType)
			{
				int width = 0;
				int height = 0;

				GetWidthHeight(liveStreamInfo->pData, qToBigEndian(liveStreamInfo->uiLength), &width, &height);	

				mStreamInfo.insert("channel", liveStreamInfo->cChannel);
				mStreamInfo.insert("pts", (unsigned long long)qToBigEndian(pBubble->uiTicket)*1000);
				mStreamInfo.insert("length", qToBigEndian(liveStreamInfo->uiLength));
				mStreamInfo.insert("data", (int)(liveStreamInfo->pData));
				mStreamInfo.insert("frametype", liveStreamInfo->cType);
				mStreamInfo.insert("width", width);
				mStreamInfo.insert("height", height);
				mStreamInfo.insert("vcodec", "H264");

				eventProcCall(QString("LiveStream"), mStreamInfo);
			}
			m_buffer.remove(0, m_nTotalBytes);
		}
		else if ('\x02'==pBubble->cCmd||'\x08'==pBubble->cCmd)
		{
			m_buffer.remove(0,11);
		}
		if (!m_buffer.startsWith("\xaa"))
		{
			delete[] dataArr;
			dataArr = NULL;
			return;
		}
		if (m_buffer.size() > 5)
		{
			pBubble = (Bubble *)m_buffer.data();
			m_nTotalBytes = qToBigEndian(pBubble->uiLength) + sizeof(pBubble->cHead) + sizeof(pBubble->uiLength);	
		}
		else
		{
			m_nTotalBytes = 1 << (sizeof(int)*8 - 1);
		}

		delete[] dataArr;
		dataArr = NULL;
	}

}


void StreamProcess::analyzeRecordStream()
{
	Bubble *pBubble = NULL;
	Message *pMsg = NULL;
	AuthorityBack *pAutoBack = NULL;
	RecordStream *pRecordStream = NULL;

	QDateTime time1 = QDateTime::currentDateTime();
	QDateTime time2 = QDateTime::currentDateTimeUtc();
	int timeDifference = qAbs(time1.time().hour() - time2.time().hour())*3600;

	while(m_buffer.size() >= m_nTotalBytes)
	{
		char *dataArr = new char[m_nTotalBytes];

		memset(dataArr, 0, m_nTotalBytes);
		memcpy(dataArr, m_buffer.data(), m_nTotalBytes);

		pBubble = (Bubble *)dataArr;

		//verify reback
		if ('\x00' == pBubble->cCmd)
		{
			pMsg = (Message *)(m_buffer.data() + 10);
			if ('\x03' == pMsg->cMessage)
			{
				pAutoBack = (AuthorityBack *)pMsg->pParameters;
				m_nVerifyResult = (int)pAutoBack->cVerified;
				g_verify.wakeOne();
			}
			m_buffer.remove(0, m_nTotalBytes);
		}
		else if('\x02' == pBubble->cCmd)//heartbeat
		{
			m_buffer.remove(0, 11);
		}
		else if ('\x09' == pBubble->cCmd)
		{
			m_buffer.remove(0, 18);
		}
		else if ('\x01' == pBubble->cCmd)//playback stream
		{
			pRecordStream = (RecordStream *)pBubble->pLoad;

			QVariantMap mStreamInfo;

			//ÒôÆµ
			if (0 == pRecordStream->cType)
			{
				int nLength = qToBigEndian(pRecordStream->nLength) -132 ;
				mStreamInfo.insert("length"         ,nLength);
				mStreamInfo.insert("frametype"      ,pRecordStream->cType);
				mStreamInfo.insert("channel"        ,pRecordStream->cChannel);
				mStreamInfo.insert("audioSampleRate",pRecordStream->nAudioSampleRate);
				mStreamInfo.insert("audioFormat"    ,pRecordStream->cAudioFormat);
				mStreamInfo.insert("audioDataWidth" ,pRecordStream->nAudioDataWidth);
				mStreamInfo.insert("pts"            ,pRecordStream->nU64TSP);
				mStreamInfo.insert("gentime"        ,pRecordStream->nGenTime - timeDifference);
				int offSet = sizeof(pRecordStream->nLength) + sizeof(pRecordStream->cType) + sizeof(pRecordStream->cChannel) + 128 + 4;
				mStreamInfo.insert("data"           ,(uint)((char*)pRecordStream + offSet));

				mStreamInfo.insert("samplerate"		,pRecordStream->nAudioSampleRate);
				mStreamInfo.insert("samplewidth"	,pRecordStream->nAudioDataWidth);
				mStreamInfo.insert("audiochannel"	,pRecordStream->nChannel);
				mStreamInfo.insert("acodec"			,pRecordStream->cAudioFormat);

				eventProcCall(QString("RecordStream")  , mStreamInfo);
			}

			//ÊÓÆµ
			if (1 == pRecordStream->cType || 2 == pRecordStream->cType)
			{
				int nLength = qToBigEndian(pRecordStream->nLength) -128 ;
				mStreamInfo.insert("length"       ,nLength);
				mStreamInfo.insert("frametype"    ,pRecordStream->cType);
				mStreamInfo.insert("channel"      ,pRecordStream->cChannel);
				mStreamInfo.insert("width"        ,pRecordStream->nFrameWidth);
				mStreamInfo.insert("height"       ,pRecordStream->nFrameHeight);
				mStreamInfo.insert("framerate"    ,pRecordStream->nFrameRate);
				mStreamInfo.insert("pts"          ,pRecordStream->nU64TSP);
				mStreamInfo.insert("gentime"      ,pRecordStream->nGenTime - timeDifference);
				int offSet = sizeof(pRecordStream->nLength) + sizeof(pRecordStream->cType) + sizeof(pRecordStream->cChannel) + 128;
				mStreamInfo.insert("data"         ,(uint)((char*)pRecordStream + offSet));

				eventProcCall(QString("RecordStream"), mStreamInfo);
			}
			m_buffer.remove(0, m_nTotalBytes);
		}
		if (!m_buffer.startsWith("\xab"))
		{
			delete[] dataArr;
			dataArr = NULL;
			return;
		}

		if (m_buffer.size() > 5)
		{
			pBubble = (Bubble *)m_buffer.data();
			m_nTotalBytes = qToBigEndian(pBubble->uiLength) + sizeof(pBubble->cHead) + sizeof(pBubble->uiLength);	
		}
		else
		{
			m_nTotalBytes = 1 << (sizeof(int)*8 - 1);
		}

		delete[] dataArr;
		dataArr = NULL;
	}
}


int StreamProcess::getVerifyResult()
{
	return m_nVerifyResult;
}

bool StreamProcess::getSupportState()
{
	return m_bIsSupportBubble;
}

int StreamProcess::getStreamListInfo(QList<QList<Stream>> &lstStreamList)
{
	if (m_lstStreamList.isEmpty())
	{
		return 1;
	}
	lstStreamList = m_lstStreamList;
	return 0;
}

void StreamProcess::showError(QAbstractSocket::SocketError sockerror)
{
	QVariantMap mStreamInfo;

	mStreamInfo.insert("connectionStatus", m_tcpSocket->state());
	mStreamInfo.insert("errorValue", m_tcpSocket->error());
	mStreamInfo.insert("errorDescription", m_tcpSocket->errorString());
	eventProcCall(QString("SocketError"), mStreamInfo);
	if (m_hearttimer!=NULL&&m_hearttimer->isActive())
	{
		m_hearttimer->stop();
		disconnect(m_hearttimer,SIGNAL(timeout()),this,SLOT(sendHeartBeat()));
		delete m_hearttimer;
		m_hearttimer=NULL;
	}
	m_tcpSocket->close();
}

void StreamProcess::stateChanged(QAbstractSocket::SocketState socketState)
{
	QVariantMap mStreamInfo;

	int nStatus = m_tcpSocket->state();
	int nRet = 0;
	if (QAbstractSocket::ConnectingState == nStatus)
	{
		nRet = IDeviceConnection::CS_Connectting;
		goto pro;
	}
	else if (QAbstractSocket::ConnectedState == nStatus)
	{
		nRet = IDeviceConnection::CS_Connected;
		if (m_hearttimer==NULL)
		{
			m_hearttimer=new QTimer;
			m_hearttimer->start(10000);
			connect(m_hearttimer,SIGNAL(timeout()),this,SLOT(sendHeartBeat()));
		}		
		goto pro;
	}
	else if (QAbstractSocket::ClosingState == nStatus)
	{
		nRet = IDeviceConnection::CS_Disconnecting;
		goto pro;
	}
	else if (QAbstractSocket::UnconnectedState == nStatus)
	{
		nRet = IDeviceConnection::CS_Disconnected;
		if (m_hearttimer!=NULL&&m_hearttimer->isActive())
		{
			m_hearttimer->stop();
			disconnect(m_hearttimer,SIGNAL(timeout()),this,SLOT(sendHeartBeat()));
			delete m_hearttimer;
			m_hearttimer=NULL;
		}
		
		goto pro;
	}
	return ;
pro:
	m_curstate=nRet;
	mStreamInfo.insert("status", nRet);

	eventProcCall(QString("StateChangeed"), mStreamInfo);
}

void StreamProcess::eventProcCall( QString sEvent,QVariantMap param )
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

void StreamProcess::sendHeartBeat()
{
	char buff[100];
	qint64 nLen = 0;

	Bubble * bubble = (Bubble *)buff;

		bubble->cHead = (char)0xaa;
		bubble->cCmd = 0x07;

	QDateTime time = QDateTime::currentDateTime();
	bubble->uiTicket = qToBigEndian((unsigned int)(time.toMSecsSinceEpoch() * 1000));

	bubble->pLoad[0] = 0x02;
	bubble->uiLength = sizeof(Bubble) - sizeof(bubble->cHead) - sizeof(bubble->uiLength);

	nLen = (qint64)(bubble->uiLength + sizeof(bubble->cHead) + sizeof(bubble->uiLength));
	bubble->uiLength = qToBigEndian(bubble->uiLength);

	QByteArray block;
	block.append(buff, nLen);
	socketWrites(block);
}
