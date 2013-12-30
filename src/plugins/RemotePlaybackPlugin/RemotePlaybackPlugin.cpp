#include <QtCore/QList>
#include <QtWebKit/QWebPluginFactory>
#include <QtPlugin>
#include <QWSWindow>
#include <libpcom.h>
#include <guid.h>
#include "RemotePlaybackPlugin.h"

RemotePlaybackPlug::RemotePlaybackPlug() :
m_nRef(0)
{
	m_RPlaybackWnd.setParent(this);
}

RemotePlaybackPlug::~RemotePlaybackPlug()
{

}

QList<QWebPluginFactory::Plugin> RemotePlaybackPlug::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-remote-playback");
	mimeType.description=QString("cms remote playback");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms remote playback");
	plugin.description = QString("cms remote playback");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * RemotePlaybackPlug::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	qDebug()<<"RemotePlaybackPlugin";
	RPlaybackWnd * pPlaybackWnd= new RPlaybackWnd();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);
	return pPlaybackWnd;
}

long __stdcall RemotePlaybackPlug::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall RemotePlaybackPlug::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall RemotePlaybackPlug::Release()
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

