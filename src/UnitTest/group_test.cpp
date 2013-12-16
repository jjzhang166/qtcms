#include "group_test.h"
#include <guid.h>
#include <IGroupManager.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QtSql>

#define  START_GROUP_UNIT_TEST(ii) IGroupManager *ii=NULL;\
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IGroupManager,(void**)&ii);\
	QVERIFY2(NULL!=ii,"Create group manager instance");
#define END_GROUP_UNIT_TEST(ii) ii->Release();
group_test::group_test()
{

}

group_test::~group_test()
{

}
//测试用例：
//前置：重置数据库，数据库中创建dev_group表
//1 添加一个新用户|返回-1（失败），返回id（成功）
//2 判断一个组id是否存在|返回false（失败），返回true（成功）
//3 删除组|删除一次|删除两次|删除不存在的id
//4 修改一个id的用户名|返回E_GROUP_NOT_FOUND（id不存在），返回0（修改成功）
//5 获取组的个数|返回组的个数，测试组个数为1
//6 获取组id的列表|空列表|id数为2的列表
//7 获取id的组名|空id|成功返回0|失败返回错误号
//8 获取id的组名|从函数返回值中返回|空id|成功返回0|失败返回错误号
void group_test::beforeGroupTest()
{
	QSqlDatabase db;
	if(QSqlDatabase::contains("grouptest")){
		db=QSqlDatabase::database("grouptest");
	}
	else{
		db=QSqlDatabase::addDatabase("QSQLITE","grouptest");
	}
	QString sAppPath = QCoreApplication::applicationDirPath();
	QString sDatabasePath = sAppPath + "/system.db";
	db.setDatabaseName(sDatabasePath);
	db.open();

	QSqlQuery _query_1(db);
	_query_1.prepare("delete from dev_group");
	_query_1.exec();
	_query_1.exec("update sqlite_sequence set seq=0 where name= 'dev_group';"); 
	db.close();
}

int  group_test::beforeChannelTest()
{
	QSqlDatabase db;
	if(QSqlDatabase::contains("grouptest")){
		db=QSqlDatabase::database("grouptest");
	}
	else{
		db=QSqlDatabase::addDatabase("QSQLITE","grouptest");
	}
	QString sAppPath = QCoreApplication::applicationDirPath();
	QString sDatabasePath = sAppPath + "/system.db";
	db.setDatabaseName(sDatabasePath);
	db.open();

	QSqlQuery _query_1(db);
	_query_1.prepare("delete from chl");
	_query_1.exec();
	_query_1.exec("update sqlite_sequence set seq=0 where name= 'chl';"); 
	int id=0;
	int dev_id=1;
	int channel_number=8;
	QString name="channel_test_one";
	QString stream_id="ovif";
	_query_1.prepare("insert into chl(dev_id,channel_number,name,stream_id) values(:dev_id,:channel_number,:name,:stream_id)");
	_query_1.bindValue(":dev_id",dev_id);
	_query_1.bindValue(":channel_number",channel_number);
	_query_1.bindValue(":name",name);
	_query_1.bindValue(":stream_id",stream_id);
	_query_1.exec();

	QString Command=QString("select * from chl");
	_query_1.exec(Command);
	if(_query_1.isActive()){
		int id_Index=_query_1.record().indexOf("id");
		if(_query_1.first()){
			id=_query_1.value(id_Index).toInt();
		}
		db.close();
		return id;
	}
	db.close();
	return id;
}

