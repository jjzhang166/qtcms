#include "autosearchdevice.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <IEventRegister.h>

int cbAutoSearchDevice(QString evName,QVariantMap evMap,void*pUser);
autoSearchDevice::autoSearchDevice():QWebPluginFWBase(this),
	m_pDeviceSearch(NULL),
	m_pUserMangerEx(NULL)
{
	connect(&m_tAutoSearchDeviceWindow,SIGNAL(sgCancel()),this,SLOT(cancelSearch()));
	pcomCreateInstance(CLSID_DeviceSearchPlugin,NULL,IID_IAutoSearchDevice,(void**)&m_pDeviceSearch);
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IUserMangerEx,(void**)&m_pUserMangerEx);
	IEventRegister *pRegister=NULL;
	m_pDeviceSearch->QueryInterface(IID_IEventRegister,(void**)&pRegister);
	pRegister->registerEvent("autoSearchDevice",cbAutoSearchDevice,this);
	pRegister->Release();
	pRegister=NULL;
}

autoSearchDevice::~autoSearchDevice()
{
	if (NULL!=m_pDeviceSearch)
	{
		m_pDeviceSearch->autoSearchStop();
		m_pDeviceSearch->Release();
		m_pDeviceSearch=NULL;
	}
	if (NULL!=m_pUserMangerEx)
	{
		m_pUserMangerEx->Release();
		m_pUserMangerEx=NULL;
	}
}

void autoSearchDevice::startAutoSearchDevice( int nTime,int nWidth,int nHeight )
{
	m_tAutoSearchDeviceWindow.resize(nWidth,nHeight);
	//load url
	QString temp = QCoreApplication::applicationDirPath();
	QSettings MainIniFile(temp + "/MainSet.ini",QSettings::IniFormat);
	QString sTheme = MainIniFile.value(QString("Configure/autoSearchDevice")).toString();
	QString sThemeDir = MainIniFile.value(sTheme + "/Dir").toString();
	QString sUiDir = temp + sThemeDir;
	m_tAutoSearchDeviceWindow.loadHtmlUrl(sUiDir);

	m_tAutoSearchDeviceWindow.show();
	m_tAutoSearchDeviceWindow.setAttribute(Qt::WA_TransparentForMouseEvents);
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
	//m_tAutoSearchDeviceWindow.hide();
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
			pIdevice->AddDevice(nArea_id,sDeviceName,sAddress,nPort,nHttp,sEseeid,sUserName,sPassword,nChlCount,nConnectMethod,sVendor);
		}
		pIdevice->Release();
		pIdevice=NULL;
	}
	else{
		qDebug()<<__FUNCTION__<<__LINE__<<"CLSID_CommonLibPlugin do not support IDeviceManager interface";
		abort();
	}
	m_tAutoSearchDeviceWindow.hide();
	QVariantMap tItem;
	tItem.insert("reFreash",true);
	EventProcCall("reFreshDeviceList",tItem);
}

void autoSearchDevice::autoSearchDeviceCb( QVariantMap tItem )
{
	m_tDeviceList.append(tItem);
}

void autoSearchDevice::showUserLoginUi(int nWidth,int nHeight)
{
	m_tAutoSearchDeviceWindow.resize(nWidth,nHeight);
	//load url
	QString temp = QCoreApplication::applicationDirPath();
	QSettings MainIniFile(temp + "/MainSet.ini",QSettings::IniFormat);
	QString sTheme = MainIniFile.value(QString("Configure/login")).toString();
	QString sThemeDir = MainIniFile.value(sTheme + "/Dir").toString();
	QString sUiDir = temp + sThemeDir;
	m_tAutoSearchDeviceWindow.loadHtmlUrl(sUiDir);

	m_tAutoSearchDeviceWindow.show();
	m_tAutoSearchDeviceWindow.setAttribute(Qt::WA_TransparentForMouseEvents);
	return;
}

int autoSearchDevice::checkUserLimit( quint64 uiCode,quint64 uiSubCode )
{
	//0：用户具有权限；1：用户未登录；2：用户登录但是没有权限
	if (NULL!=m_pUserMangerEx)
	{
		return m_pUserMangerEx->checkUserLimit(uiCode,uiSubCode);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"call abort as m_pUserMangerEx is null";
		abort();
	}
	return 0;
}

void autoSearchDevice::cancelLoginUI()
{
	m_tAutoSearchDeviceWindow.hide();
	return ;
}

int autoSearchDevice::login( QString sUserName,QString sPassword,int nCode )
{
	if (NULL!=m_pUserMangerEx)
	{
		return m_pUserMangerEx->login(sUserName,sPassword,nCode);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"call abort as m_pUserMangerEx is null";
		abort();
	}
	return 0;
}

QStringList autoSearchDevice::getUserList()
{
	QStringList tUserNameList;
	if (NULL!=m_pUserMangerEx)
	{
		m_pUserMangerEx->getUserList(tUserNameList);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"call abort as m_pUserMangerEx is null";
		abort();
	}
	return tUserNameList;
}

QVariantMap autoSearchDevice::getUserLimit(QString sUserName)
{
	quint64 uiLimit=0;
	QVariantMap tVariantMap;
	if (NULL!=m_pUserMangerEx)
	{
		m_pUserMangerEx->getUserLimit(sUserName,uiLimit,tVariantMap);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"call abort as m_pUserMangerEx is null";
		abort();
	}
	tVariantMap.insert("mainLimit",uiLimit);
	return tVariantMap;
}

int autoSearchDevice::getUserInDatabaseId( QString sUserName )
{
	int nRet=-1;
	if (NULL!=m_pUserMangerEx)
	{
		m_pUserMangerEx->getUserDatabaseId(sUserName,nRet);
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"call abort as m_pUserMangerEx is null";
		abort();
	}
	return nRet;
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
