#include "dvrsearchtest.h"
#include <guid.h>
#include <IEventRegister.h>
#include <IDeviceSearch.h>


#define START_DVRSEARCH_UNIT_TEST(ii) 	IDeviceSearch * ii = NULL; \
	pcomCreateInstance(CLSID_DvrSearch, NULL, IID_IDeviceSearch,(void **)&ii); \
	QVERIFY2(NULL != ii,"Create dvr search instance");

#define END_DVRSEARCH_UNIT_TEST(ii) ii->Release(); 

DvrSearchTest::DvrSearchTest()
{

}

DvrSearchTest::~DvrSearchTest()
{

}

//测试用例: 
//前置: 用2台DVR, 设置参数.      再定义两个回调函数 , 对每次得到的数据进行打印输出.
int DvrSearchTest::cbDeviceFoundTest(QString evName,QVariantMap evMap,void *pUser)
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
int DvrSearchTest::cbDeviceSetTest(QString evName,QVariantMap evMap,void *pUser)
{
    QVariantMap::const_iterator it;
    for (it= evMap.begin();it != evMap.end(); ++it )
    {
        QString sKey = it.key();
        QString sValue = it.value().toString();
        printf("%s\t%s\n", sKey.toLatin1().data(),sValue.toLatin1().data());
    }
    return 0;
}

//1  使用事件名SearchDeviceSuccess注册, 测试正常运行情况.          |  Start()返回0, Stop()返回0, 回调函数能得到相应数据并能打印出来
//    使用事件名deviceSet 注册 , 测试无此事件处理时情况.   |  Start()返回0, Stop()返回0, 由于没有对应处理函数而使得回调函数不会打印任何数据
void DvrSearchTest::DvrSearchCase1()
{
    START_DVRSEARCH_UNIT_TEST(dvrsearch);
    //step1:
    IEventRegister *pRet = dvrsearch->QueryEventRegister();
    QVERIFY2(NULL != pRet,"No Event Register");
    int nRet1 = pRet->registerEvent("SearchDeviceSuccess",DvrSearchTest::cbDeviceFoundTest, NULL);
    pRet->Release();
    QVERIFY2(-2 == nRet1,"No this Event");
    nRet1 = dvrsearch->Start();
    QVERIFY2(0 == nRet1,"Start() already called");

    QTime tim;
    tim.start();
    while(tim.elapsed()< 20*1000);

    dvrsearch->Stop();
    QVERIFY2(0 == nRet1,"Stop() already called");
    //step2:
    pRet = dvrsearch->QueryEventRegister();
    QVERIFY2(NULL != pRet,"No Event Register");
    nRet1 = pRet->registerEvent("deviceSet",DvrSearchTest::cbDeviceSetTest, NULL);
    pRet->Release();
    QVERIFY2(-2 == nRet1,"No this Event");
    nRet1 = dvrsearch->Start();
    QVERIFY2(0 == nRet1,"Start() already called");

    tim.start();
    while(tim.elapsed()< 20*1000);

    nRet1 = dvrsearch->Stop();
    QVERIFY2(0 == nRet1,"Stop() already called");
    END_DVRSEARCH_UNIT_TEST(dvrsearch);
}

//2  使用事件名SearchDeviceSuccess注册,启动后Stop()停止,然后刷新再调用Start()启动, 最后再停止
//       |    Start()和Stop()两次返回0,  两次回调函数都能得到相应数据并打印出来       
void DvrSearchTest::DvrSearchCase2()
{
    START_DVRSEARCH_UNIT_TEST(dvrsearch);
    IEventRegister *pRet = dvrsearch->QueryEventRegister();
    QVERIFY2(NULL != pRet,"No Event Register");
    int nRet1 = pRet->registerEvent("SearchDeviceSuccess",DvrSearchTest::cbDeviceFoundTest, NULL);
    QVERIFY2(-2 == nRet1,"SearchDeviceSuccess Event is not exist!");
    pRet->Release();
    nRet1 = dvrsearch->Start();
    QVERIFY2(0 == nRet1,"Start() already called");

    QTime tim;
    tim.start();
    while(tim.elapsed()< 20*1000);

    nRet1 = dvrsearch->Stop();
    QVERIFY2(0 == nRet1,"Stop() already called");


    dvrsearch->Flush();
    nRet1 =  dvrsearch->Start();
    QVERIFY2(-1 == nRet1,"Start() already called");
    tim.start();
    while(tim.elapsed()< 20*1000);

    nRet1 = dvrsearch->Stop();
    QVERIFY2(0 == nRet1,"Stop() already called");

    END_DVRSEARCH_UNIT_TEST(dvrsearch);
}

