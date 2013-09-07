#include "ffmpegvideoplugin.h"
#include "qffmpeg.h"
#include <QtPlugin>
#include <guid.h>
ffmpegVideoplugin::ffmpegVideoplugin()
{

}

ffmpegVideoplugin::~ffmpegVideoplugin()
{

}

QList<QWebPluginFactory::Plugin> ffmpegVideoplugin::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-ffmpeg-video");
	mimeType.description=QString("cms preview window");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms preview window");
	plugin.description = QString("cms preview window");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * ffmpegVideoplugin::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	qffmpeg * box= new qffmpeg();
	Q_UNUSED(argumentNames);
	Q_UNUSED(argumentValues);
	return box;
}

long _stdcall ffmpegVideoplugin::QueryInterface( const IID & iid,void **ppv )
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

unsigned long __stdcall ffmpegVideoplugin::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall ffmpegVideoplugin::Release()
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

Q_EXPORT_PLUGIN2("ffmpegVideoplugin.dll",ffmpegVideoplugin)