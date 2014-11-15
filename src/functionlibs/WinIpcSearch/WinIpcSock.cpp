#include "WinIpcSock.h"
#include <WINSOCK2.H>
#include <WS2TCPIP.H>
#include "IPHlpApi.h"
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#include <assert.h>
#include "stdafx.h"
#include <vector>
#include <string>
#include <QStringList>
#include <QString>

using namespace std;  
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define IPC_SEARCH_TIME (1)
#define RW_TIMEOUT (5)
#define MCASTADDR     "239.255.255.250"
#define MCASTPORT      8002
#define BUFSIZE        1024

typedef struct HiChipSearchItem
{
	CHAR name[32];
	CHAR dev_model[32];
	CHAR esee_id[32];
	CHAR ip[32];
	CHAR netmask[32];
	CHAR gateway[32];
	CHAR port[16];
	CHAR channelcnt[16];
	CHAR mac[32];
	CHAR devid[64];
}HiChipSearchItem_t;
typedef struct HiChipSetupItem
{
	CHAR name[32];
	CHAR ip[32];
	CHAR netmask[2];
	CHAR gateway[32];
	CHAR port[16];
	CHAR mac[32];
	CHAR devid[64];
	CHAR username[32];
	CHAR password[32];
}HiChipSetupItem_t;

typedef struct HiChipSetupStatusItem
{
	CHAR status[100];
	CHAR flag[10];
}HiChipSetupStatusItem_t;
SOCKET m_Sock[32];
SOCKET m_SockMulti[32];
int m_IpCount=0;


int GetAllAdapterIp(QStringList &list);
int IPCamSetup(QVariantMap IPCamSetupInfo);

WinIpcSock::WinIpcSock(void):m_nTimeInterval(10),
	m_bIsStop(true),
	m_bIsFlush(true)
{
	m_sEventList<<"SearchDeviceSuccess"<<"SettingStatus";
}


WinIpcSock::~WinIpcSock(void)
{
	m_bIsStop=true;
	while(QThread::isRunning()){
		msleep(10);
	}
}

int WinIpcSock::Start()
{
	if (!QThread::isRunning())
	{
		QThread::start();
	}
	m_bIsStop=false;
	return 0;
}

int WinIpcSock::Stop()
{
	m_bIsStop=true;
	m_bIsFlush=true;
	return 0;
}

int WinIpcSock::Flush()
{
	m_bIsFlush=true;
	return 0;
}

