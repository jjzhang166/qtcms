#include "autosearchdevice.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <IEventRegister.h>

int cbAutoSearchDevice(QString evName,QVariantMap evMap,void*pUser);
autoSearchDevice::autoSearchDevice():QWebPluginFWBase(this),
	m_pDeviceSearch(NULL)
{
	connect(&m_tAutoSearchDeviceWindow,SIGNAL(sgCancel()),this,SLOT(cancelSearch()));
	pcomCreateInstance(CLSID_DeviceSearchPlugin,NULL,IID_IAutoSearchDevice,(void**)&m_pDeviceSearch);
	IEventRegister *pRegister=NULL;
	m_pDeviceSearch->QueryInterface(IID_IEventRegister,(void**)&pRegister);
	pRegister->registerEvent("autoSearchDevice",cbAutoSearchDevice,this);
	pRegister->Release();
	pRegister=NULL;
}

autoSearchDevice::~autoSearchDevice()
{
	m_pDeviceSearch->autoSearchStop();
	m_pDeviceSearch->Release();
	m_pDeviceSearch=NULL;
}

void autoSearchDevice::startAutoSearchDevice( int nTime,int nWidth,int nHeight )
{
	m_tAutoSearchDeviceWindow.setWindowFlags(Qt::WindowStaysOnTopHint);
	//m_tAutoSearchDeviceWindow.setWindowFlags(Qt::FramelessWindowHint);
	m_tAutoSearchDeviceWindow.setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
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
	QVariantMap tItem;
	tItem.insert("reFreash",false);
	EventProcCall("reFreshDeviceList",tItem);
}

void autoSearchDevice::autoSearchDeviceTimeout()
{
	if (m_tAutoSearchDeviceWindow.isHidden())
	{
		return;
	}
	m_tAutoSearchDeviceWindow.hide();
	m_pDeviceSearch->autoSearchStop();
	IDeviceManager *pIdevice=NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDeviceManager,(void**)&pIdevice);
	if (NULL!=pIdevice)
	{
		for (int i=0;i<m_tDeviceList.size();i++)
		{
			QVariantMap tItem=m_tDeviceList.value(i);
			//根区域，默认为0
			int nArea_id=0;
			QString sDeviceName=tItem.value("SearchIP_ID").toString();
			if (tItem.value("SearchSeeId_ID").toString().size()!=0)
			{
				sDeviceName=tItem.value("SearchSeeId_ID").toString();
			}
			QString sAddress=tItem.value("SearchIP_ID").toString();
			int nPort=tItem.value("SearchMediaPort_ID").toInt();
			int nHttp=tItem.value("SearchHttpport_ID").toInt();
			QString sEseeid=tItem.value("SearchSeeId_ID").toString();
			QString sUserName="admin";
			QString sPassword;
			int nChlCount=tItem.value("SearchChannelCount_ID").toInt();
			int nConnectMethod=0;
			QString sVendor=tItem.value("SearchVendor_ID").toString();
			if ("192.168.1.168"==tItem.value("SearchIP_ID").toString())
			{
				qDebug()<<__FUNCTION__<<__LINE__<<sDeviceName;
			}
			pIdevice->AddDevice(nArea_id,sDeviceName,sAddress,nPort,nHttp,sEseeid,sUserName,sPassword,nChlCount,nConnectMethod,sVendor);
		}
		pIdevice->Release();
		pIdevice=NULL;
	}
	else{
		qDebug()<<__FUNCTION__<<__LINE__<<"CLSID_CommonLibPlugin do not support IDeviceManager interface";
		abort();
	}
	QVariantMap tItem;
	tItem.insert("reFreash",true);
	EventProcCall("reFreshDeviceList",tItem);
}

void autoSearchDevice::autoSearchDeviceCb( QVariantMap tItem )
{
	m_tDeviceList.append(tItem);
	qDebug()<<__FUNCTION__<<__LINE__<<tItem.value("SearchIP_ID");
}

int cbAutoSearchDevice( QString evName,QVariantMap evMap,void*pUser )
{
	if ("autoSearchDevice"==evName)
	{
		((autoSearchDevice*)pUser)->autoSearchDeviceCb(evMap);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"evName is not match the func,evName:"<<evName;
		return 1;
	}
}
