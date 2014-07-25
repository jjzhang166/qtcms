#pragma once
#include <QThread>
#include <IEventRegister.h>
#include "IpcDeviceClient_global.h"
#include <QMutex>
#include "QDebug"
#include <IDeviceClient.h>
#include <IRemotePreview.h>
#include <ISwitchStream.h>
#include <IDeviceConnection.h>
#include <QtNetwork/QTcpSocket>
#include "IAutoSycTime.h"
#include "IPTZControl.h"
#include "IProtocolPTZ.h"
#include "SetAutoSycTime.h"
//主码流的回调函数
int cbXLiveStream_Primary(QString evName,QVariantMap evMap,void*pUser);//预览码流
int cbXSocketError_Primary(QString evName,QVariantMap evMap,void*pUser);//连接错误
int cbXStateChange_Primary(QString evName,QVariantMap evMap,void*pUser);//状态改变
int cbXConnectRefuse_Primary(QString evName,QVariantMap evMap,void*pUser);
//次码流的回调函数
int cbXLiveStream_Minor(QString evName,QVariantMap evMap,void*pUser);
int cbXSocketError_Minor(QString evName,QVariantMap evMap,void*pUser);
int cbXStateChange_Minor(QString evName,QVariantMap evMap,void*pUser);
int cbXConnectRefuse_Minor(QString evName,QVariantMap evMap,void*pUser);
class IpcDeviceClient:public QThread,
	public IDeviceClient,
	public IEventRegister,
	public IAutoSycTime,
	public IPTZControl,
	public ISwitchStream
{
	Q_OBJECT
public:
	IpcDeviceClient(void);
	~IpcDeviceClient(void);

	virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	//IEventRegister
	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);
	//IDeviceClient
	virtual int setDeviceHost(const QString & sAddr);
	virtual int setDeviceId(const QString & isee);
	virtual int setDevicePorts(unsigned int ports);
	virtual int connectToDevice();
	virtual int checkUser(const QString & sUsername,const QString &sPassword);
	virtual int setChannelName(const QString & sChannelName);
	virtual int liveStreamRequire(int nChannel,int nStream,bool bOpen);
	virtual int closeAll();
	virtual QString getVendor();
	virtual int getConnectStatus();

	//ISwitchStream
	virtual int SwitchStream(int StreamNum);

	//IAutoSycTime
	virtual int setAutoSycTime(bool bEnabled);

	//IPTZControl
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


	public:
	void eventProcCall(QString sEvent,QVariantMap param);

	int cbLiveStream(QVariantMap &evmap);
	int cbSocketError(QVariantMap &evmap);
	int cbConnectStatusProc(QVariantMap evMap);
	int cbConnectRefuse(QVariantMap evMap);

	bool TryToConnectProtocol(CLSID clsid);
	void DeInitProtocl();

	int RegisterProc(IEventRegister *m_RegisterProc,int m_Stream);

private:
// 	void SyncTime();
	//int sndGetVesionInfo();
	//int sndSyncTimeForPreVersion();
	//int sndGetLocalSystemTime();
	//int sndSyncTimeCmd();
	int connectToDevice(const QString &sAddr,unsigned int uiPort,const QString &sEseeId);
//signals:
//	void sigSyncTime();
//private slots:
//	void Reveived();
//	void SyncTime();

private:
	int m_nRef;
	QMutex m_csRef;
	QMutex m_csDeInit;
	QMutex m_cscbLiveStream;
	

	QMutex m_csRefDelete;
	volatile int m_CurStream;
	volatile int m_IfSwithStream;
	IDeviceClient::ConnectStatus m_CurStatus;

	QStringList m_EventList;
	QMultiMap<QString,IpcDeviceClientInfoItem> m_EventMap;
	QMultiMap<QString,IpcDeviceClientToProcInfoItem> m_EventMapToProc;
	QMultiMap<int,SingleConnect> m_DeviceClentMap;
	QMultiMap<int ,CurStatusInfo> m_StreamCurStatus;
	//设备信息
	QString m_sAddr;
	QString m_sEseeId;
	unsigned int m_uiPort;
	DeviceInfo m_DeviceInfo;
	volatile bool bCloseingFlags;
	volatile bool bHadCallCloseAll;

	bool m_bIsSycTime;
	//QTcpSocket *m_tcpSocket;
	//QByteArray m_timeZone;
	//QByteArray m_softwareVersion;
	int m_steps;

	IProtocolPTZ *m_pProtocolPTZ;
	SetAutoSycTime m_tSetAutoSycTime;
};

