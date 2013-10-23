#ifndef AREA_TEST_H
#define AREA_TEST_H

#include <QObject>
#include <QString>
#include <QtTest>

class area_test : public QObject
{
	Q_OBJECT

public:
	area_test();
	~area_test();

private:
	void beforeAreaTest();
private Q_SLOTS:
	void AddArea_test();
	void RemoveAreaById_test();
	void RemoveAreaByName_test();
	void SetAreaName_test();
	void IsAreaNameExist_test();
	void IsAreaIdExist_test();
	void GetAreaCount_test();
	void GetAreaList_test();
	void GetSubArea_test();
	void GetAreaPid_test();
	void GetAreaName_test();
	void GetAreaInfo_int_test();
	void GetAreaInfo_qvariantmap_test();

};

#endif // AREA_TEST_H
