#ifndef FFMPEGVIDEOPLUGIN_H
#define FFMPEGVIDEOPLUGIN_H

#include "ffmpegvideoplugin_global.h"
#include <QObject>
#include <IWebPluginBase.h>
#include <QMutex>
#include "AvLibDll.h"
class ffmpegVideoplugin : public QObject,
	public IWebPluginBase
{
	Q_OBJECT
public:
	ffmpegVideoplugin();
	~ffmpegVideoplugin();

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