//3  使用事件名SearchDeviceSuccess注册, 启动后不调用Stop, 再调用Start启动, 然后调用Stop. | 第一次Start()返回0, 第二次返回-1
//        Stop()返回0
void DvrSearchTest::DvrSearchCase3()
{
    START_DVRSEARCH_UNIT_TEST(dvrsearch);
    //step1
    IEventRegister *pRet = dvrsearch->QueryEventRegister();
    QVERIFY2(NULL != pRet,"No Event Register");
    int nRet1 = pRet->registerEvent("SearchDeviceSuccess",DvrSearchTest::cbDeviceFoundTest, NULL);
    QVERIFY2(-2 == nRet1,"No this Event");
    pRet->Release();
    nRet1 = dvrsearch->Start();
    QVERIFY2(0 == nRet1,"Start() already called");

    QTime tim;
    tim.start();
    while(tim.elapsed()< 25*1000);

    nRet1 = dvrsearch->Start();
    QVERIFY2(-1 == nRet1,"Start() didn't called");

    tim.start();
    while(tim.elapsed()< 25*1000);

    nRet1 = dvrsearch->Stop();
    QVERIFY2(0 == nRet1,"Stop() already called");

    END_DVRSEARCH_UNIT_TEST(dvrsearch);
}

//4 使用事件名SearchDeviceSuccess注册, 启动,调用setInterval()设置间隔(较大更明显),观察打印间隔的变化 |  Start()和Stop()均返回0,setInterval()返回0
void DvrSearchTest::DvrSearchCase4()
{
    START_DVRSEARCH_UNIT_TEST(dvrsearch);
    IEventRegister *pRet = dvrsearch->QueryEventRegister();
    QVERIFY2(NULL != pRet, "No Event Register");
    int nRet1 = pRet->registerEvent("SearchDeviceSuccess",DvrSearchTest::cbDeviceFoundTest, NULL);
    pRet->Release();
    QVERIFY2(-2 == nRet1,"This Event exist");
    nRet1 = dvrsearch->Start();
    QVERIFY2(0 == nRet1,"Start() already called");

    QTime tim;
    tim.start();
    while(tim.elapsed()< 25*1000);

    nRet1 = dvrsearch->setInterval(8);
    QVERIFY2(0 == nRet1,"Interval is not set currect");
    nRet1 = dvrsearch->Start();
    QVERIFY2(-1 == nRet1,"Start() already called");
    tim.start();
    while(tim.elapsed()< 25*1000);

    nRet1 = dvrsearch->Stop();
    QVERIFY2(0 == nRet1,"Stop() already called");

    END_DVRSEARCH_UNIT_TEST(dvrsearch);
}

