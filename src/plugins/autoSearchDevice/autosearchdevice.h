#ifndef AUTOSEARCHDEVICE_H
#define AUTOSEARCHDEVICE_H

#include "autosearchdevice_global.h"
#include "guid.h"
#include <qwfw.h>
#include <autoSearchDeviceWindow.h>
#include <QTimer>
class  autoSearchDevice:public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	autoSearchDevice();
	~autoSearchDevice();
public slots:
		void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}
		void startAutoSearchDevice(int nTime,int nWidth,int nHeight);
		void cancelSearch();
		void autoSearchDeviceTimeout();
private:
	autoSearchDeviceWindow m_tAutoSearchDeviceWindow;
};

#endif // AUTOSEARCHDEVICE_H
