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
//主码流的回调函数
int cbLiveStreamFrompPotocol_Primary(QString evName,QVariantMap evMap,void*pUser);//预览码流
int cbSocketErrorFrompPotocol_Primary(QString evName,QVariantMap evMap,void*pUser);//连接错误
int cbStateChangeFrompPotocol_Primary(QString evName,QVariantMap evMap,void*pUser);//状态改变
//次码流的回调函数
int cbLiveStreamFrompPotocol_Minor(QString evName,QVariantMap evMap,void*pUser);
int cbSocketErrorFrompPotocol_Minor(QString evName,QVariantMap evMap,void*pUser);
int cbStateChangeFrompPotocol_Minor(QString evName,QVariantMap evMap,void*pUser);

class IpcDeviceClient:public QThread,
	public IDeviceClient,
	public IEventRegister
{
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
	virtual int connectToDevice(const QString &sAddr,unsigned int uiPort,const QString &sEseeId);
	virtual int checkUser(const QString & sUsername,const QString &sPassword);
	virtual int setChannelName(const QString & sChannelName);
	virtual int liveStreamRequire(int nChannel,int nStream,bool bOpen);
	virtual int closeAll();
	virtual QString getVendor();
	virtual int getConnectStatus();

	//ISwitchStream
	virtual int SwitchStream(int StreamNum);

	public:
	void eventProcCall(QString sEvent,QVariantMap param);

	int cbLiveStream(QVariantMap &evmap);
	int cbSocketError(QVariantMap &evmap);
	int cbConnectStatusProc(QVariantMap evMap);

	bool TryToConnectProtocol(CLSID clsid);

	int RegisterProc(IEventRegister *m_RegisterProc,int m_Stream);
private:
	int m_nRef;
	QMutex m_csRef;

	int m_CurStream;
	IDeviceClient::ConnectStatus m_CurStatus;

	QStringList m_EventList;
	QMultiMap<QString,IpcDeviceClientInfoItem> m_EventMap;
	QMultiMap<QString,IpcDeviceClientToProcInfoItem> m_EventMapToProc;
	QMultiMap<int,SingleConnect> m_DeviceClentMap;
	QMultiMap<int ,CurStatusInfo> m_StreamCurStatus;
	//设备信息
	DeviceInfo m_DeviceInfo;
	volatile bool bCloseingFlags;
};

