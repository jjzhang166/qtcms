#pragma once
#include <QThread>
typedef enum __tagAutoSearchDeviceStep{
	AutoSearchDeviceStep_Start,
	AutoSearchDeviceStep_NetworkConfig,
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
protected:
	void run();
private:
	void startVendorSearch();
	bool getNetworkConfig();
private:
	bool m_bStop;
};

