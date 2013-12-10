#include "SleepQt.h"
#include <QtCore/QTime>


SleepQt::SleepQt(QObject* parent)
	:QThread(parent)
{
	sectime=0;
}


SleepQt::~SleepQt(void)
{
}

void SleepQt::run()
{
	msleep(sectime);
}
void SleepQt::Sleep(unsigned long sec)
{
	sectime = sec;
	start();
	wait();
}



SleepQt sleepqt;
void SleepQ(unsigned long dwMilliseconds)
{
	sleepqt.Sleep(dwMilliseconds);
}

unsigned long GetTickCountQ(void)
{
	QTime ctime;
	ctime.start();
	return (ctime.hour()*3600+ctime.minute()*60+ctime.second())*1000+ctime.msec();
}
//unsigned long GetTickCountQ()
//{
//	static unsigned long TickCount=1000;
//	TickCount++;
//	return TickCount;
//}