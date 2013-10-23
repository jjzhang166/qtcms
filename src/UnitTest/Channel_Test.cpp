#include "Channel_Test.h"
#include <guid.h>
#include <IChannelManager.h>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>

#define START_CHL_UNIT_TEST(ii) 	IChannelManager * ii = NULL; \
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IChannelManager,(void **)&ii); \
	QVERIFY2(NULL != ii,"Create channel manager instance");

#define END_CHL_UNIT_TEST(ii) ii->Release(); 

Channel_Test::Channel_Test()
{
}


Channel_Test::~Channel_Test()
{
}


void Channel_Test::beforeChlTest()
{
	QSqlDatabase dbChl;
	if (QSqlDatabase::contains("ChannelTest_123"))
	{
	dbChl = QSqlDatabase::database("ChannelTest_123"); 
	}
	else
	{
	dbChl = QSqlDatabase::addDatabase("QSQLITE","ChannelTest_123");
	}
	QString sAppPath = QCoreApplication::applicationDirPath();
	QString sDatabasePath = sAppPath + "/system.db";
	dbChl.setDatabaseName(sDatabasePath);
	bool bRet = dbChl.open();

	QSqlQuery query1(dbChl);
	QSqlQuery query1_1(dbChl);
	QSqlQuery query1_2(dbChl);
	QSqlQuery query1_3(dbChl);
	QSqlQuery query1_4(dbChl);

	query1.exec("delete from chl;");
	query1_1.exec("delete from dev;");	
	query1_2.exec("select * from sqlite_sequence;");   
	query1_3.exec("update sqlite_sequence set seq=0 where name= 'chl';"); 
	query1_4.exec("update sqlite_sequence set seq=0 where name= 'dev';"); 

	QSqlQuery query2(dbChl);
	query2.prepare("insert into dev( area_id, address, port, http, eseeid, username, password, name, channel_count, connect_method, vendor) \
	values(:area_id,:address,:port,:http,:eseeid,:username,:password,:name,:channel_count,:connect_method,:vendor)");

	query2.bindValue(":area_id"        ,222);
	query2.bindValue(":address"        ,"127.0.0.1");
	query2.bindValue(":port"           ,8000);
	query2.bindValue(":http"           ,1000);
	query2.bindValue(":eseeid"         ,"ggggggg");
	query2.bindValue(":username"       ,"admin");
	query2.bindValue(":password"       ,QString(QCryptographicHash::hash(QString("admin2").toAscii(),QCryptographicHash::Md5).toHex()));
	query2.bindValue(":name"           ,"device1");
	query2.bindValue(":channel_count"  ,4);
	query2.bindValue(":connect_method" ,2);
	query2.bindValue(":vendor"         ,"JUAN IPC");
	query2.exec();

	QSqlQuery query3(dbChl);
	query3.prepare("insert into chl( dev_id, channel_number, name, stream_id) \
	values(:dev_id,:channel_number,:name2,:stream_id)");
	query3.bindValue(":dev_id"         ,1);
	query3.bindValue(":channel_number" ,1);
	query3.bindValue(":name2"          ,"kkkkk");
	query3.bindValue(":stream_id"      ,2);
	query3.exec();

	QSqlQuery query4(dbChl);
	query4.exec("select id from dev where area_id = 222 and name = \"device1\" ");

	dbChl.commit();/*
	QSqlError sqlerr ;
	sqlerr = dbChl.lastError();
	QString p ;
	p = sqlerr.text();
	qDebug(p.toLatin1());
	*/
	dbChl.close();

}

// 测试用例
// 前置：重置数据库，数据库中创建了chl表和dev表，两个表都对应只有一条记录：1|1|1|"kkkkk"|2
//       和 1|222|"127.0.0.1"|8000|1000|"ggggggg"|admin|admin2|"device1"|4|2|"JUAN IPC"
/* 1 测试不存在通道是否存在            |返回E_CHANNEL_NOT_FOUND
     测试  存在通道是否存在            |返回OK
   2 修改不存在通道的名字              |返回E_CHANNEL_NOT_FOUND;
     修改  存在通道的名字              |返回OK
   3 修改不存在通道的流                |返回E_CHANNEL_NOT_FOUND
     修改  存在通道的流                |返回OK
   4 获取不存在的设备dev_id下的通道数  |返回E_DEVICE_NOT_FOUND
     获取  存在的设备dev_id下的通道数  |返回 通道数
   5 获取不存在设备dev_id下的通道列表  |返回列表为空
     获取  存在设备dev_id下的通道列表  |返回列表非空
   6 获取不存在通道的名称              |返回E_CHANNEL_NOT_FOUND
     获取  存在通道的名称              |返回OK
   7 获取不存在通道的流                |返回E_CHANNEL_NOT_FOUND
     获取  存在通道的流                |返回
   8 获取不存在通道的NO                |返回E_CHANNEL_NOT_FOUND
     获取  存在通道的NO                |返回
   9 获取不存在通道的信息              |返回E_CHANNEL_NOT_FOUND
     获取  存在通道的信息              |返回
*/

