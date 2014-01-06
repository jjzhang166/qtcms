#include "RecorderTest.h"
#include <guid.h>
#include <IDisksSetting.h>
#include <IRecorder.h>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include "netlib.h"
#pragma comment(lib,"netlib.lib")

#define START_RECORDER_UNIT_TEST(ii) 	IRecorder * ii = NULL; \
   pcomCreateInstance(CLSID_Recorder, NULL, IID_IRecorder,(void **)&ii); \
    QVERIFY2(NULL != ii,"Create Recorder instance");

#define END_RECORDER_UNIT_TEST(ii) ii->Release(); 

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
// 1.测试录像文件写入大小和路径以及最后保留空间大小: 
//      getEnableDisks获取可用分区	返回值为IDisksSetting::OK | 输出值为” C:D:E:F:”
//      setUseDisks函数设置使用E:F:盘存储录像文件	| 返回值为IDisksSetting::OK;
//      getUseDisks验证使用磁盘为E:F: |	返回值为IDisksSetting::OK; 输出值为”E:F:”
//      setFilePackageSize设置录像文件包大小为200MB	| 返回值为IDisksSetting::OK;
//      getFilePackageSize获取录像文件包大小的设置	| 返回值为IDisksSetting::OK;输出值为”200”
//      setLoopRecording设置为循环录像	            | 返回值为IDisksSetting::OK;
//      getLoopRecording验证为循环录像	            | 返回值为true
//      setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB	 | 返回值为IDisksSetting::OK;
//      getDiskSpaceReservedSize验证上一步设置值	| 返回值为IDisksSetting::OK;输出值为”5000”
//      调用Start()开始录像	                    | 返回值为IRecorder::OK
//      调用InputFrame ,设置参数正确	            | 返回值为IRecorder:OK
//      在开始往F:写数据一定次数后, 调用SetDevInfo设置设备名为1000和通道号1  | 返回值为IRecorder:OK
//      调用Stop()停止     | 返回值为IRecorder::OK, 录像文件大小均为200MB左右且均在EF盘下,E盘剩余保留空间5000MB
void RecorderTest::RecorderTest1()
{
    beforeRecorderTest();
    START_RECORDER_UNIT_TEST(Itest);
    int nRet = -1;
    IDisksSetting *pDiskSetting = NULL;
    int nFileSize = 0;
    QString sDiskEnabled;
    int nDiskSpaceReservedSize = 0 ;
    nRet = Itest->QueryInterface(IID_IDiskSetting, (void**)&pDiskSetting);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

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

    nRet = Itest->Start();
    QVERIFY2(IRecorder::OK == nRet,"Start() :return");


    FILE *pFile = NULL;
    char  pFileName[] = "CIF_12fps_128kbps.h264";
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
    QVERIFY2 (NULL != (pFile = fopen(pFileName,"rb")), "File Open Error");

    NALU_HEADER_t nhead;
    uint nTmp = (uint) (((float)i64FreeBytesAvailableOfF/1024/1024 - 5000) /4.58 );
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
            QString sDevName("1000");
            nRet = Itest->SetDevInfo(sDevName,  1);
            QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
        }
        memset(&nhead,0,sizeof(NALU_HEADER_t));
        memset(data,0,sizeof(data));
        QVERIFY2((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) > 0, "fread Error");
        QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        nRet = Itest->InputFrame(nhead.isider, data,  nhead.size);
        QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
    }

    fclose(pFile);
    pFile=NULL;
    nRet = Itest->Stop();
    QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

    END_RECORDER_UNIT_TEST(Itest);
}

// 2. 测试录像文件写入大小和路径以及最后保留空间大小
//getEnableDisks获取可用分区	         |  返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用E:F:盘存储录像文件	 |  返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为E:F:	         |  返回值为IDisksSetting::OK; 输出值为”E:F:”
//setFilePackageSize设置录像文件包大小为200MB|	返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置	  |返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为循环录像	         | 返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像	         | 返回值为true
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB	| 返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值  |	返回值为IDisksSetting::OK;输出值为”5000”
//调用Start()开始录像	                     |  返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入 ,重复n次, (n与测试者磁盘可用空间大小有关)|	返回值为IRecorder:OK
//调用Stop()停止,                         |  返回值为IRecorder::OK, 录像文件大小均为200MB左右,在EF盘有录像,E盘保留空间为5000MB左右

