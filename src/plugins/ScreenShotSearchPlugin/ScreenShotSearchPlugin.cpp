#include "ScreenShotSearchPlugin.h"
#include <guid.h>

ScreenShotSearchPlugin::ScreenShotSearchPlugin() :
m_nRef(0)
{

}

ScreenShotSearchPlugin::~ScreenShotSearchPlugin()
{
	
}

QList<QWebPluginFactory::Plugin> ScreenShotSearchPlugin::plugins() const 
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-screenshot-search");
	mimeType.description=QString("cms screenshot search");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms screenshot search");
	plugin.description = QString("cms screenshot search");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;

}
QObject * ScreenShotSearchPlugin::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const 
{
	ScreenShotSch * pScreenShotSch = new ScreenShotSch();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);

	return pScreenShotSch;
}

long __stdcall ScreenShotSearchPlugin::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall ScreenShotSearchPlugin::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall ScreenShotSearchPlugin::Release()
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