// 1 测试不存在通道是否存在            |返回E_CHANNEL_NOT_FOUND
//   测试  存在通道是否存在            |返回OK
void Channel_Test::ChlCase1()
{
	qDebug("ChlCase1()  Begin:");
	beforeChlTest();
	START_CHL_UNIT_TEST(Ichl);

	//step1   
	QString sTmp;
	int nRet = -2;
	for (int i = 2; i < 100; ++i)
	{
       nRet = Ichl->GetChannelName(i,sTmp);
	   QVERIFY2(IChannelManager::E_CHANNEL_NOT_FOUND == nRet ,"step 1:");
	   if (nRet == IChannelManager::OK)
	   {
		   qDebug("Error!!  chl_id %d is Existed! ",i);
	   }
	}

	//step2
	nRet = Ichl->GetChannelName(1,sTmp);
	QVERIFY2(IChannelManager::OK == nRet ,"step 2:");

	QVariantMap variantmap;
	variantmap = Ichl->GetChannelInfo(1);
	
	qDebug()<<"Channel 1's name: " << variantmap["name"].toString();
	
    END_CHL_UNIT_TEST(Ichl);
	qDebug("ChlCase1()  End:");
}

// 2 修改不存在通道的名字              |返回E_CHANNEL_NOT_FOUND;
//   修改  存在通道的名字              |返回OK
void Channel_Test::ChlCase2()
{
	qDebug("ChlCase2()  Begin:");
	beforeChlTest();

	START_CHL_UNIT_TEST(Ichl);
	int nRet;
	//step1
	nRet = Ichl->ModifyChannelName(33,"1234");
	QVERIFY2(IChannelManager::E_CHANNEL_NOT_FOUND == nRet,"step 1:");

	//step2
	nRet = Ichl->ModifyChannelName(1,"3333");
	QVERIFY2(IChannelManager::OK == nRet,"step 2:");

	QVariantMap variantmap;
	variantmap = Ichl->GetChannelInfo(1);
	QVERIFY2("3333" == variantmap["name"].toString(), "step2 :");

	END_CHL_UNIT_TEST(Ichl);
	qDebug("ChlCase2()  End:");
}

// 3 修改不存在通道的流                |返回E_CHANNEL_NOT_FOUND
//   修改  存在通道的流                |返回OK
void Channel_Test::ChlCase3()
{
	qDebug("ChlCase3()  Begin:");
	beforeChlTest();
	START_CHL_UNIT_TEST(Ichl);

	int nRet;
	//step1
	nRet = Ichl->ModifyChannelStream(33,8888);
	QVERIFY2(IChannelManager::E_CHANNEL_NOT_FOUND == nRet,"step 1");
	//step2
	nRet = Ichl->ModifyChannelStream(1,9999);
	QVERIFY2(IChannelManager::OK == nRet,"step 2");

	QVariantMap variantmap;
	variantmap = Ichl->GetChannelInfo(1);
	QVERIFY2(9999 == variantmap["stream"].toInt(), "step2");

	END_CHL_UNIT_TEST(Ichl);
	qDebug("ChlCase3()  End:");
}

// 4 获取不存在的设备dev_id下的通道数  |返回-1(特殊情况,不能返回E_DEVICE_NOT_FOUND否则矛盾)
//   获取  存在的设备dev_id下的通道数  |返回 通道数
void Channel_Test::ChlCase4()
{
	qDebug("ChlCase4()  Begin:");
	beforeChlTest();
	START_CHL_UNIT_TEST(Ichl);

	int nRet;
	//step1
	nRet = Ichl->GetChannelCount(33);
	QVERIFY2(-1 == nRet,"step 1");
	//step2
	nRet = Ichl->GetChannelCount(1);
	QVERIFY2(nRet >= 0,"step 2");
	
	END_CHL_UNIT_TEST(Ichl);
	qDebug("ChlCase4()  End:");
}

// 5 获取不存在设备dev_id下的通道列表  |返回列表为空
//   获取  存在设备dev_id下的通道列表  |返回列表非空
void Channel_Test::ChlCase5()
{
	qDebug("ChlCase5()  Begin:");
	beforeChlTest();
	START_CHL_UNIT_TEST(Ichl);

	//step1
	QStringList sChl = Ichl->GetChannelList(33);
	QStringList::const_iterator it;
	for (it = sChl.begin(); it != sChl.end(); it ++)
	{
		QVERIFY2((*it).isEmpty() == true,"step 1:");
	}

	//step2
	sChl = Ichl->GetChannelList(1);
	for (it = sChl.begin(); it != sChl.end(); it ++)
	{
		// 和前置条件比对
		QVERIFY2(it->isEmpty() ==false,"step 2:");
		
	}


	 END_CHL_UNIT_TEST(Ichl);
	 qDebug("ChlCase5()  End:");
}

