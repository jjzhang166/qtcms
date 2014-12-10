#ifndef DVRSEARCHPLUGIN_H
#define DVRSEARCHPLUGIN_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QVariantMap>
#include "IDeviceSearch.h"
#include "IWebPluginBase.h"
#include "DeviceSearchWindows.h"
#include "IAutoSearchDevice.h"
typedef int (__cdecl *devicSearchEventCb)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagDeviceSearchProInfo{
	devicSearchEventCb proc;
	void *pUser;
}tagDeviceSearchProInfo;
class DeviceSearchPlugin : public QObject,
	public IEventRegister,
	public IWebPluginBase,
	public IDeviceSearch,
	public IAutoSearchDevice
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

	//autoSearch
	int autoSearchStart();
	int autoSearchStop();

	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

	void autoSearchDeviceCb(QVariantMap evMap);
private:
	void eventProcCall(QString sEvent,QVariantMap tInfo);
private:
	int m_nRef;
	QMutex m_csRef;
	DeviceSearchWindows m_tDeviceSearchWindows;
	QStringList m_sEventList;
	QMultiMap<QString,tagDeviceSearchProInfo> m_tEventMap;
};

#endif // DVRSEARCHPLUGIN_H
