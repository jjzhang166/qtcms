#ifndef DEVICE_TEST_H
#define DEVICE_TEST_H

#include <QObject>
#include <QString>
#include <QtTest>
class Device_Test:public QObject
{
	Q_OBJECT
public:
	Device_Test();
	~Device_Test();
private:
	int beforeTest();

private Q_SLOTS:
	void DeviceTestCase1();
	void DeviceTestCase2();
	void DeviceTestCase3();
	void DeviceTestCase4();
	void DeviceTestCase5();
	void DeviceTestCase6();
	void DeviceTestCase7();
	void DeviceTestCase8();
	void DeviceTestCase9();
	void DeviceTestCase10();
	void DeviceTestCase11();
	void DeviceTestCase12();
	void DeviceTestCase13();
	void DeviceTestCase14();
	void DeviceTestCase15();
	void DeviceTestCase16();
	void DeviceTestCase17();
	void DeviceTestCase18();
};
#endif


