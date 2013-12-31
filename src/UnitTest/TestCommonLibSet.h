#pragma once
#include <QObject>
#include <QString>
#include <QtTest>

class TestCommonLibSet : public QObject
{
	Q_OBJECT

private:
	void beforeAreaTest();


public:
	TestCommonLibSet(void);
	~TestCommonLibSet(void);
private Q_SLOTS:
	void test1();
	void test2();
	void test3();
	void test4();
	void test5();
	void test6();
	void test7();
	void test8();
	void test9();
	void test10();
	void test11();
	void test12();
};

