#ifndef AUTOSEARCHDEVICE_H
#define AUTOSEARCHDEVICE_H

#include "autosearchdevice_global.h"
#include "IDeviceManager.h"
#include "IAutoSearchDevice.h"
#include "guid.h"
#include <qwfw.h>
#include <autoSearchDeviceWindow.h>
#include <QTimer>
#include <QList>

class  autoSearchDevice:public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	autoSearchDevice();
	~autoSearchDevice();
public:
	void autoSearchDeviceCb(QVariantMap tItem);
public slots:
		void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}
		void startAutoSearchDevice(int nTime,int nWidth,int nHeight);
		void cancelSearch();
		void autoSearchDeviceTimeout();
private:
	autoSearchDeviceWindow m_tAutoSearchDeviceWindow;
	IAutoSearchDevice *m_pDeviceSearch;
	QList<QVariantMap> m_tDeviceList;
};

#endif // AUTOSEARCHDEVICE_H
