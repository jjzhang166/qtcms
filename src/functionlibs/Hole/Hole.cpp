#include "Hole.h"
#include "netlib.h"
#include <QtCore/QElapsedTimer>
#include <QtCore/QVariantMap>
#include <guid.h>

#pragma pack(4)
typedef struct _tagAudioBufAttr{
	int entries;
	int packsize;
	UINT64 pts;
	time_t * gtime;
	char encode[8];
	int samplerate;
	int samplewidth;
}AudioBufAttr;
#pragma pack()
//extern ushort htons(ushort hostshort);

int hole_CreateSession(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->CreateSession(e,pData,nDataSize));
}

int hole_RecvProc(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->RecvProc(e,pData,nDataSize));
}

int hole_SendPre(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->SendPre(e,pData,nDataSize));
}

int hole_LdPack(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->LdPack(e,pData,nDataSize));
}

int hole_SessionClose(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->SessionClose(e,pData,nDataSize));
}

int hole_HoleFromDev(CEseeXml::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->HoleFromDev(e,pData,nDataSize));
}

int hole_DevReady(CEseeXml::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->DevReady(e,pData,nDataSize));
}

int hole_SoupAuth(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->SoupAuth(e,pData,uiDataSize));
}
int hole_DevInfo(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->DevInfo(e,pData,uiDataSize));
}
int hole_Settings(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize,LPVOID pUser)
{
	return (((Hole *)pUser)->Settings(e,pData,uiDataSize));
}

Hole::Hole() :
m_nRef(0),
m_cStatus(CS_Disconnected),
m_Camcnt(-1),
m_Channel(-1),
m_Stream(-1),
m_bHoleSuccess(false),
m_bUserCheck(false),
m_bstrdataRecved(false)
{
	m_s.SetEventProc(CRudpSession::EVENT_CREATE_SESSION,::hole_CreateSession,this);
	m_s.SetEventProc(CRudpSession::EVENT_RECV,::hole_RecvProc,this);
	m_s.SetEventProc(CRudpSession::EVENT_SEND_PRE,::hole_SendPre,this);
	m_s.SetEventProc(CRudpSession::EVENT_LD_PACK,::hole_LdPack,this);
	m_s.SetEventProc(CRudpSession::EVENT_SESSION_CLOSE,::hole_SessionClose,this);

	m_esee.SetSession(&m_s);
	m_esee.SetEventPorc(CEseeXml::EVENT_HOLE_FROM_DEV,::hole_HoleFromDev,this);
	m_esee.SetEventPorc(CEseeXml::EVENT_DEV_READY,::hole_DevReady,this);

	m_soup.SetSession(&m_s);
	m_soup.SetProtocolEvent(CSoupXml::PE_AUTH,::hole_SoupAuth,this);
	m_soup.SetProtocolEvent(CSoupXml::PE_SETTINGS,::hole_Settings,this);
	m_soup.SetProtocolEvent(CSoupXml::PE_DEVINFO,::hole_DevInfo,this);

}

Hole::~Hole()
{

}

int Hole::setDeviceHost(const QString & sAddr)
{
	m_sAddr = sAddr;
	return 0;
}
int Hole::setDevicePorts(const QVariantMap & ports)
{
	m_ports = ports;
	return 0;
}
int Hole::setDeviceId(const QString & sAddress)
{
	m_sId = sAddress;
	return 0;
}
int Hole::setDeviceAuthorityInfomation(QString username,QString password)
{
	m_username = username;
	m_password  = password;
	return 0;
}
int Hole::connectToDevice()
{
	m_bHoleSuccess = false;
	CallBackStatus(IDeviceConnection::CS_Connectting);
	CRudpSession::ErrorCode errocode = m_s.Connect("192.168.1.1",80);
	if (CRudpSession::SUCCESS != errocode)
	{
		m_bHoleSuccess = false;
		CallBackStatus(IDeviceConnection::CS_Disconnected);
		return -1;
	}
	m_bConnected = true;
	CallBackStatus(IDeviceConnection::CS_Connected);
	return 0;
}
int Hole::disconnect()
{
	CallBackStatus(IDeviceConnection::CS_Disconnecting);
	CRudpSession::ErrorCode eRet = m_s.Close();
	if (CRudpSession::SUCCESS != eRet)
	{
		return -1;
	}
	m_bConnected = false;
	m_bHoleSuccess = false;
	CallBackStatus(IDeviceConnection::CS_Disconnected);
	return 0;
}
int Hole::getCurrentStatus()
{
	return m_cStatus;
}
QString Hole::getDeviceHost()
{
	return m_sAddr;
}
QString Hole::getDeviceid()
{
	return m_sId;
}
QVariantMap Hole::getDevicePorts()
{
	return m_ports;
}


