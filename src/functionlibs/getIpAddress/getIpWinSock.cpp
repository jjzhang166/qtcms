#include "getIpWinSock.h"
#include <WinSock2.h>
#include <QDebug>
#include <QRegExp>
#include <QStringList>
DWORD Domain2IP( char *pDomain ){
	if (!pDomain)
	{
		return 0;
	}
	if (strlen(pDomain)==0)
	{
		return 0;
	}
	DWORD dwIp=0;
	LPHOSTENT lpHostEntry;
	struct in_addr inIPAddress;
	lpHostEntry=gethostbyname(pDomain);
	if (lpHostEntry!=NULL)
	{
		char cTemp[128]={0};
		memcpy((void*)&inIPAddress,lpHostEntry->h_addr_list[0],sizeof(inIPAddress));
		strcpy(cTemp,inet_ntoa(inIPAddress));
		dwIp=inet_addr(cTemp);
		return dwIp;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"gethostbyname fail as lpHostEntry is null";
		return 0;
	}
}
getIpWinSock::getIpWinSock(void)
{
}


getIpWinSock::~getIpWinSock(void)
{
}

bool getIpWinSock::getIpAddressInLan( const QString sId,QString &sIp,QString &sPort,QString &sHttp )
{
	WSADATA tWsaData;
	int nRet=WSAStartup(MAKEWORD(2,2),&tWsaData);
	if (nRet==0)
	{
		int nSocket;
		nSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if (nSocket!=INVALID_SOCKET)
		{
			int nBroadcast=1;
			if (setsockopt(nSocket,SOL_SOCKET,SO_BROADCAST,(char*)&nBroadcast,sizeof(nBroadcast))==0)
			{
				int nTimeout=330;
				setsockopt(nSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&nTimeout,sizeof(nTimeout));
				struct sockaddr_in tClientAddr;
				memset(&tClientAddr,0,sizeof(tClientAddr));
				tClientAddr.sin_family=AF_INET;
				tClientAddr.sin_port=htons(0);
				tClientAddr.sin_addr.s_addr=INADDR_ANY;
				nRet=bind(nSocket,(struct sockaddr *)&tClientAddr,sizeof(tClientAddr));
				int nResendCount=0;
				do 
				{
					struct sockaddr_in tBroad;
					memset(&tBroad,0,sizeof(tBroad));
					tBroad.sin_family=AF_INET;
					tBroad.sin_addr.s_addr=INADDR_BROADCAST;
					tBroad.sin_port=htons(9013);

					char msg[256];
					char *pid;
					QByteArray tArrayId=sId.toLatin1();
					pid=tArrayId.data();
					memset(msg,0,sizeof(msg));
					sprintf(msg,"SEARCHJA%s&",pid);
					sendto(nSocket,msg,strlen(msg),0,(struct sockaddr *)&tBroad,sizeof(tBroad));

					struct sockaddr_in tSer;
					char buffer[1024];
					memset(buffer,0,1024);
					int nSerLen;
					nSerLen=sizeof(tSer);
					int nRevice=recvfrom(nSocket,buffer,1024,0,(struct sockaddr*)&tSer,&nSerLen);
					if (nRevice==SOCKET_ERROR)
					{
						int nError=WSAGetLastError();
						if (WSAEWOULDBLOCK==nError||WSAETIMEDOUT==nError)
						{
							nResendCount++;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInLan fail as recvfrom fail";
							closesocket(nSocket);
							return false;
						}
					}else{
						strupr(buffer);
						char sRes[256];
						QString sBackId;
						getProtocolValue(buffer,"ID",sBackId);
						if (sBackId==sId)
						{
							char *pIP;
							QString sBackIp;
							QString sBackPort;
							QString sBackHttp;
							getProtocolValue(buffer,"JAIP",sIp);
							getProtocolValue(buffer,"PORT",sPort);
							getProtocolValue(buffer,"HTTP",sHttp);
							closesocket(nSocket);
							WSACleanup();
							return true;
						}else{
							closesocket(nSocket);
							qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInLan fail as data error";
							WSACleanup();
							return false;
						}
					}
					if (nResendCount>3)
					{
						closesocket(nSocket);
						WSACleanup();
						qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInLan fail as nResendCount >3";
						return false;
					}
				} while (1);
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInLan fail as setsockopt SO_BROADCAST fail";
			}
		}else{
			int nError=WSAGetLastError();
			if (WSAEINPROGRESS==nError||WSAEMFILE==nError||WSAENOBUFS==nError)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInLan fail as system busy";
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInLan fail as network error";
			}
		}
		WSACleanup();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInLan fail as WSAStartup fail";
		abort();
	}
	return false;
}

