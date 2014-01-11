#include "LocalPlayerSynPlaybackTest.h"
#include <guid.h>
#include "ILocalRecordSearch.h"
#include "ILocalPlayer.h"
#include <IEventRegister.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QStringList>

#define START_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(ii) 	ILocalPlayer * ii = NULL; \
    pcomCreateInstance(CLSID_LocalPlayer,NULL,IID_ILocalPlayer,(void **)&ii); \
    QVERIFY2(NULL != ii,"Create Local Player Syn_Playback instance");

#define END_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(ii) ii->Release(); 

void LocalPlayerSynPlaybackTest::beforeTest()
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("LocalPlayerSynPlaybackTest"))
    {
        db = QSqlDatabase::database("LocalPlayerSynPlaybackTest"); 
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE","LocalPlayerSynPlaybackTest");
    }
    QString sAppPath = QCoreApplication::applicationDirPath();
    QString sDatabasePath = sAppPath + "/system.db";
    db.setDatabaseName(sDatabasePath);
    db.open();

    QSqlQuery query1(db);
    query1.prepare("delete from general_setting");
    query1.exec();
    QSqlQuery query1_1(db);
    query1_1.prepare("update sqlite_sequence set seq=0 where name= 'general_setting'");
    query1_1.exec();

    QSqlQuery query2(db);
    query2.prepare("insert into general_setting(name,value) values(:name,:value)");
    query2.bindValue(":name","storage_usedisks");
    query2.bindValue(":value","D:");
    query2.exec();
    QSqlQuery query3(db);
    query3.prepare("insert into general_setting(name,value) values(:name,:value)");
    query3.bindValue(":name","storage_cover");
    query3.bindValue(":value","true");
    query3.exec();
    QSqlQuery query4(db);
    query4.prepare("insert into general_setting(name,value) values(:name,:value)");
    query4.bindValue(":name","storage_filesize");
    query4.bindValue(":value","128");
    query4.exec();
    QSqlQuery query5(db);
    query5.prepare("insert into general_setting(name,value) values(:name,:value)");
    query5.bindValue(":name","storage_reservedsize");
    query5.bindValue(":value","1024");
    query5.exec();
    db.close();
}

int LocalPlayerSynPlaybackTest::cbProc(QString evName, QVariantMap evMap, void *pUser)
{
    QVariantMap::const_iterator it;
    for (it= evMap.begin();it != evMap.end(); ++it )
    {
        QString sKey = it.key();
        QString sValue = it.value().toString();
        printf("%22s\t%-10s\n", sKey.toLatin1().data(),sValue.toLatin1().data());
    }
    return 0;
}

LocalPlayerSynPlaybackTest::LocalPlayerSynPlaybackTest()
{
    for (int i = 0; i < sizeof(m_PlaybackWnd) / sizeof(m_PlaybackWnd[0]); ++i)
    {
        m_PlaybackWnd[i].setParent(this);
    }
}

LocalPlayerSynPlaybackTest::~LocalPlayerSynPlaybackTest()
{
}

//测试用例
//前置条件:  一块硬盘,分为C,D,E,F区, 数据库general_setting表中默认使用D盘放置录像
//       D分区放置JAREC/2014-01-11/1000/CHL01/094717.avi和JAREC/2014-01-11/1000/CHL02/094717.avi以及
//       JAREC/2014-01-11/1001/CHL01/114717..avi和JAREC/2014-01-11/1000/CHL01/143429.avi
//       JAREC/2014-01-11/1000/CHL02/143429.avi
//       F分区放置JAREC/2014-01-11/1000/CHL01/124717.avi和JAREC/2014-01-11/1000/CHL02/124717.avi

