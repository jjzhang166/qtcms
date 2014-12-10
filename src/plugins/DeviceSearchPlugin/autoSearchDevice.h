#pragma once
#include <QThread>
#include "IDeviceSearch.h"
#include "IDeviceNetModify.h"
#include <QQueue>
#include <QString>
#include <QMutex>
typedef int (__cdecl *autoSearchDeviceEventCb)(QString sEventName,QVariantMap tInfo,void *pUser);
typedef struct __tagautoSearchDeviceProInfo{
	autoSearchDeviceEventCb proc;
	void *pUser;
}tagautoSearchDeviceProInfo;
typedef enum __tagAutoSearchDeviceStep{
	AutoSearchDeviceStep_Start,
	AutoSearchDeviceStep_NetworkConfig,
	AutoSearchDeviceStep_TestAndSet,
	AutoSearchDeviceStep_Default,
	AutoSearchDeviceStep_End,
}tagAutoSearchDeviceStep;
typedef struct __tagInterfaceInfo{
	QString sIp;
	QString sMask;
	QString sGateway;
	unsigned int uiLastTestIp;
}tagInterfaceInfo;

class autoSearchDevice:public QThread
{
	Q_OBJECT
public:
	autoSearchDevice(void);
	~autoSearchDevice(void);
public:
	void startSearch();
	void stopSearch();
	void cbSearchDevice(QVariantMap item);

	int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);
protected:
	void run();
private:
	void startVendorSearch();
	bool getNetworkConfig();//»ñÈ¡Íø¹Ø£¬ÑÚÂë£¬ip,mac
	void checkAndSetConfig();
	bool isIpConflict();
	bool isJuanIpc();
	bool getUseableIp();
	bool setIpConfig();
	void eventProcCall(QString sEvent,QVariantMap tInfo);
private:
	bool m_bStop;
	QList<IDeviceSearch *> m_tDeviceList;
	IDeviceNetModify *m_pDeviceNetModify;
	QQueue<QVariantMap> m_tWaitForTestDeviceItem;
	QVariantMap m_tDeviceItem;
	QMutex m_tDeviceItemMutex;
	QMutex m_tQueueLock;
	tagInterfaceInfo m_tInterfaceInfo;
	QVariantMap m_tCurrentDeviceItem;
	QList<QString> m_tHadBeenUseIp;
	QStringList m_sEventList;
	QMultiMap<QString,tagautoSearchDeviceProInfo> m_tEventMap;
};

