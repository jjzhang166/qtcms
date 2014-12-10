#include "DeviceSearchPlugin.h"
#include <guid.h>

int cbXautoSearchDevice(QString evName,QVariantMap evMap,void*pUser);
DeviceSearchPlugin::DeviceSearchPlugin() :
m_nRef(0)
{
	m_sEventList<<"autoSearchDevice";
}

DeviceSearchPlugin::~DeviceSearchPlugin()
{
	m_tDeviceSearchWindows.stopAutoSearchDevice();
}

int DeviceSearchPlugin::Start()
{
	return 0;
}

int DeviceSearchPlugin::Stop()
{
	//m_Tabwindow.Stop();
	return 0;
}

int DeviceSearchPlugin::Flush()
{
	return 0;
}
int DeviceSearchPlugin::setInterval(int nInterval)
{
	return 0;
}
IEventRegister * DeviceSearchPlugin::QueryEventRegister()
{
	return NULL;
}
QList<QWebPluginFactory::Plugin> DeviceSearchPlugin::plugins() const 
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-dsview-windows");
	mimeType.description=QString("cms dsview windows");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms dsview window");
	plugin.description = QString("cms dsview window");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;

}
QObject * DeviceSearchPlugin::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const 
{
	DeviceSearchWindows * pDeviceSearchWindows= new DeviceSearchWindows();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);

	return pDeviceSearchWindows;
}

long __stdcall DeviceSearchPlugin::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IWebPluginBase == iid)
	{
		*ppv = static_cast<IWebPluginBase *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IDeviceSearch == iid)
	{
		*ppv = static_cast<IDeviceSearch *>(this);
	}
	else if (IID_IAutoSearchDevice==iid)
	{
		*ppv = static_cast<IAutoSearchDevice *>(this);
	}
	else if (IID_IEventRegister==iid)
	{
		*ppv = static_cast<IEventRegister *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall DeviceSearchPlugin::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall DeviceSearchPlugin::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

int DeviceSearchPlugin::autoSearchStart()
{
	m_tDeviceSearchWindows.registerEvent("autoSearchDevice",cbXautoSearchDevice,this);
	m_tDeviceSearchWindows.startAutoSearchDevice();
	return 0;
}

int DeviceSearchPlugin::autoSearchStop()
{
	m_tDeviceSearchWindows.stopAutoSearchDevice();
	return 0;
}

QStringList DeviceSearchPlugin::eventList()
{
	return m_sEventList;
}

int DeviceSearchPlugin::queryEvent( QString eventName,QStringList& eventParams )
{
	Q_UNUSED(eventParams);
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		//fix eventParams
		return IEventRegister::OK;
	}
}

int DeviceSearchPlugin::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		tagDeviceSearchProInfo tProInfo;
		tProInfo.proc=proc;
		tProInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProInfo);
		return IEventRegister::OK;
	}
}

void DeviceSearchPlugin::eventProcCall( QString sEvent,QVariantMap tInfo )
{
	if (m_sEventList.contains(sEvent))
	{
		tagDeviceSearchProInfo tProInfo=m_tEventMap.value(sEvent);
		if (NULL!=tProInfo.proc)
		{
			tProInfo.proc(sEvent,tInfo,tProInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is not register";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is  undefined";
	}
}

void DeviceSearchPlugin::autoSearchDeviceCb(QVariantMap evMap)
{
	eventProcCall("autoSearchDevice",evMap);
}

int cbXautoSearchDevice( QString evName,QVariantMap evMap,void*pUser )
{
	if ("autoSearchDevice"==evName)
	{
		((DeviceSearchPlugin*)pUser)->autoSearchDeviceCb(evMap);
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"evName is not match the func,evName:"<<evName;
		return 1;
	}
}
