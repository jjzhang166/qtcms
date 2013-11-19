#ifndef DVRSEARCHWINDOWS_H
#define DVRSEARCHWINDOWS_H

#include <QtGui/QTableWidget>
#include <QtNetwork/QUdpSocket> 
#include <QtCore/QVariantMap>
#include <QtCore/QTime>
#include "qwfw.h"
#include "IDeviceSearch.h"
//#include "IEventRegister.h"

class DeviceSearchWindows : public QTableWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	DeviceSearchWindows(QWidget *parent = 0);
	~DeviceSearchWindows(void);

public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}

	int Start();
	int Stop();
	int Flush();
	int setInterval(int nInterval);

	void addItemMap(QVariantMap item);
	void sendToHtml(QVariantMap item);

private:
	QList<IDeviceSearch *> m_deviceList;
signals:
	void addItemToUI(QVariantMap item);
};

#endif