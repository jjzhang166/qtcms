#ifndef _HICHIPTEST_HEAD_FILE_H_
#define _HICHIPTEST_HEAD_FILE_H_

#include <QObject>
#include <QString>
#include <QtTest>

class HiChipUnitTest : public QObject
{
	Q_OBJECT

public:
	HiChipUnitTest();
	~HiChipUnitTest();

private Q_SLOTS:
	void DeviceTestCase1();
	void DeviceTestCase2();
	void DeviceTestCase3();
	void DeviceTestCase4();
	void DeviceTestCase5();
	void DeviceTestCase6();
};


#endif