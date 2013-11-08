#ifndef DVRSEARCHPLUGIN_H
#define DVRSEARCHPLUGIN_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QVariantMap>
#include "IDeviceSearch.h"
#include "IWebPluginBase.h"
#include "DeviceSearchWindows.h"

class DeviceSearchPlugin : public QObject,
	public IWebPluginBase,
	public IDeviceSearch
{
	Q_OBJECT 
public:
	DeviceSearchPlugin();
	~DeviceSearchPlugin();

	int Start();
	int Stop();
	int Flush();
	int setInterval(int nInterval);
	IEventRegister * QueryEventRegister();
	
	virtual QList<QWebPluginFactory::Plugin> plugins() const ;
	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const ;

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

private:
	int m_nRef;
	QMutex m_csRef;

	//DvrTabWindows m_Tabwindow;
};

#endif // DVRSEARCHPLUGIN_H
