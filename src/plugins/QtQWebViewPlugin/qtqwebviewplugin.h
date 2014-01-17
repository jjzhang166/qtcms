#ifndef QTQWEBVIEWPLUGIN_H
#define QTQWEBVIEWPLUGIN_H


#include "IWebPluginBase.h"
#include <QObject>
#include <QMutex>
#include <QList>
#include <QtQWebView.h>
#include <QtPlugin>
#include <QtWebKit/QWebPluginFactory>

class  QtQWebViewPlugin:public IWebPluginBase
{
public:
	QtQWebViewPlugin();
	~QtQWebViewPlugin();

	virtual QList<QWebPluginFactory::Plugin> plugins() const;
	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const;
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

private:
	int						m_nRef;
	QMutex					m_csRef;
};

#endif // QTQWEBVIEWPLUGIN_H