bool getIpWinSock::getIpAddressInWan( const QString sId,QString &sIp,QString &sPort,QString &sHttp )
{
	QByteArray tName=sId.toLatin1();
	char *name=tName.data();
	QByteArray tPassword=QString("12").toLatin1();
	char *password=tPassword.data();
	WSADATA tWsaData;
	int nError=WSAStartup(MAKEWORD(2,2),&tWsaData);
	if (nError==0)
	{
		int nSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if (SOCKET_ERROR!=nSocket)
		{
			struct sockaddr_in tServer,tRem;
			memset(&tServer,0,sizeof(tServer));
			tServer.sin_family=AF_INET;
			tServer.sin_port=htons(0);
			tServer.sin_addr.s_addr=INADDR_ANY;
			if (bind(nSocket,(struct sockaddr*)&tServer,sizeof(tServer))!=SOCKET_ERROR)
			{
				//获取绑定的端口号
				int nNameLen=sizeof(struct sockaddr_in);
				unsigned short uiInterPort=0;
				int nret=getsockname(nSocket,(struct sockaddr*)&tServer,&nNameLen);
				if (nret!=SOCKET_ERROR)
				{
					uiInterPort=ntohs(tServer.sin_port);
				}
				char cPlatConnect[MAX_PATH];
				sprintf(cPlatConnect,"zhuanfa10001&&&%s---%s+++***1###", name, password);

				memset(&tRem,0,sizeof(tRem));
				tRem.sin_family=AF_INET;
				tRem.sin_port=htons(g_nNatPort);
				tRem.sin_addr.s_addr=Domain2IP(NATSERVER);
				int nTimeout=1000;
				setsockopt(nSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&nTimeout,sizeof(nTimeout));
				if (tRem.sin_addr.s_addr!=0)
				{
					int nCount=0;
					bool bFlags=false;
					do 
					{
						int nResult=sendto(nSocket,(char *)cPlatConnect,(int)strlen(cPlatConnect),0,(SOCKADDR*)&(tRem),sizeof(tRem));
						if (SOCKET_ERROR!=nResult)
						{
							struct sockaddr_in tSer;
							char buffer[1024];
							memset(buffer,0,1024);
							int nSerLen;
							nSerLen=sizeof(tSer);
							int nRevice=recvfrom(nSocket,buffer,1024,0,(struct sockaddr*)&tSer,&nSerLen);
							if (SOCKET_ERROR==nRevice)
							{
								int nError=WSAGetLastError();
								if (WSAEWOULDBLOCK==nError||WSAETIMEDOUT==nError)
								{
									//do nothing.keep going
								}else{
									break;
								}
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<buffer;
								
								getWanProtocolValue(buffer,"ip",sIp);
								getWanProtocolValue(buffer,"myserver",sPort);
								sHttp=sPort;
								bFlags=true;
								break;
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInWan as sendto fail";
							break;
						}
						nCount++;
						if (nCount>1)
						{
							qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInWan fail as nCount >1";
							break;
						}
					} while (1);
					if (bFlags)
					{
						closesocket(nSocket);
						WSACleanup();
						return true;
					}else{
						//do nothing
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInWan as Domain2IP fail";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInWan fail as bind fail";
			}
			closesocket(nSocket);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInWan fail as socket fail";
		}
		WSACleanup();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInWan fail as WSAStartup fail";
	}
	return false;
}

void getIpWinSock::getProtocolValue( char *pBuffer,QString sKeyWord,QString &sBack )
{
	QString sRex=sKeyWord+"[^&]*(?=&)";
	QRegExp rxlen(sRex);
	QString nn =QString(QLatin1String(pBuffer));
	int pos = rxlen.indexIn(nn);
	if (pos > -1) {
		sBack=rxlen.cap(0).remove(sKeyWord);
	}
}

void getIpWinSock::getWanProtocolValue( char *pBuffer,QString sKeyWord,QString &sBack )
{
	QString nn=QString(QLatin1String(pBuffer));
	if (sKeyWord=="ip")
	{
		QRegExp rxLen("ip[^#]*(?=#)");
		int pos=rxLen.indexIn(nn);
		if (pos>-1)
		{
			sBack=rxLen.cap(0).remove(sKeyWord);
		}
	}else{
		QString sRex=sKeyWord+"\\d*(?=(\\D))";
		QRegExp rxLex(sRex);
		int pos=rxLex.indexIn(nn);
		if (pos>-1)
		{
			sBack=rxLex.cap(0).remove(sKeyWord);
		}
	}
}