void RecorderTest::RecorderTest2()
{
    beforeRecorderTest();
    START_RECORDER_UNIT_TEST(Itest);
    int nRet = -1;
    IDisksSetting *pDiskSetting = NULL;
    int nFileSize = 0;
    QString sDiskEnabled;
    int nDiskSpaceReservedSize = 0 ;
    nRet = Itest->QueryInterface(IID_IDiskSetting, (void**)&pDiskSetting);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

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

    nRet = Itest->Start();
    QVERIFY2(IRecorder::OK == nRet,"Start() :return");


    FILE *pFile = NULL;
    char  pFileName[] = "CIF_12fps_128kbps.h264";
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
    QVERIFY2 (NULL != (pFile = fopen(pFileName,"rb")), "File Open Error");

    NALU_HEADER_t nhead;
    while (nTimes < nTotalLoopTimes )
    {
        if (feof(pFile))
        {
            rewind(pFile);
            ++nTimes;
        }
        memset(&nhead,0,sizeof(NALU_HEADER_t));
        memset(data,0,sizeof(data));
        QVERIFY2((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) > 0, "fread Error");
        QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
        nRet = Itest->InputFrame(nhead.isider, data,  nhead.size);
        QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
    }

    fclose(pFile);
    pFile=NULL;

    nRet = Itest->Stop();
    QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

    END_RECORDER_UNIT_TEST(Itest);
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
//调用Start()开始录像	 | 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入 ,重复n次,, 不调用Stop () | 	返回值为IRecorder:OK
//检测F:盘下录像文件大小, 和剩余空间大小, 是否有录像文件被覆盖
void RecorderTest::RecorderTest3()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest);
	int nRet = -1;
	IDisksSetting *pDiskSetting = NULL;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;
	nRet = Itest->QueryInterface(IID_IDiskSetting, (void**)&pDiskSetting);
	QVERIFY2(S_OK == nRet, "QueryInterface Error!");

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

	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");


	FILE *pFile = NULL;
	char  pFileName[] = "CIF_12fps_128kbps.h264";
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
	QVERIFY2 (NULL != (pFile = fopen(pFileName,"rb")), "File Open Error");

	NALU_HEADER_t nhead;
	while (nTimes < nTotalLoopTimes )
	{
		if (feof(pFile))
		{
			rewind(pFile);
			++nTimes;
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
		QVERIFY2((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) > 0, "fread Error");
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
		nRet = Itest->InputFrame(nhead.isider, data,  nhead.size);
		QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
	}

	fclose(pFile);
	pFile=NULL;

	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest);
}
// 4. 测试InputFrame参数cbuf和返回值
//getEnableDisks获取可用分区	 | 返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用F:盘存储录像文件	| 返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为F:	            | 返回值为IDisksSetting::OK; 输出值为” F:”
//setFilePackageSize设置录像文件包大小为200MB |	返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置	 | 返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为循环录像 |	返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像 |	返回值为true
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB | 	返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值  |	返回值为IDisksSetting::OK;输出值为”5000”
//调用Start()开始录像	 | 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入,调用InputFrame时设置参数cbuf为NULL, 其他参数正确 | 	返回值为IRecorder: E_PARAMETER_ERROR
//调用Stop()停止	 | 返回值为IRecorder::OK,  F盘录像文件无
void RecorderTest::RecorderTest4()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest);
	int nRet = -1;
	IDisksSetting *pDiskSetting = NULL;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;
	nRet = Itest->QueryInterface(IID_IDiskSetting, (void**)&pDiskSetting);
	QVERIFY2(S_OK == nRet, "QueryInterface Error!");

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

	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");


	FILE *pFile = NULL;
	char  pFileName[] = "CIF_12fps_128kbps.h264";
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
	QVERIFY2 (NULL != (pFile = fopen(pFileName,"rb")), "File Open Error");

	NALU_HEADER_t nhead;
	while (nTimes < nTotalLoopTimes)
	{
		if (feof(pFile))
		{
			rewind(pFile);
			++nTimes;
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
		QVERIFY2((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) > 0, "fread Error");
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
		nRet = Itest->InputFrame(nhead.isider, NULL,  nhead.size);
		QVERIFY2(IRecorder::E_PARAMETER_ERROR == nRet  ,"InputFrame :return");
	}

	fclose(pFile);
	pFile=NULL;
	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest);
}
// 5. 测试InputFrame参数type和返回值
//getEnableDisks获取可用分区	| 返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用F:盘存储录像文件 |	返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为F: | 	返回值为IDisksSetting::OK; 输出值为” F:”
//setFilePackageSize设置录像文件包大小为200MB|	返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置	 | 返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为循环录像	| 返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像	| 返回值为true
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB	| 返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值	| 返回值为IDisksSetting::OK;输出值为”5000”
//调用Start()开始录像	| 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入,调用InputFrame 时设置参数type为0x3, 其他参数正确	| 返回值为IRecorder: E_PARAMETER_ERROR
//调用Stop()停止	| 返回值为IRecorder::OK,  F盘录像文件无
void RecorderTest::RecorderTest5()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest);
	int nRet = -1;
	IDisksSetting *pDiskSetting = NULL;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;
	nRet = Itest->QueryInterface(IID_IDiskSetting, (void**)&pDiskSetting);
	QVERIFY2(S_OK == nRet, "QueryInterface Error!");

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

	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");

	FILE *pFile = NULL;
	char  pFileName[] = "CIF_12fps_128kbps.h264";
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
	QVERIFY2 (NULL != (pFile = fopen(pFileName,"rb")), "File Open Error");

	NALU_HEADER_t nhead;
	while (nTimes < nTotalLoopTimes)
	{
		if (feof(pFile))
		{
			rewind(pFile);
			++nTimes;
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
		QVERIFY2((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) > 0, "fread Error");
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
		nRet = Itest->InputFrame(0x3, data,  nhead.size);
		QVERIFY2(IRecorder::E_PARAMETER_ERROR == nRet  ,"InputFrame :return");
	}

	fclose(pFile);
	pFile=NULL;
	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest);
}

