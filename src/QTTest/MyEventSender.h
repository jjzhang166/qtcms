#pragma once
#include <QApplication>
class MyEventSender:public QApplication
{
public:
	MyEventSender(int argc ,char *argv[]);
	~MyEventSender(void);
public:
	bool notify(QObject *, QEvent *);
};

