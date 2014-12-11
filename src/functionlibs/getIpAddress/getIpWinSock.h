#pragma once
#include <QString>
class getIpWinSock
{
public:
	getIpWinSock(void);
	~getIpWinSock(void);
public:
	bool getIpAddressInLan(const QString sId,QString &sIp,QString &sPort,QString &sHttp);
	bool getIpAddressInWan(const QString sId,QString &sIp,QString &sPort,QString &sHttp);
};