int WinIpcSock::setInterval( int nInterval )
{
	m_nTimeInterval=nInterval;
	return 0;
}
int SendSearchReq(){
	int ret=0;
	CHAR sendbuf[BUFSIZE];
	_snprintf(sendbuf, BUFSIZE,
		"SEARCH * HDS/1.0\r\n"
		"CSeq:1\r\n"
		"Client-ID:nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC\r\n"
		"Accept-Type:text/HDP\r\n"
		"Content-Length:0\r\n"
		"\r\n");
	struct sockaddr_in peer_addr;
	memset(&peer_addr,0,sizeof(peer_addr));
	peer_addr.sin_family=AF_INET;
	peer_addr.sin_port=htons(MCASTPORT);
	peer_addr.sin_addr.s_addr=inet_addr(MCASTADDR);
	for (int i=0;i<m_IpCount;i++)
	{
		ret=sendto(m_Sock[i],(char*)sendbuf,strlen(sendbuf),0,(struct sockaddr*)&peer_addr,sizeof(peer_addr));
	}
	return 0;
}
static HiChipSearchItem_t parseSearchAck(CHAR*ack){
	char *name=NULL;
	char *dev_model=NULL;
	char *esee_id=NULL;
	char *channelcnt=NULL;
	char *ip=NULL;
	char *netmask=NULL;
	char *mac=NULL;
	char *gateway=NULL;
	char *port=NULL;
	char *devid=NULL;
	HiChipSearchItem_t item;
	memset(&item,0,sizeof(item));
	//name
	if (NULL!=(name=strstr(ack,"Device-Name=")))
	{
		name+=strlen("Device-Name=");
		strncpy(item.name,name,strstr(name,"\r\n")-name);
	}
	// device model
	if(NULL != (dev_model = strstr(ack, "Device-Model=")))
	{
		dev_model += strlen("Device-Model=");
		strncpy(item.dev_model, dev_model, strstr(dev_model, "\r\n") - dev_model);
	}
	// esee id
	if(NULL != (esee_id = strstr(ack, "Esee-ID=")))
	{
		esee_id += strlen("Esee-ID=");
		strncpy(item.esee_id, esee_id, strstr(esee_id, "\r\n") - esee_id);
	}
	// channel cnt
	if(NULL != (channelcnt = strstr(ack, "Channel-Cnt=")))
	{
		channelcnt += strlen("Channel-Cnt=");
		strncpy(item.channelcnt, channelcnt, strstr(channelcnt, "\r\n") - channelcnt);
	}
	// ip
	if(NULL != (ip = strstr(ack, "IP=")))
	{
		ip += strlen("IP=");
		strncpy(item.ip, ip, strstr(ip, "\r\n") - ip);
	}
	/*qDebug("Search ip:%s",item.ip);*/
	// netmask
	if (NULL != (netmask = strstr(ack, "MASK=")))
	{
		netmask += strlen("MASK=");
		strncpy(item.netmask, netmask, strstr(netmask, "\r\n") - netmask);
	}
	// gateway
	if(NULL != (gateway = strstr(ack, "Gateway=")))
	{
		gateway += strlen("Gateway=");
		strncpy(item.gateway, gateway, strstr(gateway, "\r\n") - gateway);
	}
	// mac
	if (NULL != (mac = strstr(ack, "MAC=")))
	{
		mac += strlen("MAC=");
		strncpy(item.mac, mac, strstr(mac, "\r\n") - mac);
	}
	// port
	if (NULL != (port = strstr(ack, "Http-Port=")))
	{
		port += strlen("Http-Port=");
		strncpy(item.port, port, strstr(port, "\r\n") - port);
	}
	// devid
	if(NULL != (devid = strstr(ack, "Device-ID=")))
	{
		devid += strlen("Device-ID=");
		strncpy(item.devid, devid, strstr(devid, "\r\n") - devid);
	}	
	return item;
}
int GetAck(HiChipSearchItem_t *ret_item,HiChipSetupStatusItem_t *status_item){

	char *p="";
	strncpy(status_item->status,p,sizeof(status_item->status));
	char *flag="";
	strncpy(status_item->flag,flag,sizeof(status_item->flag));

	int ret=0;
	CHAR recvbuf[BUFSIZE];
	struct sockaddr_in from_addr;
	int len=sizeof(struct sockaddr_in);

	fd_set ReadSet;
	FD_ZERO(&ReadSet);
	for(int i=0;i<m_IpCount;i++){
		FD_SET(m_SockMulti[i],&ReadSet);
	}
	struct timeval outTime;
	outTime.tv_sec=0;
	outTime.tv_usec=30;
	int nReadCount=select(m_IpCount,&ReadSet,NULL,NULL,&outTime);
	if (nReadCount<=0)
	{
		return -1;
	}
	for (int i=0;i<m_IpCount;i++)
	{
		if (FD_ISSET(m_SockMulti[i],&ReadSet))
		{
			ret=recvfrom(m_SockMulti[i],recvbuf,BUFSIZE,0,(struct sockaddr*)&from_addr,&len);
			recvbuf[ret]=0;
			if (ret>0)
			{
				QString sRecv(recvbuf);
				QString sRecvUper=sRecv;
				sRecvUper.toUpper();
				//fix me
				QString sClientId;
				if (strstr(recvbuf,"HDS/1.0 200 OK")&&(sRecvUper.contains("nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC")))
				{
					*ret_item=parseSearchAck(recvbuf);
					return 0;
				}
				else if (sRecvUper.contains("MCTP/1.0 200 OK") && sRecvUper.contains("[Success] set net information OK!"))
				{
					char *p="set info success";
					strncpy(status_item->status,p,sizeof(status_item->status));
					char *flag="true";
					strncpy(status_item->flag,flag,sizeof(status_item->flag));
					return 0;
				}
				else if (sRecvUper.contains("MCTP/1.0 200 OK") && sRecvUper.contains("[Success] set port !"))
				{
					char *p="set port success";
					strncpy(status_item->status,p,sizeof(status_item->status));
					char *flag="true";
					strncpy(status_item->flag,flag,sizeof(status_item->flag));
					return 0;
				}
				else if (sRecvUper.contains("HDS/1.0 401 Unauthorized"))
				{
					char *p="Unauthorized";
					strncpy(status_item->status,p,sizeof(status_item->status));
					char *flag="true";
					strncpy(status_item->flag,flag,sizeof(status_item->flag));
					return 0;
				}
			}
			break;
		}
	}
	return -1;
}
void WinIpcSock::run()
{
	int ret=0;
	// start up wsa
	WSADATA wsd;
	ret=WSAStartup(MAKEWORD(2,2),&wsd);
	assert(0 == ret);
	//get addr
	QStringList ipList;
	m_IpCount=GetAllAdapterIp(ipList);

	for(int i=0;i<m_IpCount;i++){
		// create socket;
		m_Sock[i] = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0,
			WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_OVERLAPPED);
		// bind
		struct sockaddr_in local_addr;
		memset(&local_addr, 0, sizeof(local_addr));
		local_addr.sin_family = AF_INET;
		local_addr.sin_port   = htons(MCASTPORT);
		//fix me
		qDebug()<<ipList.at(i).toLocal8Bit().constData();
		local_addr.sin_addr.s_addr = inet_addr(ipList.at(i).toLocal8Bit().constData());
		ret = bind(m_Sock[i], (struct sockaddr *)&local_addr, sizeof(local_addr));

		int optval = 0;
		ret = setsockopt(m_Sock[i], IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&optval, sizeof(optval));

		struct ip_mreq maddr;
		maddr.imr_multiaddr.s_addr = inet_addr(MCASTADDR);
		//fix me
		maddr.imr_interface.s_addr = inet_addr(ipList.at(i).toLocal8Bit().constData());
		ret = setsockopt(m_Sock[i], IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&maddr, sizeof(maddr));
		m_SockMulti[i] = m_Sock[i];

		int timeout = 1; 
		ret = setsockopt(m_SockMulti[i], SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	}

	DWORD dwSendCount = 0;
	int nSleepTime = 0;
	while(!m_bIsStop){
		if (::GetTickCount()-dwSendCount>m_nTimeInterval * 1000||m_bIsFlush)
		{
			SendSearchReq();
			dwSendCount=::GetTickCount();
			m_bIsFlush=false;
		}
		char RecvBuffer[1024]={0};
		HiChipSearchItem_t item;
		HiChipSetupStatusItem_t statusItem;
		int nRev=GetAck(&item,&statusItem);
		if (nRev==0)
		{
			//fix me
			if(m_sEventList.contains("SearchDeviceSuccess"))
			{
				QVariantMap qitem;
				qitem.insert("SearchDeviceName_ID",item.name);
				qitem.insert("SearchDeviceModelId_ID",item.dev_model);
				qitem.insert("SearchSeeId_ID",item.esee_id);
				qitem.insert("SearchIP_ID",item.ip);
				qitem.insert("SearchMask_ID",item.netmask);
				qitem.insert("SearchGateway_ID",item.gateway);
				qitem.insert("SearchHttpport_ID",item.port);
				qitem.insert("SearchMediaPort_ID",item.port);
				qitem.insert("SearchChannelCount_ID",item.channelcnt);
				qitem.insert("SearchMac_ID",item.mac);
				qitem.insert("SearchDeviceId_ID",item.devid);
				qitem.insert("SearchVendor_ID", "IPC");
				qitem.insert("SearchSendToUI_ID", item.devid);
				eventProcCall("SearchDeviceSuccess",qitem);
				/*m_SearchCallback("SearchDeviceSuccess",qitem,m_SearchCallbackParam);*/
			}
			if (m_sEventList.contains("SettingStatus"))
			{
				QVariantMap qitem;
				qitem.insert("Status",statusItem.status);
				qitem.insert("flag",statusItem.flag);
				if (qitem.value("flag").toString()=="true")
				{
					eventProcCall("SettingStatus",qitem);
				}
			}
		}
		if (m_SetupStatusParm.size()!=0)
		{
			m_SetupStatusParmMutex.lock();
			QVariantMap item=m_SetupStatusParm.dequeue();
			m_SetupStatusParmMutex.unlock();
			IPCamSetup(item);
		}
		nSleepTime ++;
		if (nSleepTime > 100)
		{
			msleep(1);
			nSleepTime = 0;
		}
	}
	for(int i=0;i<m_IpCount;i++){
		closesocket(m_SockMulti[i]);
		closesocket(m_Sock[i]);
	}
	WSACleanup();
	m_SetupStatusParmMutex.lock();
	m_SetupStatusParm.clear();
	m_SetupStatusParmMutex.unlock();
}

