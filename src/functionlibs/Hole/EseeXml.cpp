// EseeXml.cpp: implementation of the CEseeXml class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "EseeXml.h"
#include "netlib.h"
#include <QtCore/QElapsedTimer>
#include <QtNetwork/QHostInfo>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
//
//ushort htons(ushort hostshort)
//{
//	union{
//		short s;
//		char c[2];
//	}un;
//	un.c[1]=0xff&hostshort;
//	un.c[0]=0xff&(hostshort>>8);
//	return un.s;
//}
//extern unsigned int htons32(unsigned int hostshort)
//{
//	union{
//		int s;
//		char c[4];
//	}un;
//	un.c[3]=0xff&hostshort;
//	un.c[2]=0xff&(hostshort>>8);
//	un.c[1]=0xff&(hostshort>>16);
//	un.c[0]=0xff&(hostshort>>24);
//	return un.s;
//}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEseeXml::CEseeXml()
{

}

CEseeXml::~CEseeXml()
{

}

#define SERVER_DOMAIN	("www.msndvr.com")
//CEseeXml::TurnServerInfo CEseeXml::TurnReq(char *sId)
//{
//	memset(&m_ServerInfo,0,sizeof(m_ServerInfo));
//
//	char ReqStr[]="<esee ver=\"1.0\"><head><cmd>20011</cmd><tick>%d</tick></head><id>%s</id></esee>";
//	char ReqStrXml[1024] = {0};
//	sprintf(ReqStrXml,ReqStr,GetTickCountQ(),sId);
//
//	int nRetryCount = 0;
//	m_bServerInfoReady = false;
//	m_bDevReady = false;
//	while (!m_bServerInfoReady)
//	{
//		struct sockaddr_in addr;
//		addr.sin_family = AF_INET;
//		addr.sin_port = htonsQ(60101);
//		addr.sin_addr.s_addr = GetServerAddr(SERVER_DOMAIN);
//
//		m_s->DirectSendTo(ReqStrXml,strlen(ReqStrXml),0,(struct sockaddr *)&addr,sizeof(addr));
//
//		QElapsedTimer Ticket;
//		Ticket.start();
//		while(!m_bServerInfoReady && Ticket.elapsed() < 3000)
//		{
//			SleepQ(30);
//		}
//		nRetryCount ++;
//		if (nRetryCount > 3)
//		{
//			qDebug("TurnReq time out\r\n");
//			break;
//		}
//	}
//
//	return m_ServerInfo;
//}

void CEseeXml::StopReq()
{
	m_reqing = false;
}

CEseeXml::HolePeerInfo CEseeXml::HoleReq(char *sId,int nRandom)
{
	memset(&m_HolePeerInfo,0,sizeof(m_HolePeerInfo));

	char ReqStr[]="<esee ver=\"1.0\"><head><cmd>%d</cmd><tick>%d</tick></head><id>%s</id><random>%d</random></esee>";
	char ReqStrXml[1024] = {0};
	sprintf(ReqStrXml,ReqStr,Cmd_hole_client_req,GetTickCountQ(),sId,nRandom);

	int nRetryCount = 0;
	m_bHoleReqAck = false;
	m_reqing = true;
	while ((!m_bHoleReqAck) && m_reqing)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htonsQ(60101);
		addr.sin_addr.s_addr = GetServerAddr(SERVER_DOMAIN);
		
		m_s->DirectSendTo(ReqStrXml,strlen(ReqStrXml),0,(struct sockaddr *)&addr,sizeof(addr));
		
		QElapsedTimer Ticket;
		Ticket.start();
		while((!m_bHoleReqAck) && m_reqing && Ticket.elapsed() < 3000)
		{
			SleepQ(30);
		}
		nRetryCount ++;
		if (nRetryCount > 3)
		{
			qDebug("HoleReq time out\r\n");
			break;
		}
	}

	return m_HolePeerInfo;
}

int CEseeXml::HoleTo(HolePeerInfo info,int nRandom)
{
	char ReqStr[]="<esee ver=\"1.0\"><head><cmd>%d</cmd><tick>%d</tick></head><random>%d</random></esee>";
	char ReqStrXml[1024] = {0};
	sprintf(ReqStrXml,ReqStr,Cmd_hole_to_dev,GetTickCountQ(),nRandom);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htonsQ((unsigned short)info.ulPeerPort);
	addr.sin_addr.s_addr = info._U.ulPeerAddr;

	m_s->DirectSendTo(ReqStrXml,strlen(ReqStrXml),0,(struct sockaddr *)&addr,sizeof(addr));

	return 0;
}