// 6 获取不存在通道的名称              |返回E_CHANNEL_NOT_FOUND
//   获取  存在通道的名称              |返回OK
void Channel_Test::ChlCase6()
{
	qDebug("ChlCase6()  Begin:");
	beforeChlTest();
	START_CHL_UNIT_TEST(Ichl);

	int nRet;
	QString sChl;
	//step1
	nRet = Ichl->GetChannelName(36,sChl);
	QVERIFY2(IChannelManager::E_CHANNEL_NOT_FOUND == nRet,"step 1:");
	//step2
	nRet = Ichl->GetChannelName(1,sChl);
	QVERIFY2(IChannelManager::OK == nRet,"step 2:");
	qDebug()<<"id:1 name: "<<sChl<<'\n';

	END_CHL_UNIT_TEST(Ichl);
	qDebug("ChlCase6()  End:");
}

// 7 获取不存在通道的流                |返回E_CHANNEL_NOT_FOUND
//   获取  存在通道的流                |返回
void Channel_Test::ChlCase7()
{
	qDebug("ChlCase7()  Begin:");
	beforeChlTest();
	START_CHL_UNIT_TEST(Ichl);

	int nRet,nStream = 0;
	//step1
	nRet = Ichl->GetChannelStream(33,nStream);
	QVERIFY2(IChannelManager::E_CHANNEL_NOT_FOUND == nRet,"step 1:");
	//step2
	nRet = Ichl->GetChannelStream(1,nStream);
	QVERIFY2(IChannelManager::OK == nRet,"step 2:");
	qDebug()<<"stream_id:  "<< nStream<<'\n';

	END_CHL_UNIT_TEST(Ichl);
	qDebug("ChlCase7()  End:");
}


// 8 获取不存在通道的NO                |返回E_CHANNEL_NOT_FOUND
//   获取  存在通道的NO                |返回
void Channel_Test::ChlCase8()
{
	qDebug("ChlCase8()  Begin:");
	beforeChlTest();
	START_CHL_UNIT_TEST(Ichl);

	int nRet = 0,nChl = 0;
	//step1
	for (int i = 20; i < 100; ++i)
	{
		nRet = Ichl->GetChannelNumber(i,nChl);
		QVERIFY2(IChannelManager::E_CHANNEL_NOT_FOUND == nRet,"step 1:");
		if (nRet == IChannelManager::OK)
		    printf("Error:  chl_id:\t%d\t  channel_number:\t%d\t\n",i,nChl);
	}
	
	//step2
	nRet = Ichl->GetChannelNumber(1,nChl);
	QVERIFY2(IChannelManager::OK == nRet,"step 2:");

	QVariantMap variantmap;
	variantmap = Ichl->GetChannelInfo(1);
	qDebug()<<"channelnumber:  "<<variantmap["number"].toInt()<<'\n';
	QVERIFY2(nChl == variantmap["number"].toInt(), "step2 :");

	END_CHL_UNIT_TEST(Ichl);
	qDebug("ChlCase8()  End:");
}

// 9 获取不存在通道的信息              |返回E_CHANNEL_NOT_FOUND
//   获取  存在通道的信息              |返回
void Channel_Test::ChlCase9()
{
	qDebug("ChlCase9()  Begin:");
	beforeChlTest();
	START_CHL_UNIT_TEST(Ichl);

	int i ;
	int nRet   = -1;
	int nChlId ;
	QString sName;
	int     nStream = 0;
	int     nChlNum  = 0;
	//step1
	for ( i = 2; i < 100; i++)
	{
		nChlId = i;
		nRet = Ichl->GetChannelInfo(i,sName,nStream, nChlNum);

		QVariantMap variantmap;
		variantmap = Ichl->GetChannelInfo(i);
		QVERIFY2(sName   == variantmap["name"].toString(), "step1 :");
		QVERIFY2(nStream == variantmap["stream"].toInt() , "step1 :");
		QVERIFY2(nChlNum == variantmap["number"].toInt() , "step1 :");
	}

  	//step2

	nRet = Ichl->GetChannelInfo(1,sName,nStream, nChlNum);

	QVariantMap variantmap;
	variantmap = Ichl->GetChannelInfo(1);
	QVERIFY2(sName   == variantmap["name"].toString(), "step1 :");
	QVERIFY2(nStream == variantmap["stream"].toInt() , "step1 :");
	QVERIFY2(nChlNum == variantmap["number"].toInt() , "step1 :");

	END_CHL_UNIT_TEST(Ichl);
	qDebug("ChlCase9()  End:");
}