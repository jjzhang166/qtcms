#pragma once
#include "IWebPluginBase.h"
#include <QObject>
#include <QMutex>
#include <QtPlugin>
#include <QtWebKit/QWebPluginFactory>
#include "autosearchdevice.h"
class autoSearchDevicePlugin:public IWebPluginBase
{
public:
	autoSearchDevicePlugin(void);
	virtual ~autoSearchDevicePlugin(void);

	virtual QList<QWebPluginFactory::Plugin> plugins() const;
	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const;
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

private:
	int						m_nRef;
	QMutex					m_csRef;
};

