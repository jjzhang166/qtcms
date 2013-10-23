#include "area_test.h"
#include <guid.h>
#include <IAreaManager.h>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQuery>

#define  START_AREA_UNIT_TEST(ii) IAreaManager *ii=NULL;\
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IAreaManager,(void**)&ii);\
	QVERIFY2(NULL!=ii,"Create Area manager instance");
#define END_AREA_UNIT_TEST(ii) ii->Release();
area_test::area_test()
{

}

area_test::~area_test()
{

}
//测试用例：
//前置：重置数据库，数据库中创建area 表，删除所有的记录
//1 添加一个新区域|返回-1（失败），返回id（成功）|测试id是否存在
//2 根据id号删除区域|返回0（成功），返回错误号（失败）|测试区域下的所有设备，子区域，以及子区域内的所有设备是否删除|测试id是否存在
//3 根据sname删除区域|返回0（成功），返回错误号（失败）|测试sname区域是否存在|测试该区域下的所有设备，子区域，以及子区域内的所有设备是否删除
//4 设置id的区域名称|返回0（成功），返回错误号（失败）|读取数据库测试sname是否已经修改
//5 测试sname区域是否存在|返回true（成功），返回false（失败）
//6 获取区域的总数|测试总数为0|1
//7 获取区域列表|返回id号列表|测试列表结点数为0|1|2
//8 根据id号，获取id区域下一层子节点列表|返回子节点id号|测试子节点id是否正确
//9 根据id号，获取父区域id|测试父id是否正确
//10 根据id号，获取该区域的父id以及区域名|返回0（成功），返回错误号（失败）
//11 根据id号，获取该区域信息
void area_test::beforeAreaTest()
{
	QSqlDatabase db;
	if(QSqlDatabase::contains("AreaTest")){
		db=QSqlDatabase::database("AreaTest");
	}
	else{
		db=QSqlDatabase::addDatabase("QSQLITE","AreaTest");
	}
	QString sAppPath=QCoreApplication::applicationDirPath();
	QString sDatabasePath=sAppPath+"/system.db";
	db.setDatabaseName(sDatabasePath);
	db.open();

	QSqlQuery _query_1(db);
	_query_1.prepare("delete from area");
	_query_1.exec();
	_query_1.exec("update sqlite_sequence set seq=0 where name= 'area';"); 
	db.close();
}

