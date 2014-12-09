#pragma once
#include <QThread>
#include "IDeviceSearch.h"
#include "IDeviceNetModify.h"
#include <QQueue>
#include <QString>
#include <QMutex>
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
private:
	bool m_bStop;
	QList<IDeviceSearch *> m_tDeviceList;
	IDeviceNetModify *m_pDeviceNetModify;
	QQueue<QVariantMap> m_tWaitForTestDeviceItem;
	QVariantMap m_tDeviceItem;
	QMutex m_tDeviceItemMutex;
	QMutex m_tQueueLock;
	tagInterfaceInfo m_tInterfaceInfo;
	QVariantMap tCurrentDeviceItem;
};

