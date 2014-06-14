#pragma once
#include <QString>
#include <QtXml\qdom.h>
#include <QThread>
#include "IDeviceNetModify.h"
#include "guid.h"
#include "IDeviceSearch.h"
#include <QList>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkAddressEntry>


typedef struct _tagNetInfo{
	QString sDeviceID;
	QString sAddress;
	QString sMask;
	QString sGateway;
	QString sMac;
	QString sPort;
	QString sUsername;
	QString sPassword;
}NetInfo;

class SetNetwork:public QThread
{
	Q_OBJECT
public:
	SetNetwork(void);
	~SetNetwork(void);
public:
	int SetNetworkInfo(const QString &sDeviceID,
		const QString &sAddress,
		const QString &sMask,
		const QString &sGateway,
		const QString &sMac,
		const QString &sPort,
		const QString &sUsername,
		const QString &sPassword);
	int AutoSetNetworkInfo(QDomNodeList itemlist);
protected:
	void run();
	int GetHostNetInfo();
	bool GetUsableIp();
	void AutoSetNetworkInfo();
	bool GetDevInfo();
private:
	NetInfo m_netinfo;
	QDomNodeList m_itemlist;
	int m_type;//1:setnetwork;2:autosetnework
	QString m_hostip;
	QString m_hostmac;
	QString m_hostgateway;
	QString m_hostsmask;
	QString m_usableIp;
	int m_devindex;
	QList<unsigned int> m_uselessIP;
};

