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
#include <IDeviceAuth.h>
#include "RemoteBackup.h"
#include "IPTZControl.h"
#include "IProtocolPTZ.h"
#include "remotePlayBack.h"


int cbXStateChange(QString evName,QVariantMap evMap,void*pUser);
int cbXFoundFile(QString evName,QVariantMap evMap,void*pUser);
int cbXRecFileSearchFail(QString evName,QVariantMap evMap,void*pUser);
int cbXRecFileSearchFinished(QString evName,QVariantMap evMap,void*pUser);
int cbXRecordStream(QString evName,QVariantMap evMap,void*pUser);
int cbXLiveStream(QString evName,QVariantMap evMap,void*pUser);
int cbXSocketError(QString evName,QVariantMap evMap,void*pUser);
int cbXConnectRefuse(QString evName,QVariantMap evMap,void*pUser);
int cbXAuthority(QString evName,QVariantMap evMap,void*pUser);
class  DeviceClient:public QThread,
	public IDeviceClient,
	public IEventRegister,
	public IDeviceSearchRecord,
	public IDeviceGroupRemotePlayback,
	public IPTZControl,
	public IRemoteBackup,
	public IDeviceAuth
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
	virtual int setDeviceHost(const QString & sAddr);
	virtual int setDevicePorts(unsigned int ports);
	virtual int setDeviceId(const QString & isee);
	virtual int connectToDevice();
	virtual int checkUser(const QString & sUsername,const QString &sPassword);
	virtual int setChannelName(const QString & sChannelName);
	virtual int liveStreamRequire(int nChannel,int nStream,bool bOpen);
	virtual int closeAll();
	virtual QString getVendor();
	virtual int getConnectStatus();

	int ConnectStatusProc(QVariantMap evMap);

	// IDeviceAuth
	virtual void setDeviceAuth( const QString & sUsername, const QString & sPassword );

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
	virtual int GroupSetVolume(unsigned int uiPersent, QWidget* pWnd);
	virtual int GroupSpeedFast();
	virtual int GroupSpeedSlow();
	virtual int GroupSpeedNormal();

	int recordFrame(QVariantMap &evMap);
	int cbFoundFile(QVariantMap &evmap);
	int cbRecFileSearchFinished(QVariantMap &evmap);
	int cbRecFileSearchFail(QVariantMap &evmap);
	int cbLiveStream(QVariantMap &evmap);
	int cbSocketError(QVariantMap &evmap);
	int cbConnectRefuse(QVariantMap &evMap);
	int cbAuthority(QVariantMap &evMap);
	//IRemoteBackup
	virtual int startBackup(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,
		int nChannel,
		int nTypes,
		const QString &sDeviceName,
		const QDateTime & startTime,
		const QDateTime & endTime,
		const QString & sbkpath);
	virtual int stopBackup();
	virtual float getProgress();

	//ptz control
	virtual int ControlPTZUp(const int &nChl, const int &nSpeed);
	virtual int ControlPTZDown(const int &nChl, const int &nSpeed);
	virtual int ControlPTZLeft(const int &nChl, const int &nSpeed);
	virtual int ControlPTZRight(const int &nChl, const int &nSpeed);
	virtual int ControlPTZIrisOpen(const int &nChl, const int &nSpeed);
	virtual int ControlPTZIrisClose(const int &nChl, const int &nSpeed);
	virtual int ControlPTZFocusFar(const int &nChl, const int &nSpeed);
	virtual int ControlPTZFocusNear(const int &nChl, const int &nSpeed);
	virtual int ControlPTZZoomIn(const int &nChl, const int &nSpeed);
	virtual int ControlPTZZoomOut(const int &nChl, const int &nSpeed);
	virtual int ControlPTZAuto(const int &nChl, bool bOpend);
	virtual int ControlPTZStop(const int &nChl, const int &nCmd);

private slots:
	void action(QString options, BufferManager*);
	void bufferStatus(int persent, BufferManager* pBuff);
private:
	int m_nRef;
	QMutex m_csRef;
	QMutex m_csCloseAll;

	QStringList m_EventList;
	QMultiMap<QString,DeviceClientInfoItem> m_EventMap;

	QMultiMap<QString,DeviceClientInfoItem> m_EventMapToPro;
	IDeviceConnection *m_DeviceConnecton;
	IDeviceConnection *m_DeviceConnectonBubble;
	IDeviceConnection *m_DeviceConnectonHole;
	IDeviceConnection *m_DeviceConnectonTurn;
	QString m_sAddr;
	QString m_sEseeId;
	unsigned int m_uiPort;
	QVariantMap m_ports;
	QString m_sChannelName;

    bool m_bIsInitFlags;
    volatile bool m_bCloseingFlags;

	IDeviceClient::ConnectStatus m_CurStatus;
	IRemotePlayback *m_pRemotePlayback;
	QMap<int, WndPlay> m_groupMap;
	uint m_nChannels;
	int m_nSpeedRate;
	int m_nStartTimeSeconds;
	QString m_sUserName;
	QString m_sPassWord;
	bool m_bGroupStop;
	
	RemoteBackup m_RemoteBackup;
	int m_channelWithAudio;

	IProtocolPTZ *m_pProtocolPTZ;

	remotePlayBack m_remotePlayback;
private:
	int cbInit();
	void eventProcCall(QString sEvent,QVariantMap param);
	bool removeRepeatWnd(QWidget*);
	int connectToDevice(const QString &sAddr,unsigned int uiPort,const QString &sEseeId);
signals:
	void TerminateConnectSignal();

};

#endif // DEVICECLIENT_H
