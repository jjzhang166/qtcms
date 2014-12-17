#pragma once
#include <QString>
#define MAX_PATH 260
#define NATPORT 50100
#define NATSERVER	"www.msndvr.com"
#define g_nNatPort 50101
class getIpWinSock
{
public:
	getIpWinSock(void);
	~getIpWinSock(void);
public:
	bool getIpAddressInLan(const QString sId,QString &sIp,QString &sPort,QString &sHttp);
	bool getIpAddressInWan(const QString sId,QString &sIp,QString &sPort,QString &sHttp);
private:
	void getProtocolValue(char *pBuffer,QString sKeyWord,QString &sBack);
	void getWanProtocolValue(char *pBuffer,QString sKeyWord,QString &sBack);
	bool getDeviceOnServer(QString sId,QString &sServer);
};

