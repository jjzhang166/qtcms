#ifndef DEVICECLIENT_H_AIFQ90SDGSGSDGPZ1283R__
#define DEVICECLIENT_H_AIFQ90SDGSGSDGPZ1283R__

#include "deviceclient_global.h"
#include <IDeviceClient.h>
#include <IDeviceConnection.h>
#include <IRemotePreview.h>
#include <QMultiMap>
#include <QMutex>

#include <IEventRegister.h>

class  DeviceClient:public IDeviceClient,
	public IEventRegister
{
public:
	DeviceClient();
	~DeviceClient();

	virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

	virtual int connectToDevice(const QString &sAddr,unsigned int uiPort,const QString &sEseeId);
	virtual int checkUser(const QString & sUsername,const QString &sPassword);
	virtual int setChannelName(const QString & sChannelName);
	virtual int liveStreamRequire(int nChannel,int nStream,bool bOpen);
	virtual int closeAll();
	virtual QString getVendor();
	virtual int getConnectStatus();
private:
	int m_nRef;
	QMutex m_csRef;

	QStringList m_EventList;
	QMultiMap<QString,DeviceClientInfoItem> m_EventMap;

	IDeviceConnection *m_DeviceConnecton;
	QString m_sAddr;
	QString m_sEseeId;
	unsigned int m_uiPort;
	QVariantMap m_ports;
	QString m_sChannelName;

private:
	int cbInit();

};

#endif // DEVICECLIENT_H
