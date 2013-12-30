#include "DisksSettingTest.h"
#include <guid.h>
#include <IDisksSetting.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>


#define START_DISKSETTING_UNIT_TEST(ii) 	IDisksSetting * ii = NULL; \
    pcomCreateInstance(CLSID_CommonLibPlugin, NULL, IID_IDiskSetting,(void **)&ii); \
    QVERIFY2(NULL != ii,"Create DisksSetting instance");

#define END_DISKSETTING_UNIT_TEST(ii) ii->Release(); 

DisksSettingTest::DisksSettingTest()
{

}
DisksSettingTest::~DisksSettingTest()
{

}


void DisksSettingTest::beforeDisksSettingTest()
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("DisksSettingTest"))
    {
        db = QSqlDatabase::database("DisksSettingTest"); 
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE","DisksSettingTest");
    }
    QString sAppPath = QCoreApplication::applicationDirPath();
    QString sDatabasePath = sAppPath + "/system.db";
    db.setDatabaseName(sDatabasePath);
    db.open();

    QSqlQuery query1(db);
    query1.prepare("delete from general_setting");
    query1.exec();

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
//测试用例:
// 前置：重置数据库，数据库中创建了disks_setting表，数据库中仅有一条记录：D:|true|128|1024|
// 两块分区数目不同的硬盘A,B;A分为C,D,E,F区;B分为C,D,E,F,G区, 数据库默认都只使用D盘放置录像;录像文件默认最大512MB

// 1. 使用硬盘A,setUseDisks设置录像使用的磁盘分区, 使用格式为: "DD:",”D;”,”G:”|返回E_PARAMETER_ERROR ;
//           使用正确格式如"E:", 返回S_OK 
void DisksSettingTest::DisksSettingTest1()
{
    beforeDisksSettingTest();
    START_DISKSETTING_UNIT_TEST(Itest);
    int nRet = -1;
    // step1
    nRet = Itest->setUseDisks("DD:");
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 1:return");
    nRet = Itest->setUseDisks("D;");
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 1:return");
    nRet = Itest->setUseDisks("G:");
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 1:return");
    // step2
    nRet = Itest->setUseDisks("E:");
    QVERIFY2(IDisksSetting::OK == nRet,"step 2:return");
    END_DISKSETTING_UNIT_TEST(Itest);
}

// 2.  使用硬盘A,setUseDisks设置录像使用的磁盘为E:F:, getUseDisks查询返回并比对设置值与传出值是否一致
//    | 设置返回S_OK, 查询返回E:F:
void DisksSettingTest::DisksSettingTest2()
{
    beforeDisksSettingTest();
    START_DISKSETTING_UNIT_TEST(Itest);
    int nRet = -1;
    //step1
    nRet = Itest->setUseDisks("E:F:");
    QVERIFY2(IDisksSetting::OK == nRet,"step 1:return");
    //step2
    QString sRet;
    nRet = Itest->getUseDisks(sRet);
    QVERIFY2(sRet == "E:F:" ,"step 2:return");
    QVERIFY2(nRet == IDisksSetting::OK ,"step 2:return");
    END_DISKSETTING_UNIT_TEST(Itest);
}
// 3.  使用硬盘A, getEnableDisks获取系统可用的磁盘分区 | 返回结果为"D:E:F:"
//     使用硬盘B, getEnableDisks获取系统可用的磁盘分区 | 返回结果为"D:E:F:G:" 
void DisksSettingTest::DisksSettingTest3()
{
    beforeDisksSettingTest();
    START_DISKSETTING_UNIT_TEST(Itest);
    int nRet = -1;
    QString sRet;
    //step1
    nRet = Itest->getEnableDisks(sRet);
    QVERIFY2(sRet == "C:D:E:F:","step 1:return"); //硬盘A
    //QVERIFY2(sRet == "D:E:F:G:","step 1:return"); //硬盘B
    QVERIFY2(IDisksSetting::OK == nRet,"step 1:return");
    
    END_DISKSETTING_UNIT_TEST(Itest);
}
// 4. 使用硬盘A,setFilePackageSize设置录像文件包大小为小于0的数-200 | 返回E_PARAMETER_ERROR
//    使用硬盘A,setFilePackageSize设置录像文件包大小为大于默认最大值的700MB时 | 返回E_PARAMETER_ERROR
//    使用硬盘A,setFilePackageSize设置录像文件包大小为500MB  | 返回S_OK 
void DisksSettingTest::DisksSettingTest4()
{
    beforeDisksSettingTest();
    START_DISKSETTING_UNIT_TEST(Itest);
    int nRet = -1;
    int nFileSize = -200;
    nRet = Itest->setFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 1:return");
    nFileSize = 0;
    nRet = Itest->setFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 2:return");
    nFileSize = 700;
    nRet = Itest->setFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 3:return");
    nFileSize = 500;
    nRet = Itest->setFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::OK == nRet,"step 4:return");
    END_DISKSETTING_UNIT_TEST(Itest);
}
// 5. 使用硬盘A,setFilePackageSize设置录像文件包大小为500MB, 和getFilePackageSize函数参数传出值比较 | setFilePackageSize返回S_OK , 传出500
//    使用硬盘A,setFilePackageSize设置录像文件包大小为0MB, 和getFilePackageSize函数参数传出值比较 | setFilePackageSize返回E_PARAMETER_ERROR , 传出之前的值500
//    使用硬盘A,setFilePackageSize设置录像文件包大小为-200MB, 和getFilePackageSize函数参数传出值比较 | setFilePackageSize返回E_PARAMETER_ERROR , 传出之前的值500
void DisksSettingTest::DisksSettingTest5()
{
    beforeDisksSettingTest();
    START_DISKSETTING_UNIT_TEST(Itest);
    int nRet = -1;
    int nFileSize = 500;
    //step1
    nRet = Itest->setFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::OK == nRet,"step 1:return");
    nFileSize = 0;
    nRet = Itest->getFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::OK == nRet,"step 1:return");
    QVERIFY2(nFileSize == 500,"step 1:return");
    //step2
    nFileSize = 0;
    nRet = Itest->setFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 2:return");
    nFileSize = 0;
    nRet = Itest->getFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::OK == nRet,"step 2:return");
    QVERIFY2(nFileSize == 500,"step 2:return");

    //step3
    nFileSize = -200;
    nRet = Itest->setFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 2:return");
    nFileSize = 0;
    nRet = Itest->getFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::OK == nRet,"step 2:return");
    QVERIFY2(nFileSize == 500,"step 2:return");
    END_DISKSETTING_UNIT_TEST(Itest);
}

