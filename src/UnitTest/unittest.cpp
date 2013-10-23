#include "unittest.h"
#include <guid.h>
#include <IUserManager.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#define START_USER_UNIT_TEST(ii) 	IUserManager * ii = NULL; \
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IUserManager,(void **)&ii); \
	QVERIFY2(NULL != ii,"Create user manager instance");

#define END_USER_UNIT_TEST(ii) ii->Release(); 

UnitTest::UnitTest()
{

}

UnitTest::~UnitTest()
{

}

//测试用例:
// 前置：重置数据库，数据库中创建了user_infomation表，数据库中仅有一条记录：admin|md5('admin')|0|-1|-1
// 1 #008 添加重复用户|返回E_USER_EXISTS，GetUserCount返回2(admin及第一次添加的用户),GetUserList返回两个用户名(admin及第一次添加的用户)
// 2 添加用户后删除用户两次|第一次删除返回OK，第二次删除返回E_USER_NOT_FOUND，GetUserList中无之前添加的用户
// 3 删除不存在用户|返回E_USER_NOT_FOUND
// 4 修改不存在用户的密码|返回E_USER_NOT_FOUND
// 5 修改admin密码|返回OK，使用修改前密码调用CheckUser返回false,使用修改后密码返回true
// 6 修改任意用户权限等级[0-3]|返回OK,GetUserLevel返回OK，nLevel为修改后的权限等级
// 7 修改不存在用户的权限等级[0-3]|返回E_USER_NOT_FOUND,GetUserLevel获取所有用户权限等级，与调用前保持不变
// 8 修改任意用户权限等级[^0-3]|返回E_SYSTEM_FAILED,GetUserLevel返回OK，nLevel为修改前的权限等级
// 9 修改任意用户权限掩码|返回OK,GetUserAuthorityMask返回OK,nAuthorityMask1和nAuthorityMask2为修改后的值
//10 修改不存在用户的权限掩码|返回E_USER_NOT_FOUND,GetUserAuthorityMask获取所有用户权限掩码，与调用前保持不变
//11 测试admin是否存在|返回true
//12 测试不存在用户是否存在|返回false
//13 使用密码admin校验用户admin|返回true
//14 使用密码1234校验用户admin|返回false
//15 获取用户admin的权限等级|返回OK，nLevel为0
//16 获取不存在用户的权限等级|返回E_USER_NOT_FOUND
//17 获取用户admin的权限掩码|返回OK,nAuthorityMask1和nAuthorityMask1为-1
//18 获取不存在用户的权限掩码|返回E_USER_NOT_FOUND
//19 获取系统内用户数|返回1
//20 获取系统内用户列表|列表内只存在admin用户

// 1 添加重复用户|返回E_USER_EXISTS，GetUserCount返回2(admin及第一次添加的用户),GetUserList返回两个用户名(admin及第一次添加的用户)
void UnitTest::UserCase1()
{
	beforeUserTest();

	START_USER_UNIT_TEST(Iuser);
	// step 1
	int nRet = Iuser->AddUser("test","test",1,-1,-1);

	QVERIFY2(IUserManager::OK == nRet,"step 1:return");

	int nCount = Iuser->GetUserCount();
	QVERIFY2(2 == nCount,"step 1:GetUserCount");

	QStringList users = Iuser->GetUserList();
	QStringList::const_iterator it;
	for (it = users.begin(); it != users.end(); it ++)
	{
		QVERIFY2(*it == "admin" || *it == "test","step 1:GetUserList");
	}

	// step 2
	nRet = Iuser->AddUser("test","test",1,-1,-1);

	QVERIFY2(IUserManager::E_USER_EXISTS == nRet,"step 2:return");

	nCount = Iuser->GetUserCount();
	QVERIFY2(2 == nCount,"step 2:GetUserCount");

	users = Iuser->GetUserList();
	for (it = users.begin(); it != users.end(); it ++)
	{
		QVERIFY2(*it == "admin" || *it == "test","step 2:GetUserList");
	}

	END_USER_UNIT_TEST(Iuser);
}