int Hole::authority()
{
	QElapsedTimer eltime;
	m_soup.CheckUserMsg(m_username.toAscii().data(),m_password.toAscii().data());
	eltime.start();
	while(!m_bUserCheck && eltime.elapsed()<2000)
	{
		SleepQ(30);
	}
	return 	m_bUserCheck;
}
int Hole::getLiveStream(int nChannel, int nStream)
{
	m_Channel=nChannel;
	m_Stream=nStream;

	return 	m_soup.OpenChannel(m_Channel,m_Stream,true);
}
int Hole::stopStream()
{
	return 	m_soup.OpenChannel(m_Channel,m_Stream,false);
}
int Hole::pauseStream(bool bPaused)
{
	if (bPaused)
	{
		return 	m_soup.OpenChannel(m_Channel,m_Stream,false);
	}
	return 0;
}
int Hole::getStreamCount()
{
	QElapsedTimer eltime;
	m_soup.GetChannelCount();
	eltime.start();
	while(m_Camcnt==-1 && eltime.elapsed()<2000)
	{
		SleepQ(30);
	}
	return m_Camcnt;
}
int Hole::getStreamInfo(int nStreamId,QVariantMap &streamInfo)
{
	if (!m_bstrdataRecved)
	{
		QElapsedTimer eltime;
		m_soup.GetStreamData(0);
		eltime.start();
		while(!m_bstrdataRecved && eltime.elapsed()<2000)
		{
			SleepQ(30);
		}
	}

	m_cstreamDec.lock();
	QMap<int,QVariantMap>::iterator iter = m_StaeamList.find(nStreamId);
	if (iter!=m_StaeamList.end())
	{
		streamInfo = *iter;
		return true;
	}
	m_cstreamDec.unlock();
	return false;
}

QStringList Hole::eventList()
{
	return eventMap.keys();
}
int Hole::queryEvent(QString eventName,QStringList& eventParams)
{
	eventParams.empty();
	if (eventName=="LiveStream")
	{
		eventParams<<"channel"<<"pts"<<"length"<<"data"<<"frametype"<<"width"
			<<"height"<<"vcodec"<<"samplerate"<<"samplewidth"<<"audiochannel"
			<<"acodec";
	}
	else if (eventName=="StateChangeed")
	{
		eventParams<<"status";
	}
	
	return 0;
}
int Hole::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
{
	EventData evdata={proc,pUser};
	if (eventMap.find(eventName)==eventMap.end())

	{
		eventMap.insert(eventName,evdata);
		return true;
	}
	else return false;
}

QString Hole::getModeName()
{
	return QString("Hole");
}

