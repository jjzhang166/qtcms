#include "Turn.h"
#include "netlib.h"
#include <QtCore/QElapsedTimer>
#include <QtCore/QVariantMap>
#include <QDebug>
#include <guid.h>

#pragma pack(4)
typedef struct _tagAudioBufAttr{
	int entries;
	int packsize;
	UINT64 pts;
	time_t gtime;
	char encode[8];
	int samplerate;
	int samplewidth;
}AudioBufAttr;
#pragma pack()
//extern ushort htons(ushort hostshort);
//extern unsigned int htons32(unsigned int hostshort);

int turn_CreateSession(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->CreateSession(e,pData,nDataSize));
}

int turn_RecvProc(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->RecvProc(e,pData,nDataSize));
}

int turn_PreCreatePack(CRudpSession::EventType type,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->PreCreatePack(type,pData,nDataSize));
}

int turn_SendPre(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->SendPre(e,pData,nDataSize));
}

int turn_LdPack(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->LdPack(e,pData,nDataSize));
}

int turn_SessionClose(CRudpSession::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->SessionClose(e,pData,nDataSize));
}

//int hole_HoleFromDev(CEseeXml::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
//{
//	return (((Turn *)pUser)->HoleFromDev(e,pData,nDataSize));
//}

int turn_DevReady(CEseeXml::EventType e,LPVOID pData,int nDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->DevReady(e,pData,nDataSize));
}

int turn_SoupAuth(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->SoupAuth(e,pData,uiDataSize));
}
int turn_DevInfo(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->DevInfo(e,pData,uiDataSize));
}
int turn_Settings(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize,LPVOID pUser)
{
	return (((Turn *)pUser)->Settings(e,pData,uiDataSize));
}

unsigned char aesKey[]= {0x37, 0x4e, 0x6a, 0xda, 0xf7, 0x5b, 0x99, 0x21, 0xe3, 0x68, 0xe5, 0xdd, 0x26, 0x08,0xd3, 0x66};

Turn::Turn() :
m_nRef(0),
m_cStatus(CS_Disconnected),
m_Camcnt(-1),
m_Channel(-1),
m_Stream(-1),
m_bUserCheck(false),
m_bstrdataRecved(false),
m_bGetTurnInfo(false)
{
	m_s.SetEventProc(CRudpSession::EVENT_CREATE_SESSION,::turn_CreateSession,this);
	m_s.SetEventProc(CRudpSession::EVENT_RECV,::turn_RecvProc,this);
	m_s.SetEventProc(CRudpSession::EVENT_PRE_CREATEPACK,::turn_PreCreatePack,this);
	m_s.SetEventProc(CRudpSession::EVENT_SEND_PRE,::turn_SendPre,this);
	m_s.SetEventProc(CRudpSession::EVENT_LD_PACK,::turn_LdPack,this);
	m_s.SetEventProc(CRudpSession::EVENT_SESSION_CLOSE,::turn_SessionClose,this);

	m_esee.SetSession(&m_s);
	//m_esee.SetEventPorc(CEseeXml::EVENT_HOLE_FROM_DEV,::hole_HoleFromDev,this);
	m_esee.SetEventPorc(CEseeXml::EVENT_DEV_READY,::turn_DevReady,this);

	m_soup.SetSession(&m_s);
	m_soup.SetProtocolEvent(CSoupXml::PE_AUTH,::turn_SoupAuth,this);
	m_soup.SetProtocolEvent(CSoupXml::PE_SETTINGS,::turn_Settings,this);
	m_soup.SetProtocolEvent(CSoupXml::PE_DEVINFO,::turn_DevInfo,this);

	m_pCypher = new AES(Bits128,aesKey);
}

Turn::~Turn()
{
	if (NULL != m_pCypher)
	{
		delete m_pCypher;
	}
	m_s.Close();
}

