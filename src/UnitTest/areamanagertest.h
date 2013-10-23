#ifndef AREAMANAGERTEST_H
#define AREAMANAGERTEST_H

#include <QObject>
#include <QtTest/QTest>

class AreaManagerTest : public QObject
{
	Q_OBJECT

public:
	AreaManagerTest();
	~AreaManagerTest();

private Q_SLOTS:
	void TestCase1();
	void TestCase2();
};

#endif // AREAMANAGERTEST_H
