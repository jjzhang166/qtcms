#pragma once
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
#include <QStringList>
#include <QVariantMap>
#include <QMultiMap>
#include <QtNetwork/QTcpSocket>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QDateTime>
typedef int (__cdecl *setAutoSycTimeEventCb)(QString eventName,QVariantMap tInfo,void *pUser);
typedef struct __tagSetAutoSycTimeProInfo{
	setAutoSycTimeEventCb proc;
	void *pUser;
}tagSetAutoSycTimeProInfo;

typedef struct __tagAutoSycTimeDeviceInfo{
	QString sAddr;
	quint16 uiPort;
	QString sUserName;
	QString sPassWord;
}tagAutoSycTimeDeviceInfo;
typedef enum __tagSetAutoSycTimeStepCode{
	SYC_CONNECT,
	SYC_GETVESIONINFO,
	SYS_SYCTIMETOOLDVERSION,
	SYS_GETLOCALTIME,
	SYS_SYCTIMETONEWVERSION,
	SYC_REVICEDATA,
	SYC_FAIL,
	SYC_SUCCESS,
	SYC_END,
}tagSetAutoSycTimeStepCode;
typedef enum __tagReceiveStepCode{
	RECV_VERSERINFO,//接收版本信息
	RECV_OLDVERSER,//接收1.1.3以前版本回复的信息
	RECV_LOCALSYSTEMTIME,//接收当地时区信息
	RECV_NEWVERSER,//接收1.1.3以后版本的回复信息
}tagReceiveStepCode;
class SetAutoSycTime:public QThread
{
	Q_OBJECT
public:
	SetAutoSycTime(void);
	~SetAutoSycTime(void);
public:
	void setAutoSycTime(QString sAddr,quint16 uiPort,QString sUserName,QString sPassWord);
	void registerEvent(QString eventName,int (__cdecl *proc)(QString ,QVariantMap,void*),void *pUser);

protected:
	void run();
private:
	void sleepEx(int time);
	void eventCallBack(QString sEventName,QVariantMap evMap);
private slots:
	void slCheckBlock();
private:
	int m_nSleepSwitch;
	tagAutoSycTimeDeviceInfo m_tDeviceInfo;
	bool m_bIsBlock;
	int m_nPosition;
	QMultiMap<QString,tagSetAutoSycTimeProInfo> m_tEventMap;
	QStringList m_sEventList;
	bool m_bStop;
	QTimer m_tCheckTimer;
};

