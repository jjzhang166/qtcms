#include "DeviceSearch.h"
#include <QTime>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>
void cbSearchHook(const char *bind_host, unsigned char *ip,unsigned short port, char *name, char *location, char *firmware);

DeviceSearch::DeviceSearch()
	: QThread(),
	m_bStop(true),
	m_bFlush(false),
	m_i32Interval(10)
{

}

DeviceSearch::~DeviceSearch()
{
	qDebug()<<__FUNCTION__<<__LINE__<<"delete:out"<<this<<this->thread()->currentThread()->currentThreadId();;
}

int DeviceSearch::Start()
{
	qDebug()<<__FUNCTION__<<__LINE__<<"start:"<<this<<this->thread()->currentThread()->currentThreadId();;
	if (!isRunning())
	{
		start();
	}
	m_bStop = false;
	return 0;
}

int DeviceSearch::Stop()
{
	m_bStop = true;
	while (isRunning())
	{
		QEventLoop eventloop;
		QTimer::singleShot(10, &eventloop, SLOT(quit()));
		eventloop.exec();
	}
	return 0;
}

int DeviceSearch::Flush()
{
	m_bFlush = true;
	return 0;
}

int DeviceSearch::setInterval( int nInterval )
{
	if (nInterval > 0 && nInterval < 100)
	{
		m_i32Interval = nInterval;
		return 0;
	}
	else
		return 1;
}

void DeviceSearch::run()
{
	ONVIF_CLIENT_init(1, 1, 1, false, 2);
	ONVIF_search(ONVIF_DEV_ALL, false, 2, m_hook, NULL, m_customCtx);

	QTime timer;
	timer.start();
	while (!m_bStop)
	{
		if (timer.elapsed() > m_i32Interval*1000 || m_bFlush)
		{
			timer.start();
			ONVIF_search(ONVIF_DEV_ALL, false, 2, m_hook, NULL, m_customCtx);
			m_bFlush = false;
		}
		msleep(1);
	}

	ONVIF_CLIENT_deinit();
}

void DeviceSearch::setHook( fOnvifSearchFoundHook hook, void *customCtx )
{
	m_hook = hook;
	m_customCtx = customCtx;
}
