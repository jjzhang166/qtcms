#include "StreamProcess.h"
#include <QDateTime>
#include <QtEndian>
#include <QMessageBox>
#include "h264wh.h"
#include "IDeviceConnection.h"

StreamProcess::StreamProcess():
m_nRemainBytes(0),
m_nTotalBytes(0),
m_bStop(false),
m_bIsHead(true),
m_nPort(80),
m_nVerifyResult(1)
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
		connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveStream())/*,Qt::DirectConnection*/);
		connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(showError(QAbstractSocket::SocketError))/*,Qt::DirectConnection*/);
	}
	qDebug()<<this;
    m_tcpSocket->abort();
	//connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveStream())/*,Qt::DirectConnection*/);
	//connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(showError(QAbstractSocket::SocketError))/*,Qt::DirectConnection*/);

    m_tcpSocket->connectToHost(hostAddr, ui16Port);
    if (m_tcpSocket->waitForConnected())
    {
        qDebug()<<"Connected!!!!!!!!!!!!!!";
		m_bStop = false;
		m_nRemainBytes = 0;
		m_nTotalBytes=0;
		m_bIsHead=true;
		m_nVerifyResult=1;
    }

	g_mutex.unlock();
}


void StreamProcess::stopStream()
{
	qDebug()<<this;
	m_bStop = true;

	if (NULL != m_tcpSocket)
	{
	    m_tcpSocket->disconnectFromHost();
	}
}

void StreamProcess::socketWrites(QByteArray block)
{
	qDebug()<<this;
	if (NULL != m_tcpSocket && IDeviceConnection::CS_Connected == getSocketState())
	{
		m_tcpSocket->write(block);  
	}
}


void StreamProcess::receiveStream()
{
	Bubble *pBubble = NULL;
	Message *pMsg = NULL;
	AuthorityBack *pAutoBack = NULL;

 	while(m_tcpSocket->bytesAvailable() > m_nRemainBytes && !m_bStop)
 	{
		if (m_bIsHead)
		{
			m_buffer.clear();
			m_buffer = m_tcpSocket->read(16);
			pBubble = (Bubble *)m_buffer.data();
			m_nTotalBytes = qToBigEndian(pBubble->uiLength) + sizeof(pBubble->cHead) + sizeof(pBubble->uiLength);
			m_nRemainBytes = m_nTotalBytes - 16;
			if ('\x00' == pBubble->cCmd)
			{
				pMsg = (Message *)pBubble->pLoad;
				if ('\x03' == pMsg->cMessage)
				{
					pAutoBack = (AuthorityBack *)pMsg->pParameters;
					m_nVerifyResult = (int)pAutoBack->cVerified;
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

void StreamProcess::showError(QAbstractSocket::SocketError sockerror)
{
	QVariantMap mStreamInfo;

	mStreamInfo.insert("errorValue", m_tcpSocket->error());
	mStreamInfo.insert("errorDescription", m_tcpSocket->errorString());

	eventProcCall(QString("SocketError"), mStreamInfo);
	m_tcpSocket->close();
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