int Turn::setDeviceHost(const QString & sAddr)
{
	m_sAddr = sAddr;
	return 0;
}
int Turn::setDevicePorts(const QVariantMap & ports)
{
	m_ports = ports;
	return 0;
}
int Turn::setDeviceId(const QString & sAddress)
{
	m_sId = sAddress;
	return 0;
}
int Turn::setDeviceAuthorityInfomation(QString username,QString password)
{
	m_username = username;
	m_password  = password;
	return 0;
}
int Turn::connectToDevice()
{
	CallBackStatus(IDeviceConnection::CS_Connectting);
	CRudpSession::ErrorCode errocode = m_s.Connect("192.168.1.25",8880);
	m_connenting = false;
	if (CRudpSession::SUCCESS != errocode)
	{
		CallBackStatus(IDeviceConnection::CS_Disconnected);
		return 1;
	}
	m_bConnected = true;
	CallBackStatus(IDeviceConnection::CS_Connected);
	return 0;
}
int Turn::disconnect()
{
	if (m_connenting)
	{
		m_esee.CloseReq();
		m_connenting = false;
	}
	CallBackStatus(IDeviceConnection::CS_Disconnecting);
	CRudpSession::ErrorCode eRet = m_s.Close();
	if (CRudpSession::SUCCESS != eRet)
	{
		return 1;
	}
	m_bConnected = false;
	CallBackStatus(IDeviceConnection::CS_Disconnected);
	return 0;
}
int Turn::getCurrentStatus()
{
	return m_cStatus;
}
QString Turn::getDeviceHost()
{
	return m_sAddr;
}
QString Turn::getDeviceid()
{
	return m_sId;
}
QVariantMap Turn::getDevicePorts()
{
	return m_ports;
}


int Turn::authority()
{
	QElapsedTimer eltime;
	m_soup.CheckUserMsg(m_username.toAscii().data(),m_password.toAscii().data());
	eltime.start();
	while(!m_bUserCheck && eltime.elapsed()<2000)
	{
		SleepQ(30);
	}
	return 	m_bUserCheck?0:1;
}
int Turn::getLiveStream(int nChannel, int nStream)
{
	m_Channel=nChannel;
	m_Stream=nStream;

	return 	m_soup.OpenChannel(m_Channel,m_Stream,true)?1:0;
}
int Turn::stopStream()
{
	return 	m_soup.OpenChannel(m_Channel,m_Stream,false)?1:0;
}
int Turn::pauseStream(bool bPaused)
{
	if (bPaused)
	{
		return 	m_soup.OpenChannel(m_Channel,m_Stream,false)?1:0;
	}
	return 0;
}
int Turn::getStreamCount()
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
int Turn::getStreamInfo(int nStreamId,QVariantMap &streamInfo)
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
		return 0;
	}
	m_cstreamDec.unlock();
	return 1;
}

QStringList Turn::eventList()
{
	return eventMap.keys();
}
int Turn::queryEvent(QString eventName,QStringList& eventParams)
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
int Turn::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
{
	EventData evdata={proc,pUser};
	if (eventMap.find(eventName)==eventMap.end())
	{
		eventMap.insert(eventName,evdata);
		return true;
	}
	else return false;
}

QString Turn::getModeName()
{
	return QString("Hole");
}