long __stdcall Hole::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IDeviceConnection == iid)
	{
		*ppv = static_cast<IDeviceConnection *>(this);
	}
	else if (IID_IRemotePreview == iid)
	{
		*ppv = static_cast<IRemotePreview *>(this);
	}
	else if (IID_IEventRegister == iid)
	{
		*ppv = static_cast<IEventRegister *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall Hole::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall Hole::Release()
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

int Hole::CreateSession(CRudpSession::EventType e,LPVOID pData,int nDataSize)
{
	if (m_bHoleSuccess)
	{
		return 0;
	}
	int *nRandom = (int *)pData;
	m_nRandom = *nRandom;

	int nStep = 0;
	//DWORD dwHoleTicket;
	QElapsedTimer HoleTime;
	bool bQuitHole = false;
	int nHoleRetryCount = 0;
	int nHoleFailedCount = 0;
	int nHoleReqRetry = 0;

	while(!bQuitHole)
	{
		switch (nStep)
		{
		case 0: // Hole Req
			{
				// 向服务器发起穿透请求，完成echo获取到对端ip和端口
				m_bDevReady = false;
				m_peerInfo = m_esee.HoleReq(m_sId.toAscii().data(),m_nRandom);
				qDebug("Hole req\r\n");

				// 穿透请求失败
				if (0 == m_peerInfo._U.ulPeerAddr)
				{
					qDebug("Hole req time out\r\n");
					return -1;
				}

				// 成功则开始发送HoleTo
				nHoleRetryCount = 0;
				nHoleFailedCount = 0;
				m_bHoleRecved = false;
				nStep = 1;
			}
			break;
		case 1: // Hole To
			{
				m_esee.HoleTo(m_peerInfo,m_nRandom);
				qDebug("Hole to %d.%d.%d.%d:%d\r\n",m_peerInfo._U._b.b1,
					m_peerInfo._U._b.b2,
					m_peerInfo._U._b.b3,
					m_peerInfo._U._b.b4,
					m_peerInfo.ulPeerPort);
				//dwHoleTicket = ::GetTickCount();
				HoleTime.start();
				nStep = 2;
			}
			break;
		case 2: // 等待Hole To 返回
			{
				SleepQ(30);
				if (m_bHoleRecved)
				{
					nStep = 3;
					break;
				}

				if (/*::GetTickCount() - dwHoleTicket*/ HoleTime.elapsed()> 3000)
				{// 返回超时
					// 重试Hole
					nStep = 1;
					nHoleRetryCount ++;
				}

				if (nHoleRetryCount > 3)
				{
					if (m_bDevReady)
					{
						// Hole 超时
						nHoleFailedCount ++;
						if (nHoleFailedCount > 3)
						{
							// 连续Hole失败，则直接判定Hole失败
							return -1;
						}
						else
						{
							// 继续hole
							nHoleRetryCount = 0;
							nStep = 1;
						}

					}
					else
					{
						nHoleReqRetry ++;
						if (nHoleReqRetry > 3)
						{
							// Hole 失败
							return -1;

						}
						else
						{
							// 从头再来
							nStep = 0;
						}
					}
				}
			}
			break;
		case 3: // Hole成功
			{
				qDebug("Hole success\r\n");
				bQuitHole = true;
				m_bHoleSuccess = true;
			}
			break;
		}
	}

	return 0;
}
int Hole::RecvProc(CRudpSession::EventType e,LPVOID pData,int nDataSize)
{
	m_esee.DataProc(e,pData,nDataSize);
	return 0;
}
int Hole::SendPre(CRudpSession::EventType e,LPVOID pData,int nDataSize)
{
	CRudpSession::PreSendData * Param = (CRudpSession::PreSendData *)pData;
	Param->TargetAddress.sin_addr.s_addr = m_peerInfo._U.ulPeerAddr;
	Param->TargetAddress.sin_port = htonsQ((unsigned short)m_peerInfo.ulPeerPort);

	return 0;
}
int Hole::LdPack(CRudpSession::EventType type,LPVOID pData,int nDataSize)
{
	CRudpSession::LdPackData * PackData = (CRudpSession::LdPackData *)pData;	
	// 码流
	CSoupXml::FrameHead *Frame = (CSoupXml::FrameHead *)PackData->pData;
	if (0x534f55ff == Frame->magic)
	{
		// 码流处理
		StreamData(PackData->pData,PackData->uiPackSize);
	}
	else
	{
		// SOUP信令		
		m_soup.DataProc((char *)PackData->pData,PackData->uiPackSize);
	}
	return CRudpSession::SUCCESS;
}
int Hole::SessionClose(CRudpSession::EventType type,LPVOID pData,int nDataSize)
{
	m_bConnected = false;
	m_bHoleSuccess = false;
	m_cStatus = IDeviceConnection::CS_Disconnected;
	/*if (NULL != m_OnClose)
	{
		m_OnClose(m_pOnCloseParam);
	}*/
	return CRudpSession::SUCCESS;
}
int Hole::HoleFromDev(CEseeXml::EventType e,LPVOID pData,int nDataSize)
{
	CEseeXml::HoleFromData * DataTemp = (CEseeXml::HoleFromData *)pData;
	int nRandom = DataTemp->uiRandom;
	qDebug("hole from dev:%d %d\r\n",nRandom,m_nRandom);
	if (nRandom == m_nRandom)
	{
		struct sockaddr_in * fromAddr;
		fromAddr = (struct sockaddr_in *)(&(DataTemp->from));
		if (fromAddr->sin_addr.S_un.S_addr != m_peerInfo._U.ulPeerAddr || htonsQ(fromAddr->sin_port) != m_peerInfo.ulPeerPort)
		{
			m_peerInfo._U.ulPeerAddr = fromAddr->sin_addr.S_un.S_addr;
			m_peerInfo.ulPeerPort = (htonsQ(fromAddr->sin_port));
		}
		m_bHoleRecved = true;
	}
	return 0;
}
int Hole::DevReady(CEseeXml::EventType e,LPVOID pData,int nDataSize)
{
	int nRandom = *((int *)pData);
	if (nRandom == m_nRandom)
	{
		m_bDevReady = true;
	}
	return 0;
}
int Hole::SoupAuth(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize)
{
	CSoupXml::AuthData *auth = (CSoupXml::AuthData *)pData;
	switch(auth->nErrorCode)
	{
	case 0:
		m_bUserCheck = true;
		break;
	default:
		m_bUserCheck = false;
	}
	m_bUserVerified = true;
	return 0;
}
int Hole::DevInfo(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize)
{
	m_Camcnt=*((int*)pData);
	return 0;
}
int Hole::Settings(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize)
{
	int amount = *((int*)pData);
	CSoupXml::StreamItem* StreamItem = (CSoupXml::StreamItem*)((int*)pData + 1);
	if (amount<=0)
	{
		return -1;
	}
	m_cstreamDec.lock();
	m_StaeamList.clear();
	for (int i=0;i<amount;i++)
	{
		QVariantMap istream;
		istream.insert("streamid",StreamItem[i].streamid);
		istream.insert("sname",StreamItem[i].sname);
		istream.insert("width",StreamItem[i].width);
		istream.insert("height",StreamItem[i].height);
		m_StaeamList.insert(StreamItem[i].streamid,istream);
	}
	m_cstreamDec.unlock();
	m_bstrdataRecved = true;
	return 0;
}

void Hole::StreamData(LPVOID pData,int nDataSize)
{
	CSoupXml::FrameHead * Frame = (CSoupXml::FrameHead *)pData;
	char *pFrameData = (char *)pData + sizeof(CSoupXml::FrameHead) + Frame->externsize * 4;

	QVariantMap frameitem;
	if (1 == Frame->frametype || 2 == Frame->frametype)
	{
		if (1 == Frame->frametype)
		{
			//if (NULL != m_onIFrame)
			//{
			//	m_onIFrame(m_pOnIFrameParame);
			//}	
			qDebug("get the I Frame!!!");
		}
		frameitem.insert("channel",m_Channel);
		frameitem.insert("pts",(UINT64)Frame->pts);
		frameitem.insert("length",Frame->framesize);
		frameitem.insert("data",(int)pFrameData);
		frameitem.insert("frametype",Frame->frametype);
		frameitem.insert("width",Frame->_U.v.width);
		frameitem.insert("height",Frame->_U.v.height);
		frameitem.insert("vcodec",(unsigned)Frame->_U.v.enc);

	}
	else if (0 == Frame->frametype)
	{
		AudioBufAttr * AudioHead = (AudioBufAttr *)pFrameData;
		int nBufSize = AudioHead->entries * AudioHead->packsize;
		//frameitem.insert("pts",(UINT64)AudioHead->pts);
		frameitem.insert("pts",(UINT64)Frame->pts);
		//frameitem.insert("length",nBufSize);
		frameitem.insert("length",Frame->framesize);
		frameitem.insert("data",(int)(pFrameData+sizeof(AudioBufAttr)));
		frameitem.insert("frametype",Frame->frametype);
		frameitem.insert("samplerate",Frame->_U.a.samplerate);
		frameitem.insert("samplewidth",Frame->_U.a.samplewidth);
		frameitem.insert("vcodec",(unsigned)Frame->_U.a.enc);
		frameitem.insert("audiochannel",1);
	}

	applyEventProc("LiveStream",frameitem);
}
int Hole::applyEventProc(QString eventName,QVariantMap datainfo)
{
	QMap<QString,EventData>::Iterator eviter = eventMap.find(eventName);
	if(eviter!=eventMap.end())
	{
		eventcallback evcallback=eviter->eventproc;
		if (evcallback)
		{
			evcallback(eventName,datainfo,eviter->puser);
			return true;
		}
	}

	return false;
}

void Hole::CallBackStatus(_enConnectionStatus status)
{
	m_cStatus = status;
	QVariantMap cState;
	cState.insert("status",(int)m_cStatus);
	applyEventProc("StateChangeed",cState);
}