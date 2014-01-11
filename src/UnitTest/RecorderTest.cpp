#include "RecorderTest.h"
#include <guid.h>
#include <IDisksSetting.h>
#include <IRecorder.h>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <QDateTime>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include "netlib.h"
#pragma comment(lib,"netlib.lib")

#define START_RECORDER_UNIT_TEST(ii, iDiskSetting) 	IRecorder * ii = NULL; IDisksSetting *iDiskSetting = NULL;\
   pcomCreateInstance(CLSID_Recorder, NULL, IID_IRecorder,(void **)&ii);\
   pcomCreateInstance(CLSID_CommonLibPlugin, NULL, IID_IDiskSetting,(void **)&iDiskSetting);\
    QVERIFY2(NULL != ii,"Create Recorder instance");\
    QVERIFY2(NULL != iDiskSetting,"Create commonlib instance");\

#define END_RECORDER_UNIT_TEST(ii, iDiskSetting) ii->Release(); iDiskSetting->Release();\


RecorderTest::RecorderTest()
{

}
RecorderTest::~RecorderTest()
{

}
typedef struct test_nalu_header{
    unsigned long flag;
    unsigned long size;
    unsigned long isider;  //1 是I帧, 0x02是P帧, 0x00是音频帧
}NALU_HEADER_t;