int CEseeXml::WaitForReadySignal()
{
	QElapsedTimer Ticket;
	Ticket.start();
	while (!m_bDevReady && Ticket.elapsed() < 6000)
	{
		SleepQ(30);
	}
	if (!m_bDevReady)
	{
		// timeout
		qDebug("WaitForReadySignal time out\r\n");
		return -1;
	}
	return 0;
}

void CEseeXml::SetSession(CRudpSession *s)
{
	m_s = s;
}

int CEseeXml::EventCall(EventType e,LPVOID pData,int nDataSize)
{
	if (e >= EVENT_CNT)
	{
		return E_INVALID_PARAM;
	}

	if (m_eMap[e].proc)
	{
		return m_eMap[e].proc(m_eMap[e].e,pData,nDataSize,m_eMap[e].pUserData);
	}

	return E_EVENT_NOT_SET;
}

int CEseeXml::GetRandomFromProtocol(QDomElement RootElement)
{
	QDomElement randomElement = RootElement.elementsByTagName("random").at(0).toElement();

	int nRandom = -1;
	if (!randomElement.isNull())
	{
		//nRandom = atoi(randomElement->GetText());
		nRandom = randomElement.childNodes().item(0).toText().data().toInt();
	}
	return nRandom;
}

CEseeXml::HolePeerInfo CEseeXml::ParseHolePeerInfo(QDomElement RootElement)
{
	HolePeerInfo holePeer;
	QString tempip,tempport;

	RootElement = RootElement.firstChildElement();

	QDomElement dvripElement = RootElement.nextSiblingElement("dvrip");
	tempip = dvripElement.childNodes().item(0).toText().data();

	QDomElement dvrportElement = RootElement.nextSiblingElement("dvrport");
	tempport = dvrportElement.childNodes().item(0).toText().data();
	/*holePeer._U.ulPeerAddr = inet_addr(dvripElement->GetText());
	holePeer.ulPeerPort = atoi(dvrportElement->GetText());*/
	holePeer._U.ulPeerAddr = htonlQ(QHostAddress(tempip).toIPv4Address());
	holePeer.ulPeerPort = tempport.toInt();

	return holePeer;
}

CEseeXml::TurnServerInfo CEseeXml::ParseServerInfo(QDomElement RootElement)
{
	TurnServerInfo turn;
	//TraversalTask *taskTemp = NULL;

	//taskTemp = StartTaskByName(RootElement,"dvrip");
	//TiXmlElement *dvripElement = FindNextElement(taskTemp);
	//CloseTask(taskTemp);
	QDomElement dvripElement = RootElement.nextSiblingElement("dvrip");

	//taskTemp = StartTaskByName(RootElement,"dvrport");
	//TiXmlElement *dvrportElement = FindNextElement(taskTemp);
	//CloseTask(taskTemp);
	QDomElement dvrportElement = RootElement.nextSiblingElement("dvrport");

	//taskTemp = StartTaskByName(RootElement,"turnserver");
	//TiXmlElement *turnserverElement = FindNextElement(taskTemp);
	//CloseTask(taskTemp);
	QDomElement turnserverElement = RootElement.nextSiblingElement("turnserver");
	//turn.ulDeviceAddr = inet_addr(dvripElement->GetText());
	turn.ulDeviceAddr = QHostAddress(dvripElement.text()).toIPv4Address();
	//turn.ulDevicePort = atoi(dvrportElement->GetText());
	turn.ulDevicePort = dvrportElement.text().toInt();
	/*char *sServerInfo = (char *)turnserverElement->GetText();
	char sServerIp[32] = {0};
	char *pDivAddr = strchr(sServerInfo,':');
	if (NULL != pDivAddr)
	{
		strncpy(sServerIp,sServerInfo,pDivAddr - sServerInfo);
	}*/
	
	//turn.ulServerAddr = inet_addr(sServerIp);
	QString qsIpPort = turnserverElement.text();
	int ddindex = qsIpPort.indexOf(':');
	turn.ulServerAddr = QHostAddress(qsIpPort.left(ddindex)).toIPv4Address();

	/*char sPort[32] = {0};
	strncpy(sPort,pDivAddr + 1,strlen(sServerInfo) - strlen(sServerIp) - 1);
	turn.ulServerPort = atoi(sPort);*/

	turn.ulServerPort = qsIpPort.right(qsIpPort.length()-ddindex+1).toInt();
	return turn;
}

