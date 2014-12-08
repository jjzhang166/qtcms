#include "autosearchdevice.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
autoSearchDevice::autoSearchDevice():QWebPluginFWBase(this),
	m_pDeviceSearch(NULL)
{
	connect(&m_tAutoSearchDeviceWindow,SIGNAL(sgCancel()),this,SLOT(cancelSearch()));
	pcomCreateInstance(CLSID_DeviceSearchPlugin,NULL,IID_IAutoSearchDevice,(void**)&m_pDeviceSearch);
}

autoSearchDevice::~autoSearchDevice()
{
	m_pDeviceSearch->Release();
}

void autoSearchDevice::startAutoSearchDevice( int nTime,int nWidth,int nHeight )
{
	qDebug()<<__FUNCTION__<<__LINE__<<nTime;
	
	m_tAutoSearchDeviceWindow.setWindowFlags(Qt::WindowStaysOnTopHint);
	m_tAutoSearchDeviceWindow.resize(QApplication::desktop()->width()/nWidth,QApplication::desktop()->height()/nHeight);
	m_tAutoSearchDeviceWindow.show();
	m_pDeviceSearch->autoSearchStart();
	QTimer::singleShot(nTime*1000, this, SLOT(autoSearchDeviceTimeout()));
	return ;
}

void autoSearchDevice::cancelSearch()
{
	m_tAutoSearchDeviceWindow.hide();
	m_pDeviceSearch->autoSearchStop();
	qDebug()<<__FUNCTION__<<__LINE__<<"cancelSearch";
}

void autoSearchDevice::autoSearchDeviceTimeout()
{
	m_tAutoSearchDeviceWindow.hide();
	m_pDeviceSearch->autoSearchStop();
	qDebug()<<__FUNCTION__<<__LINE__<<"cancelSearch";
}
