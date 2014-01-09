#include "SetRecordTimeTest.h"
#include <guid.h>
#include <ISetRecordTime.h>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>


#define START_SETRECORDTIME_UNIT_TEST(ii) 	ISetRecordTime * ii = NULL; \
    pcomCreateInstance(CLSID_CommonLibPlugin, NULL, IID_ISetRecordTime,(void **)&ii); \
    QVERIFY2(NULL != ii,"Create SetRecordTime instance");

#define END_SETRECORDTIME_UNIT_TEST(ii) ii->Release(); 

SetRecordTimeTest::SetRecordTimeTest()
{

}
SetRecordTimeTest::~SetRecordTimeTest()
{

}


void SetRecordTimeTest::beforeSetRecordTimeTest()
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("SetRecordTimeTest"))
    {
        db = QSqlDatabase::database("SetRecordTimeTest"); 
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE","SetRecordTimeTest");
    }
    QString sAppPath = QCoreApplication::applicationDirPath();
    QString sDatabasePath = sAppPath + "/system.db";
    db.setDatabaseName(sDatabasePath);
    db.open();

    QSqlQuery query1(db);
    query1.prepare("delete from recordtime");
    query1.exec();
	query1.finish();
	query1.exec("delete from chl");
   
    QSqlQuery query2(db);
    query2.prepare("update sqlite_sequence set seq=0 where name= 'recordtime'");
    query2.exec();
	query2.finish();
	query2.exec("update sqlite_sequence set seq=0 where name='chl'");
	query2.finish();
	query2.exec("insert into chl(dev_id,channel_number,name,stream_id) values(1,0,\"test\",0)");
	query2.finish();

    QSqlQuery query3(db);
	query3.exec("update recordtime set chl_id=1,schedule_id=0,weekday=0,starttime='2013-12-31 00:00:00',endtime='2013-12-31 23:59:59',enable=0");
	query3.finish();

    query3.exec();
    
    db.close();
}
//测试用例:
// 前置：重置数据库，数据库中创建了recordtime表，数据库中仅有一条记录：0|0|0|”2013-12-31 00:00:00”|” 2013-12-31 23:59:59” | 0 
//      自增长id从1开始;
// 1. 测试函数ModifyRecordTime的输入参数和返回值: 
//      对starttime和endtime使用格式为空字符串 | 返回1
//      对starttime和endtime使用格式为除"yyyy-MM-dd hh:mm:ss"之外的其他形式: “yyyyMMdd hh:mm:ss”,”yyyy”还有2013/12/31 | 返回1
//      使用合理的值: starttime 设为“2013-12-31 01:00:00” endtime设为” 2013-12-31 22:00:00”, enable 设为0和1两种情况  | 返回0
//      使用正确的格式但错误的值: starttime 设为“2012-12-31 01:00:00” endtime设为” 2013-12-31 22:00:00”, enable 设为0和1两种情况 | 返回1
void SetRecordTimeTest::SetRecordTimeTest1()
{
    beforeSetRecordTimeTest();
    START_SETRECORDTIME_UNIT_TEST(Itest);
    int nRet = -1;

    nRet = Itest->ModifyRecordTime(1, "", "", true);
    QVERIFY2(1 == nRet,"step 1:return");
    nRet = Itest->ModifyRecordTime(1, "20131230 00:00:00", "20131230 23:59:59", true);
    QVERIFY2(1 == nRet,"step 2:return");
    nRet = Itest->ModifyRecordTime(1, "2013", "2014", true);
    QVERIFY2(1 == nRet,"step 3:return");
    nRet = Itest->ModifyRecordTime(1, "2013/12/31 00:00:00", "2013/12/31 23:59:59", true);
    QVERIFY2(1 == nRet,"step 4:return");
    nRet = Itest->ModifyRecordTime(1, "2013-12-31 00:00:00", "2013/12/31 23:59:59", true);
    QVERIFY2(1 == nRet,"step 5:return");
    nRet = Itest->ModifyRecordTime(1, "2012-12-31 00:00:00", "2013-12-31 23:59:59", true);
    QVERIFY2(1 == nRet,"step 6:return");
    nRet = Itest->ModifyRecordTime(1, "2013-12-31 00:00:00", "2013-12-31 23:59:59", true);
    QVERIFY2(0 == nRet,"step 7:return");
    nRet = Itest->ModifyRecordTime(1, "2013-12-31 00:00:00", "2013-12-31 23:59:59", false);
    QVERIFY2(0 == nRet,"step 8:return");

    END_SETRECORDTIME_UNIT_TEST(Itest);
}

