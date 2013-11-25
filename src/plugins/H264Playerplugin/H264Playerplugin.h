#ifndef H264PLAYERPLUGIN_H
#define H264PLAYERPLUGIN_H

#include <QObject>
#include <IWebPluginBase.h>
#include <QMutex>
#include "H264Playerplugin_global.h"
#include "H264PlayerWindow.h"

class H264Playerplugin : public QObject,
	public IWebPluginBase
{
	Q_OBJECT
public:
	H264Playerplugin();
	~H264Playerplugin();

	virtual QList<QWebPluginFactory::Plugin> plugins() const;

	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const;

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();

private:
	int m_nRef;
	QMutex m_csRef;
};

#endif // FFMPEGVIDEOPLUGIN_H