long __stdcall Turn::QueryInterface( const IID & iid,void **ppv )
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
	else if (IID_IProtocolPTZ == iid)
	{
		*ppv = static_cast<IProtocolPTZ *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall Turn::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall Turn::Release()
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

int Turn::CreateSession(CRudpSession::EventType e,LPVOID pData,int nDataSize)
{
	if (m_bGetTurnInfo)
	{
		return 0;
	}
	int *nRandom = (int *)pData;
	m_nRandom = *nRandom;

	//else m_cStatus = IDeviceConnection::CS_Disconnected;

	int nRetryCount = 0;
	bool bReady = false;
	m_connenting = true;
	while (m_connenting && nRetryCount < 3)
	{
		m_ServerInfo = m_esee.TurnReq(m_sId.toAscii().data());
		if (0 == m_ServerInfo.ulServerAddr)
		{
			nRetryCount ++;
			continue;
		}

	// wait for ready ,or timeout
		if (m_esee.WaitForReadySignal())
		{
			nRetryCount ++;
		}
		else
		{
			qDebug("Esee finished\r\n");
			bReady = true;
			break;
		}
	}

	if (!bReady)
	{
		qDebug("Esee time out\r\n");
		return -1;
	}


	// GetInfo
	int nRet = GetTurnInfo();
	if (0 != nRet)
	{
		return -1;
	}
	return 0;
}

int Turn::RecvProc(CRudpSession::EventType type,LPVOID pData,int nDataSize)
{
	int nRet;
	nRet = DataProc(type,pData,nDataSize);
	if (0 == nRet)
	{
		return 0;
	}

	nRet = m_esee.DataProc(type,pData,nDataSize);
	return 0;
}

int Turn::PreCreatePack(CRudpSession::EventType type,LPVOID pData,int nDataSize)
{	
	int *nRedundancySize = (int *)pData;
	*nRedundancySize = sizeof(TurnHead);
	return 0;
}

int Turn::SendPre(CRudpSession::EventType e,LPVOID pData,int nDataSize)
{
	if (!m_bGetTurnInfo)
	{
		return -1;
	}
	CRudpSession::PreSendData * PreSend = (CRudpSession::PreSendData *)pData;
	TurnHead* pHead = (TurnHead*)PreSend->pHead;
	PreSend->TargetAddress = m_TurnServerAddr;
	PreSend->uiSendDataSize +=sizeof(TurnHead);
	pHead->uiHeader = PROTOCOL_HEAD;
	pHead->uiCmd = CS_Turn_Data;
	pHead->uiRadom = m_nRandom;
	pHead->_U.TurnData.uiIp = m_uiTurnIp;
	pHead->_U.TurnData.uiPort = m_uiTurnPort;
	
	return 0;
}
int Turn::LdPack(CRudpSession::EventType type,LPVOID pData,int nDataSize)
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
int Turn::SessionClose(CRudpSession::EventType type,LPVOID pData,int nDataSize)
{
	m_bConnected = false;
	m_cStatus = IDeviceConnection::CS_Disconnected;
	/*if (NULL != m_OnClose)
	{
		m_OnClose(m_pOnCloseParam);
	}*/
	return CRudpSession::SUCCESS;
}
//int Turn::HoleFromDev(CEseeXml::EventType e,LPVOID pData,int nDataSize)
//{
//	CEseeXml::HoleFromData * DataTemp = (CEseeXml::HoleFromData *)pData;
//	int nRandom = DataTemp->uiRandom;
//	qDebug("hole from dev:%d %d\r\n",nRandom,m_nRandom);
//	if (nRandom == m_nRandom)
//	{
//		struct sockaddr_in * fromAddr;
//		fromAddr = (struct sockaddr_in *)(&(DataTemp->from));
//		if (fromAddr->sin_addr.S_un.S_addr != m_peerInfo._U.ulPeerAddr || htons(fromAddr->sin_port) != m_peerInfo.ulPeerPort)
//		{
//			m_peerInfo._U.ulPeerAddr = fromAddr->sin_addr.S_un.S_addr;
//			m_peerInfo.ulPeerPort = (htons(fromAddr->sin_port));
//		}
//		m_bHoleRecved = true;
//	}
//	return 0;
//}
int Turn::DevReady(CEseeXml::EventType e,LPVOID pData,int nDataSize)
{
	int nRandom = *((int *)pData);
	if (nRandom == m_nRandom)
	{
		m_bDevReady = true;
	}
	return 0;
}
int Turn::SoupAuth(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize)
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
int Turn::DevInfo(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize)
{
	m_Camcnt=*((int*)pData);
	return 0;
}
int Turn::Settings(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize)
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

void Turn::StreamData(LPVOID pData,int nDataSize)
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
			/*qDebug("get the I Frame!!!");*/
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
		//AudioBufAttr * AudioHead = (AudioBufAttr *)pFrameData;
		//int nBufSize = AudioHead->entries * AudioHead->packsize;
		unsigned int nBufSize=Frame->framesize;
		//frameitem.insert("pts",(UINT64)AudioHead->pts);
		frameitem.insert("pts",(UINT64)Frame->pts);
		frameitem.insert("length",nBufSize);
		//frameitem.insert("data",(int)(pFrameData+sizeof(AudioBufAttr)));
		frameitem.insert("data",(int)pFrameData);
		frameitem.insert("frametype",Frame->frametype);
		frameitem.insert("samplerate",Frame->_U.a.samplerate);
		frameitem.insert("samplewidth",Frame->_U.a.samplewidth);
		frameitem.insert("acodec",(unsigned)Frame->_U.a.enc);
		frameitem.insert("audiochannel",1);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"turn undefined data type";
	}

	applyEventProc("LiveStream",frameitem);
}
int Turn::applyEventProc(QString eventName,QVariantMap datainfo)
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

void Turn::CallBackStatus(_enConnectionStatus status)
{
	m_cStatus = status;
	QVariantMap cState;
	cState.insert("status",(int)m_cStatus);
	applyEventProc("StateChangeed",cState);
}

