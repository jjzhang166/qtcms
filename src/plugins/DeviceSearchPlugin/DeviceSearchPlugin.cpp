#include "DeviceSearchPlugin.h"
#include <guid.h>


DeviceSearchPlugin::DeviceSearchPlugin() :
m_nRef(0)
{

}

DeviceSearchPlugin::~DeviceSearchPlugin()
{
	
}

int DeviceSearchPlugin::Start()
{
	//m_Tabwindow.Start();
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