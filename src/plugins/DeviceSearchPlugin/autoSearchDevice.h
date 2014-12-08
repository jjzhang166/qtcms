#pragma once
#include <QThread>
#include "IDeviceSearch.h"
#include "IDeviceNetModify.h"
#include <QQueue>
#include <QMutex>
typedef enum __tagAutoSearchDeviceStep{
	AutoSearchDeviceStep_Start,
	AutoSearchDeviceStep_NetworkConfig,
	AutoSearchDeviceStep_TestAndSet,
	AutoSearchDeviceStep_Default,
	AutoSearchDeviceStep_End,
}tagAutoSearchDeviceStep;
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
	bool getNetworkConfig();
private:
	bool m_bStop;
	QList<IDeviceSearch *> m_tDeviceList;
	IDeviceNetModify *m_pDeviceNetModify;
	QQueue<QVariantMap> m_tWaitForTestDeviceItem;
	QVariantMap m_tDeviceItem;
	QMutex m_tDeviceItemMutex;
	QMutex m_tQueueLock;
};

