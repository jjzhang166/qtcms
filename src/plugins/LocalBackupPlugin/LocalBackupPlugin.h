#ifndef LOCALBACKUPPLUGIN_H
#define LOCALBACKUPPLUGIN_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QVariantMap>
#include "IWebPluginBase.h"
#include "LocalBackupWindows.h"


class LocalBackupPlugin : public QObject,
	public IWebPluginBase
{
	Q_OBJECT 
public:
	LocalBackupPlugin();
	~LocalBackupPlugin();

	virtual QList<QWebPluginFactory::Plugin> plugins() const ;
	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const ;

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

private:
	int m_nRef;
	QMutex m_csRef;
};

#endif // LOCALBACKUPPLUGIN_H
