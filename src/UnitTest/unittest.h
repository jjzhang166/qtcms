#ifndef UNITTEST_H
#define UNITTEST_H

#include <QObject>
#include <QString>
#include <QtTest>

class UnitTest : public QObject
{
	Q_OBJECT

public:
	UnitTest();
	~UnitTest();

private Q_SLOTS:
	void testCase1();
	
};

#endif // UNITTEST_H
