#include "TestCommonLibSet.h"
#include <guid.h>
#include <ILocalSetting.h>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQuery>

#define  START_AREA_UNIT_TEST(ii) ILocalSetting *ii=NULL;\
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_ILocalSetting,(void**)&ii);\
	QVERIFY2(NULL!=ii,"Create ILocalSetting instance");
#define END_AREA_UNIT_TEST(ii) ii->Release();

TestCommonLibSet::TestCommonLibSet(void)
{
}


TestCommonLibSet::~TestCommonLibSet(void)
{
}
//测试用例：

void TestCommonLibSet::beforeAreaTest()
{
	QSqlDatabase db;
	if (QSqlDatabase::contains("LocalSettingTest"))
	{
		db=QSqlDatabase::database("LocalSettingTest");
	}
	else{
		db=QSqlDatabase::addDatabase("QSQLITE","DisksSettingTest");
	}
	QString sAppPath=QCoreApplication::applicationDirPath();
	QString sDatabasePath=sAppPath+"/system.db";
	db.setDatabaseName(sDatabasePath);
	db.open();

	QSqlQuery query1(db);
	query1.prepare("delete from general_setting");
	query1.exec();
	db.close();
}
//1.测试 int setLaunguage(const QString & sLanguage)输入与输出
//前置条件：清空数据库
//步骤：
//step1：输入非法语言字符，如“agagl”，正常字符表参见 附表1；预期：返回E_PARAMETER_ERROR 
//step2:调用QString getLaunguage();预期：返回“en_GB”（默认值)
//step3:  输入正常参数，正确参数见附表1，如“zh_CN” ;预期： 返回S_OK  
//step4:调用QString getLaunguage();预期：返回zh_CN
//step5:输入非法语言字符，如“agagl”，正常字符表参见 附表1 ;预期：返回E_PARAMETER_ERROR
//step6:调用QString getLaunguage() ;预期：返回zh_CN
void TestCommonLibSet::test1()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	QString m_sLanguage="agagl";
	int nRet=-1;
	nRet=ILocalSet->setLanguage(m_sLanguage);
	QVERIFY2(ILocalSetting::E_PARAMETER_ERROR==nRet,"step 1:return");

	m_sLanguage.clear();
	m_sLanguage=ILocalSet->getLanguage();
	QVERIFY2("en_GB"==m_sLanguage,"step 2:return");

	m_sLanguage.clear();
	nRet=-1;
	m_sLanguage.append("zh_CN");
	nRet=ILocalSet->setLanguage(m_sLanguage);
	QVERIFY2(ILocalSetting::OK==nRet,"step 3:return");

	m_sLanguage.clear();
	m_sLanguage=ILocalSet->getLanguage();
	QVERIFY2("zh_CN"==m_sLanguage,"step 4:return");

	m_sLanguage.clear();
	nRet=-1;
	nRet=ILocalSet->setLanguage("agagl");
	QVERIFY2(ILocalSetting::E_PARAMETER_ERROR==nRet,"step 5:return");

	m_sLanguage=ILocalSet->getLanguage();
	QVERIFY2("zh_CN"==m_sLanguage,"step 6:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//2.测试 QString getLaunguage()输入与输出
//前置条件：清空数据库
//步骤：
//step1：调用函数；预期：返回“en_GB”（默认值）
//step2:调用setLaunguage（“zh_CN”）;预期：返回S_OK
//step3: 调用函数 ;预期：返回zh_CN  
void TestCommonLibSet::test2()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	QString m_sLanguage;
	m_sLanguage=ILocalSet->getLanguage();
	QVERIFY2("en_GB"==m_sLanguage,"step 1:return");
	m_sLanguage.clear();
	int nRet=-1;
	nRet=ILocalSet->setLanguage("zh_CN");
	QVERIFY2(ILocalSetting::OK==nRet,"step 2:return");
	m_sLanguage=ILocalSet->getLanguage();
	QVERIFY2("zh_CN"==m_sLanguage,"step 3:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//3.测试 int setAutoPollingTime(int aptime)输入与输出
//前置条件：清空数据库,正常参数输入范围为“30-86400”
//步骤：
//step1：输入错误参数 “0”；预期：返回E_PARAMETER_ERROR
//step2:调用int getAutoPollingTime();预期：返回30（默认值）
//step3:输入正常参数 “60”;预期： 返回S_OK  
//step4:调用int getAutoPollingTime();预期：返回‘60’
//step5:输入错误参数 “0”;预期：返回E_PARAMETER_ERROR
//step6:调用int getAutoPollingTime();预期： 返回‘60’ 
void TestCommonLibSet::test3()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	int nRet=-1;
	nRet=ILocalSet->setAutoPollingTime(0);
	QVERIFY2(ILocalSetting::E_PARAMETER_ERROR==nRet,"step 1:return");
	nRet=-1;
	nRet=ILocalSet->getAutoPollingTime();
	QVERIFY2(30==nRet,"step 2:return");
	nRet=-1;
	nRet=ILocalSet->setAutoPollingTime(60);
	QVERIFY2(ILocalSetting::OK==nRet,"step 3:return");
	nRet=-1;
	nRet=ILocalSet->getAutoPollingTime();
	QVERIFY2(60==nRet,"step 4:return");
	nRet=-1;
	nRet=ILocalSet->setAutoPollingTime(0);
	QVERIFY2(ILocalSetting::E_PARAMETER_ERROR==nRet,"step 5:return");
	nRet=-1;
	nRet=ILocalSet->getAutoPollingTime();
	QVERIFY2(60==nRet,"step 6:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//4.测试 int getAutoPollingTime()输入与输出
//前置条件：清空数据库
//步骤：
//step1：调用函数；预期：返回30
//step2: 调用函数setAutoPollingTime（60） ;预期：返回S_OK
//step3: 调用函数 ;预期：返回 60 
void TestCommonLibSet::test4()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	int nRet=-1;
	nRet=ILocalSet->getAutoPollingTime();
	QVERIFY2(30==nRet,"step 1:return");
	nRet=-1;
	nRet=ILocalSet->setAutoPollingTime(60);
	QVERIFY2(nRet==60,"step 2:return");
	nRet=-1;
	nRet=ILocalSet->getAutoPollingTime();
	QVERIFY2(60==nRet,"step 3:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//5.测试int setSplitScreenMode(const QString &smode)输入与输出
//前置条件：清空数据库
//步骤：
//step1:输入参数 为空；预期：返回E_PARAMETER_ERROR
//step2: 调用QString getSplitScreenMode() ;预期：返回 “div4_4”(默认值)
//step3: 输入参数 为“div2_2” ;预期： 返回S_OK  
//step4:调用QString getSplitScreenMode();预期：返回 div2_2 
//step5:输入参数 为空;预期：返回E_PARAMETER_ERROR
//step6:调用QString getSplitScreenMode();预期： 返回 div2_2 
void TestCommonLibSet::test5()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	int nRet=-1;
	QString m_smode;
	nRet=ILocalSet->setSplitScreenMode(m_smode);
	QVERIFY2(ILocalSetting::E_PARAMETER_ERROR==nRet,"step 1:return");
	m_smode=ILocalSet->getSplitScreenMode();
	QVERIFY2("div4_4"==m_smode,"step 2:return");
	nRet=-1;
	m_smode.clear();
	nRet=ILocalSet->setSplitScreenMode("div2_2");
	QVERIFY2(ILocalSetting::OK==nRet,"step 3:return");
	m_smode=ILocalSet->getSplitScreenMode();
	QVERIFY2("div2_2"==m_smode,"step 4:return");
	nRet=-1;
	m_smode.clear();
	nRet=ILocalSet->setSplitScreenMode(m_smode);
	QVERIFY2(ILocalSetting::E_PARAMETER_ERROR==nRet,"step 5:return");
	nRet=-1;
	m_smode.clear();
	nRet=ILocalSet->setSplitScreenMode("div2_2");
	QVERIFY2("div2_2"==m_smode,"step 6:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//6.测试 int getSplitScreenMode()输入与输出
//前置条件：清空数据库
//步骤：
//step1：调用函数；预期：返回 “div4_4”（默认）
//step2: 调用函数setSplitScreenMode（“div1_1”）;预期：返回S_OK
//step3: 调用函数 ;预期：返回div1_1
void TestCommonLibSet::test6()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	int nRet=-1;
	QString m_smode;
	m_smode=ILocalSet->getSplitScreenMode();
	QVERIFY2("div4_4"==m_smode,"step 1:return");
	m_smode.clear();
	m_smode.append("div1_1");
	nRet=ILocalSet->setSplitScreenMode(m_smode);
	QVERIFY2(ILocalSetting::OK==nRet,"step 2:return");
	m_smode.clear();
	m_smode=ILocalSet->getSplitScreenMode();
	QVERIFY2("div1_1"==m_smode,"step 1:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//7.测试int setAutoLogin(bool alogin)输入与输出
//前置条件：清空数据库
//步骤：
//step1:输入 正确参数 “false”；预期：返回 S_OK
//step2: 调用bool getAutoLogin();预期：返回 “false”
//step3: 输入 正确参数 “true” ;预期： 返回S_OK  
//step4:调用bool getAutoLogin();预期： 返回 “true” 
void TestCommonLibSet::test7()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	int nRet=-1;
	nRet=ILocalSet->setAutoLogin(false);
	QVERIFY2(ILocalSetting::OK==nRet,"step 1:return");
	bool bRet=true;
	bRet=ILocalSet->getAutoLogin();
	QVERIFY2(false==bRet,"step 2:return");
	nRet=-1;
	nRet=ILocalSet->setAutoLogin(true);
	QVERIFY2(ILocalSetting::OK,"step 3:return");
	bRet=false;
	bRet=ILocalSet->getAutoLogin();
	QVERIFY2(true==bRet,"step 4:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//8.测试 bool getAutoLogin()输入与输出
//前置条件：清空数据库
//步骤：
//step1：调用函数；预期：返回 false （默认值）
//step2:  调用函数int setAutoLogin(true) ;预期：返回S_OK
//step3: 调用函数 ;预期：返回 true
void TestCommonLibSet::test8()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	bool bRet=true;
	bRet=ILocalSet->getAutoLogin();
	QVERIFY2(false==bRet,"step 1:return");
	int nRet=-1;
	nRet=ILocalSet->setAutoLogin(true);
	QVERIFY2(ILocalSetting::OK==nRet,"step 2:return");
	bRet=false;
	bRet=ILocalSet->getAutoLogin();
	QVERIFY2(true==bRet,"step 3:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//9.测试int setAutoSyncTime(bool synctime)输入与输出
//前置条件：清空数据库
//步骤：
//step1:输入 正确参数 “false”；预期：返回 S_OK
//step2: 调用bool getAutoSyncTime();预期：返回 “false”
//step3: 输入 正确参数 “true” ;预期： 返回S_OK  
//step4:调用bool getAutoSyncTime();预期： 返回 “true” 
void TestCommonLibSet::test9()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	int nRet=-1;
	nRet=ILocalSet->setAutoSyncTime(false);
	QVERIFY2(ILocalSetting::OK==nRet,"step 1:return");
	bool bRet=true;
	bRet=ILocalSet->getAutoSyncTime();
	QVERIFY2(false==bRet,"step 2:return");
	nRet=-1;
	nRet=ILocalSet->setAutoSyncTime(true);
	QVERIFY2(ILocalSetting::OK==nRet,"step 3:return");
	bRet=false;
	bRet=ILocalSet->getAutoSyncTime();
	QVERIFY2(true==bRet,"step 4:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//10.测试 bool getAutoSyncTime()输入与输出
//前置条件：清空数据库
//步骤：
//step1：调用函数；预期：返回 false （默认值）
//step2:  调用函数int setAutoSyncTime(true);预期：返回S_OK
//step3: 调用函数 ;预期：返回 true
void TestCommonLibSet::test10()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	bool bRet=true;
	bRet=ILocalSet->getAutoSyncTime();
	QVERIFY2(false==bRet,"step 1:return");
	int nRet=-1;
	nRet=ILocalSet->setAutoSyncTime(true);
	QVERIFY2(ILocalSetting::OK==nRet,"step 2:return");
	bRet=false;
	bRet=ILocalSet->getAutoSyncTime();
	QVERIFY2(true==bRet,"step 3:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//11.测试int setAutoConnent(bool aconnent)输入与输出
//前置条件：清空数据库
//步骤：
//step1:输入 正确参数 “false”；预期：返回 S_OK
//step2: 调用bool getAutoConnent();预期：返回 “false”
//step3: 输入 正确参数 “true” ;预期： 返回S_OK  
//step4:调用bool getAutoConnent();预期： 返回 “true” 
void TestCommonLibSet::test11()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	int nRet=-1;
	nRet=ILocalSet->setAutoConnect(false);
	QVERIFY2(ILocalSetting::OK==nRet,"step 1:return");
	bool bRet=true;
	bRet=ILocalSet->getAutoConnect();
	QVERIFY2(false==bRet,"step 2:return");
	nRet=-1;
	nRet=ILocalSet->setAutoConnect(true);
	QVERIFY2(ILocalSetting::OK==nRet,"step 3:return");
	bRet=false;
	bRet=ILocalSet->getAutoConnect();
	QVERIFY2(true==bRet,"step 4:return");
	END_AREA_UNIT_TEST(ILocalSet);
}
//12.测试 bool getAutoConnent()输入与输出
//前置条件：清空数据库
//步骤：
//step1：调用函数；预期：返回 false （默认值）
//step2:  调用函数int setAutoConnent(true);预期：返回S_OK
//step3: 调用函数 ;预期：返回 true
void TestCommonLibSet::test12()
{
	beforeAreaTest();
	START_AREA_UNIT_TEST(ILocalSet);
	bool bRet=true;
	bRet=ILocalSet->getAutoConnect();
	QVERIFY2(false==bRet,"step 1:return");
	int nRet=-1;
	nRet=ILocalSet->setAutoConnect(true);
	QVERIFY2(ILocalSetting::OK==nRet,"step 2:return");
	bRet=false;
	bRet=ILocalSet->getAutoConnect();
	QVERIFY2(true==bRet,"step 3:return");
	END_AREA_UNIT_TEST(ILocalSet);
}

