#ifndef DEVICECLIENT_H_AIFQ90SDGSGSDGPZ1283R__
#define DEVICECLIENT_H_AIFQ90SDGSGSDGPZ1283R__

#include "deviceclient_global.h"
#include "DeviceGlobalSettings.h"
#include "GlobalSettings.h"
#include <IDeviceClient.h>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <IDeviceConnection.h>
#include <IRemotePreview.h>
#include <QMultiMap>
#include <QMutex>
#include <IDeviceRemotePlayback.h>
#include <IDeviceSearchRecord.h>
#include <IRemotePlayback.h>
#include <IEventRegister.h>
#include <IRemoteBackup.h>
#include "RemoteBackup.h"

int cbStateChangeFormIprotocl(QString evName,QVariantMap evMap,void*pUser);
int cbRecordStream(QString evName,QVariantMap evMap,void*pUser);

class  DeviceClient:public QThread,
	public IDeviceClient,
	public IEventRegister,
	public IDeviceSearchRecord,
	public IDeviceGroupRemotePlayback,
	public IRemoteBackup
{
	Q_OBJECT
public:
	DeviceClient();
	~DeviceClient();

	virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();
	//IEventRegister
	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);
	//IDeviceClient
	virtual int connectToDevice(const QString &sAddr,unsigned int uiPort,const QString &sEseeId);
	virtual int checkUser(const QString & sUsername,const QString &sPassword);
	virtual int setChannelName(const QString & sChannelName);
	virtual int liveStreamRequire(int nChannel,int nStream,bool bOpen);
	virtual int closeAll();
	virtual QString getVendor();
	virtual int getConnectStatus();

	int ConnectStatusProc(QVariantMap evMap);
	
	//IDeviceSearchRecord
	virtual int startSearchRecFile(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime);
	//IDeviceGroupRemotePlayback
	virtual int AddChannelIntoPlayGroup(int nChannel,QWidget * wnd);
	virtual int GroupPlay(int nTypes,const QDateTime & start,const QDateTime & end);
	virtual QDateTime GroupGetPlayedTime();
	virtual int GroupPause();
	virtual int GroupContinue();
	virtual int GroupStop();
	virtual bool GroupEnableAudio(bool bEnable);
	virtual int GroupSpeedFast();
	virtual int GroupSpeedSlow();
	virtual int GroupSpeedNormal();

	int recordFrame(QVariantMap &evMap);

	//IRemoteBackup
	virtual int startBackup(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,
		int nChannel,
		int nTypes,
		const QDateTime & startTime,
		const QDateTime & endTime,
		const QString & sbkpath);
	virtual int stopBackup();
	virtual float getProgress();

private slots:
	void action(QString options, BufferManager*);

private:
	int m_nRef;
	QMutex m_csRef;

	QStringList m_EventList;
	QMultiMap<QString,DeviceClientInfoItem> m_EventMap;

	IDeviceConnection *m_DeviceConnecton;
	IDeviceConnection *m_DeviceConnectonBubble;
	IDeviceConnection *m_DeviceConnectonHole;
	IDeviceConnection *m_DeviceConnectonTurn;
	QString m_sAddr;
	QString m_sEseeId;
	unsigned int m_uiPort;
	QVariantMap m_ports;
	QString m_sChannelName;

	bool bIsInitFlags;
	volatile bool bCloseingFlags;

	IDeviceClient::ConnectStatus m_CurStatus;
	IRemotePlayback *m_pRemotePlayback;
	QMap<int, WndPlay> m_groupMap;
	int m_nChannels;
	int m_nSpeedRate;
	QString m_sUserName;
	QString m_sPassWord;
	bool m_bGroupStop;
	
	RemoteBackup m_RemoteBackup;

private:
	int cbInit();
	void eventProcCall(QString sEvent,QVariantMap param);
	bool removeRepeatWnd(QWidget*);
signals:
	void TerminateConnectSignal();
	
};

#endif // DEVICECLIENT_H
