#include "RemoteBackupTest.h"
#include <guid.h>
#include "IEventRegister.h"
#include "IRemoteBackup.h"

#define START_REMOTEBACKUP_UNIT_TEST(ii) 	IRemoteBackup * ii = NULL; \
    pcomCreateInstance(CLSID_DeviceClient, NULL, IID_IRemoteBackup,(void **)&ii); \
    QVERIFY2(NULL != ii,"Create remotebackup instance");

#define END_REMOTEBACKUP_UNIT_TEST(ii) ii->Release(); 

bool stopbackup ;
RemoteBackupTest::RemoteBackupTest()
{
}

RemoteBackupTest::~RemoteBackupTest()
{
}

//备份结束事件回调
int cbStopBackup(QString evName, QVariantMap evMap, void *pUser)
{ 
    if("backupEvent" == evName)
    {
		
		if ("stopBackup" == evMap["types"].toString())
		{
			stopbackup = true;
		}
		else if ("startBackup" == evMap["types"].toString())
		{
		}
		else if ("diskfull" == evMap["types"].toString())
		{
			stopbackup = true;
		}
		else if ("backupFinished" == evMap["types"].toString())
		{
			stopbackup = true;
		}
		else if ("noStream" == evMap["types"].toString())
		{
			stopbackup = true;
		}
		qDebug()<<evMap["types"].toString();
		
    }

    return 0;
}
//前置条件 存在分区D  且D下存在目录RemoteBackup
//存在可用主机 ip"192.168.2.132" 端口8785
//准备合法通道号0~31  合法时间 开始时间(系统当前时间-1小时)<结束时间（系统当前时间） 合法路径F:/RemoteBackup

//1.  输入参数测试 
void RemoteBackupTest::RemoteBackupCase1()
{
    START_REMOTEBACKUP_UNIT_TEST(Itest);
	
	int nRet;
	int channel = 1;
	int types = 15;
	QString sAddr = "192.168.2.132";
	unsigned int uiport = 8785;
	QString eseeid = "12345678";
	QDateTime etime =  QDateTime::currentDateTime();
	QDateTime stime = QDateTime(etime.date(),QTime(etime.time().hour()-1,etime.time().minute()));
	QString spath = "F:/RemoteBackup";
	/*QString sAddr = "192.168.2.113";
	unsigned int uiport = 80*/;
	/*QDateTime stime = QDateTime::fromString("2013-3-23 12:00:00","yyyy-MM-dd hh:mm:ss");
	QDateTime etime =  QDateTime::fromString("2013-3-23 12:30:00","yyyy-MM-dd hh:mm:ss");*/


	//错误主机信息测试(不可用主机"129.168.2.210")
	nRet = Itest->startBackup("129.168.2.210",uiport,eseeid,channel,types,etime,stime,spath);
	QVERIFY2(IRemoteBackup::E_PARAMETER_ERROR == nRet  ,"Illegal host");

	//错误通道号测试(-1)
	nRet = Itest->startBackup(sAddr,uiport,eseeid,-1,types,stime,etime,spath);
	QVERIFY2(IRemoteBackup::E_PARAMETER_ERROR == nRet  ,"Illegal channel");

	//错误时间测试(起止时间对换)
	nRet = Itest->startBackup(sAddr,uiport,eseeid,channel,types,etime,stime,spath);
	QVERIFY2(IRemoteBackup::E_PARAMETER_ERROR == nRet  ,"Illegal times");

	//错误备份路径测试("ADBC:/DDDD")
	nRet = Itest->startBackup(sAddr,uiport,eseeid,channel,types,stime,etime,"ADBC:/DDDD");
	QVERIFY2(IRemoteBackup::E_PARAMETER_ERROR == nRet  ,"Illegal path");
 
	//合法参数测试
	nRet = Itest->startBackup(sAddr,uiport,eseeid,channel,types,stime,etime,spath);
	QVERIFY2(IRemoteBackup::OK == nRet  ,"start backup! legal param");
	QTest::qSleep(20000);
	Itest->stopBackup();

    END_REMOTEBACKUP_UNIT_TEST(Itest);
}
 //2.  手动开启/关闭备份 及事件回调
void RemoteBackupTest::RemoteBackupCase2()
{
	START_REMOTEBACKUP_UNIT_TEST(Itest);

	int nRet;
	int channel = 1;
	int types = 15;
	QString sAddr = "192.168.2.132";
	unsigned int uiport = 8785;
	QString eseeid = "12345678";
	QDateTime etime =  QDateTime::currentDateTime();
	QDateTime stime = QDateTime(etime.date(),QTime(etime.time().hour()-1,etime.time().minute()));
	QString spath = "F:/RemoteBackup";

	IEventRegister *pRegist = NULL;
	nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
	QVERIFY2(S_OK == nRet, "QueryInterface Error!");
	nRet = pRegist->registerEvent("backupEvent", cbStopBackup, NULL);
	QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");

	nRet = Itest->startBackup(sAddr,uiport,eseeid,channel,types,stime,etime,spath);
	QVERIFY2(IRemoteBackup::OK == nRet  ,"start backup! legal param");

	QTest::qSleep(20000);//20秒

	nRet = nRet = Itest->stopBackup();
	QVERIFY2(IRemoteBackup::OK == nRet  ,"stop backup! ");

	END_REMOTEBACKUP_UNIT_TEST(Itest);
}
// 3. 获取备份进度 及 备份结束事件回调测试
void RemoteBackupTest::RemoteBackupCase3()
{
	START_REMOTEBACKUP_UNIT_TEST(Itest);

	int nRet;
	int channel = 1;
	int types = 15;
	QString sAddr = "192.168.2.132";
	unsigned int uiport = 8785;
	QString eseeid = "12345678";
	QDateTime etime =  QDateTime::currentDateTime();
	QDateTime stime = QDateTime(etime.date(),QTime(etime.time().hour()-1,etime.time().minute()));
	QString spath = "F:/RemoteBackup";

	IEventRegister *pRegist = NULL;
	nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
	QVERIFY2(S_OK == nRet, "QueryInterface Error!");
	nRet = pRegist->registerEvent("backupEvent", cbStopBackup, NULL);
	QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");
	pRegist->Release();

	stopbackup = false;
	nRet = Itest->startBackup(sAddr,uiport,eseeid,channel,types,stime,etime,spath);
	QVERIFY2(IRemoteBackup::OK == nRet  ,"start backup! legal param");

	while(!stopbackup)
	{
		qDebug("get the progress is : %.2f%",Itest->getProgress()*100);
		QTest::qSleep(1000);
	}

	END_REMOTEBACKUP_UNIT_TEST(Itest);
}
