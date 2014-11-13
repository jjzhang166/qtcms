#ifndef DEVICESEARCH_H
#define DEVICESEARCH_H

#include <QThread>
#include "onvif.h"
#include "OnvifProtocol_global.h"
#pragma comment(lib, "libonvifc.lib")

void cbSearchHook(const char *bind_host, unsigned char *ip,unsigned short port, char *name, char *location, char *firmware, void* customCtx);

class DeviceSearch : public QThread
{
	Q_OBJECT

public:
	DeviceSearch();
	~DeviceSearch();
	int Start();
	int Stop();
	int Flush();
	int setInterval(int nInterval);
	void setHook(QString sEvent, tagOnvifProInfo proInfo);
	void analyzeDeviceInfo(unsigned char *ip,unsigned short port, char *name, char *location, char *firmware);
protected:
	void run();
private:
	bool m_bStop;
	bool m_bFlush;
	qint32 m_i32Interval;
	QString m_sEvent;
	tagOnvifProInfo m_proInfo;
};

#endif // DEVICESEARCH_H
