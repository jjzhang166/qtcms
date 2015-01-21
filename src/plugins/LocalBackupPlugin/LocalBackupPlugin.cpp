#include "LocalBackupPlugin.h"
#include <guid.h>

LocalBackupPlugin::LocalBackupPlugin() :
m_nRef(0)
{

}

LocalBackupPlugin::~LocalBackupPlugin()
{
	
}

QList<QWebPluginFactory::Plugin> LocalBackupPlugin::plugins() const 
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-local-backup");
	mimeType.description=QString("cms local backup");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms local backup");
	plugin.description = QString("cms local backup");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;

}
QObject * LocalBackupPlugin::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const 
{
	LocalBackupWindows * pRemoteBackupWindows= new LocalBackupWindows();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);

	return pRemoteBackupWindows;
}

long __stdcall LocalBackupPlugin::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall LocalBackupPlugin::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall LocalBackupPlugin::Release()
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