void area_test::AddArea_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	//step 1: test nPid is not Exist
	int nPid = 1;
	QString TestArea_one="TestArea_one";
	int nret=IArea->AddArea(nPid,TestArea_one);
	QVERIFY2(IAreaManager::E_AREA_NOT_FOUND==nret,"AddArea:step 1: test nPid is not Exist");
	//step 2: test AddArea add succeed
	int nret_id=IArea->AddArea(nPid-1,TestArea_one);
	bool nret_exist=IArea->IsAreaIdExist(nret_id);
	QVERIFY2(true==nret_exist,"AddArea:step 2: test AddArea add succeed");
	QString get_sname=IArea->GetAreaName(nret_id);
	QVERIFY2(get_sname==TestArea_one,"AddArea:step 2: test AddArea add succeed");
	//step 3:test the nPid which is not equal to zero
	QString TestArea_two="TestArea_two";
	int nret_id_two=IArea->AddArea(nret_id,TestArea_two);
	nret_exist=IArea->IsAreaIdExist(nret_id_two);
	QVERIFY2(true==nret_exist,"AddArea:test the nPid which is not equal to zero");
	QString get_TestArea_two=IArea->GetAreaName(nret_id_two);
	QVERIFY2(get_TestArea_two==TestArea_two,"AddArea:test the nPid which is not equal to zero");
	//step 4: test sName have been exist
	nret=IArea->AddArea(nPid-1,TestArea_one);
	QVERIFY2(IAreaManager::E_AREA_NAME_EXISTS==nret,"AddArea:step 3: test sName have been exist");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::RemoveAreaById_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	QString AreaTest_two="AreaTest_two";
	QString AreaTest_three="AreaTest_three";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	int nret_id_1=IArea->AddArea(nret_id_0,AreaTest_two);
	int nret_id_2=IArea->AddArea(nret_id_1,AreaTest_three);
	//step 1:test remove succeed
	int nret_remove=IArea->RemoveAreaById(nret_id_0);
	QVERIFY2(0==nret_remove,"RemoveAreaById");
	bool nret_true=IArea->IsAreaIdExist(nret_id_0);
	QVERIFY2(false==nret_true,"RemoveAreaById");
	nret_true=IArea->IsAreaIdExist(nret_id_1);
	QVERIFY2(false==nret_true,"RemoveAreaById");
	nret_true=IArea->IsAreaIdExist(nret_id_2);
	QVERIFY2(false==nret_true,"RemoveAreaById");
	//step 2:test remove id which is not exist
	nret_remove=IArea->RemoveAreaById(nret_id_0);
	QVERIFY2(IAreaManager::E_AREA_NOT_FOUND==nret_remove,"RemoveAreaById");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::RemoveAreaByName_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	QString AreaTest_two="AreaTest_two";
	QString AreaTest_three="AreaTest_three";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	int nret_id_1=IArea->AddArea(nret_id_0,AreaTest_two);
	int nret_id_2=IArea->AddArea(nret_id_1,AreaTest_three);
	//step 1:test remove succeed
	int nret_remove=IArea->RemoveAreaByName(AreaTest_one);
	QVERIFY2(0==nret_remove,"RemoveAreaByName");
	bool nret_true=IArea->IsAreaIdExist(nret_id_0);
	QVERIFY2(false==nret_true,"RemoveAreaByName");
	nret_true=IArea->IsAreaIdExist(nret_id_1);
	QVERIFY2(false==nret_true,"RemoveAreaByName");
	nret_true=IArea->IsAreaIdExist(nret_id_2);
	QVERIFY2(false==nret_true,"RemoveAreaByName");
	//step 2:test remove the sname which is not exist
	nret_remove=IArea->RemoveAreaByName(AreaTest_one);
	QVERIFY2(IAreaManager::E_AREA_NAME_EXISTS==nret_remove,"RemoveAreaByName");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::SetAreaName_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	QString AreaTest_two="AreaTest_two";
	QString AreaTest_three="AreaTest_three";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	int nret_id_1=IArea->AddArea(nret_id_0,AreaTest_two);
	//step 1:test setAreaName succeed
	int nret_setname=IArea->SetAreaName(nret_id_1,AreaTest_three);
	QVERIFY2(0==nret_setname,"SetAreaName");
	QString get_name=IArea->GetAreaName(nret_id_1);
	QVERIFY2(get_name==AreaTest_three,"SetAreaName");
	//step 2:test set name whose id is not exist
	int nret_remove=IArea->RemoveAreaById(nret_id_1);
	QVERIFY2(0==nret_remove,"SetAreaName:RemoveAreaById");
	nret_setname=IArea->SetAreaName(nret_id_1,AreaTest_three);
	QVERIFY2(IAreaManager::E_AREA_NOT_FOUND==nret_setname,"SetAreaName");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::IsAreaNameExist_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	//step 1:test IsAreaNameExist succeed
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	QString AreaTest_two="AreaTest_two";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	bool nret_true=IArea->IsAreaNameExist(AreaTest_one);
	QVERIFY2(true==nret_true,"IsAreaNameExist");
	//step 2:test the sname which is not exist
	nret_true=IArea->IsAreaNameExist(AreaTest_two);
	QVERIFY2(false==nret_true,"IsAreaNameExist");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::IsAreaIdExist_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	//step 1:test IsAreaName succeed
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	bool nret_true=IArea->IsAreaIdExist(nret_id_0);
	QVERIFY2(true==nret_true,"IsAreaIdExist");
	//step 2:test the id which is not exist
	int nret_remove=IArea->RemoveAreaById(nret_id_0);
	QVERIFY2(0==nret_remove,"IsAreaIdExist:RemoveAreaById");
	nret_true=IArea->IsAreaIdExist(nret_id_0);
	QVERIFY2(false==nret_true,"IsAreaIdExist");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::GetAreaCount_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	//step 2:test the number of area is zero
	int nret_areacount=IArea->GetAreaCount();
	QVERIFY2(0==nret_areacount,"GetAreaCount");
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	QString AreaTest_two="AreaTest_two";
	QString AreaTest_three="AreaTest_three";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	int nret_id_1=IArea->AddArea(nret_id_0,AreaTest_two);
	int nret_id_2=IArea->AddArea(nret_id_1,AreaTest_three);
	//step 1:test the number of area is three
	nret_areacount=IArea->GetAreaCount();
	QVERIFY2(3==nret_areacount,"GetAreaCount");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::GetAreaList_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	//step 1:test the number of area is zero
	QStringList id_list;
	id_list.empty();
	id_list=IArea->GetAreaList();
	QVERIFY2(true==id_list.isEmpty(),"GetAreaList");
	//step 2:test the number of area is three
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	QString AreaTest_two="AreaTest_two";
	QString AreaTest_three="AreaTest_three";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	int nret_id_1=IArea->AddArea(nret_id_0,AreaTest_two);
	int nret_id_2=IArea->AddArea(nret_id_1,AreaTest_three);
	id_list=IArea->GetAreaList();
	QVERIFY2(3==id_list.size(),"GetAreaList");
	for(int i=0;i<id_list.size();i++){
		int nret_id=id_list.at(i).toInt();
		QVERIFY2(nret_id_0==nret_id||nret_id_1==nret_id||nret_id==nret_id_2,"GetAreaList");
	}
	END_AREA_UNIT_TEST(IArea);
}

