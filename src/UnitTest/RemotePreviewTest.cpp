#include "RemotePreviewTest.h"
#include <guid.h>
#include <IEventRegister.h>
#include <IDeviceConnection.h>
#include <IRemotePreview.h>

#define START_BUBBLEPROTOCOL_UNIT_TEST(ii) 	IRemotePreview * ii = NULL; \
    pcomCreateInstance(CLSID_RemotePreview, NULL, IID_IRemotePreview,(void **)&ii); \
    QVERIFY2(NULL != ii,"Create Bubble Protocol instance");

#define END_BUBBLEPROTOCOL_UNIT_TEST(ii) ii->Release(); 


RemotePreviewTest::RemotePreviewTest()
{
}

RemotePreviewTest::~RemotePreviewTest()
{
}

//测试用例: 
//前置: 用2台能传输视频数据的DVR.    再定义两个回调函数, 一个LiveStream,另一个Other ,
//
int RemotePreviewTest::cbLiveStream(QString evName, QVariantMap evMap, void *pUser)
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

int RemotePreviewTest::cbOther(QString evName, QVariantMap evMap, void *pUser)
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

//1. 使用LiveStream注册,设置合法的IP地址和端口,connectToDevice()连接到设备获取设备信息,获取码流,测试正常运行情况,
// 然后调用stopStream()关闭码流,disconnect(),中间调用getCurrentStatus()取得当前状态    | 调用getLiveStream()后打印出结果, 调用stopStream()后不再打印
void RemotePreviewTest::RemotePreviewCase1()
{
    START_BUBBLEPROTOCOL_UNIT_TEST(Itest);
    QString evName("LiveStream");
    IEventRegister *pRegist = NULL;
    int nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    pRegist->Release();
    nRet = pRegist->registerEvent(evName, cbLiveStream, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");

    IDeviceConnection *pConnect = NULL;
    nRet = Itest->QueryInterface(IID_IDeviceConnection, (void**)&pConnect);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    nRet = pConnect->setDeviceHost("192.168.2.171");
    QVERIFY2(0 == nRet, "set Device host Error");
    QString host = pConnect->getDeviceHost();
    QVERIFY2("192.168.1.207" == host, "device host is wrong");

    QVariantMap ports;
    ports.insert("media", 80);
    nRet = pConnect->setDevicePorts(ports);
    QVERIFY2(0 == nRet, "set Device port wrong");
    ports.clear();
    ports = pConnect->getDevicePorts();
    if (ports.contains("media"))
    {
        QVERIFY2(80 == ports.value("media"), "set device port error");
    }
    nRet = pConnect->connectToDevice();
    QVERIFY2(0 == nRet, "connect to device Error");

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connect Error");

    nRet = Itest->getLiveStream(0, 0);
    QVERIFY2(0 == nRet, "Get live stream Error");

    QEventLoop eventloop;
    QTimer::singleShot(5000, &eventloop, SLOT(quit()));
    eventloop.exec();

    nRet = Itest->stopStream();
    QVERIFY2(0 == nRet, "Stop live stream Error");

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connected Error");

    nRet = pConnect->disconnect();
    QVERIFY2(0 == nRet, "disconnect Error");

    QTest::qWait(1000);

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Disconnected == nRet, "Disconnected Error");

    END_BUBBLEPROTOCOL_UNIT_TEST(Itest);
}
// 2.  使用其他名字如RemotePreview注册,connectToDevice()连接到设备,获取码流, 测试运行情况,
// 然后调用stopStream()关闭码流,再disconnect().中间调用getCurrentStatus()取得当前状态   |
void RemotePreviewTest::RemotePreviewCase2()
{
    START_BUBBLEPROTOCOL_UNIT_TEST(Itest);
    QString evName("RemotePreview");
    IEventRegister *pRegist = NULL;
    int nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    pRegist->Release();
    nRet = pRegist->registerEvent(evName, cbOther, NULL);
    QVERIFY2(IEventRegister::E_EVENT_NOT_SUPPORT == nRet, "Event Regist Error!");

    IDeviceConnection *pConnect = NULL;
    nRet = Itest->QueryInterface(IID_IDeviceConnection, (void**)&pConnect);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    nRet = pConnect->setDeviceHost("192.168.1.207");
    QVERIFY2(0 == nRet, "set Device host Error");
    QString host = pConnect->getDeviceHost();
    QVERIFY2("192.168.1.207" == host, "device host is wrong");

    QVariantMap ports;
    ports.insert("media", 80);
    nRet = pConnect->setDevicePorts(ports);
    QVERIFY2(0 == nRet, "set Device port wrong");
    ports.clear();
    ports = pConnect->getDevicePorts();
    if (ports.contains("media"))
    {
        QVERIFY2(80 == ports.value("media"), "set device port error");
    }
    nRet = pConnect->connectToDevice();
    QVERIFY2(0 == nRet, "connect to device Error");

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connect Error");

    nRet = Itest->getLiveStream(0, 0);
    QVERIFY2(0 == nRet, "Get live stream Error");
    
    nRet = Itest->stopStream();
    QVERIFY2(0 == nRet, "Stop live stream Error");

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connected Error");

    nRet = pConnect->disconnect();
    QVERIFY2(0 == nRet, "disconnect Error");

    QTest::qWait(1000);

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Disconnected == nRet, "Disconnected Error");

    END_BUBBLEPROTOCOL_UNIT_TEST(Itest);
}
// 3. 使用LiveStream注册,调用setDeviceHost()设置多种非法的IP, 与getDeviceHost()返回值比较 | setDeviceHost()返回-1; 
//   使用LiveStream注册,调用setDeviceHost()设置合法的IP地址, 与getDeviceHost()返回值比较 | setDeviceHost()返回0,  
void RemotePreviewTest::RemotePreviewCase3()
{
    START_BUBBLEPROTOCOL_UNIT_TEST(Itest);
    QString evName("LiveStream");
    IEventRegister *pRegist = NULL;
    int nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    pRegist->Release();
    nRet = pRegist->registerEvent(evName, cbLiveStream, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");

    IDeviceConnection *pConnect = NULL;
    nRet = Itest->QueryInterface(IID_IDeviceConnection, (void**)&pConnect);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    //step1
    nRet = pConnect->setDeviceHost("192");
    QVERIFY2(-1 == nRet, "set Device host Error");
    QString host = pConnect->getDeviceHost();
    QVERIFY2("" == host, "Get device host Error");
    nRet = pConnect->setDeviceHost("192.256.256.2");
    QVERIFY2(-1 == nRet, "set Device host Error");
    host = pConnect->getDeviceHost();
    QVERIFY2("" == host, "Get device host Error");
    nRet = pConnect->setDeviceHost("192.253.253.-2");
    QVERIFY2(-1 == nRet, "set Device host Error");
    host = pConnect->getDeviceHost();
    QVERIFY2("" == host, "Get device host Error");
    //step2
    nRet = pConnect->setDeviceHost("192.168.2.73");
    QVERIFY2(0 == nRet, "set Device host Error");
    host = pConnect->getDeviceHost();
    QVERIFY2("192.168.2.73" == host, "Get device host Error");

    END_BUBBLEPROTOCOL_UNIT_TEST(Itest);
}
//4. 使用LiveStream注册,调用setDevicePorts()设置多种非法的端口, 与getDeviceHost()返回值比较 | setDevicePorts()返回-1;
//   使用LiveStream注册,调用setDevicePorts()设置合法的Port号,  与getDeviceHost()返回值比较 | setDevicePorts()返回0
void RemotePreviewTest::RemotePreviewCase4()
{
    START_BUBBLEPROTOCOL_UNIT_TEST(Itest);
    QString evName("LiveStream");
    IEventRegister *pRegist = NULL;
    int nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    pRegist->Release();
    nRet = pRegist->registerEvent(evName, cbLiveStream, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");

    IDeviceConnection *pConnect = NULL;
    nRet = Itest->QueryInterface(IID_IDeviceConnection, (void**)&pConnect);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");
    // step1
    QVariantMap ports;
    ports.insert("media", 65537);
    nRet = pConnect->setDevicePorts(ports);
    QVERIFY2(-1 == nRet, "set Device port wrong");
    ports.clear();
    ports = pConnect->getDevicePorts();
    if (ports.contains("media"))
    {
        QVERIFY2(ports.isEmpty(), "set device port error");
    }

    ports.insert("media", -5);
    nRet = pConnect->setDevicePorts(ports);
    QVERIFY2(-1 == nRet, "set Device port wrong");
    ports.clear();
    ports = pConnect->getDevicePorts();
    if (ports.contains("media"))
    {
        QVERIFY2(ports.isEmpty(), "set device port error");
    }

    //step2
    ports.insert("media", 80);
    nRet = pConnect->setDevicePorts(ports);
    QVERIFY2(0 == nRet, "set Device port right");
    ports.clear();
    ports = pConnect->getDevicePorts();
    if (ports.contains("media"))
    {
        QVERIFY2(80 == ports.value("media"), "set device port error");
    }

    END_BUBBLEPROTOCOL_UNIT_TEST(Itest);
}
//5. 使用LiveStream注册,调用setDeviceId()设置设备ID, 与getDeviceid()返回值比较 | setDeviceId()返回0
void RemotePreviewTest::RemotePreviewCase5()
{
    START_BUBBLEPROTOCOL_UNIT_TEST(Itest);
    QString evName("LiveStream");
    IEventRegister *pRegist = NULL;
    int nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    pRegist->Release();
    nRet = pRegist->registerEvent(evName, cbLiveStream, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");

    IDeviceConnection *pConnect = NULL;
    nRet = Itest->QueryInterface(IID_IDeviceConnection, (void**)&pConnect);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    nRet = pConnect->setDeviceId("___**192XXX");
    QVERIFY2(0 == nRet, "set Device Id Error");
    QString sDevId = pConnect->getDeviceid();
    QVERIFY2("___**192XXX" == sDevId, "Get device Id Error");
}
//6. 使用LiveStream注册,设置合法的IP地址和端口后,connectToDevice()连接到设备获取设备信息,测试调用getStreamInfo()
//正确参数和错误参数时的情况和getStreamCount()的值,最后disconnect()   .中间调用getCurrentStatus()取得当前状态
// | getStreamInfo()参数给错时返回-1,正确返回0, getStreamCount()192.168.1.207这个IP对应的返回值是3
void RemotePreviewTest::RemotePreviewCase6()
{
    START_BUBBLEPROTOCOL_UNIT_TEST(Itest);
    QString evName("LiveStream");
    IEventRegister *pRegist = NULL;
    int nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    pRegist->Release();
    nRet = pRegist->registerEvent(evName, cbLiveStream, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");

    IDeviceConnection *pConnect = NULL;
    nRet = Itest->QueryInterface(IID_IDeviceConnection, (void**)&pConnect);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    nRet = pConnect->setDeviceHost("192.168.1.207");
    QVERIFY2(0 == nRet, "set Device host Error");
    QString host = pConnect->getDeviceHost();
    QVERIFY2("192.168.1.207" == host, "device host is wrong");

    QVariantMap ports;
    ports.insert("media", 80);
    nRet = pConnect->setDevicePorts(ports);
    QVERIFY2(0 == nRet, "set Device port wrong");
    ports.clear();
    ports = pConnect->getDevicePorts();
    if (ports.contains("media"))
    {
        QVERIFY2(80 == ports.value("media"), "set device port error");
    }
    nRet = pConnect->connectToDevice();
    QVERIFY2(0 == nRet, "connect to device Error");

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connect Error");
    
	nRet = Itest->getLiveStream(0,0);
	QVERIFY2(0 == nRet, "get live stream");

    //step1
    QVariantMap streamInfo;
    nRet = Itest->getStreamInfo(3, streamInfo);
    QVERIFY2(-1 == nRet, "Get  Info Error");

    //step2
    streamInfo.clear();
    nRet = Itest->getStreamInfo(2, streamInfo);
    QVERIFY2(0 == nRet, "Get  Info Error");
    QVERIFY2(streamInfo.value(QString("name")) == QVariant("qvga.264"), "Get Info Error");
    QVERIFY2(streamInfo.value(QString("size")) == QVariant("320x240"), "Get Info Error");
    QVERIFY2(streamInfo.value(QString("x1")) == QVariant("yes"), "Get Info Error");
    QVERIFY2(streamInfo.value(QString("x2")) == QVariant("yes"), "Get Info Error");
    QVERIFY2(streamInfo.value(QString("x4")) == QVariant("yes"), "Get Info Error");

    nRet = Itest->getStreamCount();
    QVERIFY2(3 == nRet, "Get Stream count Error");

    nRet = pConnect->disconnect();
    QVERIFY2(0 == nRet, "disconnect Error");

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Disconnected == nRet, "Disconnected Error");

    END_BUBBLEPROTOCOL_UNIT_TEST(Itest);
}
//7. 使用LiveStream注册,设置合法的IP地址和端口后,connectToDevice()连接到设备获取设备信息, getLiveStream()后调用pauseStream()暂停, 
//然后调用stopStream()停止 最后disconnect()|调用getLiveStream()后打印出结果, 调用pauseStream()后不再打印 
void RemotePreviewTest::RemotePreviewCase7()
{
    START_BUBBLEPROTOCOL_UNIT_TEST(Itest);
    int nTmp = 0;
    QString evName("LiveStream");
    IEventRegister *pRegist = NULL;
    int nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    pRegist->Release();
    nRet = pRegist->registerEvent(evName, cbLiveStream, NULL);
    QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");

    IDeviceConnection *pConnect = NULL;
    nRet = Itest->QueryInterface(IID_IDeviceConnection, (void**)&pConnect);
    QVERIFY2(S_OK == nRet, "QueryInterface Error!");

    nRet = pConnect->setDeviceHost("192.168.1.207");
    QVERIFY2(0 == nRet, "set Device host Error");
    QString host = pConnect->getDeviceHost();
    QVERIFY2("192.168.1.207" == host, "device host is wrong");

    QVariantMap ports;
    ports.insert("media", 80);
    nRet = pConnect->setDevicePorts(ports);
    QVERIFY2(0 == nRet, "set Device port wrong");
    ports.clear();
    ports = pConnect->getDevicePorts();
    if (ports.contains("media"))
    {
        QVERIFY2(80 == ports.value("media"), "set device port error");
    }
    nRet = pConnect->connectToDevice();
    QVERIFY2(0 == nRet, "connect to device Error");

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connect Error");

    nRet = Itest->getLiveStream(0, 0);
    QVERIFY2(0 == nRet, "Get live stream Error");

    QEventLoop eventloop;
    QTimer::singleShot(5000, &eventloop, SLOT(quit()));
    eventloop.exec();

    nRet = Itest->pauseStream(true);
    QVERIFY2(0 == nRet, "Pause stream Error");

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connected Error");

    nRet = Itest->stopStream();
    QVERIFY2(0 == nRet, "Stop live stream Error");

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connected Error");
    
    nRet = pConnect->disconnect();
    QVERIFY2(0 == nRet, "disconnect Error");

    QTest::qWait(1000);

    nRet = pConnect->getCurrentStatus();
    QVERIFY2(IDeviceConnection::CS_Disconnected == nRet, "Disconnected Error");

    END_BUBBLEPROTOCOL_UNIT_TEST(Itest);
}

//8. 使用LiveStream注册,设置合法的IP地址和端口后,connectToDevice()连接到设备获取设备信息,分正确和错误情况来设置setDeviceAuthorityInfomation授权信息.
//经验证192.168.2.73的帐号：admin｜""可用。调用authority()获取授权信息通过与否,然后调用disconnect()停止.  | 授权通过时authority()返回0,否则校验失败.
void RemotePreviewTest::RemotePreviewCase8()
{
	START_BUBBLEPROTOCOL_UNIT_TEST(Itest);
	int nTmp = 0;
	QString evName("LiveStream");
	IEventRegister *pRegist = NULL;
	int nRet = Itest->QueryInterface(IID_IEventRegister, (void**)&pRegist);
	QVERIFY2(S_OK == nRet, "QueryInterface Error!");

	pRegist->Release();
	nRet = pRegist->registerEvent(evName, cbLiveStream, NULL);
	QVERIFY2(IEventRegister::OK == nRet, "Event Regist Error!");

	IDeviceConnection *pConnect = NULL;
	nRet = Itest->QueryInterface(IID_IDeviceConnection, (void**)&pConnect);
	QVERIFY2(S_OK == nRet, "QueryInterface Error!");

	nRet = pConnect->setDeviceHost("192.168.2.73");
	QVERIFY2(0 == nRet, "set Device host Error");

	QVariantMap ports;
	ports.insert("media", 80);
	nRet = pConnect->setDevicePorts(ports);
	QVERIFY2(0 == nRet, "set Device port wrong");

	QString sName("admin"),sPwd("");
	nRet = pConnect->setDeviceAuthorityInfomation(sName,sPwd);
	QVERIFY2(0 == nRet, "Authority setted Error");

	nRet = pConnect->connectToDevice();
	QVERIFY2(0 == nRet, "connect to device Error");

	nRet = pConnect->getCurrentStatus();
	QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connect Error");

	nRet = pConnect->authority();
	QVERIFY2(0 == nRet, "Authority Error");

	nRet = pConnect->disconnect();
	QVERIFY2(0 == nRet, "disconnect Error");

	QTest::qWait(1000);

	sName = "xxee";
	nRet = pConnect->setDeviceAuthorityInfomation(sName,sPwd);
	QVERIFY2(0 == nRet, "Authority setted Error");

	nRet = pConnect->connectToDevice();
	QVERIFY2(0 == nRet, "connect to device Error");

	nRet = pConnect->getCurrentStatus();
	QVERIFY2(IDeviceConnection::CS_Connected == nRet, "Connect Error");

	nRet = pConnect->authority();
	QVERIFY2(1 == nRet, "Authority Error");

	nRet = pConnect->disconnect();
	QVERIFY2(0 == nRet, "disconnect Error");

	nRet = pConnect->getCurrentStatus();
	QVERIFY2(IDeviceConnection::CS_Disconnected == nRet, "Disconnected Error");

	END_BUBBLEPROTOCOL_UNIT_TEST(Itest);
}