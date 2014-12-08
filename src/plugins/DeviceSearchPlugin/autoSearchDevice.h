#pragma once
#include <QThread>
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
};

