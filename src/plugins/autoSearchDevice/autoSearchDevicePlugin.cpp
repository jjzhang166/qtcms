#include "autoSearchDevicePlugin.h"
#include <guid.h>

autoSearchDevicePlugin::autoSearchDevicePlugin(void)
{
}


autoSearchDevicePlugin::~autoSearchDevicePlugin(void)
{
}

QList<QWebPluginFactory::Plugin> autoSearchDevicePlugin::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-autoSearchDevice");
	mimeType.description=QString("cms autoSearchDevice");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms autoSearchDevice");
	plugin.description = QString("cms autoSearchDevice");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * autoSearchDevicePlugin::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	autoSearchDevice *pAutoSearchDevice=new autoSearchDevice();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);
	return pAutoSearchDevice;
}

long __stdcall autoSearchDevicePlugin::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IWebPluginBase==iid)
	{
		*ppv=static_cast<IWebPluginBase*>(this);
	}
	else if (IID_IPcomBase==iid)
	{
		*ppv=static_cast<IPcomBase*>(this);
	}
	else{
		*ppv=NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase*>(this)->AddRef();
	return S_OK;
}

unsigned long __stdcall autoSearchDevicePlugin::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall autoSearchDevicePlugin::Release()
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