// 2 添加用户后删除用户两次|第一次删除返回OK，第二次删除返回E_USER_NOT_FOUND，GetUserList中无之前添加的用户
void UnitTest::UserCase2()
{
	int nRet;
	int nCount;
	QStringList users;
	QStringList::const_iterator it;
	beforeUserTest();

	START_USER_UNIT_TEST(Iuser);

	// step1
	nRet = Iuser->AddUser("test","test",1,-1,-1);
	QVERIFY2(IUserManager::OK == nRet,"step 1");

	nCount = Iuser->GetUserCount();
	QVERIFY2(2 == nCount,"step 1:GetUserCount");

	users = Iuser->GetUserList();
	for (it = users.begin();it != users.end();it++)
	{
		QVERIFY2(*it == "admin" || *it == "test","step 1:GetUserList");
	}

	// step2
	nRet = Iuser->RemoveUser("test");
	QVERIFY2(IUserManager::OK == nRet,"step 2");

	nCount = Iuser->GetUserCount();
	QVERIFY2(1 == nCount,"step 2:GetUserCount");

	users = Iuser->GetUserList();
	for (it = users.begin();it != users.end();it++)
	{
		QVERIFY2(*it == "admin","step 2:GetUserList");
	}

	// step3
	nRet = Iuser->RemoveUser("test");
	QVERIFY2(IUserManager::E_USER_NOT_FOUND == nRet,"step 3");

	nCount = Iuser->GetUserCount();
	QVERIFY2(1 == nCount,"step 3:GetUserCount");

	users = Iuser->GetUserList();
	for (it = users.begin();it != users.end();it++)
	{
		QVERIFY2(*it == "admin","step 3:GetUserList");
	}

	END_USER_UNIT_TEST(Iuser);

}

// 3 删除不存在用户|返回E_USER_NOT_FOUND
void UnitTest::UserCase3()
{
	int nRet;
	int nCount;
	QStringList users;
	QStringList::const_iterator it;
	beforeUserTest();

	START_USER_UNIT_TEST(Iuser);

	// step 1
	nRet = Iuser->RemoveUser("test");
	QVERIFY2(IUserManager::E_USER_NOT_FOUND,"step1");

	nCount = Iuser->GetUserCount();
	QVERIFY2(1 == nCount,"step 1:GetUserCount");

	users = Iuser->GetUserList();
	for (it = users.begin(); it != users.end(); it ++)
	{
		QVERIFY2("admin" == *it,"step 1:GetUserList");
	}

	END_USER_UNIT_TEST(Iuser);
}

// 4 修改不存在用户的密码|返回E_USER_NOT_FOUND
void UnitTest::UserCase4()
{
	int nRet;
	beforeUserTest();

	START_USER_UNIT_TEST(Itest);

	// step 1
	nRet = Itest->ModifyUserPassword("test","1234");
	QVERIFY2(IUserManager::E_USER_NOT_FOUND == nRet,"step1");

	END_USER_UNIT_TEST(Itest);
}

// 5 修改admin密码|返回OK，使用修改前密码调用CheckUser返回false,使用修改后密码返回true
void UnitTest::UserCase5()
{
	int nRet;
	bool bRet;
	beforeUserTest();

	START_USER_UNIT_TEST(Itest);

	// step 1
	nRet = Itest->ModifyUserPassword("admin","check");
	QVERIFY2(IUserManager::OK == nRet,"step 1");

	// step 2
	bRet = Itest->CheckUser("admin","admin");
	QVERIFY2(false == bRet,"step 2");

	// step 3
	bRet = Itest->CheckUser("admin","check");
	QVERIFY2(true == bRet,"step 3");	

	END_USER_UNIT_TEST(Itest);
}

