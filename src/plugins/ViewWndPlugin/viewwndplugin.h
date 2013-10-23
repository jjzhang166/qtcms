#ifndef VIEWWNDPLUGIN_H
#define VIEWWNDPLUGIN_H

#include "viewwndplugin_global.h"
#include <QObject>
#include <IWebPluginBase.h>
#include <QMutex>

class ViewWndPlugin : public QObject,
	public IWebPluginBase
{
	Q_OBJECT
public:
	ViewWndPlugin();
	~ViewWndPlugin();

	virtual QList<QWebPluginFactory::Plugin> plugins() const;

	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const;

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();
	
public:

private:
	int m_nRef;
	QMutex m_csRef;
};

#endif // VIEWWNDPLUGIN_H