void area_test::GetSubArea_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	//step 1:test the number of SubArea is zero
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	QStringList id_list;
	id_list.empty();
	id_list=IArea->GetSubArea(nret_id_0);
	QVERIFY2(true==id_list.isEmpty(),"GetSubArea");
	//step 2:test the number of SubArea is two
	QString AreaTest_two="AreaTest_two";
	QString AreaTest_three="AreaTest_three";
	int nret_id_1=IArea->AddArea(nret_id_0,AreaTest_two);
	int nret_id_2=IArea->AddArea(nret_id_1,AreaTest_three);
	id_list=IArea->GetSubArea(nret_id_0);
	QVERIFY2(2==id_list.size(),"GetSubArea");
	for(int i=0;i<id_list.size();i++){
		int nret_id=id_list.at(i).toInt();
		QVERIFY2(nret_id_1==nret_id||nret_id==nret_id_2,"GetSubArea");
	}
	//step 3:test the id which is not exist
	int nret_remove=IArea->RemoveAreaById(nret_id_0);
	id_list.empty();
	id_list=IArea->GetSubArea(nret_id_0);
	QVERIFY2(true==id_list.isEmpty(),"GetSubArea");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::GetAreaPid_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	//step 1:test the id which is exist
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	int nret_getpid=IArea->GetAreaPid(nret_id_0);
	QVERIFY2(0==nret_getpid,"GetAreaPid");

	QString AreaTest_two="AreaTest_two";
	int nret_id_1=IArea->AddArea(nret_id_0,AreaTest_two);
	nret_getpid=IArea->GetAreaPid(nret_id_1);
	QVERIFY2(nret_getpid==nret_id_0,"AreaTest_two");
	//step 2:test the id which is not exist
	int nret_remove=IArea->RemoveAreaById(nret_id_1);
	nret_getpid=IArea->GetAreaPid(nret_id_1);
	QVERIFY2(1==nret_getpid,"GetAreaPid");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::GetAreaName_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	//step 1:test the id which is exist
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	QString get_name;
	get_name.clear();
	get_name=IArea->GetAreaName(nret_id_0);
	QVERIFY2(get_name==AreaTest_one,"GetAreaName");
	//step 2:test the id which is not exist
	get_name.clear();
	int nret_remove=IArea->RemoveAreaById(nret_id_0);
	get_name=IArea->GetAreaName(nret_id_0);
	QVERIFY2(true==get_name.isEmpty(),"GetAreaName");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::GetAreaInfo_int_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	int nPid=0;
	int nret_Pid=-1;
	QString nret_sname;
	nret_sname.clear();
	QString AreaTest_one="AreaTest_one";
	int nret_id_0=IArea->AddArea(nPid,AreaTest_one);
	//step 1:test the nId which is exist
	int nret_GetAreaInfo=IArea->GetAreaInfo(nret_id_0,nret_Pid,nret_sname);
	QVERIFY2(0==nret_GetAreaInfo,"GetAreaInfo");
	QVERIFY2(0==nret_Pid,"GetAreaInfo");
	QVERIFY2(AreaTest_one==nret_sname,"GetAreaInfo");
	//step 2:test the nId which is not exist
	int nret_remove=IArea->RemoveAreaById(nret_id_0);
	nret_GetAreaInfo=IArea->GetAreaInfo(nret_id_0,nret_Pid,nret_sname);
	QVERIFY2(IAreaManager::E_AREA_NOT_FOUND==nret_GetAreaInfo,"GetAreaInfo");
	END_AREA_UNIT_TEST(IArea);
}

void area_test::GetAreaInfo_qvariantmap_test()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(IArea);
	QVariantMap nRet_Map;
	nRet_Map.clear();
	//step 1:test the nId which is exist
	int nPid=0;
	QString AreaTest_one="AreaTest_one";
	int nRet_id_0=IArea->AddArea(nPid,AreaTest_one);
	nRet_Map=IArea->GetAreaInfo(nRet_id_0);
	QVERIFY2(true==nRet_Map.contains(QString("%1").arg(nPid)),"GetAreaInfo");
	QVERIFY2(AreaTest_one==nRet_Map.value(QString("%1").arg(nPid)),"GetAreaInfo");
	//step 1:test the nId which is not exist
	int nRet_Rmove=IArea->RemoveAreaById(nRet_id_0);
	nRet_Map.clear();
	nRet_Map=IArea->GetAreaInfo(nRet_id_0);
	QVERIFY2(true==nRet_Map.isEmpty(),"GetAreaInfo");
	END_AREA_UNIT_TEST(IArea);
}