// 6 修改任意用户权限等级[0-3]|返回OK,GetUserLevel返回OK，nLevel为修改后的权限等级
void UnitTest::UserCase6()
{
	int nRet;
	beforeUserTest();

	START_USER_UNIT_TEST(Itest);

	int i;
	for (i = 0;i < 4;i++)
	{
		int nLevel;
		QVERIFY2(IUserManager::OK == Itest->ModifyUserLevel("admin",i),"ModifyUserLevel");
		QVERIFY2(IUserManager::OK == Itest->GetUserLevel("admin",nLevel),"GetUserLevel");
		QCOMPARE(i,nLevel);
	}

	END_USER_UNIT_TEST(Itest);
}

// 7 修改不存在用户的权限等级[0-3]|返回E_USER_NOT_FOUND,GetUserLevel获取所有用户权限等级，与调用前保持不变
void UnitTest::UserCase7()
{
	beforeUserTest();

	START_USER_UNIT_TEST(Itest);

	int i;
	for (i = 0; i < 4 ;i ++)
	{
		QVERIFY2(IUserManager::E_USER_NOT_FOUND == Itest->ModifyUserLevel("test",i),"");
	}

	END_USER_UNIT_TEST(Itest);
}

// 8 修改任意用户权限等级[^0-3]|返回E_SYSTEM_FAILED,GetUserLevel返回OK，nLevel为修改前的权限等级
void UnitTest::UserCase8()
{
	beforeUserTest();

	START_USER_UNIT_TEST(Itest);
	int nLevelBefor;
	QVERIFY2(IUserManager::OK == Itest->GetUserLevel("admin",nLevelBefor),"");

	int n;
	qsrand(QTime::currentTime().msec());
	n = qrand();
	int i;
	for ( i = 0; i < n; i ++)
	{
		int nLevel = 3 + qrand();
		QVERIFY2(IUserManager::E_SYSTEM_FAILED == Itest->ModifyUserLevel("admin",nLevel),"");
		QVERIFY2(IUserManager::OK == Itest->GetUserLevel("admin",nLevel),"");
		QCOMPARE(nLevel,nLevelBefor);
	}


	END_USER_UNIT_TEST(Itest);
}

// 9 修改任意用户权限掩码|返回OK,GetUserAuthorityMask返回OK,nAuthorityMask1和nAuthorityMask2为修改后的值
void UnitTest::UserCase9()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	qsrand(QTime::currentTime().msec());
	int n = qrand();
	int i;
	for (i = 0; i < n; i++)
	{
		int mask1 = qrand();
		int mask2 = qrand();
		QVERIFY2(IUserManager::OK == Itest->ModifyUserAuthorityMask("admin",mask1,mask2),"");
		int currentMask1,currentMask2;
		QVERIFY2(IUserManager::OK == Itest->GetUserAuthorityMask("admin",currentMask1,currentMask2),"");
		QCOMPARE(mask1,currentMask1);
		QCOMPARE(mask2,currentMask2);
	}

	END_USER_UNIT_TEST(Itest);
}

//10 修改不存在用户的权限掩码|返回E_USER_NOT_FOUND,GetUserAuthorityMask获取所有用户权限掩码，与调用前保持不变
void UnitTest::UserCase10()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	qsrand(QTime::currentTime().msec());
	int mask1 = qrand();
	int mask2 = qrand();
	QVERIFY2(IUserManager::E_USER_NOT_FOUND == Itest->ModifyUserAuthorityMask("test",mask1,mask2),"");

	END_USER_UNIT_TEST(Itest);
}

//11 测试admin是否存在|返回true
void UnitTest::UserCase11()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	QVERIFY2(true == Itest->IsUserExists("admin"),"");

	END_USER_UNIT_TEST(Itest);
}

//12 测试不存在用户是否存在|返回false
void UnitTest::UserCase12()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	QVERIFY2(false == Itest->IsUserExists("test"),"");

	END_USER_UNIT_TEST(Itest);
}

//13 使用密码admin校验用户admin|返回true
void UnitTest::UserCase13()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	QVERIFY2(true == Itest->CheckUser("admin","admin"),"");

	END_USER_UNIT_TEST(Itest);
}

