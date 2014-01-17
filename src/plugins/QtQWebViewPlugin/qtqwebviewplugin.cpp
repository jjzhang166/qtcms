#include "qtqwebviewplugin.h"
#include <guid.h>
#include <libpcom.h>
QtQWebViewPlugin::QtQWebViewPlugin()
{

}

QtQWebViewPlugin::~QtQWebViewPlugin()
{

}

QList<QWebPluginFactory::Plugin> QtQWebViewPlugin::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-qwebview");
	mimeType.description=QString("cms qwebview");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms qwebview");
	plugin.description = QString("cms qwebview");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * QtQWebViewPlugin::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	QtQWebView *pQtQWebView=new QtQWebView();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);
	return pQtQWebView;
}

long __stdcall QtQWebViewPlugin::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall QtQWebViewPlugin::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall QtQWebViewPlugin::Release()
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
