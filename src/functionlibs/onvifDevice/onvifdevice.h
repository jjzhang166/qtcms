#ifndef ONVIFDEVICE_H
#define ONVIFDEVICE_H

#include "onvifdevice_global.h"
#include "IEventRegister.h"
#include <IDeviceClient.h>
#include <ISwitchStream.h>
#include <IRemotePreview.h>
#include <IDeviceConnection.h>
#include <QDebug>
#include <QMutex>
#include <QThread>
#include <QList>
#include <QObject>

typedef int (__cdecl *onvifDeviceEventCb)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagOnvifDeviceProcInfo{
	onvifDeviceEventCb proc;
	void *pUser;
}tagOnvifDeviceProcInfo;
typedef struct __tagDeviceParamInfo{
	QString sAddress;
	int nPorts;
	QString sEsee;
	QString sUserName;
	QString sPassword;
	QString sChannelName;
}tagDeviceParamInfo;
typedef struct __tagOnvifProtocolInfo{
	IDeviceConnection *pOnvifProctol;
	IDeviceClient::ConnectStatus tConnectStatus;
}tagOnvifProtocolInfo;
class  onvifDevice:public QObject,
	public IEventRegister,
	public ISwitchStream,
	public IDeviceClient
{
	Q_OBJECT
public:
	onvifDevice();
	~onvifDevice();

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	//IEventRegister
	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList &eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

	//IDeviceClient
	virtual int setDeviceHost(const QString & sAddr);
	virtual int setDevicePorts(unsigned int nPorts);
	virtual int setDeviceId(const QString & sEsee);
	virtual int connectToDevice();
	virtual int checkUser(const QString & sUsername,const QString &sPassword);
	virtual int setChannelName(const QString & sChannelName);
	virtual int liveStreamRequire(int nChannel,int nStream,bool bOpen);
	virtual int closeAll();
	virtual QString getVendor();
	virtual int getConnectStatus();

	//ISwitchStream
	virtual int SwitchStream(int nStreamNum);
public:
	//callback
	int cbConnectStatusChange(QVariantMap &tInfo);
	int cbLiveStream(QVariantMap &tInfo);
	int cbAuthority(QVariantMap &tInfo);
private:
	void eventProcCall(QString sEvent,QVariantMap tInfo);
	void backToMainThread(QString sEvName,QVariantMap tInfo);
	void clearProtocol();
	void registerEventCb(int nStreamNum,IEventRegister *pRegister);
private slots:
	void slbackToMainThread(QString sEvName,QVariantMap evMap);
signals:
	void sgbackToMainThread(QString sEvName,QVariantMap evMap);
private:
	int m_nRef;
	QMutex m_csRef;
	QMutex m_tpOnvifProtocolLock;
	QMutex m_tLiveStreamLock;
	QMultiMap<QString,tagOnvifDeviceProcInfo> m_tEventMap;
	QStringList m_sEventList;
	tagDeviceParamInfo m_tDeviceParamInfo;
	IDeviceClient::ConnectStatus m_tConnectStatus;
	Qt::HANDLE m_hMainThread;
	QMap<int,tagOnvifProtocolInfo> m_tOnvifProtocolInfo;
	volatile int m_nSwithStream;
	volatile int m_nCurrentStream;
};

#endif // ONVIFDEVICE_H