// 6.  使用硬盘A,setLoopRecording设置循环录像bcover为false, 调用getLoopRecording 获取当前状态,然后
//     调用setLoopRecording开启循环录像, 调用getLoopRecording 获取当前状态 | 第二次调用setLoopRecording设置的true与
//     getLoopRecording 返回值一致
void DisksSettingTest::DisksSettingTest6()
{
    beforeDisksSettingTest();
    START_DISKSETTING_UNIT_TEST(Itest);
    int nRet = -1;
    bool bRet = false;
    nRet = Itest->setLoopRecording(false);
    QVERIFY2(IDisksSetting::OK == nRet,"step 1:return");
    bRet = Itest->getLoopRecording();
    QVERIFY2(false == bRet,"step 2:return");

    nRet = Itest->setLoopRecording(true);
    QVERIFY2(IDisksSetting::OK == nRet,"step 3:return");
    bRet = Itest->getLoopRecording();
    QVERIFY2(true == bRet,"step 4:return");
    END_DISKSETTING_UNIT_TEST(Itest);
}
// 7.  使用硬盘A的D分区, D盘大小194560MB, 可用空间178176MB;
//     setDiskSpaceReservedSize设置磁盘保留空间大小, 参数设为-2000和0  | 返回E_PARAMETER_ERROR
//     参数设为大于磁盘分区大小的数: 394560MB  | 返回E_PARAMETER_ERROR
//     参数设置为合理的数550MB  | 返回S_OK
//     参数设为大于磁盘分区可用大小的数: 394560MB  | 返回E_PARAMETER_ERROR
void DisksSettingTest::DisksSettingTest7()
{
    beforeDisksSettingTest();
    START_DISKSETTING_UNIT_TEST(Itest);
    int nRet = -1;
    int spacereservedsize = -2000;
    nRet = Itest->setDiskSpaceReservedSize(spacereservedsize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 1:return");

    spacereservedsize =  0;
    nRet = Itest->setDiskSpaceReservedSize(spacereservedsize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 2:return");

    spacereservedsize =  394560;
    nRet = Itest->setDiskSpaceReservedSize(spacereservedsize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 3:return");

    spacereservedsize =  550;
    nRet = Itest->setDiskSpaceReservedSize(spacereservedsize);
    QVERIFY2(IDisksSetting::OK == nRet,"step 4:return");

    spacereservedsize =  394560;
    nRet = Itest->setDiskSpaceReservedSize(spacereservedsize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 5:return");
    END_DISKSETTING_UNIT_TEST(Itest);
}
//8. 使用硬盘A的D分区, D盘大小194560MB, 可用空间178176MB;
//   setDiskSpaceReservedSize设置合理的数2048MB, 和getDiskSpaceReservedSize传出值比较 | 前者返回S_OK, 后者传出值为2048
//   setDiskSpaceReservedSize设置不合理的数-2000MB, 和getDiskSpaceReservedSize传出值比较 | 前者返回E_PARAMETER_ERROR, 后者传出值为之前设置的2048
void DisksSettingTest::DisksSettingTest8()
{
    beforeDisksSettingTest();
    START_DISKSETTING_UNIT_TEST(Itest);
    int nRet = -1;
    int spacereservedsize = 2048;
    //step1
    nRet = Itest->setDiskSpaceReservedSize(spacereservedsize);
    QVERIFY2(IDisksSetting::OK == nRet,"step 1:return");
    spacereservedsize = 0;
    nRet = Itest->getDiskSpaceReservedSize(spacereservedsize);
    QVERIFY2(IDisksSetting::OK == nRet,"step 1:return");
    QVERIFY2(spacereservedsize == 2048,"step 1:return");
    //step2
    spacereservedsize = -2000;
    nRet = Itest->setDiskSpaceReservedSize(spacereservedsize);
    QVERIFY2(IDisksSetting::E_PARAMETER_ERROR == nRet,"step 2:return");
    spacereservedsize = 0;
    nRet = Itest->getDiskSpaceReservedSize(spacereservedsize);
    QVERIFY2(IDisksSetting::OK == nRet,"step 2:return");
    QVERIFY2(spacereservedsize == 2048,"step 2:return");
    END_DISKSETTING_UNIT_TEST(Itest);
}
