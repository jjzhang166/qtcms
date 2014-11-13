#ifndef DEVICESEARCH_H
#define DEVICESEARCH_H

#include <QThread>
#include "onvif.h"
#pragma comment(lib, "libonvifc.lib")

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
	void setHook(fOnvifSearchFoundHook hook, void *customCtx);
protected:
	void run();
private:
	bool m_bStop;
	bool m_bFlush;
	qint32 m_i32Interval;
	fOnvifSearchFoundHook m_hook;
	void *m_customCtx;
};

#endif // DEVICESEARCH_H
