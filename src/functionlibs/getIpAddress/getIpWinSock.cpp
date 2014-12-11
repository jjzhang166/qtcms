#include "getIpWinSock.h"
#include <WinSock2.h>
#include <QDebug>
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
		}else{
			int nError=WSAGetLastError();
			if (WSAEINPROGRESS)
			{
			}
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getIpAddressInLan fail as WSAStartup fail";
		abort();
	}
	return false;
}

bool getIpWinSock::getIpAddressInWan( const QString sId,QString &sIp,QString &sPort,QString &sHttp )
{
	return false;
}
