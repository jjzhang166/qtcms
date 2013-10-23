#ifndef CHANNNEL_TEST_H
#define CHANNNEL_TEST_H

#include <QObject>
#include <QString>
#include <QtTest>
class Channel_Test:public QObject
{
	Q_OBJECT
public:
	Channel_Test();
	~Channel_Test();
private:
	void beforeChlTest();
private Q_SLOTS:
	void ChlCase1();
	void ChlCase2();
	void ChlCase3();
	void ChlCase4();
	void ChlCase5();
	void ChlCase6();
	void ChlCase7();
	void ChlCase8();
	void ChlCase9();
};

#endif