void RecorderTest::beforeRecorderTest()
{
    QSqlDatabase db;
    if (QSqlDatabase::contains("RecorderTest"))
    {
        db = QSqlDatabase::database("RecorderTest"); 
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE","RecorderTest");
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
// 前置：重置数据库，数据库中创建了disks_setting表，数据库中仅有一条记录：D:|true|128|1024|; 程序Debug目录下.h264文件: CIF_12fps_128kbps.h264
// 1.测试最后保留空间大小, 录像时用SetDevInfo改写设置,测试文件写入大小和路径: 
//      getEnableDisks获取可用分区	返回值为IDisksSetting::OK | 输出值为” C:D:E:F:”
//      setUseDisks函数设置使用E:F:盘存储录像文件	| 返回值为IDisksSetting::OK;
//      getUseDisks验证使用磁盘为E:F: |	返回值为IDisksSetting::OK; 输出值为”E:F:”
//      setFilePackageSize设置录像文件包大小为200MB	| 返回值为IDisksSetting::OK;
//      getFilePackageSize获取录像文件包大小的设置	| 返回值为IDisksSetting::OK;输出值为”200”
//      setLoopRecording设置为循环录像	            | 返回值为IDisksSetting::OK;
//      getLoopRecording验证为循环录像	            | 返回值为true
//      setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB	 | 返回值为IDisksSetting::OK;
//      getDiskSpaceReservedSize验证上一步设置值	| 返回值为IDisksSetting::OK;输出值为”5000”
//      调用SetDevInfo设置设备名为1000和通道数1     | 返回值为IRecorder:OK
//      调用Start()开始录像	                    | 返回值为IRecorder::OK
//      调用InputFrame ,设置参数正确	            | 返回值为IRecorder:OK, 在E盘下\REC\”日期”\1000\CHL01目录下有录像
//      在开始往F:写数据一定次数后, 调用SetDevInfo设置设备名为1001和通道数1  | 返回值为IRecorder:OK,在F:\REC\”日期”\1000\CHL01目录下有录像文件
//      继续将读取到的帧数据通过InputFrame写入 ,重复多次 | 返回值为IRecorder:OK, 在F:\REC\”日期”\1001\CHL01目录下有录像文件
//      调用Stop()停止     | 返回值为IRecorder::OK, E盘录像文件大小均为200MB±3MB(除文件名最大的外) ,E盘剩余保留空间5000MB±200MB
void RecorderTest::RecorderTest1()
{
    beforeRecorderTest();
    START_RECORDER_UNIT_TEST(Itest, pDiskSetting);
    int nRet = -1;
    int nFileSize = 0;
    QString sDiskEnabled;
    int nDiskSpaceReservedSize = 0 ;

    nRet = pDiskSetting->getEnableDisks(sDiskEnabled);
    QVERIFY2(IDisksSetting::OK == nRet  ,"getEnableDisks :return");
    QVERIFY2(sDiskEnabled == "C:D:E:F:" ,"getEnableDisks :out");
    nRet = pDiskSetting->setUseDisks("E:F:");
    QVERIFY2(IDisksSetting::OK == nRet  ,"setUseDisks :return");
    sDiskEnabled.clear();
    nRet = pDiskSetting->getUseDisks(sDiskEnabled);
    QVERIFY2(IDisksSetting::OK == nRet  ,"getUseDisks :return");
    QVERIFY2(sDiskEnabled == "E:F:"     ,"getUseDisks :out");
    nRet = pDiskSetting->setFilePackageSize(200);
    QVERIFY2(IDisksSetting::OK == nRet  ,"setFilePackageSize :return");
    nRet = pDiskSetting->getFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::OK == nRet  ,"getFilePackageSize :return");
    QVERIFY2(200 == nFileSize           ,"getFilePackageSize :out");
    nRet = pDiskSetting->setLoopRecording(true);
    QVERIFY2(IDisksSetting::OK == nRet  ,"setLoopRecording :return");
    bool bIsLoopRecord = pDiskSetting->getLoopRecording();
    QVERIFY2(true == bIsLoopRecord      ,"getLoopRecording :return");
    nRet = pDiskSetting->setDiskSpaceReservedSize(5000);
    QVERIFY2(IDisksSetting::OK == nRet  ,"setDiskSpaceReservedSize :return");
    nRet = pDiskSetting->getDiskSpaceReservedSize(nDiskSpaceReservedSize);
    QVERIFY2(IDisksSetting::OK == nRet  ,"getDiskSpaceReservedSize :return");
    QVERIFY2(5000 == nDiskSpaceReservedSize,"getDiskSpaceReservedSize :out");
    
    QString sDevName("1000");
    nRet = Itest->SetDevInfo(sDevName,  1);
    QVERIFY2(IRecorder::OK == nRet  ,"SetDevInfo :return");
    nRet = Itest->Start();
    QVERIFY2(IRecorder::OK == nRet,"Start() :return");

    FILE *pFile = NULL;
    QString sFileName = QCoreApplication::applicationDirPath() + "/CIF_12fps_128kbps.h264";
    
    int   nlen = 0;
    char  data[1280*720];
    uint  nTimes = 0;
    uint  nTotalLoopTimes = 0;
    unsigned long long   i64FreeBytesAvailableOfE = 0;
    unsigned long long   i64TotalNumberOfBytesOfE = 0;
    unsigned long long   i64TotalNumberOfFreeBytesOfE = 0;
    unsigned long long   i64FreeBytesAvailableOfF = 0;
    unsigned long long   i64TotalNumberOfBytesOfF = 0;
    unsigned long long   i64TotalNumberOfFreeBytesOfF = 0;
    GetDiskFreeSpaceExQ("E:\\"
        , &i64FreeBytesAvailableOfE
        , &i64TotalNumberOfBytesOfE
        , &i64TotalNumberOfFreeBytesOfE);
    GetDiskFreeSpaceExQ("F:\\"
        , &i64FreeBytesAvailableOfF
        , &i64TotalNumberOfBytesOfF
        , &i64TotalNumberOfFreeBytesOfF);
    nTotalLoopTimes = (uint)( ((float)i64FreeBytesAvailableOfE/1024/1024 + (float)i64FreeBytesAvailableOfF/1024/1024 - 10000) / 4.58);
    QVERIFY2 (NULL != (pFile = fopen(sFileName.toAscii().data(),"rb")), "File Open Error");

    NALU_HEADER_t nhead;
    QVariantMap frameInfo ;
    uint nTmp = (uint) (((float)i64FreeBytesAvailableOfE/1024/1024 - 5000) /4.58 );
    bool bIsDevInfoSetFlag = false;
    while (nTimes < nTotalLoopTimes)
    {
        if (feof(pFile))
        {
            rewind(pFile);
            ++nTimes;
        }
        if(nTimes > nTmp && !bIsDevInfoSetFlag)
        {
            bIsDevInfoSetFlag = true;
            sDevName.clear();
            sDevName = "1001";
            nRet = Itest->SetDevInfo(sDevName,  1);
            QVERIFY2(IRecorder::OK == nRet  ,"SetDevInfo :return");
        }
        memset(&nhead,0,sizeof(NALU_HEADER_t));
        memset(data,0,sizeof(data));
        if ((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) <= 0)
            continue;
        QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        frameInfo.insert("frametype", QVariant((uint)nhead.isider)) ;
        frameInfo.insert("data", QVariant(data));
        frameInfo.insert("length", QVariant((uint)nhead.size));
        frameInfo.insert("channel", QVariant(1));
        quint64 u64_tTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        frameInfo.insert("pts", QVariant(u64_tTime));
        
        nRet = Itest->InputFrame(frameInfo);
        QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
    }

    fclose(pFile);
    pFile=NULL;
    nRet = Itest->Stop();
    QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

    END_RECORDER_UNIT_TEST(Itest, pDiskSetting);
}

// 2. 录像时不通过SetDevInfo改设置, 测试录像文件写入大小和路径以及最后保留空间大小
//getEnableDisks获取可用分区	         |  返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用E:F:盘存储录像文件	 |  返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为E:F:	         |  返回值为IDisksSetting::OK; 输出值为”E:F:”
//setFilePackageSize设置录像文件包大小为200MB|	返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置	  |返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为循环录像	         | 返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像	         | 返回值为true
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB	| 返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值  |	返回值为IDisksSetting::OK;输出值为”5000”
//调用SetDevInfo设置设备名为2000和通道数4     | 返回值为IRecorder:OK
//调用Start()开始录像	                     |  返回值为IRecorder::OK

//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入 ,重复n次, (n与测试者磁盘可用空间大小有关)|	返回值为IRecorder:OK
//调用Stop()停止,                         |  返回值为IRecorder::OK, 录像文件大小均为200MB左右,在EF盘有录像,E盘保留空间为5000MB左右

void RecorderTest::RecorderTest2()
{
    beforeRecorderTest();
    START_RECORDER_UNIT_TEST(Itest, pDiskSetting);
    int nRet = -1;
    int nFileSize = 0;
    QString sDiskEnabled;
    int nDiskSpaceReservedSize = 0 ;

    nRet = pDiskSetting->getEnableDisks(sDiskEnabled);
    QVERIFY2(IDisksSetting::OK == nRet  ,"getEnableDisks :return");
    QVERIFY2(sDiskEnabled == "C:D:E:F:" ,"getEnableDisks :out");
    nRet = pDiskSetting->setUseDisks("E:F:");
    QVERIFY2(IDisksSetting::OK == nRet  ,"setUseDisks :return");
    sDiskEnabled.clear();
    nRet = pDiskSetting->getUseDisks(sDiskEnabled);
    QVERIFY2(IDisksSetting::OK == nRet  ,"getUseDisks :return");
    QVERIFY2(sDiskEnabled == "E:F:"     ,"getUseDisks :out");
    nRet = pDiskSetting->setFilePackageSize(200);
    QVERIFY2(IDisksSetting::OK == nRet  ,"setFilePackageSize :return");
    nRet = pDiskSetting->getFilePackageSize(nFileSize);
    QVERIFY2(IDisksSetting::OK == nRet  ,"getFilePackageSize :return");
    QVERIFY2(200 == nFileSize           ,"getFilePackageSize :out");
    nRet = pDiskSetting->setLoopRecording(true);
    QVERIFY2(IDisksSetting::OK == nRet  ,"setLoopRecording :return");
    bool bIsLoopRecord = pDiskSetting->getLoopRecording();
    QVERIFY2(true == bIsLoopRecord      ,"getLoopRecording :return");
    nRet = pDiskSetting->setDiskSpaceReservedSize(5000);
    QVERIFY2(IDisksSetting::OK == nRet  ,"setDiskSpaceReservedSize :return");
    nRet = pDiskSetting->getDiskSpaceReservedSize(nDiskSpaceReservedSize);
    QVERIFY2(IDisksSetting::OK == nRet  ,"getDiskSpaceReservedSize :return");
    QVERIFY2(5000 == nDiskSpaceReservedSize,"getDiskSpaceReservedSize :out");

    QString sDevName("2000");
    nRet = Itest->SetDevInfo(sDevName,  4);
    QVERIFY2(IRecorder::OK == nRet  ,"SetDevInfo :return");
    nRet = Itest->Start();
    QVERIFY2(IRecorder::OK == nRet,"Start() :return");

    FILE *pFile = NULL;
    QString sFileName = QCoreApplication::applicationDirPath() + "/CIF_12fps_128kbps.h264";
    int   nlen = 0;
    char  data[1280*720];
    uint  nTimes = 0;
    uint  nTotalLoopTimes = 0;
    unsigned long long   i64FreeBytesAvailableOfE = 0;
    unsigned long long   i64TotalNumberOfBytesOfE = 0;
    unsigned long long   i64TotalNumberOfFreeBytesOfE = 0;
    unsigned long long   i64FreeBytesAvailableOfF = 0;
    unsigned long long   i64TotalNumberOfBytesOfF = 0;
    unsigned long long   i64TotalNumberOfFreeBytesOfF = 0;
    GetDiskFreeSpaceExQ("E:\\"
        , &i64FreeBytesAvailableOfE
        , &i64TotalNumberOfBytesOfE
        , &i64TotalNumberOfFreeBytesOfE);
    GetDiskFreeSpaceExQ("F:\\"
        , &i64FreeBytesAvailableOfF
        , &i64TotalNumberOfBytesOfF
        , &i64TotalNumberOfFreeBytesOfF);
    nTotalLoopTimes = (uint)( ((float)i64FreeBytesAvailableOfE/1024/1024 + (float)i64FreeBytesAvailableOfF/1024/1024 - 10000) / 4.58);
    QVERIFY2 (NULL != (pFile = fopen(sFileName.toAscii().data(),"rb")), "File Open Error");

    NALU_HEADER_t nhead;
    QVariantMap frameInfo ;
    while (nTimes < nTotalLoopTimes )
    {
        if (feof(pFile))
        {
            rewind(pFile);
            ++nTimes;
        }
        memset(&nhead,0,sizeof(NALU_HEADER_t));
        memset(data,0,sizeof(data));
        if ((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) <= 0)
            continue;
        QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        frameInfo.insert("frametype", QVariant((uint)nhead.isider)) ;
        frameInfo.insert("data", QVariant(data));
        frameInfo.insert("length", QVariant((uint)nhead.size));
        frameInfo.insert("channel", QVariant(4));
        quint64 u64_tTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        frameInfo.insert("pts", QVariant(u64_tTime));
        
        nRet = Itest->InputFrame(frameInfo);
        QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
    }

    fclose(pFile);
    pFile=NULL;

    nRet = Itest->Stop();
    QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

    END_RECORDER_UNIT_TEST(Itest, pDiskSetting);
}
// 3.  测试 不循环录像是否正常停止而不覆盖和测试分区保留空间大小以及路径是否正确
//getEnableDisks获取可用分区	| 返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用F:盘存储录像文件 | 返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为F:   |	返回值为IDisksSetting::OK; 输出值为”F: ”
//setFilePackageSize设置录像文件包大小为200MB	| 返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置	| 返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为非循环录像	 | 返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像	| 返回值为false
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB	| 返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值  |	返回值为IDisksSetting::OK;输出值为”5000”
//调用SetDevInfo设置设备名为3000和通道数1     | 返回值为IRecorder:OK
//调用Start()开始录像	 | 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame循环写入 ,重复n次,记录录像文件中最小的时间戳| 	返回值为IRecorder:OK
//在磁盘可用空间只剩下5000MB±200MB时, 再继续1000次循环 | 时间戳最小的录像文件仍然存在, 且大小为200MB±3MB; F盘保留空间为5000MB±200MB
void RecorderTest::RecorderTest3()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest, pDiskSetting);
	int nRet = -1;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;

	nRet = pDiskSetting->getEnableDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getEnableDisks :return");
	QVERIFY2(sDiskEnabled == "C:D:E:F:" ,"getEnableDisks :out");
	nRet = pDiskSetting->setUseDisks("F:");
	QVERIFY2(IDisksSetting::OK == nRet  ,"setUseDisks :return");
	sDiskEnabled.clear();
	nRet = pDiskSetting->getUseDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getUseDisks :return");
	QVERIFY2(sDiskEnabled == "F:"     ,"getUseDisks :out");
	nRet = pDiskSetting->setFilePackageSize(200);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setFilePackageSize :return");
	nRet = pDiskSetting->getFilePackageSize(nFileSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getFilePackageSize :return");
	QVERIFY2(200 == nFileSize           ,"getFilePackageSize :out");
	nRet = pDiskSetting->setLoopRecording(false);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setLoopRecording :return");
	bool bIsLoopRecord = pDiskSetting->getLoopRecording();
	QVERIFY2(false == bIsLoopRecord      ,"getLoopRecording :return");
	nRet = pDiskSetting->setDiskSpaceReservedSize(5000);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setDiskSpaceReservedSize :return");
	nRet = pDiskSetting->getDiskSpaceReservedSize(nDiskSpaceReservedSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getDiskSpaceReservedSize :return");
	QVERIFY2(5000 == nDiskSpaceReservedSize,"getDiskSpaceReservedSize :out");

    QString sDevName("3000");
    nRet = Itest->SetDevInfo(sDevName,  1);
    QVERIFY2(IRecorder::OK == nRet  ,"SetDevInfo :return");
	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");

	FILE *pFile = NULL;
    QString sFileName = QCoreApplication::applicationDirPath() + "/CIF_12fps_128kbps.h264";

	int   nlen = 0;
	char  data[1280*720];
	uint  nTimes = 0;
	uint  nTotalLoopTimes = 0;
	unsigned long long   i64FreeBytesAvailableOfE = 0;
	unsigned long long   i64TotalNumberOfBytesOfE = 0;
	unsigned long long   i64TotalNumberOfFreeBytesOfE = 0;
	
	GetDiskFreeSpaceExQ("E:\\"
		, &i64FreeBytesAvailableOfE
		, &i64TotalNumberOfBytesOfE
		, &i64TotalNumberOfFreeBytesOfE);
	nTotalLoopTimes = (uint)( ((float)i64FreeBytesAvailableOfE/1024/1024 - 5000) / 4.58);
	QVERIFY2 (NULL != (pFile = fopen(sFileName.toAscii().data(),"rb")), "File Open Error");

	NALU_HEADER_t nhead;
    QVariantMap frameInfo ;
	while (nTimes < nTotalLoopTimes )
	{
		if (feof(pFile))
		{
			rewind(pFile);
			++nTimes;
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
        if ((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) <= 0)
            continue;
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        frameInfo.insert("frametype", QVariant((uint)nhead.isider)) ;
        frameInfo.insert("data", QVariant(data));
        frameInfo.insert("length", QVariant((uint)nhead.size));
        frameInfo.insert("channel", QVariant(1));
        quint64 u64_tTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        frameInfo.insert("pts", QVariant(u64_tTime));
        nRet = Itest->InputFrame(frameInfo);
        QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
	}
    nTimes = 1000;
    while (nTimes--)
    {
        if (feof(pFile))
        {
            rewind(pFile);
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
        if ((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) <= 0)
            continue;
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        frameInfo.insert("frametype", QVariant((uint)nhead.isider)) ;
        frameInfo.insert("data", QVariant(data));
        frameInfo.insert("length", QVariant((uint)nhead.size));
        frameInfo.insert("channel", QVariant(1));
        quint64 u64_tTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        frameInfo.insert("pts", QVariant(u64_tTime));
        nRet = Itest->InputFrame(frameInfo);
        QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
	}

	fclose(pFile);
	pFile=NULL;

	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest, pDiskSetting);
}
// 4. 测试InputFrame参数frameinfo里面pData和函数返回值
//getEnableDisks获取可用分区	 | 返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用F:盘存储录像文件	| 返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为F:	            | 返回值为IDisksSetting::OK; 输出值为” F:”
//setFilePackageSize设置录像文件包大小为200MB |	返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置	 | 返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为循环录像 |	返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像 |	返回值为true
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB | 	返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值  |	返回值为IDisksSetting::OK;输出值为”5000”
//调用SetDevInfo设置设备名为4000和通道数1     | 返回值为IRecorder:OK
//调用Start()开始录像	 | 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入,调用InputFrame时设置参数frameinfo里面的pData为NULL, 其他参数正确 | 	返回值为IRecorder: E_PARAMETER_ERROR
//调用Stop()停止	 | 返回值为IRecorder::OK,  F盘录像文件无
void RecorderTest::RecorderTest4()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest, pDiskSetting);
	int nRet = -1;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;

    nRet = pDiskSetting->getEnableDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getEnableDisks :return");
	QVERIFY2(sDiskEnabled == "C:D:E:F:" ,"getEnableDisks :out");
	nRet = pDiskSetting->setUseDisks("F:");
	QVERIFY2(IDisksSetting::OK == nRet  ,"setUseDisks :return");
	sDiskEnabled.clear();
	nRet = pDiskSetting->getUseDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getUseDisks :return");
	QVERIFY2(sDiskEnabled == "F:"       ,"getUseDisks :out");
	nRet = pDiskSetting->setFilePackageSize(200);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setFilePackageSize :return");
	nRet = pDiskSetting->getFilePackageSize(nFileSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getFilePackageSize :return");
	QVERIFY2(200 == nFileSize           ,"getFilePackageSize :out");
	nRet = pDiskSetting->setLoopRecording(true);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setLoopRecording :return");
	bool bIsLoopRecord = pDiskSetting->getLoopRecording();
	QVERIFY2(true == bIsLoopRecord      ,"getLoopRecording :return");
	nRet = pDiskSetting->setDiskSpaceReservedSize(5000);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setDiskSpaceReservedSize :return");
	nRet = pDiskSetting->getDiskSpaceReservedSize(nDiskSpaceReservedSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getDiskSpaceReservedSize :return");
	QVERIFY2(5000 == nDiskSpaceReservedSize,"getDiskSpaceReservedSize :out");

    QString sDevName("4000");
    nRet = Itest->SetDevInfo(sDevName,  1);
    QVERIFY2(IRecorder::OK == nRet  ,"SetDevInfo :return");
	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");

	FILE *pFile = NULL;
    QString sFileName = QCoreApplication::applicationDirPath() + "/CIF_12fps_128kbps.h264";

	int   nlen = 0;
	char  data[1280*720];
	uint  nTimes = 0;
	uint  nTotalLoopTimes = 0;
	unsigned long long   i64FreeBytesAvailableOfE = 0;
	unsigned long long   i64TotalNumberOfBytesOfE = 0;
	unsigned long long   i64TotalNumberOfFreeBytesOfE = 0;

	GetDiskFreeSpaceExQ("E:\\"
		, &i64FreeBytesAvailableOfE
		, &i64TotalNumberOfBytesOfE
		, &i64TotalNumberOfFreeBytesOfE);
	nTotalLoopTimes = (uint)( ((float)i64FreeBytesAvailableOfE/1024/1024  - 5000) / 4.58);
	QVERIFY2 (NULL != (pFile = fopen(sFileName.toAscii().data(),"rb")), "File Open Error");

	NALU_HEADER_t nhead;
    QVariantMap frameInfo ;
	while (nTimes < nTotalLoopTimes)
	{
		if (feof(pFile))
		{
			rewind(pFile);
			++nTimes;
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
        if ((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) <= 0)
            continue;
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        frameInfo.insert("frametype", QVariant((uint)nhead.isider)) ;
        frameInfo.insert("data", QVariant(NULL));
        frameInfo.insert("length", QVariant((uint)nhead.size));
        frameInfo.insert("channel", QVariant(1));
        quint64 u64_tTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        frameInfo.insert("pts", QVariant(u64_tTime));
		nRet = Itest->InputFrame(frameInfo);
		QVERIFY2(IRecorder::E_PARAMETER_ERROR == nRet  ,"InputFrame :return");
	}

	fclose(pFile);
	pFile=NULL;
	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest, pDiskSetting);
}
// 5. 测试InputFrame参数frameinfo里面type和返回值
//getEnableDisks获取可用分区	| 返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用F:盘存储录像文件 |	返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为F: | 	返回值为IDisksSetting::OK; 输出值为” F:”
//setFilePackageSize设置录像文件包大小为200MB|	返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置	 | 返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为循环录像	| 返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像	| 返回值为true
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB	| 返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值	| 返回值为IDisksSetting::OK;输出值为”5000”
//调用SetDevInfo设置设备名为5000和通道数1     | 返回值为IRecorder:OK
//调用Start()开始录像	| 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入,调用InputFrame 时设置参数type为0x3, 其他参数正确	| 返回值为IRecorder: E_PARAMETER_ERROR
//调用Stop()停止	| 返回值为IRecorder::OK,  F盘录像文件无
void RecorderTest::RecorderTest5()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest, pDiskSetting);
	int nRet = -1;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;

	nRet = pDiskSetting->getEnableDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getEnableDisks :return");
	QVERIFY2(sDiskEnabled == "C:D:E:F:" ,"getEnableDisks :out");
	nRet = pDiskSetting->setUseDisks("F:");
	QVERIFY2(IDisksSetting::OK == nRet  ,"setUseDisks :return");
	sDiskEnabled.clear();
	nRet = pDiskSetting->getUseDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getUseDisks :return");
	QVERIFY2(sDiskEnabled == "F:"       ,"getUseDisks :out");
	nRet = pDiskSetting->setFilePackageSize(200);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setFilePackageSize :return");
	nRet = pDiskSetting->getFilePackageSize(nFileSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getFilePackageSize :return");
	QVERIFY2(200 == nFileSize           ,"getFilePackageSize :out");
	nRet = pDiskSetting->setLoopRecording(true);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setLoopRecording :return");
	bool bIsLoopRecord = pDiskSetting->getLoopRecording();
	QVERIFY2(true == bIsLoopRecord      ,"getLoopRecording :return");
	nRet = pDiskSetting->setDiskSpaceReservedSize(5000);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setDiskSpaceReservedSize :return");
	nRet = pDiskSetting->getDiskSpaceReservedSize(nDiskSpaceReservedSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getDiskSpaceReservedSize :return");
	QVERIFY2(5000 == nDiskSpaceReservedSize,"getDiskSpaceReservedSize :out");

    QString sDevName("5000");
    nRet = Itest->SetDevInfo(sDevName,  1);
    QVERIFY2(IRecorder::OK == nRet  ,"SetDevInfo :return");
	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");

	FILE *pFile = NULL;
    QString sFileName = QCoreApplication::applicationDirPath() + "/CIF_12fps_128kbps.h264";
	int   nlen = 0;
	char  data[1280*720];
	uint  nTimes = 0;
	uint  nTotalLoopTimes = 0;
	unsigned long long   i64FreeBytesAvailableOfE = 0;
	unsigned long long   i64TotalNumberOfBytesOfE = 0;
	unsigned long long   i64TotalNumberOfFreeBytesOfE = 0;

	GetDiskFreeSpaceExQ("E:\\"
		, &i64FreeBytesAvailableOfE
		, &i64TotalNumberOfBytesOfE
		, &i64TotalNumberOfFreeBytesOfE);
	nTotalLoopTimes = (uint)( ((float)i64FreeBytesAvailableOfE/1024/1024  - 5000) / 4.58);
	QVERIFY2 (NULL != (pFile = fopen(sFileName.toAscii().data(),"rb")), "File Open Error");

	NALU_HEADER_t nhead;
    QVariantMap frameInfo ;
	while (nTimes < nTotalLoopTimes)
	{
		if (feof(pFile))
		{
			rewind(pFile);
			++nTimes;
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
        if ((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) <= 0)
            continue;
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        frameInfo.insert("frametype", QVariant(0x3)) ;
        frameInfo.insert("data", QVariant(data));
        frameInfo.insert("length", QVariant((uint)nhead.size));
        frameInfo.insert("channel", QVariant(1));
        quint64 u64_tTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        frameInfo.insert("pts", QVariant(u64_tTime));
        nRet = Itest->InputFrame(frameInfo);
        QVERIFY2(IRecorder::E_PARAMETER_ERROR == nRet  ,"InputFrame :return");
	}

	fclose(pFile);
	pFile=NULL;
	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest, pDiskSetting);
}

// 6. 测试InputFrame参数frameinfo里面uiDataSize和返回值
//getEnableDisks获取可用分区	| 返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用F:盘存储录像文件 | 返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为F:  | 	返回值为IDisksSetting::OK; 输出值为” F:”
//setFilePackageSize设置录像文件包大小为200MB	| 返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置	 | 返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为循环录像 |	返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像 |	返回值为true
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB	| 返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值	| 返回值为IDisksSetting::OK;输出值为”5000”
//调用SetDevInfo设置设备名为6000和通道数1     | 返回值为IRecorder:OK
//调用Start()开始录像	| 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入 ,重复n次,调用InputFrame时设置参数frameinfo里面uiDataSize为0, 其他参数正确	| 返回值为IRecorder: E_PARAMETER_ERROR
//调用Stop()停止	| 返回值为IRecorder::OK,  F盘录像文件无
void RecorderTest::RecorderTest6()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest, pDiskSetting);
	int nRet = -1;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;

	nRet = pDiskSetting->getEnableDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getEnableDisks :return");
	QVERIFY2(sDiskEnabled == "C:D:E:F:" ,"getEnableDisks :out");
	nRet = pDiskSetting->setUseDisks("F:");
	QVERIFY2(IDisksSetting::OK == nRet  ,"setUseDisks :return");
	sDiskEnabled.clear();
	nRet = pDiskSetting->getUseDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getUseDisks :return");
	QVERIFY2(sDiskEnabled == "F:"       ,"getUseDisks :out");
	nRet = pDiskSetting->setFilePackageSize(200);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setFilePackageSize :return");
	nRet = pDiskSetting->getFilePackageSize(nFileSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getFilePackageSize :return");
	QVERIFY2(200 == nFileSize           ,"getFilePackageSize :out");
	nRet = pDiskSetting->setLoopRecording(true);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setLoopRecording :return");
	bool bIsLoopRecord = pDiskSetting->getLoopRecording();
	QVERIFY2(true == bIsLoopRecord      ,"getLoopRecording :return");
	nRet = pDiskSetting->setDiskSpaceReservedSize(5000);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setDiskSpaceReservedSize :return");
	nRet = pDiskSetting->getDiskSpaceReservedSize(nDiskSpaceReservedSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getDiskSpaceReservedSize :return");
	QVERIFY2(5000 == nDiskSpaceReservedSize,"getDiskSpaceReservedSize :out");

    QString sDevName("6000");
    nRet = Itest->SetDevInfo(sDevName,  1);
    QVERIFY2(IRecorder::OK == nRet  ,"SetDevInfo :return");
	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");

	FILE *pFile = NULL;
    QString sFileName = QCoreApplication::applicationDirPath() + "/CIF_12fps_128kbps.h264";
	int   nlen = 0;
	char  data[1280*720];
	uint  nTimes = 0;
	uint  nTotalLoopTimes = 0;
	unsigned long long   i64FreeBytesAvailableOfE = 0;
	unsigned long long   i64TotalNumberOfBytesOfE = 0;
	unsigned long long   i64TotalNumberOfFreeBytesOfE = 0;

	GetDiskFreeSpaceExQ("E:\\"
		, &i64FreeBytesAvailableOfE
		, &i64TotalNumberOfBytesOfE
		, &i64TotalNumberOfFreeBytesOfE);
	nTotalLoopTimes = (uint)( ((float)i64FreeBytesAvailableOfE/1024/1024  - 5000) / 4.58);
	QVERIFY2 (NULL != (pFile = fopen(sFileName.toAscii().data(),"rb")), "File Open Error");

	NALU_HEADER_t nhead;
    QVariantMap frameInfo ;
	while (nTimes < nTotalLoopTimes)
	{
		if (feof(pFile))
		{
			rewind(pFile);
			++nTimes;
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
        if ((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) <= 0)
            continue;
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        frameInfo.insert("frametype", QVariant((uint)nhead.isider)) ;
        frameInfo.insert("data", QVariant(data));
        frameInfo.insert("length", QVariant(0));
        frameInfo.insert("channel", QVariant(1));
        quint64 u64_tTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        frameInfo.insert("pts", QVariant(u64_tTime));

        nRet = Itest->InputFrame(frameInfo);
		QVERIFY2(IRecorder::E_PARAMETER_ERROR == nRet  ,"InputFrame :return");
	}

	fclose(pFile);
	pFile=NULL;
	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest, pDiskSetting);
}

// 7. 测试SetDevInfo参数nChannelNum和返回值
//getEnableDisks获取可用分区 |	返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用F:盘存储录像文件 |返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为F:|	返回值为IDisksSetting::OK; 输出值为” F:”
//setFilePackageSize设置录像文件包大小为200MB|	返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置|	返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为循环录像	|返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像	|返回值为true
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB |	返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值 | 返回值为IDisksSetting::OK;输出值为”5000”
//调用SetDevInfo设置设备名为7000和通道数1     | 返回值为IRecorder:OK
//调用Start()开始录像 | 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入 ,重复n次,设置正确的参数	| 返回值为IRecorder: :OK
//达到n次后, 调用SetDevInfo设置nChannelNum为-200 devname设为"7001"| 返回值为IRecorder: : E_PARAMETER_ERROR
//继续读取文件的帧数据通过InputFrame写入 ,重复1000次,设置正确的参数  | 返回值为IRecorder: :OK
//调用Stop()停止 | 返回值为IRecorder::OK,  F盘有录像文件但是无目录为-200

void RecorderTest::RecorderTest7()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest, pDiskSetting);
	int nRet = -1;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;

	nRet = pDiskSetting->getEnableDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getEnableDisks :return");
	QVERIFY2(sDiskEnabled == "C:D:E:F:" ,"getEnableDisks :out");
	nRet = pDiskSetting->setUseDisks("F:");
	QVERIFY2(IDisksSetting::OK == nRet  ,"setUseDisks :return");
	sDiskEnabled.clear();
	nRet = pDiskSetting->getUseDisks(sDiskEnabled);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getUseDisks :return");
	QVERIFY2(sDiskEnabled == "F:"     ,"getUseDisks :out");
	nRet = pDiskSetting->setFilePackageSize(200);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setFilePackageSize :return");
	nRet = pDiskSetting->getFilePackageSize(nFileSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getFilePackageSize :return");
	QVERIFY2(200 == nFileSize           ,"getFilePackageSize :out");
	nRet = pDiskSetting->setLoopRecording(false);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setLoopRecording :return");
	bool bIsLoopRecord = pDiskSetting->getLoopRecording();
	QVERIFY2(false == bIsLoopRecord      ,"getLoopRecording :return");
	nRet = pDiskSetting->setDiskSpaceReservedSize(5000);
	QVERIFY2(IDisksSetting::OK == nRet  ,"setDiskSpaceReservedSize :return");
	nRet = pDiskSetting->getDiskSpaceReservedSize(nDiskSpaceReservedSize);
	QVERIFY2(IDisksSetting::OK == nRet  ,"getDiskSpaceReservedSize :return");
	QVERIFY2(5000 == nDiskSpaceReservedSize,"getDiskSpaceReservedSize :out");

    QString sDevName("7000");
    nRet = Itest->SetDevInfo(sDevName,  1);
    QVERIFY2(IRecorder::OK == nRet  ,"SetDevInfo :return");
	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");


	FILE *pFile = NULL;
    QString sFileName = QCoreApplication::applicationDirPath() + "/CIF_12fps_128kbps.h264";

	int   nlen = 0;
	char  data[1280*720];
	uint  nTimes = 0;
	uint  nTotalLoopTimes = 0;
	unsigned long long   i64FreeBytesAvailableOfE = 0;
	unsigned long long   i64TotalNumberOfBytesOfE = 0;
	unsigned long long   i64TotalNumberOfFreeBytesOfE = 0;

	GetDiskFreeSpaceExQ("E:\\"
		, &i64FreeBytesAvailableOfE
		, &i64TotalNumberOfBytesOfE
		, &i64TotalNumberOfFreeBytesOfE);
	nTotalLoopTimes = (uint)( ((float)i64FreeBytesAvailableOfE/1024/1024 - 5000) / 4.58);
	QVERIFY2 (NULL != (pFile = fopen(sFileName.toAscii().data(),"rb")), "File Open Error");

	NALU_HEADER_t nhead;
    QVariantMap frameInfo ;
    bool bIsDevInfoSetFlag = false;
	while (nTimes < nTotalLoopTimes - 900)
	{
		if (feof(pFile))
		{
			rewind(pFile);
			++nTimes;
		}
		if(nTimes > nTotalLoopTimes - 1000 && !bIsDevInfoSetFlag)
		{
			bIsDevInfoSetFlag = true;
            sDevName.clear();
			sDevName="7001";
			nRet = Itest->SetDevInfo(sDevName,  17);
			QVERIFY2(IRecorder::E_PARAMETER_ERROR== nRet  ,"SetDevInfo :return");
			nRet = Itest->SetDevInfo(sDevName,  -200);
			QVERIFY2(IRecorder::E_PARAMETER_ERROR== nRet  ,"SetDevInfo :return");
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
        if ((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) <= 0)
            continue;
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        frameInfo.insert("frametype", QVariant((uint)nhead.isider)) ;
        frameInfo.insert("data",    QVariant(data));
        frameInfo.insert("length",  QVariant((uint)nhead.size));
        frameInfo.insert("channel", QVariant(1));
        quint64 u64_tTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        frameInfo.insert("pts", QVariant(u64_tTime));

        nRet = Itest->InputFrame(frameInfo);
		QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
	}
    nTimes = 1000;
    while (nTimes--)
    {
        if (feof(pFile))
        {
            rewind(pFile);
        }
        memset(&nhead,0,sizeof(NALU_HEADER_t));
        memset(data,0,sizeof(data));
        if ((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) <= 0)
            continue;
        QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        frameInfo.insert("frametype", QVariant((uint)nhead.isider)) ;
        frameInfo.insert("data", QVariant(data));
        frameInfo.insert("length", QVariant((uint)nhead.size));
        frameInfo.insert("channel", QVariant(1));
        quint64 u64_tTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        frameInfo.insert("pts", QVariant(u64_tTime));
        
        nRet = Itest->InputFrame(frameInfo);
        QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
    }
	fclose(pFile);
	pFile=NULL;

	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest, pDiskSetting);

}