// 6. 测试InputFrame参数buffersize和返回值
//getEnableDisks获取可用分区	| 返回值为IDisksSetting::OK; 输出值为” C:D:E:F:”
//setUseDisks函数设置使用F:盘存储录像文件 | 返回值为IDisksSetting::OK;
//getUseDisks验证使用磁盘为F:  | 	返回值为IDisksSetting::OK; 输出值为” F:”
//setFilePackageSize设置录像文件包大小为200MB	| 返回值为IDisksSetting::OK;
//getFilePackageSize获取录像文件包大小的设置	 | 返回值为IDisksSetting::OK;输出值为”200”
//setLoopRecording设置为循环录像 |	返回值为IDisksSetting::OK;
//getLoopRecording验证为循环录像 |	返回值为true
//setDiskSpaceReservedSize设置磁盘保留空间大小为5000MB	| 返回值为IDisksSetting::OK;
//getDiskSpaceReservedSize验证上一步设置值	| 返回值为IDisksSetting::OK;输出值为”5000”
//调用Start()开始录像	| 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入 ,重复n次,调用InputFrame时设置参数buffersize为-300, 其他参数正确	| 返回值为IRecorder: E_PARAMETER_ERROR
//调用Stop()停止	| 返回值为IRecorder::OK,  F盘录像文件无
void RecorderTest::RecorderTest6()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest);
	int nRet = -1;
	IDisksSetting *pDiskSetting = NULL;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;
	nRet = Itest->QueryInterface(IID_IDiskSetting, (void**)&pDiskSetting);
	QVERIFY2(S_OK == nRet, "QueryInterface Error!");

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

	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");

	FILE *pFile = NULL;
	char  pFileName[] = "CIF_12fps_128kbps.h264";
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
	QVERIFY2 (NULL != (pFile = fopen(pFileName,"rb")), "File Open Error");

	NALU_HEADER_t nhead;
	while (nTimes < nTotalLoopTimes)
	{
		if (feof(pFile))
		{
			rewind(pFile);
			++nTimes;
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
		QVERIFY2((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) > 0, "fread Error");
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
		nRet = Itest->InputFrame(nhead.isider, data,  -300);
		QVERIFY2(IRecorder::E_PARAMETER_ERROR == nRet  ,"InputFrame :return");
	}

	fclose(pFile);
	pFile=NULL;
	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest);
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
//调用Start()开始录像 | 返回值为IRecorder::OK
//打开CIF_12fps_128kbps.h264文件,读取到的帧数据通过InputFrame写入 ,重复n次,设置正确的参数	| 返回值为IRecorder: :OK
//达到n次后, 调用SetDevInfo设置nChannelNum为17和-200 | 返回值为IRecorder: : E_PARAMETER_ERROR
//调用Stop()停止 | 返回值为IRecorder::OK,  F盘有录像文件但是无目录为17和-200

void RecorderTest::RecorderTest7()
{
	beforeRecorderTest();
	START_RECORDER_UNIT_TEST(Itest);
	int nRet = -1;
	IDisksSetting *pDiskSetting = NULL;
	int nFileSize = 0;
	QString sDiskEnabled;
	int nDiskSpaceReservedSize = 0 ;
	nRet = Itest->QueryInterface(IID_IDiskSetting, (void**)&pDiskSetting);
	QVERIFY2(S_OK == nRet, "QueryInterface Error!");

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

	nRet = Itest->Start();
	QVERIFY2(IRecorder::OK == nRet,"Start() :return");


	FILE *pFile = NULL;
	char  pFileName[] = "CIF_12fps_128kbps.h264";
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
	QVERIFY2 (NULL != (pFile = fopen(pFileName,"rb")), "File Open Error");

	NALU_HEADER_t nhead;
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
			QString sDevName("1000");
			nRet = Itest->SetDevInfo(sDevName,  17);
			QVERIFY2(IRecorder::E_PARAMETER_ERROR== nRet  ,"SetDevInfo :return");
			nRet = Itest->SetDevInfo(sDevName,  -200);
			QVERIFY2(IRecorder::E_PARAMETER_ERROR== nRet  ,"SetDevInfo :return");
		}
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		memset(data,0,sizeof(data));
		QVERIFY2((nlen = fread(&nhead,sizeof(NALU_HEADER_t),1,pFile)) > 0, "fread Error");
		QVERIFY2((nlen = fread(data,1,nhead.size,pFile)) > 0, "fread Error");
		nRet = Itest->InputFrame(nhead.isider, data,  nhead.size);
		QVERIFY2(IRecorder::OK == nRet  ,"InputFrame :return");
	}
	fclose(pFile);
	pFile=NULL;

	nRet = Itest->Stop();
	QVERIFY2(IRecorder::OK == nRet,"Stop() :return");

	END_RECORDER_UNIT_TEST(Itest);

}
