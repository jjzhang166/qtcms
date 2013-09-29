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

private:
	void beforeUserTest();

private Q_SLOTS:
	void UserCase1();
	void UserCase2();
	void UserCase3();
	void UserCase4();
	void UserCase5();
	void UserCase6();
	void UserCase7();
	void UserCase8();
	void UserCase9();
	void UserCase10();
	void UserCase11();
	void UserCase12();
	void UserCase13();
	void UserCase14();
	void UserCase15();
	void UserCase16();
	void UserCase17();
	void UserCase18();
	void UserCase19();
	void UserCase20();
};

#endif // UNITTEST_H