//5 先使用事件deviceSet注册,再用SearchDeviceSuccess注册,使用eventList()返回事件列表 | registerEvent()均返回-2, Start()和Stop()均返回0,
void DvrSearchTest::DvrSearchCase5()
{
    START_DVRSEARCH_UNIT_TEST(dvrsearch);
    IEventRegister *pRet = dvrsearch->QueryEventRegister();
    QVERIFY2(NULL != pRet,"No Event Register");
    pRet->Release();
    int nRet1 = pRet->registerEvent("deviceSet",DvrSearchTest::cbDeviceSetTest, NULL);
    QVERIFY2(-2 == nRet1,"Event Exists");
    nRet1 = pRet->registerEvent("SearchDeviceSuccess"  ,DvrSearchTest::cbDeviceFoundTest, NULL);
    QVERIFY2(-2 == nRet1,"Event Exists");

    nRet1 = dvrsearch->Start();
    QVERIFY2(0 == nRet1,"Start() already called");
    QTime tim;
    tim.start();
    while(tim.elapsed()< 25*1000);

    QStringList strList =  pRet->eventList();
    QStringList::const_iterator it;
    for (it = strList.begin(); it != strList.end(); it ++)
    {
        QVERIFY2(*it == "deviceSet" || *it == "SearchDeviceSuccess"," EventList Wrong");
    }
    nRet1 = dvrsearch->Stop();
    QVERIFY2(0 == nRet1, "Stop() already called");
    END_DVRSEARCH_UNIT_TEST(dvrsearch);
}

//6 先使用事件deviceSet注册,再用SearchDeviceSuccess注册,使用queryEvent()进行查询      |  registerEvent()均返回-2, Start()和Stop()均返回0,
//  "deviceSet"事件的queryEvent()返回-1, "SearchDeviceSuccess"事件的queryEvent()返回0
void DvrSearchTest::DvrSearchCase6()
{
    START_DVRSEARCH_UNIT_TEST(dvrsearch);
    IEventRegister *pRet = dvrsearch->QueryEventRegister();
    QVERIFY2(NULL != pRet,"No Event Register");
    pRet->Release();
    int nRet1 = pRet->registerEvent("deviceSet",DvrSearchTest::cbDeviceSetTest, NULL);
    QVERIFY2(-2 == nRet1, "Event Exists");
    nRet1 = pRet->registerEvent("SearchDeviceSuccess",DvrSearchTest::cbDeviceFoundTest, NULL);
    QVERIFY2(-2 == nRet1, "Event Exists");

    nRet1 = dvrsearch->Start();
    QVERIFY2(0 == nRet1,"Start() already called");

    QTime tim;
    tim.start();
    while(tim.elapsed()< 25*1000);

    QStringList evParamList;
    nRet1 =  pRet->queryEvent("deviceSet", evParamList);
    QVERIFY2(-1 == nRet1,"Error");

    nRet1 =  pRet->queryEvent("SearchDeviceSuccess", evParamList);
    QVERIFY2(0 == nRet1,"Matched Error");

    dvrsearch->Stop();
    QVERIFY2(0 == nRet1,"Stop() already called");

    END_DVRSEARCH_UNIT_TEST(dvrsearch);
}

//7 用SearchDeviceSuccess连续注册2次, 观察registerEvent()的返回值, 并用eventList()列出事件列表
// | registerEvent()第一次返回-2,第二次返回-1, eventList()返回的list只有一个"SearchDeviceSuccess"
void DvrSearchTest::DvrSearchCase7()
{
    START_DVRSEARCH_UNIT_TEST(dvrsearch);
    IEventRegister *pRet = dvrsearch->QueryEventRegister();
    QVERIFY2(NULL != pRet,"No Event Register");
    pRet->Release();
    int nRet1 = pRet->registerEvent("SearchDeviceSuccess",DvrSearchTest::cbDeviceFoundTest, NULL);
    QVERIFY2(-2 == nRet1, "Event Exists");
    nRet1 = pRet->registerEvent("SearchDeviceSuccess",DvrSearchTest::cbDeviceFoundTest, NULL);
    QVERIFY2(-1 == nRet1, "Event Not Exists");

    nRet1 = dvrsearch->Start();
    QVERIFY2(0 == nRet1,"Start() already called");

    QTime tim;
    tim.start();
    while(tim.elapsed()< 25*1000);

    QStringList evList;
    evList =  pRet->eventList();
    QVERIFY2(1 == evList.count("SearchDeviceSuccess"),"Error");

    dvrsearch->Stop();
    QVERIFY2(0 == nRet1,"Stop() already called");

    END_DVRSEARCH_UNIT_TEST(dvrsearch);
}