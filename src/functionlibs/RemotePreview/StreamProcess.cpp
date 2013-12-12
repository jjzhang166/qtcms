#include "StreamProcess.h"
#include <QDateTime>
#include <QtEndian>
#include <QDomDocument>
#include "h264wh.h"
#include "IDeviceConnection.h"


StreamProcess::StreamProcess():
m_nRemainBytes(0),
m_nTotalBytes(0),
m_bIsHead(true),
m_nPort(80),
m_bIsSupportBubble(true),
m_nVerifyResult(0)
{
    m_tcpSocket = NULL; 
}

StreamProcess::~StreamProcess(void)
{
	if ( NULL != m_tcpSocket)
	{
		delete m_tcpSocket;
	}
}

int  StreamProcess::getSocketState()
{
	if (NULL == m_tcpSocket)
	{
		return 0;
	}

    int nStatus = m_tcpSocket->state();
    int nRet = 0;
    if (QAbstractSocket::ConnectingState == nStatus)
    {
        nRet = IDeviceConnection::CS_Connectting;
    }
    else if (QAbstractSocket::ConnectedState == nStatus)
    {
        nRet = IDeviceConnection::CS_Connected;
    }
    else if (QAbstractSocket::ClosingState == nStatus)
    {
        nRet = IDeviceConnection::CS_Disconnecting;
    }
    else if (QAbstractSocket::UnconnectedState == nStatus)
    {
        nRet = IDeviceConnection::CS_Disconnected;
    }
    else
    {
        //nothing
    }
    return nRet;
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
		m_tcpSocket = new QTcpSocket;
	}
	else
	{
		m_tcpSocket->disconnectFromHost();
	}

    m_tcpSocket->abort();
    connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveStream()));
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(showError(QAbstractSocket::SocketError)));
	connect(m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));

    m_tcpSocket->connectToHost(hostAddr, ui16Port);
    if (m_tcpSocket->waitForConnected(1000))
    {
		m_nRemainBytes = 0;
		m_nTotalBytes = 0;
		m_bIsHead = true;
		m_nPort = 80;
		m_nVerifyResult = 0;
    }
}


void StreamProcess::stopStream()
{
	if (NULL != m_tcpSocket)
	{
		disconnect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveStream()));
		disconnect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(showError(QAbstractSocket::SocketError)));
	    m_tcpSocket->disconnectFromHost();
	}
}

void StreamProcess::socketWrites(QByteArray block)
{
	if (NULL != m_tcpSocket && IDeviceConnection::CS_Connected == getSocketState())
	{
		m_tcpSocket->write(block);  
	}
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
}


void StreamProcess::receiveStream()
{
	Bubble *pBubble = NULL;
	Message *pMsg = NULL;
	AuthorityBack *pAutoBack = NULL;

 	while(m_tcpSocket->bytesAvailable() > m_nRemainBytes && m_tcpSocket->bytesAvailable() > 14)
 	{
		if (m_bIsHead)
		{
			m_buffer.clear();
            m_buffer = m_tcpSocket->read(14);

			if (m_buffer.contains("HTTP/1.1 200"))
			{
				m_buffer += m_tcpSocket->read(1024 -14);
				analyzeBubbleInfo();
				m_bIsSupportBubble = true;
				g_verify.wakeOne();
				continue;
			}
			else if (m_buffer.contains("HTTP/1.1 404"))
			{
				m_bIsSupportBubble = false;
				m_buffer.clear();
				//暂时不知要清空多大的缓冲区,此处待定
			}

			pBubble = (Bubble *)m_buffer.data();
			m_nTotalBytes = qToBigEndian(pBubble->uiLength) + sizeof(pBubble->cHead) + sizeof(pBubble->uiLength);
            m_nRemainBytes = m_nTotalBytes - 14;
			

			if ('\x00' == pBubble->cCmd)
			{
				m_buffer += m_tcpSocket->read(m_nRemainBytes);
				pMsg = (Message *)(m_buffer.data() + 10);

				m_nRemainBytes = 0;
				if ('\x03' == pMsg->cMessage)
				{
					pAutoBack = (AuthorityBack *)pMsg->pParameters;
					m_nVerifyResult = (int)pAutoBack->cVerified;
					g_verify.wakeOne();
					continue;
				}
			}
		}

		if (m_tcpSocket->bytesAvailable() >= m_nRemainBytes)
		{
			m_buffer += m_tcpSocket->read(m_nRemainBytes);
			//解析码流
			analyzeStream();
			m_nRemainBytes = 0;
			m_bIsHead = true;
		}
		else
		{
			m_bIsHead = false;
		}
	}

}

void StreamProcess::analyzeStream()
{

	Bubble *bubble = NULL;
	LiveStream *liveStreamInfo = NULL;

	char *dataArr = new char[m_buffer.size()];
	int readBytes = 0;

	memset(dataArr, 0, m_buffer.size());
	memcpy(dataArr, m_buffer.data(), m_buffer.size());

	//拷贝除负载数据以外的信息
	bubble = (Bubble *)dataArr;

	readBytes = sizeof(Bubble) - 1;

	liveStreamInfo = (LiveStream*)(dataArr + readBytes);
	readBytes += sizeof(LiveStream) - 1;

	QVariantMap mStreamInfo;

	//音频
	if (0 == liveStreamInfo->cType)
	{
		LiveStreamAudio *audioStream = (LiveStreamAudio*)(dataArr + readBytes);

		mStreamInfo.insert("channel", liveStreamInfo->cChannel);
		mStreamInfo.insert("pts", audioStream->ui64Pts);
		mStreamInfo.insert("length", qToBigEndian(liveStreamInfo->uiLength));
		mStreamInfo.insert("data", (int)audioStream);
		mStreamInfo.insert("frametype", liveStreamInfo->cType);
		mStreamInfo.insert("samplerate", audioStream->uiSampleRate);
		mStreamInfo.insert("samplewidth", audioStream->uiSampleWidth);
		mStreamInfo.insert("audiochannel", liveStreamInfo->cChannel);
		mStreamInfo.insert("acodec", audioStream->sEncode);

		eventProcCall(QString("LiveStream"), mStreamInfo);
	}

	//视频
	if (1 == liveStreamInfo->cType || 2 == liveStreamInfo->cType)
	{
		int width = 0;
		int height = 0;

		GetWidthHeight(dataArr + readBytes, m_buffer.size() - readBytes, &width, &height);	

		mStreamInfo.insert("channel", liveStreamInfo->cChannel);
		mStreamInfo.insert("pts", qToBigEndian(bubble->uiTicket));
		mStreamInfo.insert("length", qToBigEndian(liveStreamInfo->uiLength));
		mStreamInfo.insert("data", (int)(dataArr + readBytes));
		mStreamInfo.insert("frametype", liveStreamInfo->cType);
		mStreamInfo.insert("width", width);
		mStreamInfo.insert("height", height);

		eventProcCall(QString("LiveStream"), mStreamInfo);
	}

	delete[] dataArr;
	dataArr = NULL;
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
	m_tcpSocket->close();
}

void StreamProcess::stateChanged(QAbstractSocket::SocketState socketState)
{
	QVariantMap mStreamInfo;

	mStreamInfo.insert("status", socketState);

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
