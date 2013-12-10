#pragma once
#include <QtCore/QThread>
class SleepQt :
	public QThread
{
public:
	SleepQt(QObject* parent=NULL);
	~SleepQt(void);

	void Sleep(unsigned long sec);
protected:
	void run();
private:
	unsigned long sectime;
};


extern "C"
{
	 void SleepQ(unsigned long dwMilliseconds);
	 unsigned long GetTickCountQ();
};