int group_test::beforeGroupChannelTest(int group_id,int channel_id)
{
	QSqlDatabase db;
	if(QSqlDatabase::contains("grouptest")){
		db=QSqlDatabase::database("grouptest");
	}
	else{
		db=QSqlDatabase::addDatabase("QSQLITE","grouptest");
	}
	QString sAppPath = QCoreApplication::applicationDirPath();
	QString sDatabasePath = sAppPath + "/system.db";
	db.setDatabaseName(sDatabasePath);
	db.open();

	QSqlQuery _query_1(db);
	_query_1.prepare("delete from r_chl_group");
	_query_1.exec();
	_query_1.exec("update sqlite_sequence set seq=0 where name= 'r_chl_group';"); 
	QString name="rgc_name_test_one";
	_query_1.prepare("insert into r_chl_group(chl_id,group_id,name) values(:chl_id,:group_id,:name)");
	_query_1.bindValue(":chl_id",channel_id);
	_query_1.bindValue(":group_id",group_id);
	_query_1.bindValue(":name",name);
	_query_1.exec();
	int id =0;
	if(_query_1.isActive()){
		QString command=QString("select max(id) as id from r_chl_group");
		_query_1.exec(command);
		int id_Index=_query_1.record().indexOf("id");
		if(_query_1.first()){
			id=_query_1.value(id_Index).toInt();
		}
		db.close();
		return id;
	}
	db.close();
	return id;
}
//1 添加一个新用户|返回-1（失败），返回id（成功）
void group_test::Group_AddGroup_test()
{
	beforeGroupTest();
	START_GROUP_UNIT_TEST(Igroup);
	//step1: test return id 
	int nret = Igroup->AddGroup("admin");
	QVERIFY2(-1!=nret,"AddGroup");
	//step2:test id IsExist
	bool nret_true=Igroup->IsGroupExists(nret);
	QVERIFY2(true==nret_true,"AddGroup:id is not exists;");
	//step3: test name IsExist
	QString name=Igroup->GetGroupName(nret);
	QVERIFY2(name=="admin","AddGroup:name is not correct");
	END_GROUP_UNIT_TEST(Igroup);
}
//2 判断一个组id是否存在|返回false（失败），返回true（成功）
void group_test::Group_IsGroupExist_test()
{
	beforeGroupTest();
	START_GROUP_UNIT_TEST(Igroup);
	//step 1 ：测试id存在的情况
	int group_id = Igroup->AddGroup("admin");
	bool nret_true=Igroup->IsGroupExists(group_id);
	QVERIFY2(true==nret_true,"IsGroupExists");
	//step 2 ：测试id不存在的情况
	bool nret_false=Igroup->IsGroupExists(group_id+1);
	QVERIFY2(false==nret_false,"IsGroupExists");
	END_GROUP_UNIT_TEST(Igroup);
}
//3 删除组|删除一次|删除两次|删除不存在的id
void group_test::Group_RemoveGroup_test()
{
	beforeGroupTest();
	START_GROUP_UNIT_TEST(Igroup);
	//step 1:测试id存在的情况
	int group_id=Igroup->AddGroup("admin");
	int nret_0=Igroup->RemoveGroup(group_id);
	QVERIFY2(0==nret_0,"RemoveGroup");
	//step 2：test id IsExists
	bool nret_false=Igroup->IsGroupExists(group_id);
	QVERIFY2(false==nret_false,"RemoveGroup:id is not exists");
	//step 3:test remove id which is not IsExist
	int nret_d=Igroup->RemoveGroup(group_id);
	QVERIFY2(IGroupManager::E_GROUP_NOT_FOUND==nret_d,"RemoveGroup:remove id which is not IsExist");
	END_GROUP_UNIT_TEST(Igroup);
}
//4 修改一个id的用户名|返回E_GROUP_NOT_FOUND（id不存在），返回0（修改成功）
void group_test::Group_ModifyGroup_test()
{
	beforeGroupTest();
	START_GROUP_UNIT_TEST(Igroup);
	//step 1:测试id存在的情况，将用户名admin修改成guest
	int group_id=Igroup->AddGroup("admin");
	int nret_0=Igroup->ModifyGroupName(group_id,"guest");
	QVERIFY2(0==nret_0,"ModifyGroupName");
	//step 2:test whether name is modify
	QString name=Igroup->GetGroupName(group_id);
	QVERIFY2(name=="guest","GetGroupName:name do not modify");
	//step 3:test modify the id which is not IsExist
	int nret_group_no_found=Igroup->ModifyGroupName(group_id+1,"guest");
	QVERIFY2(IGroupManager::E_GROUP_NOT_FOUND==nret_group_no_found,"ModifyGroupName");
	END_GROUP_UNIT_TEST(Igroup);
}
//5 获取组的个数|返回组的个数，测试组个数为1|0
void group_test::Group_GetGroupCount_test()
{
	beforeGroupTest();
	START_GROUP_UNIT_TEST(Igroup);
	//step 1 :组数为0时
	int group_id=Igroup->AddGroup("admin");
	Igroup->RemoveGroup(group_id);
	int nret_0=Igroup->GetGroupCount();
	QVERIFY2(0==nret_0,"GetGroupCount");
	//step 2 :组数为1时
	Igroup->AddGroup("admin");
	int nret_1=Igroup->GetGroupCount();
	QVERIFY2(1==nret_1,"GetGroupCount");
	END_GROUP_UNIT_TEST(Igroup);
}
//6 获取组id的列表|空列表|id数为2的列表
void group_test::Group_GetGroupList_test()
{
	beforeGroupTest();
	START_GROUP_UNIT_TEST(Igroup);
	//step 1:测试组数为2
	int group_id=Igroup->AddGroup("admin");
	group_id=Igroup->AddGroup("admin");
	QStringList group_list=Igroup->GetGroupList();
	for(int i=0;i<group_list.size();i++){
		int id=group_list.at(i).toInt();
		QVERIFY2(group_id==group_list.at(i).toInt()||(group_id-1)==group_list.at(i).toInt(),"GetGroupList");
	}
	//step 2:测试组数为0
	Igroup->RemoveGroup(group_id);
	Igroup->RemoveGroup(group_id-1);
	QStringList group_list_empty=Igroup->GetGroupList();
	bool nret= group_list_empty.isEmpty();
	QVERIFY2(true==nret,"GetGroupList");
	END_GROUP_UNIT_TEST(Igroup);
}
//7 获取id的组名|从参数中返回|空id|成功返回0|失败返回错误号
void group_test::Group_GetGroupName_int_test()
{
	beforeGroupTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	QString sname;
	//step 1:存在的id
	Igroup->GetGroupName(group_id,sname);
	QVERIFY2("admin"==sname,"GetGroupName");
	//step 2:id 不存在
	Igroup->RemoveGroup(group_id);
	QString sname_empty;
	sname_empty.clear();
	Igroup->GetGroupName(group_id,sname_empty);
	bool nret=sname_empty.isEmpty();
	QVERIFY2(true==nret,"GetGroupName");
	END_GROUP_UNIT_TEST(Igroup);
}
//8 获取id的组名|从函数返回值中返回|空id|成功返回0|失败返回错误号
void group_test::Group_GetGroupName_QString_test()
{
	beforeGroupTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	//step 1:存在的id
	QString sname;
	sname=Igroup->GetGroupName(group_id);
	QVERIFY2("admin"==sname,"GetGroupName");
	//step 2:id 不存在
	Igroup->RemoveGroup(group_id);
	QString sname_empty;
	sname_empty.clear();
	sname_empty=Igroup->GetGroupName(group_id);
	bool nret=sname_empty.isEmpty();
	QVERIFY2(true==nret,"GetGroupName");
	END_GROUP_UNIT_TEST(Igroup);
	
}
//9 判断通道Id是否存在|存在返回true|不存在返回fail
void group_test::Group_IsChannelExists_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:Id存在的情况，返回true
	bool nRet_bool=false;
	nRet_bool=Igroup->IsChannelExists(chl_id);
	QVERIFY2(true==nRet_bool,"step 1");
	//step 2: Id不存在的情况，返回fail
	nRet_bool=true;
	nRet_bool=Igroup->IsChannelExists(chl_id-1);
	QVERIFY2(false==nRet_bool,"step 2");
	END_GROUP_UNIT_TEST(Igroup);
}
//10 判断通道组id是否存在|存在返回true|不存在返回fail
void group_test::Group_IsR_Channel_GroupExist_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rcg_id存在的情况，返回true
	bool nRet_bool=false;
	nRet_bool=Igroup->IsR_Channel_GroupExist(rgc_id);
	QVERIFY2(true==nRet_bool,"step 1");
	nRet_bool=true;
	nRet_bool=Igroup->IsR_Channel_GroupExist(rgc_id-1);
	QVERIFY2(false==nRet_bool,"step 2");
	END_GROUP_UNIT_TEST(Igroup);
}
//11 添加通道到组|成功返回组-通道id|失败返回-1
void group_test::Group_AddChannelInGroup_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:添加存在的group_id,chl_id,sname，放回rgc_id
	int nRet_rgc_id=Igroup->AddChannelInGroup(group_id,chl_id,"rgc_name_test_one");
	bool nRet_bool=Igroup->IsR_Channel_GroupExist(rgc_id);
	QVERIFY2(true==nRet_bool,"step 1:IsR_Channel_GroupExist");
	int nRet_chl_id=0;
	int nRet_group_id=0;
	QString nRet_sname;
	nRet_sname.clear();
	Igroup->GetChannelInfoFromGroup(rgc_id,nRet_chl_id,nRet_group_id,nRet_sname);
	QVERIFY2(nRet_chl_id==chl_id,"step 1:chl_id");
	QVERIFY2(nRet_group_id==group_id,"step 1:group_id");
	QVERIFY2(nRet_sname=="rgc_name_test_one","step 1:rgc_name_test_one");
	//step 2:chl_id 不存在，返回-1
	nRet_rgc_id=Igroup->AddChannelInGroup(group_id,chl_id-1,"rgc_name_test_one");
	nRet_bool=Igroup->IsR_Channel_GroupExist(nRet_rgc_id);
	QVERIFY2(-1==nRet_rgc_id,"step 2:nRet_rgc_id");
	QVERIFY2(false==nRet_bool,"step 2:IsR_Channel_GroupExist");
	//step 3:group_id 不存在，返回-1
	nRet_rgc_id=Igroup->AddChannelInGroup(group_id-1,chl_id,"rgc_name_test_one");
	nRet_bool=Igroup->IsR_Channel_GroupExist(nRet_rgc_id);
	QVERIFY2(-1==nRet_rgc_id,"step 2:nRet_rgc_id");
	QVERIFY2(false==nRet_bool,"step 2:IsR_Channel_GroupExist");
	END_GROUP_UNIT_TEST(Igroup);
}
//12 移除组-通道id|成功返回0|失败返回错误号
void group_test::Group_RemoveChannelFromGroup_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rgc_id 存在，返回0
	int nRet_rgc_id=Igroup->RemoveChannelFromGroup(rgc_id);
	QVERIFY2(0==nRet_rgc_id,"step 1:nRet_rgc_id");
	bool nRet_bool=false;
	nRet_bool=Igroup->IsR_Channel_GroupExist(rgc_id);
	QVERIFY2(false==nRet_bool,"step 1:IsR_Channel_GroupExist");
	//step 2:rgc_id 不存在，返回错误号
	int nRet_int=Igroup->RemoveChannelFromGroup(rgc_id+1);
	QVERIFY2(IGroupManager::E_CHANNEL_NOT_IN_GROUP==nRet_int,"step 2:nRet_int");
	END_GROUP_UNIT_TEST(Igroup);
}
//13 修改-通道rgc-id的名称|成功返回0|失败返回错误号
void group_test::Group_ModifyGroupChannelName_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rgc_id存在，修改成功，返回0
	QString sName_test_two="sName_test_two";
	int nRet_int=Igroup->ModifyGroupChannelName(rgc_id,sName_test_two);
	QVERIFY2(0==nRet_int,"step 1:nRet_int");
	QString nRet_sName=Igroup->GetGroupChannelName(rgc_id);
	QVERIFY2(nRet_sName==sName_test_two,"step 1:nRet_sName");
	//step 2:rgc_id不存在，修改失败，返回错误号
	nRet_int=-1;
	nRet_int=Igroup->ModifyGroupChannelName(rgc_id+1,sName_test_two);
	QVERIFY2(IGroupManager::E_CHANNEL_NOT_IN_GROUP==nRet_int,"step 2:nRet_int");
	nRet_sName.clear();
	nRet_sName=Igroup->GetGroupChannelName(rgc_id+1);
	QVERIFY2(true==nRet_sName.isEmpty(),"step 2:nRet_sName");
	END_GROUP_UNIT_TEST(Igroup);
}
//14 将组-通道移动到组group_id|成功返回0|失败返回错误号
void group_test::Group_MoveChannelToGroup_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int group_id_two=Igroup->AddGroup("admin_two");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rgc_id 存在，group_id存在，返回0
	int nRet_int=Igroup->MoveChannelToGroup(rgc_id,group_id_two);
	QVERIFY2(0==nRet_int,"step 1:nRet_int");
	QVariantMap  nRet_QVariantMap =Igroup->GetChannelInfoFromGroup(rgc_id);
	int nRet_group_id=nRet_QVariantMap.value("group_id").toInt();
	QVERIFY2(nRet_group_id==group_id_two,"step 1:nRet_group_id");
	//step 2:rgc_id 不存在，返回IGroupManager::E_CHANNEL_NOT_IN_GROUP
	nRet_int=Igroup->MoveChannelToGroup(rgc_id+1,group_id_two);
	QVERIFY2(IGroupManager::E_CHANNEL_NOT_IN_GROUP==nRet_int,"step 2:IGroupManager::E_CHANNEL_NOT_IN_GROUP");
	//step 3:group_id 不存在，返回IGroupManager::E_GROUP_NOT_FOUND
	nRet_int=Igroup->MoveChannelToGroup(rgc_id,group_id_two+1);
	QVERIFY2(IGroupManager::E_GROUP_NOT_FOUND==nRet_int,"step 3:nRet_int");
	END_GROUP_UNIT_TEST(Igroup);
}
//15 获取rgc_id的名称，名称在sname中返回|成功返回0|失败返回错误号
void group_test::Group_GetGroupChannelName_int_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rgc 存在的情况，返回0
	QString nRet_sName;
	nRet_sName.clear();
	int nRet_int=Igroup->GetGroupChannelName(rgc_id,nRet_sName);
	QVERIFY2(0==nRet_int,"step 1:nRet_int");
	QVERIFY2(nRet_sName=="rgc_name_test_one","step 1:nRet_sName");
	//step 2:rgc 不存在的情况，返回错误号
	nRet_int=Igroup->GetGroupChannelName(rgc_id+1,nRet_sName);
	QVERIFY2(IGroupManager::E_CHANNEL_NOT_IN_GROUP==nRet_int,"step 2:nRet_int");
	QVERIFY2(true==nRet_sName.isEmpty(),"step 2:nRet_sName");
	END_GROUP_UNIT_TEST(Igroup);
}
//16 获取组group_id下组-通道id的总个数|成功返回个数
void group_test::Group_GetGroupChannelCount_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:组内存在一组rgc_id时，返回1
	int nRet_count=Igroup->GetGroupChannelCount(group_id);
	QVERIFY2(1==nRet_count,"step 1:nRet_count");
	QStringList nRet_rgc_id=Igroup->GetGroupChannelList(group_id);
	QString QS_rgc_id=QString("%1").arg(rgc_id);
	bool nRet_bool=nRet_rgc_id.contains(QS_rgc_id);
	QVERIFY2(true==nRet_bool,"step 1:GetGroupChannelList");
	//step 2:组内不存在rgc_id,返回0
	Igroup->RemoveChannelFromGroup(rgc_id);
	nRet_count=Igroup->GetGroupChannelCount(group_id);
	QVERIFY2(0==nRet_count,"step 1:nRet_count");
	nRet_rgc_id.clear();
	nRet_rgc_id=Igroup->GetGroupChannelList(group_id);
	QS_rgc_id=QString("%1").arg(rgc_id);
	nRet_bool=nRet_rgc_id.contains(QS_rgc_id);
	QVERIFY2(false==nRet_bool,"step 1:GetGroupChannelList");
	END_GROUP_UNIT_TEST(Igroup);
}
//17 获取组group_id下组-通道id列表
void group_test::Group_GetGroupChannelList_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:group_id的rgc_id数为1，返回一组
	QStringList nRet_rgc_id;
	nRet_rgc_id.clear();
	nRet_rgc_id=Igroup->GetGroupChannelList(group_id);
	QString QS_rgc_id=QString("%1").arg(rgc_id);
	bool nRet_bool=nRet_rgc_id.contains(QS_rgc_id);
	QVERIFY2(true==nRet_bool,"step 1:contains");
	int nRet_size=nRet_rgc_id.size();
	QVERIFY2(1==nRet_size,"step 1:nRet_size");
	//step 2:group_id不存在的情况下,返回空的列表
	nRet_rgc_id.clear();
	nRet_rgc_id=Igroup->GetGroupChannelList(group_id+1);
	QVERIFY2(true==nRet_rgc_id.isEmpty(),"step 2:isEmpty");
	END_GROUP_UNIT_TEST(Igroup);
}
//18 获取组-通道rgc_id的名称
void group_test::Group_GetGroupChannelName_qstring_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rgc_id 存在的情况，返回sname
	QString nRet_sName=Igroup->GetGroupChannelName(rgc_id);
	QVERIFY2(nRet_sName=="rgc_name_test_one","step 1:nRet_sName");
	//step 2:rgc_id不存在的情况，返回sname为空
	nRet_sName.clear();
	nRet_sName=Igroup->GetGroupChannelName(rgc_id+1);
	QVERIFY2(true==nRet_sName.isEmpty(),"step 1:isEmpty");
	END_GROUP_UNIT_TEST(Igroup);
}
//19 获取组-通道rgc_id对应的ID，在参数中返回
void group_test::Group_GetChannelIdFromGroup_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rgc_id 存在时，返回0
	int nRet_chl_id=0;
	int nRet_int=Igroup->GetChannelIdFromGroup(rgc_id,nRet_chl_id);
	QVERIFY2(0==nRet_int,"step 1:nRet_int");
	QVERIFY2(nRet_chl_id==chl_id,"step 1:chl_id");
	//step 2:rgc_id不存在时，返回错误号
	nRet_int=Igroup->GetChannelIdFromGroup(rgc_id+1,nRet_chl_id);
	QVERIFY2(IGroupManager::E_CHANNEL_NOT_IN_GROUP==nRet_int,"step 1:nRet_int");
	END_GROUP_UNIT_TEST(Igroup);
}
//20 获取组-通道rgc_id对应的ID
void group_test::Group_GetChannelIdFromGroup_one_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rgc_id 存在的情况，返回通道号
	int nRet_chl_id=Igroup->GetChannelIdFromGroup(rgc_id);
	QVERIFY2(nRet_chl_id==chl_id,"step 1:nRet_chl_id");
	//step 2:rgc_id 不存在的情况，返回错误号
	nRet_chl_id=Igroup->GetChannelIdFromGroup(rgc_id+1);
	QVERIFY2(IGroupManager::E_CHANNEL_NOT_IN_GROUP==nRet_chl_id,"step 2:nRet_chl_id");
	END_GROUP_UNIT_TEST(Igroup);
}
//21 获取组-通道rgc_id的相关信息，所在组在group_id内返回，对应的通道ID在chl_id内返回，组-通道名称在sName内返回。
void group_test::Group_GetChannelInfoFromGroup_int_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rgc_id存在的情况，成功返回0
	int nRet_chl_id=-1;
	int nRet_group_id=-1;
	QString nRet_sName;
	int nRet_int=Igroup->GetChannelInfoFromGroup(rgc_id,nRet_chl_id,nRet_group_id,nRet_sName);
	QVERIFY2(0==nRet_int,"step 1:nRet_int");
	QVERIFY2(chl_id==nRet_chl_id,"step 1:chl_id");
	QVERIFY2(group_id==nRet_group_id,"step 1:group_id");
	QVERIFY2("rgc_name_test_one"==nRet_sName,"step 1:sName");
	//step 2:rgc_id不存在的情况，返回错误号
	nRet_int=Igroup->GetChannelInfoFromGroup(rgc_id+1,nRet_chl_id,nRet_group_id,nRet_sName);
	QVERIFY2(IGroupManager::E_CHANNEL_NOT_IN_GROUP==nRet_int,"step 2:nRet_int");
	END_GROUP_UNIT_TEST(Igroup);
}
// 22 获取组-通道rgc_id的相关信息
void group_test::Group_GetChannelInfoFromGroup_QVariant_test()
{
	beforeGroupTest();
	int chl_id=beforeChannelTest();
	START_GROUP_UNIT_TEST(Igroup);
	int group_id=Igroup->AddGroup("admin");
	int rgc_id=beforeGroupChannelTest(group_id,chl_id);
	//step 1:rgc_id存在的情况，成功返回QVariantMap
	QVariantMap nRet_QVariantMap;
	nRet_QVariantMap.clear();
	nRet_QVariantMap=Igroup->GetChannelInfoFromGroup(rgc_id);
	QVERIFY2(false==nRet_QVariantMap.isEmpty(),"step 1:isEmpty");
	int nRet_chl_id=nRet_QVariantMap.value("chl_id").toInt();
	int nRet_group_id=nRet_QVariantMap.value("group_id").toInt();
	QString nRet_sName=nRet_QVariantMap.value("name").toString();
	QVERIFY2(nRet_chl_id==chl_id,"step 1:nRet_chl_id");
	QVERIFY2(nRet_group_id==group_id,"step 1:group_id");
	QVERIFY2(nRet_sName=="rgc_name_test_one","step 1:nRet_sName");
	//step 2:rgc_id 不存在的情况，返回空的QVariantMap
	nRet_QVariantMap.clear();
	nRet_QVariantMap=Igroup->GetChannelInfoFromGroup(rgc_id+1);
	QVERIFY2(true==nRet_QVariantMap.isEmpty(),"step 2:isEmpty");
	END_GROUP_UNIT_TEST(Igroup);
}