#ifndef DVRSEARCHWINDOWS_H
#define DVRSEARCHWINDOWS_H

#include <QtGui/QTableWidget>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkAddressEntry>
#include <QtCore/QVariantMap>
#include <QtCore/QTime>
#include <QDebug>
#include <QVariant>
#include "qwfw.h"
#include "IDeviceSearch.h"
#include "IDeviceNetModify.h"
#include "DeviceSearchWindows_global.h"
#include <QMutex>
//#include "IEventRegister.h"

class DeviceSearchWindows : public QTableWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	DeviceSearchWindows(QWidget *parent = 0);
	~DeviceSearchWindows(void);
private:
	int __GetNetworkInfo(QString &address,QString &netmask);
	QVariant __QueryValue(QString sElementId);
	int __GetInitAddress(QString address,QString netmask);
	int __flushAddress();
	int __ApplyAddress(QString &sAddress,QString lSAddress);
public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}

	int Start();
	int Stop();
	int Flush();
	int setInterval(int nInterval);

	int SetNetworkInfo(const QString &sDeviceID,
		const QString &sAddress,
		const QString &sMask,
		const QString &sGateway,
		const QString &sMac,
		const QString &sPort,
		const QString &sUsername,
		const QString &sPassword);
	int AutoSetNetworkInfo();
	void addItemMap(QVariantMap item);
	void sendToHtml(QVariantMap item);
	void sendInfoToUI(QVariantMap item);

private:
	QList<IDeviceSearch *> m_deviceList;
	IDeviceNetModify *m_pDeviceNetModify;
	DevNetworkInfo m_HistoryAddress;
	DevNetworkInfo m_HistoryNetMask;
	DevNetworkInfo m_HistoryGateWay;
	QVariantMap m_DeviceItem;
	QMutex m_DeviceItemMutex;
	
signals:
	void addItemToUI(QVariantMap item);
};

#endif