// 2. 测试函数GetRecordTimeBydevId的输入参数和返回值: 
//      使用通道号0查询 | 返回默认的那组数据的id号1
//      使用不存在的通道号 200和-200查询 | 返回1空的QStringList
void SetRecordTimeTest::SetRecordTimeTest2()
{
    beforeSetRecordTimeTest();
    START_SETRECORDTIME_UNIT_TEST(Itest);
    QStringList strlist;
    //step1
    strlist = Itest->GetRecordTimeBydevId(1);
    QVERIFY2(strlist.size() == 28  ,"step 1:");
	bool bContains = true;
	int i;
	for (i = 0; i < strlist.size(); i++)
	{
		if (!strlist.contains(QString::number(i + 1)))
		{
			bContains = false;
		}
	}
    QVERIFY2(bContains,"step 1:");
    //step2
    strlist.clear();
    strlist = Itest->GetRecordTimeBydevId(200);
    QVERIFY2(strlist.size() ==  0,"step 2:");
    strlist = Itest->GetRecordTimeBydevId(-200);
    QVERIFY2(strlist.size() ==  0,"step 2:");
    END_SETRECORDTIME_UNIT_TEST(Itest);
}
// 3.  查询数据库记录是否成功
//     数据库中预置的一条记录0|0|0|”2013-12-31 00:00:00”|” 2013-12-31 23:59:59” | 0;
//        再增加2条记录0|1|1|”2014-01-01 00:00:00”|” 2014-01-01 23:59:59” | 0;
//        和1|0|1|”2014-01-01 00:00:00”|” 2014-01-01 23:59:59” | 0;
//     使用0通道为参数用GetRecordTimeBydevId查询通道0的id记录 | 返回结果是2条, 分别是"1""2"
//     使用1通道为参数用GetRecordTimeBydevId查询通道1的id记录 | 返回结果是1条,"3"
void SetRecordTimeTest::SetRecordTimeTest3()
{
    beforeSetRecordTimeTest();
    START_SETRECORDTIME_UNIT_TEST(Itest);
    QSqlDatabase db;
    if (QSqlDatabase::contains("SetRecordTimeTest"))
    {
        db = QSqlDatabase::database("SetRecordTimeTest"); 
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE","SetRecordTimeTest");
    }
    QString sAppPath = QCoreApplication::applicationDirPath();
    QString sDatabasePath = sAppPath + "/system.db";
    db.setDatabaseName(sDatabasePath);
    db.open();

    QSqlQuery query2(db);
    query2.prepare("insert into recordtime(chl_id ,schedule_id ,weekday ,starttime ,endtime,enable  ) values(:chl_id,:schedule_id,:weekday,:starttime,:endtime,:enable)");
    query2.bindValue(":chl_id"     ,2);
    query2.bindValue(":schedule_id" ,0);
    query2.bindValue(":weekday"    ,1);
    query2.bindValue(":starttime"  ,"2014-01-01 00:00:00");
    query2.bindValue(":endtime"    ,"2014-01-01 23:59:59");
    query2.bindValue(":enable"     ,0);

    query2.exec();
    db.close();
    QStringList strlist;

    //step1
    strlist = Itest->GetRecordTimeBydevId(1);
    QVERIFY2(strlist.size() == 28,"step 1:");

	bool bContains = true;
	int i;
	for (i = 0; i < strlist.size(); i++)
	{
		if (!strlist.contains(QString::number(i + 1)))
		{
			bContains = false;
		}
	}
	QVERIFY2(bContains,"step 1:");

    //step2
    strlist = Itest->GetRecordTimeBydevId(2);
	QVERIFY2(strlist.isEmpty(),"step 2");
//    QVERIFY2(strlist.size() == 1,"step 2:");
//    QVERIFY2(strlist[0]     == "29","step 2:");

    END_SETRECORDTIME_UNIT_TEST(Itest);
}
// 4. 使用非法参数设置GetRecordTimeInfo 函数;
//    参数recordtime_id 使用小于0的数; | 返回0条记录
//       recordtime_id 使用超出数据库实际最大id的数200  | 返回0条记录
//       recordtime_id 使用合理的数1  | 返回1条记录, 返回” chl_id”为0, “schedule_id”为0, “weekday”返回为0, “starttime”返回为” 2013-12-31 00:00:00”, endtime返回为” 2013-12-31 23:59:59”,enable返回0
void SetRecordTimeTest::SetRecordTimeTest4()
{
    beforeSetRecordTimeTest();
    START_SETRECORDTIME_UNIT_TEST(Itest);
    QVariantMap vMap;
    //step1
    vMap = Itest->GetRecordTimeInfo(-200);
    QVERIFY2(vMap.size() == 0,"step 1:return");
    vMap.clear();
    //step2
    vMap = Itest->GetRecordTimeInfo(200);
    QVERIFY2(vMap.size() == 0,"step 2:return");
    vMap.clear();
    //step3
    vMap = Itest->GetRecordTimeInfo(1);
    QVERIFY2(1 == vMap["chl_id"].toInt()                ,"step 3:return");
    QVERIFY2(0 == vMap["schedule_id"].toInt()            ,"step 3:return");
    QVERIFY2(0 == vMap["weekday"].toInt()               ,"step 3:return");
    QVERIFY2(vMap["starttime"] == "2013-12-31 00:00:00" ,"step 3:return");
    QVERIFY2(vMap["endtime"]   == "2013-12-31 23:59:59" ,"step 3:return");
    QVERIFY2(0 ==  vMap["enable"]                       ,"step 3:return");

    END_SETRECORDTIME_UNIT_TEST(Itest);
}
// 5. 修改值和查询值结果一致:
//       使用GetRecordTimeInfo查询id为1的录像时间记录; | 返回”chl_id”为0, “schedule_id”为0, “weekday”返回为0, “starttime”返回为” 2013-12-31 00:00:00”, endtime返回为” 2013-12-31 23:59:59”,enable返回0
//       使用ModifyRecordTime(使用正确的参数)来修改录像时间id为1的记录的开始时间为” 2014-01-01 00:00:00”结束时间为” 2014-01-01 23:59:59”的录像时间 | 函数返回0
//       用GetRecordTimeInfo查询id为1的录像时间记录; | 返回”chl_id”为0, “schedule_id”为0, “weekday”返回为0, “starttime”返回为” 2014-01-01 00:00:00”, endtime返回为” 2014-01-01 23:59:59”,enable返回0
void SetRecordTimeTest::SetRecordTimeTest5()
{
    beforeSetRecordTimeTest();
    START_SETRECORDTIME_UNIT_TEST(Itest);
    int nRet = -1;
    QVariantMap vMap;

    //step1
    vMap = Itest->GetRecordTimeInfo(1);
    QVERIFY2(1 == vMap["chl_id"].toInt()                ,"step 1:return");
    QVERIFY2(0 == vMap["schedule_id"].toInt()            ,"step 1:return");
    QVERIFY2(0 == vMap["weekday"].toInt()               ,"step 1:return");
    QVERIFY2(vMap["starttime"] == "2013-12-31 00:00:00" ,"step 1:return");
    QVERIFY2(vMap["endtime"]   == "2013-12-31 23:59:59" ,"step 1:return");
    QVERIFY2(0 ==  vMap["enable"]                       ,"step 1:return");
    //step2
    nRet = Itest->ModifyRecordTime(1, "2014-01-01 00:00:00", "2014-01-01 23:59:59", true);
    QVERIFY2(0 == nRet,"step 2:return");
    //step3
    vMap.clear();
    vMap = Itest->GetRecordTimeInfo(1);
    QVERIFY2(1 == vMap["chl_id"].toInt()                ,"step 3:return");
    QVERIFY2(0 == vMap["schedule_id"].toInt()            ,"step 3:return");
    QVERIFY2(0 == vMap["weekday"].toInt()               ,"step 3:return");
    QVERIFY2(vMap["starttime"] == "2014-01-01 00:00:00" ,"step 3:return");
    QVERIFY2(vMap["endtime"]   == "2014-01-01 23:59:59" ,"step 3:return");
    QVERIFY2(1 ==  vMap["enable"]                       ,"step 3:return");
    END_SETRECORDTIME_UNIT_TEST(Itest);
}

