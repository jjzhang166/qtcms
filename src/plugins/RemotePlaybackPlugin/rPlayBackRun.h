#pragma once
#include <QThread>
#include <QVariantMap>
#include <QQueue>
#include <QDebug>
#include <QTimer>
#include <QApplication>
#include <QFile>
#include <QtXml/QtXml>
#include "IDeviceRemotePlayback.h"
#include <guid.h>
#include <IEventRegister.h>
#include <QMutex>
#include <QMultiMap>
#include <IDeviceSearchRecord.h>
#include <IDeviceClient.h>
#include <QMultiMap>
typedef int (__cdecl *runEventCallBack)(QString,QVariantMap,void *);
typedef struct __tagEventCB{
	runEventCallBack evCBName;
	void*         pUser;
}EventCBInfo, *lpEventCBInfo;
typedef struct _tagRunDevInfo{
	QString m_sAddress;
	unsigned int m_uiPort;
	QString m_sEseeId;
	unsigned int m_uiChannelId;
	int m_uiChannelIdInDataBase;
	unsigned int m_uiStreamId;
	QString m_sUsername;
	QString m_sPassword;
	QString m_sCameraname;
	QString m_sVendor;
}RunDevInfo;

class rPlayBackRun:public QThread
{
	Q_OBJECT
public:
	rPlayBackRun(void);
	~rPlayBackRun(void);

	// 设置设备连接信息
	int setDeviceHostInfo(const QString & sAddress,unsigned int uiPort,const QString &eseeID);
	// 设置设备厂商信息
	int setDeviceVendor(const QString & vendor);
	// 设置回放同步组
	int AddChannelIntoPlayGroup(uint uiWndId,int uiChannelId);
	int GetWndInfo(int uiWndId );
	void setUserVerifyInfo(const QString & sUsername,const QString & sPassword);

	int startSearchRecFile(int nChannel,int nTypes,const QString & startTime,const QString & endTime);
	QString GetNowPlayedTime();

	int	GroupPlay(int nTypes,const QString & start,const QString & end);
	int	GroupPause();
	int	GroupContinue();
	int GroupStop();
	int AudioEnabled(bool bEnable);
	int SetVolume(const unsigned int &uiPersent);
	int GroupSpeedFast() ;
	int GroupSpeedSlow();
	int GroupSpeedNormal();
	void	cbRegisterEvent(QString eventName,runEventCallBack eventCB,void *pUser);

protected:
	void run();
private:
	QQueue<QVariantMap> m_FunctionMap;
	bool m_ForbinFreOpera;
	QTimer *m_timer;
	RunDevInfo m_RunDevInfo;
	volatile bool m_brunning;
	QMutex m_FunctionMapMutex;
	QVariantMap m_FuncInfo;
	QVariantMap m_FuncIndex;
	QMultiMap<QString,   EventCBInfo>m_mEventCBMap;
	volatile bool m_bStop;
private:
	int __AddChannelIntoPlayGroup(uint uiWndId,int uiChannelId);
	int __GetWndInfo(int uiWndId );
	bool __startSearchRecFile();
	QString __GetNowPlayedTime();

	int   __GroupPlay(int nTypes,const QString & start,const QString & end);
	int   __GroupPause();
	int   __GroupContinue();
	int   __GroupStop();
	int   __AudioEnabled(bool bEnable);
	int	  __SetVolume(const unsigned int &uiPersent);
	int   __GroupSpeedFast() ;
	int   __GroupSpeedSlow();
	int   __GroupSpeedNormal();
	int   __InsertFunction(QVariantMap item);
	bool  __BuildDev(void **ppv);
	bool  __InitSearchCb(IDeviceGroupRemotePlayback *pSearch);
	void  __SwitchFunction();
	void __eventProcCall(QString sEventName,QVariantMap parm);
public slots:
	void  __ResetForbinFreOpera();
};

