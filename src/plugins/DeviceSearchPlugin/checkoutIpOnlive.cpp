#include "checkoutIpOnlive.h"
#include "qarplib.h"
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkAddressEntry>
checkoutIpOnlive::checkoutIpOnlive(void)
{
}


checkoutIpOnlive::~checkoutIpOnlive(void)
{
}

bool checkoutIpOnlive::ipIsOnlive( int nTimeout,QString sIp )
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"make sure the thread had been stop";
		//设备在线
		return true;
	}else{
		m_sIp=sIp;
		m_bIsOnLive=false;
		QThread::start();
		int nConut=0;
		while(nConut<nTimeout&&m_bIsOnLive==false&&QThread::isRunning()){
			msleep(10);
			nConut=10+nConut;
		}
		return m_bIsOnLive;
	}
}

bool checkoutIpOnlive::checkIsRuning()
{
	return QThread::isRunning();
}

void checkoutIpOnlive::run()
{
	QStringList tonetlsit=m_sIp.split(".");
	QString tonet;
	for (int i=tonetlsit.size()-1;i>=0;i--)
	{
		if (i!=tonetlsit.size()-1)
		{
			tonet.append(".");
		}else{
			//do nothing
		}
		tonet.append(tonetlsit.at(i));
	}
	if (qsendarp(QHostAddress(tonet).toIPv4Address()))
	{
		//ip 不在线
		m_bIsOnLive=false;
	}else{
		//设备在线
		m_bIsOnLive=true;
	}
}
