#include "autosearchdevice.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <IEventRegister.h>
int cbAutoSearchDevice(QString evName,QVariantMap evMap,void*pUser);
int g_nLoginRet=2;
autoSearchDevice::autoSearchDevice():QWebPluginFWBase(this),
	m_pDeviceSearch(NULL),
	m_pUserMangerEx(NULL)
{
	connect(&m_tAutoSearchDeviceWindow,SIGNAL(sgCancel()),this,SLOT(cancelSearch()));
	connect(&m_tAutoSearchDeviceWindow,SIGNAL(sgCancelLoginUI()),this,SLOT(cancelLoginUI()));
	connect(&m_tCheckUserStatsTimer,SIGNAL(timeout()),this,SLOT(slCheckUserStatusChange()));
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
	m_tCheckUserStatsTimer.stop();
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
	m_bCancelAutoSearchDevice=true;
	return ;
}

void autoSearchDevice::cancelSearch()
{
	m_tAutoSearchDeviceWindow.hide();
	m_pDeviceSearch->autoSearchStop();
	m_bCancelAutoSearchDevice=false;
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
	if (m_bCancelAutoSearchDevice==false)
	{
		return;
	}
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

int autoSearchDevice::showUserLoginUi(int nWidth,int nHeight)
{
	if (m_tAutoSearchDeviceWindow.isHidden()==false)
	{
		return g_nLoginRet;
	}
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
	m_bCallHide=false;
	g_nLoginRet=2;
	while(m_bCallHide==false){
		QTime dieTime=QTime::currentTime().addMSecs(1);
		while(QTime::currentTime()<dieTime){
			QCoreApplication::processEvents(QEventLoop::AllEvents,10);
		}
	}
	return g_nLoginRet;
}

int autoSearchDevice::checkUserLimit( quint64 uiCode,quint64 uiSubCode )
{
	//0：用户具有权限；1：用户未登录；2：用户登录但是没有权限
	if (NULL!=m_pUserMangerEx)
	{
		int nRet;
		nRet=m_pUserMangerEx->checkUserLimit(uiCode,uiSubCode);
		return nRet;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"call abort as m_pUserMangerEx is null";
		abort();
	}
	return 0;
}

void autoSearchDevice::cancelLoginUI()
{
	qDebug()<<__FUNCTION__<<__LINE__<<"call cancelLoginUI";
	m_bCallHide=true;
	if (m_tAutoSearchDeviceWindow.isHidden())
	{
		return;
	}
	m_tAutoSearchDeviceWindow.hide();
	return ;
}
int autoSearchDevice::loginEx()
{
	if (NULL!=m_pUserMangerEx)
	{
		m_pUserMangerEx->loginEx();
	}else{
		//do nothing
	}
	return 0;
}
int autoSearchDevice::login( QString sUserName,QString sPassword,int nCode )
{
	if (NULL!=m_pUserMangerEx)
	{
		int nRet=m_pUserMangerEx->login(sUserName,sPassword,nCode);
		if (nRet==0&&nCode==0)
		{
			g_nLoginRet=0;
		}else if (nRet==0&&nCode==1)
		{
			g_nLoginRet=1;
		}else{
			g_nLoginRet=2;
		}
		if (nCode==0)
		{
			QString sUser;
			QString sPass;
			if (m_pUserMangerEx->getIsKeepCurrentUserPassWord(sUser,sPass))
			{
				m_pUserMangerEx->setCurrentUserInfo(sUserName,sPassword);
			}
		}else{
			//do nothing
		}
		return nRet;
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

QString autoSearchDevice::getCurrentUser()
{
	QString sRet;
	if (NULL!=m_pUserMangerEx)
	{
		sRet=m_pUserMangerEx->getCurrentUser();
	}
	return sRet;
}

void autoSearchDevice::slCheckUserStatusChange()
{
	QString sCurrentUserName=getCurrentUser();
	if (sCurrentUserName.isEmpty()&&g_sHisUserName.isEmpty())
	{
		//do nothing
	}
	if (sCurrentUserName.isEmpty()&&(!g_sHisUserName.isEmpty()))
	{
		//m_sHisUserName 用户退出
		QVariantMap tItem;
		tItem.insert("userName",g_sHisUserName);
		tItem.insert("status",1);
		EventProcCall("useStateChange",tItem);
		g_sHisUserName=sCurrentUserName;
	}
	if ((!sCurrentUserName.isEmpty())&&g_sHisUserName.isEmpty())
	{
		//sCurrentUserName用户登录
		QVariantMap tItem;
		tItem.insert("userName",sCurrentUserName);
		tItem.insert("status",0);
		EventProcCall("useStateChange",tItem);
		g_sHisUserName=sCurrentUserName;
	}
	if (sCurrentUserName.isEmpty()==false&&g_sHisUserName.isEmpty()==false&&g_sHisUserName!=sCurrentUserName)
	{
		//用户切换
		QVariantMap tItem;
		tItem.insert("userName",g_sHisUserName);
		tItem.insert("status",1);
		EventProcCall("useStateChange",tItem);
		tItem.clear();
		tItem.insert("userName",sCurrentUserName);
		tItem.insert("status",0);
		EventProcCall("useStateChange",tItem);
		g_sHisUserName=sCurrentUserName;
	}
}

void autoSearchDevice::setIsKeepCurrentUserPassWord(bool bFlags)
{
	if (m_pUserMangerEx!=NULL)
	{
		m_pUserMangerEx->setIsKeepCurrentUserPassWord(bFlags);
	}else{
		//do nothing
	}
}

QVariantMap autoSearchDevice::getIsKeepCurrentUserPassWord()
{
	QVariantMap tItem;
	if (m_pUserMangerEx!=NULL)
	{
		QString sUserName;
		QString sPassWord;
		bool bIsKeep=false;
		bIsKeep=m_pUserMangerEx->getIsKeepCurrentUserPassWord(sUserName,sPassWord);
		tItem.insert("bIsKeep",bIsKeep);
		tItem.insert("sUserName",sUserName);
		tItem.insert("sPassWord",sPassWord);
	}else{
		//do nothing
	}
	return tItem;
}

void autoSearchDevice::startGetUserLoginStateChangeTime()
{
	m_tCheckUserStatsTimer.start(500);
}

int autoSearchDevice::getLoginOutInterval( QString sUserName )
{
	int  nRet=0;
	if (NULL!=m_pUserMangerEx)
	{
		nRet=m_pUserMangerEx->getLoginOutInterval(sUserName);
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