// 6. 修改值和查询值结果一致:
//       使用GetRecordTimeInfo查询id为1的录像时间记录; | 返回”chl_id”为0, “schedule_id”为0, “weekday”返回为0, “starttime”返回为” 2013-12-31 00:00:00”, endtime返回为” 2013-12-31 23:59:59”,enable返回0
//       使用ModifyRecordTime(使用错误的参数)来修改录像时间id为1的记录的开始时间为” 2012-01-01 00:00:00”结束时间为” 2014-01-01 23:59:59”的录像时间 | 函数返回0
//       用GetRecordTimeInfo查询id为1的录像时间记录; | 返回”chl_id”为0, “schedule_id”为0, “weekday”返回为0, “starttime”返回为” 2014-01-01 00:00:00”, endtime返回为” 2014-01-01 23:59:59”,enable返回0
void SetRecordTimeTest::SetRecordTimeTest6()
{
    beforeSetRecordTimeTest();
    START_SETRECORDTIME_UNIT_TEST(Itest);
    int nRet = -1;
    QVariantMap vMap;

    //step1
    vMap = Itest->GetRecordTimeInfo(1);
    QVERIFY2(1 == vMap["chl_id"].toInt()                ,"step 1:return");
    QVERIFY2(0 == vMap["schedule_id"].toInt()            ,"step 1:return");
    QVERIFY2(0 == vMap["weekday"].toInt()               ,"step 1:return");
    QVERIFY2(vMap["starttime"] == "2013-12-31 00:00:00" ,"step 1:return");
    QVERIFY2(vMap["endtime"]   == "2013-12-31 23:59:59" ,"step 1:return");
    QVERIFY2(0 ==  vMap["enable"]                       ,"step 1:return");
    //step2
    nRet = Itest->ModifyRecordTime(1, "2012-01-01 00:00:00", "2014-01-01 23:59:59", true);
    QVERIFY2(1 == nRet,"step 2:return");
    //step3
    vMap.clear();
    vMap = Itest->GetRecordTimeInfo(1);
    QVERIFY2(1 == vMap["chl_id"].toInt()                ,"step 3:return");
    QVERIFY2(0 == vMap["schedule_id"].toInt()            ,"step 3:return");
    QVERIFY2(0 == vMap["weekday"].toInt()               ,"step 3:return");
    QVERIFY2(vMap["starttime"] == "2013-12-31 00:00:00" ,"step 3:return");
    QVERIFY2(vMap["endtime"]   == "2013-12-31 23:59:59" ,"step 3:return");
    QVERIFY2(0 ==  vMap["enable"]                       ,"step 3:return");
    END_SETRECORDTIME_UNIT_TEST(Itest);
}