int WinIpcSock::registerEvent( QString eventName,WINIPCSearchCB eventCB,void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		return -1;
	}
	WINEventCBInfo procInfo;
	procInfo.evCBName=eventCB;
	procInfo.pUser=pUser;
	m_mEventCBMap.insert(eventName,procInfo);
	return 0;
}

void WinIpcSock::eventProcCall( QString sEvent,QVariantMap param )
{
	if (m_sEventList.contains(sEvent))
	{
		WINEventCBInfo eventDes=m_mEventCBMap.value(sEvent);
		if (NULL!=eventDes.evCBName)
		{
			eventDes.evCBName(sEvent,param,eventDes.pUser);
		}
	}
}

int WinIpcSock::SetNetworkInfo( const QString &sDeviceID, const QString &sAddress, const QString &sMask, const QString &sGateway, const QString &sMac, const QString &sPort, const QString &sUsername, const QString &sPassword )
{
	QVariantMap item;
	item.insert("sDeviceID",sDeviceID);
	item.insert("sAddress",sAddress);
	item.insert("sMask",sMask);
	item.insert("sGateway",sGateway);
	item.insert("sMac",sMac);
	item.insert("sPort",sPort);
	item.insert("sUsername",sUsername);
	item.insert("sPassword",sPassword);
	if (QThread::isRunning())
	{
		m_SetupStatusParmMutex.lock();
		m_SetupStatusParm.enqueue(item);
		m_SetupStatusParmMutex.unlock();
	}else{
		IPCamSetup(item);
	}
	return 0;
}

