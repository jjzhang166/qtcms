#ifndef ONVIFSETTING_H
#define ONVIFSETTING_H

#include "onvifsetting_global.h"
#include <IWebPluginBase.h>
#include <QObject>
#include <QMutex>
class  onvifSetting:public QObject,
	public IWebPluginBase
{
	Q_OBJECT
public:
	onvifSetting();
	~onvifSetting();

	virtual QList<QWebPluginFactory::Plugin> plugins() const ;
	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const ;

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

private:
	int m_nRef;
	QMutex m_csRef;
};

#endif // ONVIFSETTING_H