int Turn::GetTurnInfo()
{
	// use the server info
	ServerInfo server = GetServerInfo(m_sId.toAscii().data());

	m_TurnServerAddr = server.ServerAddr;
	m_bGetTurnInfo = false;
	TurnHead GetDvrInfo;
	GetDvrInfo.uiCmd = CS_Turn_Req;
	GetDvrInfo.uiRadom = m_nRandom;

	char cTemp[16] = {0};
	char cTempOut[16] = {0};
	strcpy(cTemp,m_sId.toAscii().data());
	// xor
	int *nF4 = (int *)cTemp;
	*nF4 = *nF4 ^ m_nRandom;
	// crypt
	m_pCypher->Cipher((unsigned char *)cTemp,(unsigned char *)cTempOut);
	memcpy(GetDvrInfo._U.TurnReq.id,cTempOut,16);
	QElapsedTimer ticktime;
	ticktime.start();
	int nRepeatCount = 0;

	m_s.DirectSendTo((char *)&GetDvrInfo,sizeof(GetDvrInfo),0,(struct sockaddr *)&(server.ServerAddr),sizeof(struct sockaddr_in));
	ticktime.start();
	nRepeatCount++;
	while (!m_bGetTurnInfo)
	{
		if (ticktime.elapsed() > 3000)
		{
			m_s.DirectSendTo((char *)&GetDvrInfo,sizeof(GetDvrInfo),0,(struct sockaddr *)&(server.ServerAddr),sizeof(struct sockaddr_in));
			ticktime.start();
			nRepeatCount++;
		}
		if (nRepeatCount > 3)
		{
			break;
		}
	}

	if (!m_bGetTurnInfo)
	{
		return -1;
	}

	return 0;
}
Turn::ServerInfo Turn::GetServerInfo(char *sId)
{
	ServerInfo ret = {0};
	ret.ServerAddr.sin_family = AF_INET;
	ret.ServerAddr.sin_port = htonsQ((short)(m_ServerInfo.ulServerPort));
	ret.ServerAddr.sin_addr.S_un.S_addr = htonlQ(m_ServerInfo.ulServerAddr);
	return ret;
}
int Turn::DataProc(CRudpSession::EventType type,LPVOID pData,int nDataSize)
{
	CRudpSession::Recv * pRecvData = (CRudpSession::Recv *)pData;
	TurnHead * RecvData = (TurnHead *)pRecvData->pRealBuffer;
	if (PROTOCOL_HEAD != RecvData->uiHeader)
	{
		return -1;
	}

	switch(RecvData->uiCmd)
	{
	case SC_Dev_Info:
		m_uiTurnIp = RecvData->_U.DevInfo.uiIp;
		m_uiTurnPort = RecvData->_U.DevInfo.uiPort;
		m_bGetTurnInfo = true;
		break;
	default:
		break;
	}
	pRecvData->bProduced = true;
	return 0;
}

int Turn::PTZUp( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "up", (char)nSpeed, 0)?1:0;
}

int Turn::PTZDown( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "down", (char)nSpeed, 0)?1:0;
}

int Turn::PTZLeft( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "left", (char)nSpeed, 0)?1:0;
}

int Turn::PTZRight( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "right", (char)nSpeed, 0)?1:0;
}

int Turn::PTZIrisOpen( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "iris_o", (char)nSpeed, 0)?1:0;
}

int Turn::PTZIrisClose( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "iris_c", (char)nSpeed, 0)?1:0;
}

int Turn::PTZFocusFar( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "focus_f", (char)nSpeed, 0)?1:0;
}

int Turn::PTZFocusNear( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "focus_n", (char)nSpeed, 0)?1:0;
}

int Turn::PTZZoomIn( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "zoom_i", (char)nSpeed, 0)?1:0;
}

int Turn::PTZZoomOut( const int &nChl, const int &nSpeed )
{
	return m_soup.PtzCtrl(nChl, "zoom_o", (char)nSpeed, 0)?1:0;
}

int Turn::PTZAuto( const int &nChl, bool bOpend )
{
	return m_soup.PtzCtrl(nChl, "auto", bOpend, 0)?1:0;
}

int Turn::PTZStop( const int &nChl, const int &nCmd )
{
	return m_soup.PtzCtrl(nChl, "stop", 0, 0)?1:0;
}
