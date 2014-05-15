#include "RemoteBackupPlugin.h"
#include <guid.h>

RemoteBackupPlugin::RemoteBackupPlugin() :
m_nRef(0)
{

}

RemoteBackupPlugin::~RemoteBackupPlugin()
{
	
}

int RemoteBackupPlugin::startBackup(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,
	int nChannel,
	int nTypes,
	const QString & startTime,
	const QString & endTime,
	const QString & sbkpath)
{
	return 0;
}
int RemoteBackupPlugin::stopBackup()
{
	return 0;
}
float RemoteBackupPlugin::getProgress()
{
	return 0.0f;
}

QList<QWebPluginFactory::Plugin> RemoteBackupPlugin::plugins() const 
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-remote-backup");
	mimeType.description=QString("cms remote backup");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms remote backup");
	plugin.description = QString("cms remote backup");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;

}
QObject * RemoteBackupPlugin::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const 
{
	RemoteBackupWindows * pRemoteBackupWindows= new RemoteBackupWindows();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);

	return pRemoteBackupWindows;
}

long __stdcall RemoteBackupPlugin::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall RemoteBackupPlugin::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall RemoteBackupPlugin::Release()
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