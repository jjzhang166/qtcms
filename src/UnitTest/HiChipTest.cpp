#include "HiChipTest.h"
#include <QDebug>
#include <guid.h>
#include <IEventRegister.h>
#include <IDeviceSearch.h>
#include <QDateTime>
#include <QWaitCondition>
#include <QMutex>

#define START_HICHIPSEARCH_UNIT_TEST(ii) 	IDeviceSearch * ii = NULL; \
	pcomCreateInstance(CLSID_HiChipSearch,NULL,IID_IDeviceSearch,(void **)&ii); \
	QVERIFY2(NULL != ii,"Create device manager instance");\
	QMutex Sig;\
	Sig.lock();

#define END_HICHIPSEARCH_UNIT_TEST(ii)		Sig.unlock();\
	ii->Release(); 

//测试用例：
//前置：,与测试组合作，添加一台IPC，记录默认参数，确保测试过程中数据不被修改。
//1,不注册事件SearchDeviceSuccess，调用start（）开始搜索，通过回调函数取得							｜start返回0，取不到数据，stop返回0,不能取得数据
//查询信息与设置好的数据对比，查看是否一致。调用stop结束。  
//2,注册事件SearchDeviceSuccess，调用start（）开始搜索，通过回调函数取得							｜start返回0，回调取得的数据与设置的数据一致，stop返回0,不能取得数据
//查询信息与设置好的数据对比，查看是否一致。 调用stop结束。 
//3,注册事件SearchDeviceSuccess，调用start（）开始搜索，再次调用start（）通							｜start第一次返回0，取得的查询信息与设置的一致，第二次返回-1，stop返回0,不能取得数据
//过回调函数取得查询信息与设置好的数据对比，查看是否一致。调用stop结束。
//4,注册事件SearchDeviceSuccess，调用start（）开始搜索，调用start（），再							｜start返回0，取得的查询信息与设置的一致，stop第一次返回0，第二次返回-1,不能取得数据
//次调用stop（）通过回调函数取得查询信息与设置好的数据对比，查看是否一致。
//5,注册事件SearchDeviceSuccess，调用start（）开始搜索，调用flush（）刷新，							｜start返回0，flush返回0，回调取得的数据与设置的数据一致，stop返回0,不能取得数据
//通过回调函数取得查询信息与设置好的数据对比，查看是否一致。调用stop结束。 
//6,注册事件SearchDeviceSuccess，调用start（）开始搜索，记录收到数据的时间T1。						｜start返回OK，setInterval返回OK，T2>T1且T2=30秒
//调用setInterval（）设置时间间隔为30S，记录收到数据的时间T2，比较T1,T2。


int __cdecl OUTPUT(QString evname,QVariantMap info,void *puser)
{
	if ( "SearchDeviceSuccess" == evname )
	{
		QVariantMap::iterator it;
		for (it = info.begin(); it != info.end(); it++)
		{
			qDebug()<<it.key()<<":"<<it.value().toString();
		}
	}
	return 0;
}


HiChipUnitTest::HiChipUnitTest()
{

}

HiChipUnitTest::~HiChipUnitTest()
{

}


void WaitSeconds(int second)
{
	QDateTime start = QDateTime::currentDateTime();
	while(1)
	{
		QDateTime end = QDateTime::currentDateTime();
		if (end.toTime_t() - start.toTime_t() > second)
		{
			break;
		}		
	}
}

void HiChipUnitTest::DeviceTestCase1()
{
	START_HICHIPSEARCH_UNIT_TEST(ITest);

	int Ret = ITest->Start();
	QVERIFY2(0 == Ret, "start thread");

	WaitSeconds(15);
	Ret = ITest->Stop();
	QVERIFY2(0 == Ret, "stop thread");

	END_HICHIPSEARCH_UNIT_TEST(ITest);
}

void HiChipUnitTest::DeviceTestCase2()
{
	START_HICHIPSEARCH_UNIT_TEST(ITest);

	QString evName("SearchDeviceSuccess");
	int Ret = ITest->QueryEventRegister()->registerEvent(evName, OUTPUT,NULL);
	QVERIFY2(IEventRegister::OK == Ret, "Regist event success");

	Ret = ITest->Start();
	QVERIFY2(0 == Ret, "start thread");
	WaitSeconds(15);
	Ret = ITest->Stop();
	QVERIFY2(0 == Ret, "stop thread");

	END_HICHIPSEARCH_UNIT_TEST(ITest);
}


void HiChipUnitTest::DeviceTestCase3()
{
	START_HICHIPSEARCH_UNIT_TEST(ITest);

	QString evName("SearchDeviceSuccess");
	int Ret = ITest->QueryEventRegister()->registerEvent(evName, OUTPUT,NULL);
	QVERIFY2(IEventRegister::OK == Ret, "Regist event success");
	Ret = ITest->Start();
	QVERIFY2(0 == Ret, "start thread");

	WaitSeconds(10);
	Ret = ITest->Start();
	QVERIFY2(-1 == Ret, "start thread again");

	Ret = ITest->Stop();
	QVERIFY2(0 == Ret, "stop thread");

	END_HICHIPSEARCH_UNIT_TEST(ITest);
}

void HiChipUnitTest::DeviceTestCase4()
{
	START_HICHIPSEARCH_UNIT_TEST(ITest);

	QString evName("SearchDeviceSuccess");
	int Ret = ITest->QueryEventRegister()->registerEvent(evName, OUTPUT,NULL);
	QVERIFY2(IEventRegister::OK == Ret, "Regist event success");
	Ret = ITest->Start();
	QVERIFY2(0 == Ret, "start thread");

	WaitSeconds(10);
	Ret = ITest->Stop();
	QVERIFY2(0 == Ret, "stop thread");

	Ret = ITest->Stop();
	QVERIFY2(-1 == Ret, "stop thread again");

	END_HICHIPSEARCH_UNIT_TEST(ITest);
}

void HiChipUnitTest::DeviceTestCase5()
{
	START_HICHIPSEARCH_UNIT_TEST(ITest);

	QString evName("SearchDeviceSuccess");
	int Ret = ITest->QueryEventRegister()->registerEvent(evName, OUTPUT,NULL);
	QVERIFY2(IEventRegister::OK == Ret, "Regist event success");
	Ret = ITest->Start();
	QVERIFY2(0 == Ret, "start thread");

	WaitSeconds(10);
	Ret = ITest->Flush();
	QVERIFY2(0 == Ret, "flush");

	Ret = ITest->Stop();
	QVERIFY2(0 == Ret, "stop thread");

	END_HICHIPSEARCH_UNIT_TEST(ITest);
}
//此用例无法记录时间间隔
void HiChipUnitTest::DeviceTestCase6()
{
	START_HICHIPSEARCH_UNIT_TEST(ITest);

	QString evName("SearchDeviceSuccess");
	int Ret = ITest->QueryEventRegister()->registerEvent(evName, OUTPUT,NULL);
	QVERIFY2(IEventRegister::OK == Ret, "Regist event success");
	Ret = ITest->Start();
	QVERIFY2(0 == Ret, "start thread");

	WaitSeconds(10);
	Ret = ITest->setInterval(10);
	QVERIFY2(0 == Ret, "change time interval");

	Ret = ITest->Stop();
	QVERIFY2(0 == Ret, "stop thread");

	END_HICHIPSEARCH_UNIT_TEST(ITest);
}