//1. AddFileIntoPlayGroup参数以及返回值测试
//    注册事件:"GetRecordDate","GetRecordFile"和"SearchStop"
//    参数设置:start为” 2014-01-11 08:00:00”,end为” 2014-01-11 07:00:00”, 其他参数给正确值 | 返回3:失败
//    参数设置:start为” 2014-01-11 08:00:00”,end为” 2014-01-11 22:00:00”, 播放窗口指针给NULL  | 返回3:失败
//    参数设置:start为” 2014-01-11 08:00:00”,end为” 2014-01-11 22:00:00”, 其他参数给正确值  | 返回0, 添加成功
//    参数设置:start为” 2014-01-11 08:00:00”,end为” 2014-01-11 22:00:00”, 其他参数和4的完全一样 | 返回2, 添加失败，窗口已经被占用
void LocalPlayerSynPlaybackTest::TestCase1()
{
    beforeTest();
    START_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
    int nRet = 0;
    QString evName("GetRecordDate");
    IEventRegister *pRegist = NULL;
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();

    evName = "GetRecordFile";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    evName = "SearchStop";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    //steps
    QStringList sFileList;
    sFileList<<"D:/JAREC/2014-01-11/1000/CHL01/094717.avi"
             <<"D:/JAREC/2014-01-11/1001/CHL01/114717.avi"
             <<"D:/JAREC/2014-01-11/1000/CHL01/143429.avi";
    QDateTime sStart = QDateTime::fromString("2014-01-11 08:00:00", "yyyy-MM-dd hh:mm:ss");
    QDateTime sEnd = QDateTime::fromString("2014-01-11 07:00:00", "yyyy-MM-dd hh:mm:ss");
    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[0].ui->widget_display , sStart, sEnd);
    QVERIFY2(3 == nRet, "AddFileIntoPlayGroup return Error! : step1");

    sStart = QDateTime::fromString("2014-01-11 08:00:00", "yyyy-MM-dd hh:mm:ss");
    sEnd   = QDateTime::fromString("2014-01-11 22:00:00", "yyyy-MM-dd hh:mm:ss"); 
    nRet = Itest->AddFileIntoPlayGroup(sFileList, NULL, sStart, sEnd);
    QVERIFY2(3 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    sStart = QDateTime::fromString("2014-01-11 08:00:00", "yyyy-MM-dd hh:mm:ss");
    sEnd   = QDateTime::fromString("2014-01-11 22:00:00", "yyyy-MM-dd hh:mm:ss"); 
    nRet = Itest->AddFileIntoPlayGroup(sFileList, m_PlaybackWnd[2].ui->widget_display, sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step3");

    sStart = QDateTime::fromString("2014-01-11 08:00:00", "yyyy-MM-dd hh:mm:ss");
    sEnd   = QDateTime::fromString("2014-01-11 22:00:00", "yyyy-MM-dd hh:mm:ss"); 
    nRet = Itest->AddFileIntoPlayGroup(sFileList,  m_PlaybackWnd[2].ui->widget_display , sStart, sEnd);
    QVERIFY2(2 == nRet, "AddFileIntoPlayGroup return Error! : step4");
    END_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
}


//2. SetSynGroupNum测试参数值和返回值
//    注册事件:"GetRecordDate","GetRecordFile"和"SearchStop"
//    参数给-3  |  返回1设置失败
//    参数给0   |  返回1设置失败
//    参数给4   |  返回0设置成功
//    参数给100 |  返回1设置失败
void LocalPlayerSynPlaybackTest::TestCase2()
{
    beforeTest();
    START_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
    int nRet = 0;
    QString evName("GetRecordDate");
    IEventRegister *pRegist = NULL;
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();

    evName = "GetRecordFile";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    evName = "SearchStop";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    //steps:
    nRet = Itest->SetSynGroupNum(-3);
    QVERIFY2(1 == nRet, "SetSynGroupNum return Error! :step1");
    nRet = Itest->SetSynGroupNum(0);
    QVERIFY2(1 == nRet, "SetSynGroupNum return Error! :step2");
    nRet = Itest->SetSynGroupNum(4);
    QVERIFY2(0  == nRet, "SetSynGroupNum return Error! :step3");
    nRet = Itest->SetSynGroupNum(100);
    QVERIFY2(1  == nRet, "SetSynGroupNum return Error! :step4");
    END_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
}

//3. 设置同步组数量,看有没有对添加到组进行限制
//    注册事件:"GetRecordDate","GetRecordFile"和"SearchStop"
//    SetSynGroupNum参数设置为2;  |  返回0:成功
//    连续2次调用函数AddFileIntoPlayGroup 中设置start为” 2014-01-11 08:00:00”,end为” 2014-01-11 22:00:00”, 其他参数给正确值 | 返回0:添加成功
//    再调用函数AddFileIntoPlayGroup 中设置start为” 2014-01-11 08:00:00”,end为” 2014-01-11 22:00:00”, 其他参数给正确值 | 返回1:通道组已满
void LocalPlayerSynPlaybackTest::TestCase3()
{
    beforeTest();
    START_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
    int nRet = 0;
    QString evName("GetRecordDate");
    IEventRegister *pRegist = NULL;
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();

    evName = "GetRecordFile";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    evName = "SearchStop";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    //steps:
    QStringList sFileList;
    sFileList<<"D:/JAREC/2014-01-11/1000/CHL01/094717.avi"
        <<"D:/JAREC/2014-01-11/1001/CHL01/114717.avi"
        <<"D:/JAREC/2014-01-11/1000/CHL01/143429.avi";

    nRet = Itest->SetSynGroupNum(2);
    QVERIFY2(0 == nRet, "SetSynGroupNum return Error! : step1");

    QDateTime sStart = QDateTime::fromString("2014-01-11 08:00:00", "yyyy-MM-dd hh:mm:ss");
    QDateTime sEnd   = QDateTime::fromString("2014-01-11 22:00:00", "yyyy-MM-dd hh:mm:ss");
    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[0].ui->widget_display , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[1].ui->widget_display , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[2].ui->widget_display , sStart, sEnd);
    QVERIFY2(1 == nRet, "AddFileIntoPlayGroup return Error! : step3");

    END_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
}

