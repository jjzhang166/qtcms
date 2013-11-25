#include "H264Playerplugin.h"
#include <QtPlugin>
#include <guid.h>
H264Playerplugin::H264Playerplugin()
{

}

H264Playerplugin::~H264Playerplugin()
{

}

QList<QWebPluginFactory::Plugin> H264Playerplugin::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-h264-player");
	mimeType.description=QString("cms h264player window");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms h264player window");
	plugin.description = QString("cms h264player window");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * H264Playerplugin::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	H264PlayerWindow * pH264PlayerWindow= new H264PlayerWindow();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);
	return pH264PlayerWindow;
}

long _stdcall H264Playerplugin::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall H264Playerplugin::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall H264Playerplugin::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		qDebug("delete this;");
		delete this;
	}
	return nRet;
}

Q_EXPORT_PLUGIN2("H264Playerplugin.dll",H264Playerplugin)