//14 使用密码1234校验用户admin|返回false
void UnitTest::UserCase14()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	QVERIFY2(false == Itest->CheckUser("admin","1234"),"");

	END_USER_UNIT_TEST(Itest);
}

//15 获取用户admin的权限等级|返回OK，nLevel为0
void UnitTest::UserCase15()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	int nLevel;
	QVERIFY2(IUserManager::OK == Itest->GetUserLevel("admin",nLevel),"");
	QCOMPARE(0,nLevel);

	END_USER_UNIT_TEST(Itest);
}

//16 获取不存在用户的权限等级|返回E_USER_NOT_FOUND
void UnitTest::UserCase16()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	int nLevel;
	QVERIFY2(IUserManager::E_USER_NOT_FOUND == Itest->GetUserLevel("test",nLevel),"");

	END_USER_UNIT_TEST(Itest);
}

//17 获取用户admin的权限掩码|返回OK,nAuthorityMask1和nAuthorityMask1为-1
void UnitTest::UserCase17()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	int nAuthorityMask1,nAuthorityMask2;
	QVERIFY2(IUserManager::OK == Itest->GetUserAuthorityMask("admin",nAuthorityMask1,nAuthorityMask2),"");
	QCOMPARE(-1,nAuthorityMask1);
	QCOMPARE(-1,nAuthorityMask2);

	END_USER_UNIT_TEST(Itest);
}

//18 获取不存在用户的权限掩码|返回E_USER_NOT_FOUND
void UnitTest::UserCase18()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	int nAuthorityMask1,nAuthorityMask2;
	QVERIFY2(IUserManager::E_USER_NOT_FOUND == Itest->GetUserAuthorityMask("test",nAuthorityMask1,nAuthorityMask2),"");

	END_USER_UNIT_TEST(Itest);
}

//19 获取系统内用户数|返回1
void UnitTest::UserCase19()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	QCOMPARE(1,Itest->GetUserCount());
	QVERIFY2(IUserManager::OK == Itest->AddUser("test","1234",1,-1,-1),"");
	QCOMPARE(2,Itest->GetUserCount());

	END_USER_UNIT_TEST(Itest);
}

//20 获取系统内用户列表|列表内只存在admin用户
void UnitTest::UserCase20()
{
	beforeUserTest();
	START_USER_UNIT_TEST(Itest);

	QStringList users = Itest->GetUserList();
	QVERIFY2(users.contains("admin"),"");
	QCOMPARE(1,users.count());
	QVERIFY2(IUserManager::OK == Itest->AddUser("test","1234",1,-1,-1),"");
	users = Itest->GetUserList();
	QVERIFY2(users.contains("admin") && users.contains("test"),"");
	QCOMPARE(2,users.count());

	END_USER_UNIT_TEST(Itest);
}

void UnitTest::beforeUserTest()
{
	QSqlDatabase db;
	if (QSqlDatabase::contains("usertest"))
	{
		db = QSqlDatabase::database("usertest"); 
	}
	else
	{
		db = QSqlDatabase::addDatabase("QSQLITE","usertest");
	}
	QString sAppPath = QCoreApplication::applicationDirPath();
	QString sDatabasePath = sAppPath + "/system.db";
	db.setDatabaseName(sDatabasePath);
	db.open();

	QSqlQuery query1(db);
	query1.prepare("delete from user_infomation");
	query1.exec();

	QSqlQuery query2(db);
	query2.prepare("insert into user_infomation(username,password,level,mask1,mask2) values(:username,:password,:level,:mask1,:mask2)");
	query2.bindValue(":username","admin");
	query2.bindValue(":password",QString(QCryptographicHash::hash(QString("admin").toAscii(),QCryptographicHash::Md5).toHex()));
	query2.bindValue(":level",0);
	query2.bindValue(":mask1",-1);
	query2.bindValue(":maks2",-1);
	query2.exec();

	db.close();
}