void CEseeXml::SetEventPorc(EventType e,EventProc func,LPVOID pUserData)
{
	if (e >= EVENT_CNT)
	{
		return;
	}

	m_eMap[e].e = e;
	m_eMap[e].proc = func;
	m_eMap[e].pUserData = pUserData;
}

int CEseeXml::DataProc(CRudpSession::EventType type,LPVOID pData,int nDataSize)
{
	CRudpSession::Recv * pRecvData = (CRudpSession::Recv *)pData;
	char *sXml = pRecvData->pRealBuffer;

	QString cMsg=sXml;
	if (! cMsg.startsWith("<esee"))
	{
		return -1;
	}
	QDomDocument doc;
	doc.setContent(cMsg);

	QDomElement RootElement = doc.documentElement();

	unsigned int uiCmd = ParseCmd(RootElement);
	switch(uiCmd)
	{
	case 0: // Cmd not Found
		break;
	case Cmd_turn_server_info:
		{
			m_ServerInfo = ParseServerInfo(RootElement);
			qDebug("Get server info %d %d %d %d\r\n",m_ServerInfo.ulDeviceAddr,m_ServerInfo.ulDevicePort,m_ServerInfo.ulServerAddr,m_ServerInfo.ulServerPort);
			m_bServerInfoReady = true;
		}
		break;
	case Cmd_turn_dev_ready:
		{
			qDebug("Get ready signal\r\n");
			m_bDevReady = true;
		}
		break;
	case Cmd_hole_dev_info:
		{
			m_HolePeerInfo = ParseHolePeerInfo(RootElement);
			qDebug("Get hole peer %d.%d.%d.%d:%d\r\n",m_HolePeerInfo._U._b.b1,
				m_HolePeerInfo._U._b.b2,
				m_HolePeerInfo._U._b.b3,
				m_HolePeerInfo._U._b.b4,
				m_HolePeerInfo.ulPeerPort);
			m_bHoleReqAck = true;
		}
		break;
	case Cmd_hole_from_dev:
		{
			HoleFromData holeFromData;
			holeFromData.uiRandom = GetRandomFromProtocol(RootElement);
			holeFromData.from = *(pRecvData->from);
			
			// CallBack
			EventCall(EVENT_HOLE_FROM_DEV,&holeFromData,sizeof(holeFromData));
		}
		break;
	case Cmd_hole_dev_ready:
		{
			qDebug("Get hole ready signal\r\n");
			int nRandom = GetRandomFromProtocol(RootElement);

			// CallBack
			EventCall(EVENT_DEV_READY,&nRandom,sizeof(nRandom));
		}
		break;
	}

	pRecvData->bProduced = true;

	return 0;
}

//typedef struct  hostent {
//	char    FAR * h_name;           /* official name of host */
//	char    FAR * FAR * h_aliases;  /* alias list */
//	short   h_addrtype;             /* host address type */
//	short   h_length;               /* length of address */
//	char    FAR * FAR * h_addr_list; /* list of addresses */
//#define h_addr  h_addr_list[0]          /* address, for backward compat */
//}FAR *LPHOSTENT;

//DWORD CEseeXml::GetServerAddr()
//{
//	DWORD dwIP = 0;
//	static QHostInfo hinfo = QHostInfo::fromName(SERVER_DOMAIN);
//	if (hinfo.error()!=QHostInfo::NoError)
//	{
//
//		dwIP = QHostAddress(SERVER_DOMAIN).toIPv4Address();
//	}
//	else
//	{
//		dwIP = hinfo.addresses()[0].toIPv4Address();
//	}
//	return htonlQ(dwIP);
//}

unsigned int CEseeXml::ParseCmd(QDomElement & RootElement)
{
	//"<esee ver="1.0"><head><cmd>21101</cmd><tick>0</tick></head><dvrip>14.154.201.40</dvrip><dvrport>25369</dvrport><random>28755</random></esee>"
	unsigned int nRet = 0;

	QDomElement nextElement = RootElement.firstChildElement("head");
	if (!nextElement.isNull())
	{

		nextElement = nextElement.firstChildElement("cmd");
		if (! nextElement.isNull())
		{
			nRet = nextElement.childNodes().item(0).toText().data().toInt();

		}
	}
	
	return nRet;
}