int GetAllAdapterIp(QStringList &list){
	list.clear();
	// Get buffer size that the output needed.
	ulong ulSize;
	GetAdaptersInfo(NULL,&ulSize);
	// Malloc memories that recieve the infomations
	char *pInfoBuffer=(char *)malloc(ulSize);

	// Get infomations of adapters
	GetAdaptersInfo((PIP_ADAPTER_INFO)pInfoBuffer,&ulSize);
	// list all adapters
	PIP_ADAPTER_INFO info=(PIP_ADAPTER_INFO)pInfoBuffer;
	while(info){
		PIP_ADDR_STRING addr=&info->IpAddressList;
		do 
		{
			list.append(QString(addr->IpAddress.String));
			addr=addr->Next;
		} while (addr);
		info=info->Next;
	}
	free(pInfoBuffer);
	return list.size();
}
int IPCamSetup(QVariantMap item){
	HiChipSetupItem_t IPCamSetupInfo;
	strcpy(IPCamSetupInfo.devid,item.value("sDeviceID").toString().toLocal8Bit().data());
	strcpy(IPCamSetupInfo.ip,item.value("sAddress").toString().toLocal8Bit().data());
	strcpy(IPCamSetupInfo.netmask,item.value("sMask").toString().toLocal8Bit().data());
	strcpy(IPCamSetupInfo.gateway,item.value("sGateway").toString().toLocal8Bit().data());
	strcpy(IPCamSetupInfo.mac,item.value("sMac").toString().toLocal8Bit().data());
	strcpy(IPCamSetupInfo.port,item.value("sPort").toString().toLocal8Bit().data());
	strcpy(IPCamSetupInfo.username,item.value("sUsername").toString().toLocal8Bit().data());
	strcpy(IPCamSetupInfo.password,item.value("sPassword").toString().toLocal8Bit().data());
	printf("_dev_id=%s, _ip=%s, _mac=%s, _port=%s, _usr=%s, _pwd=%s",
		IPCamSetupInfo.devid, 
		IPCamSetupInfo.ip, 
		IPCamSetupInfo.mac, 
		IPCamSetupInfo.port, 
		IPCamSetupInfo.username, 
		IPCamSetupInfo.password);
	int ret;
	int count=0;
	CHAR sendbuf[BUFSIZE];
	int nCSeq=0;
	char content[256]={0};
	struct sockaddr_in peer_addr;
	memset(&peer_addr,0,sizeof(peer_addr));
	peer_addr.sin_family=AF_INET;
	peer_addr.sin_port=htons(MCASTPORT);
	peer_addr.sin_addr.s_addr=inet_addr(MCASTADDR);

	if (strlen(IPCamSetupInfo.ip)>0
		||strlen(IPCamSetupInfo.mac)>0
		||strlen(IPCamSetupInfo.gateway)>0
		||strlen(IPCamSetupInfo.port)>0)
	{
		strcpy(content, "netconf set");
		if(strlen(IPCamSetupInfo.ip) > 0)
		{
			sprintf(content + strlen(content), " -ipaddr %s", IPCamSetupInfo.ip);
		}
		if(strlen(IPCamSetupInfo.mac) > 0)
		{
			sprintf(content + strlen(content), " -hwaddr %s", IPCamSetupInfo.mac);
		}
		if(strlen(IPCamSetupInfo.gateway) > 0)
		{
			sprintf(content + strlen(content), " -gateway %s", IPCamSetupInfo.gateway);
		}
		sprintf(content + strlen(content), "\r\n");
		if(strlen(IPCamSetupInfo.port) > 0)
		{
			sprintf(content + strlen(content), "httpport set -httpport %s", IPCamSetupInfo.port);
		}

		_snprintf(sendbuf, BUFSIZE, "CMD * HDS/1.0\r\n"
			"CSeq:%d\r\n"
			"Client-ID:nvmOPxEnYfQRAeLFdsMrpBbnMDbEPiMC\r\n"
			"Accept-Type:text/HDP\r\n"
			"Authorization:Basic %s:%s\r\n"
			"Device-ID:%s\r\n"
			"Content-Length:%d\r\n"
			"\r\n"
			"%s\r\n", nCSeq, IPCamSetupInfo.username, IPCamSetupInfo.password, IPCamSetupInfo.devid, strlen(content), content);
		for(int i=0;i<m_IpCount;i++){
			ret = sendto(m_Sock[i], (char *)sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
		}
		nCSeq++;
		Sleep(100);
	}
	return 0;
}
