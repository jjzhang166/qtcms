#pragma once

#include <QThread>
#include <QDebug>
#include <QVariantMap>
#include <QQueue>
#include <QStringList>
#include <QMutex>
typedef int (__cdecl *WINIPCSearchCB)(QString name, QVariantMap info, void* pUser);
typedef struct __tagWINEventCB{
	WINIPCSearchCB evCBName;
	void *pUser;
}WINEventCBInfo,*lpWINEventCBInfo;
class WinIpcSock:public QThread
{
public:
	WinIpcSock(void);
	~WinIpcSock(void);
public:
	int Start() ;
	int Stop() ;
	int Flush() ;
	int setInterval(int nInterval) ;
	int registerEvent( QString eventName,WINIPCSearchCB eventCB,void *pUser );
	int SetNetworkInfo(const QString &sDeviceID,
		const QString &sAddress,
		const QString &sMask,
		const QString &sGateway,
		const QString &sMac,
		const QString &sPort,
		const QString &sUsername,
		const QString &sPassword);
protected:
	void run();
private:
	void eventProcCall(QString sEvent,QVariantMap param);
private:
	int m_nTimeInterval;
	volatile bool m_bIsStop;
	volatile bool m_bIsFlush;
	QMultiMap<QString,WINEventCBInfo>m_mEventCBMap;  
	QQueue<QVariantMap> m_SetupStatusParm;
	QStringList m_sEventList;
	QMutex m_SetupStatusParmMutex;
};