//4. 测试播放, 暂停, 暂停后继续, 停止
//    注册事件:"GetRecordDate","GetRecordFile"和"SearchStop"
//    SetSynGroupNum参数设置为2;  |  返回0:成功
//    连续3次调用函数AddFileIntoPlayGroup 中设置start为” 2014-01-11 08:00:00”,end为” 2014-01-11 22:00:00”, 其他参数给正确值 | 前2次返回0:添加成功
//最后一次返回1: 通道组已满
//    调用GroupPlay  | 返回0
//    10分钟后调用GroupPause  | 返回0
//    10分钟后调用GroupContinue  | 返回0
//    10分钟后调用GroupStop  | 返回0
void LocalPlayerSynPlaybackTest::TestCase4()
{
    beforeTest();
    START_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
    int nRet = 0;
    QString evName("GetRecordDate");
    IEventRegister *pRegist = NULL;
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();

    evName = "GetRecordFile";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    evName = "SearchStop";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    //steps:
    QStringList sFileList;
    sFileList<<"D:/JAREC/2014-01-11/1000/CHL01/094717.avi"
        <<"D:/JAREC/2014-01-11/1001/CHL01/114717.avi"
        <<"D:/JAREC/2014-01-11/1000/CHL01/143429.avi";

    nRet = Itest->SetSynGroupNum(2);
    QVERIFY2(0 == nRet, "SetSynGroupNum return Error! : step1");

    QDateTime sStart = QDateTime::fromString("2014-01-11 08:00:00", "yyyy-MM-dd hh:mm:ss");
    QDateTime sEnd   = QDateTime::fromString("2014-01-11 22:00:00", "yyyy-MM-dd hh:mm:ss");
    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[0].ui->widget_display , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[1].ui->widget_display , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[2].ui->widget_display , sStart, sEnd);
    QVERIFY2(1 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->GroupPlay();
    QVERIFY2(0 == nRet, "GroupPlay return Error! : step3");

    QTest::qWait(10000*60);

    nRet = Itest->GroupPause();
    QVERIFY2(0 == nRet, "GroupPause return Error! : step4");

    QTest::qWait(10000*60);

    nRet = Itest->GroupContinue();
    QVERIFY2(0 == nRet, "GroupContinue return Error! : step5");

    QTest::qWait(10000*60);

    nRet = Itest->GroupStop();
    QVERIFY2(0 == nRet, "GroupStop return Error! : step6");
    END_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
}

