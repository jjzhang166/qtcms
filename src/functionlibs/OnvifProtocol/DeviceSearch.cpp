#include "DeviceSearch.h"
#include <QTime>
#include <QEventLoop>
#include <QTimer>
#include <QCoreApplication>
#include <QStringList>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDebug>

DeviceSearch::DeviceSearch()
	: QThread(),
	m_bStop(true),
	m_bFlush(false),
	m_i32Interval(10)
{

}

DeviceSearch::~DeviceSearch()
{
	qDebug()<<__FUNCTION__<<__LINE__<<"stop out:"<<this<<this->thread()->currentThread()->currentThreadId();;
}

int DeviceSearch::Start()
{
	if (!isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"start:"<<this<<this->thread()->currentThread()->currentThreadId();;
		start();
	}
	m_bStop = false;
	return 0;
}

int DeviceSearch::Stop()
{
	m_bStop = true;
	while (isRunning())
	{
		QTime dieTime=QTime::currentTime().addMSecs(1);
		while(QTime::currentTime()<dieTime){
			QCoreApplication::processEvents(QEventLoop::AllEvents,10);
		}
	}
	qDebug()<<__FUNCTION__<<__LINE__<<"stop out:"<<this<<this->thread()->currentThread()->currentThreadId();;
	return 0;
}

int DeviceSearch::Flush()
{
	m_bFlush = true;
	return 0;
}

int DeviceSearch::setInterval( int nInterval )
{
	if (nInterval > 0 && nInterval < 100)
	{
		m_i32Interval = nInterval;
		return 0;
	}
	else
		return 1;
}

void DeviceSearch::run()
{
	//get all ip address
	QStringList ipList;
	QList<QHostAddress> addrList = QNetworkInterface::allAddresses();
	foreach(QHostAddress ip, addrList)
	{
		if (QAbstractSocket::IPv4Protocol == ip.protocol())
		{
			ipList.append(ip.toString());
		}
	}

	//send search msg by bind each ip
	ONVIF_CLIENT_init(1, 1, 1, false, 2);
	for (int index = 0; index < ipList.size(); ++index)
	{
		ONVIF_search(ONVIF_DEV_NVT, false, 2, cbSearchHook, ipList[index].toLatin1().data(), this);
	}

	QTime timer;
	timer.start();
	while (!m_bStop)
	{
		if (timer.elapsed() > m_i32Interval*1000 || m_bFlush)
		{
			timer.start();
			for (int index = 0; index < ipList.size(); ++index)
			{
				ONVIF_search(ONVIF_DEV_NVT, false, 2, cbSearchHook, ipList[index].toLatin1().data(), this);
			}
			m_bFlush = false;
		}
		msleep(1);
	}

	ONVIF_CLIENT_deinit();
}

void DeviceSearch::setHook( QString sEvent, tagOnvifProInfo proInfo )
{
	m_sEvent = sEvent;
	m_proInfo = proInfo;
}

void DeviceSearch::analyzeDeviceInfo( unsigned char *ip,unsigned short port, char *name, char *location, char *firmware )
{
	QString ipStr;
	ipStr = ipStr.sprintf("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	QVariantMap deviceInfo;
	deviceInfo.insert("SearchDeviceName_ID"    ,QVariant(QString(name)));
	deviceInfo.insert("SearchDeviceId_ID"      ,QVariant(""));
	deviceInfo.insert("SearchDeviceModelId_ID" ,QVariant(QString(firmware)));
	deviceInfo.insert("SearchSeeId_ID"         ,QVariant(""));
	deviceInfo.insert("SearchChannelCount_ID"  ,QVariant("1"));
	deviceInfo.insert("SearchIP_ID"            ,QVariant(ipStr));
	deviceInfo.insert("SearchMask_ID"          ,QVariant(""));
	deviceInfo.insert("SearchMac_ID"           ,QVariant(""));
	deviceInfo.insert("SearchGateway_ID"       ,QVariant(""));     
	deviceInfo.insert("SearchHttpport_ID"      ,QVariant(port));
	deviceInfo.insert("SearchMediaPort_ID"     ,QVariant(port));
	deviceInfo.insert("SearchVendor_ID"        ,QVariant("ONVIF"));
	QString SearchSendToUI_ID=ipStr+"ONVIF";
	deviceInfo.insert("SearchSendToUI_ID"       ,QVariant(SearchSendToUI_ID));
	if (m_proInfo.proc && m_proInfo.pUser)
	{
		m_proInfo.proc(m_sEvent, deviceInfo, m_proInfo.pUser);
	}
}

void cbSearchHook( const char *bind_host, unsigned char *ip,unsigned short port, char *name, char *location, char *firmware, void *customCtx )
{
	((DeviceSearch*)customCtx)->analyzeDeviceInfo(ip, port, name, location, firmware);
}