//5. 测试播放停止后再启动播放是否成功
//   SetSynGroupNum 设置同步组数量为3  |  返回0:成功
//   3次调用AddFileIntoPlayGroup 中设置start为” 2014-01-11 08:00:00”,end为” 2014-01-11 22:00:00”, 其他参数给正确值 | 3次返回0:添加成功
//   调用GroupPlay	 | 返回0
//   10分钟后调用GroupStop	| 返回0
//   5分钟后调用GroupPlay	 | 返回0
//   10分钟后调用GroupStop	| 返回0
void LocalPlayerSynPlaybackTest::TestCase5()
{
    beforeTest();
    START_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
    int nRet = 0;
    QString evName("GetRecordDate");
    IEventRegister *pRegist = NULL;
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();

    evName = "GetRecordFile";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    evName = "SearchStop";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    //steps:
    QStringList sFileList;
    sFileList<<"D:/JAREC/2014-01-11/1000/CHL01/094717.avi"
        <<"D:/JAREC/2014-01-11/1001/CHL01/114717.avi"
        <<"D:/JAREC/2014-01-11/1000/CHL01/143429.avi";

    nRet = Itest->SetSynGroupNum(3);
    QVERIFY2(0 == nRet, "SetSynGroupNum return Error! : step1");

    QDateTime sStart = QDateTime::fromString("2014-01-11 08:00:00", "yyyy-MM-dd hh:mm:ss");
    QDateTime sEnd   = QDateTime::fromString("2014-01-11 22:00:00", "yyyy-MM-dd hh:mm:ss");
    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[0].ui->widget_display  , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[1].ui->widget_display  , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[2].ui->widget_display  , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->GroupPlay();
    QVERIFY2(0 == nRet, "GroupPlay return Error! : step3");

    QTest::qWait(10000*60);

    nRet = Itest->GroupStop();
    QVERIFY2(0 == nRet, "GroupStop return Error! : step4");

    QTest::qWait(10000*60);

    nRet = Itest->GroupPlay();
    QVERIFY2(0 == nRet, "GroupPlay return Error! : step5");

    QTest::qWait(10000*60);

    nRet = Itest->GroupStop();
    QVERIFY2(0 == nRet, "GroupStop return Error! : step6");
    END_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
}

//6  测试快速/慢速播放是否成功
//   SetSynGroupNum 设置同步组数量为3	|  返回0:成功
//   3次调用AddFileIntoPlayGroup 中设置start为” 2014-01-11 08:00:00”,end为” 2014-01-11 22:00:00”, 其他参数给正确值	3次
//   	|  返回0:添加成功
//   调用GroupPlay  |  返回0
//   5分钟后调用GroupSpeedFast2倍速	|  返回0
//   5分钟后调用GroupSpeedFast8倍速	|  返回0
//   5分钟后调用GroupSpeedSlow设置1/2倍速	|  返回0
//   5分钟后调用GroupSpeedNormal设置正常播放	|  	返回0
//   5分钟后GroupStop停止播放	|  返回0
void LocalPlayerSynPlaybackTest::TestCase6()
{
    beforeTest();
    START_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
    int nRet = 0;
    QString evName("GetRecordDate");
    IEventRegister *pRegist = NULL;
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();

    evName = "GetRecordFile";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    evName = "SearchStop";
    nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    nRet = pRegist->registerEvent(evName, cbProc, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
    pRegist->Release();
    //steps:
    QStringList sFileList;
    sFileList<<"D:/JAREC/2014-01-11/1000/CHL01/094717.avi"
        <<"D:/JAREC/2014-01-11/1001/CHL01/114717.avi"
        <<"D:/JAREC/2014-01-11/1000/CHL01/143429.avi";

    nRet = Itest->SetSynGroupNum(3);
    QVERIFY2(0 == nRet, "SetSynGroupNum return Error! : step1");

    QDateTime sStart = QDateTime::fromString("2014-01-11 08:00:00", "yyyy-MM-dd hh:mm:ss");
    QDateTime sEnd   = QDateTime::fromString("2014-01-11 22:00:00", "yyyy-MM-dd hh:mm:ss");
    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[0].ui->widget_display , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[1].ui->widget_display , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->AddFileIntoPlayGroup(sFileList,m_PlaybackWnd[2].ui->widget_display , sStart, sEnd);
    QVERIFY2(0 == nRet, "AddFileIntoPlayGroup return Error! : step2");

    nRet = Itest->GroupPlay();
    QVERIFY2(0 == nRet, "GroupPlay return Error! : step3");

    QTest::qWait(1000*60);

    nRet = Itest->GroupSpeedFast(2);
    QVERIFY2(0 == nRet, "GroupSpeedFast return Error! : step4");
    QTest::qWait(5000*60);
    nRet = Itest->GroupSpeedFast(8);
    QVERIFY2(0 == nRet, "GroupSpeedFast return Error! : step5");

    QTest::qWait(5000*60);
    nRet = Itest->GroupSpeedSlow(2);
    QVERIFY2(0 == nRet, "GroupSpeedSlow return Error! : step6");

    QTest::qWait(5000*60);
    nRet = Itest->GroupSpeedNormal();
    QVERIFY2(0 == nRet, "GroupSpeedNormal return Error! : step7");

    QTest::qWait(5000*60);
    nRet = Itest->GroupStop();
    QVERIFY2(0 == nRet, "GroupStop return Error! : step8");

    END_LOCALPLAYERSYNPLAYBACK_UNIT_TEST(Itest);
}







