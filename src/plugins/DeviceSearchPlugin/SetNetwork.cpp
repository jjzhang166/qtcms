#include "SetNetwork.h"
#include <QDebug>
#include <QtCore\QFile>
#include <QtCore\QCoreApplication>
#include "qarplib.h"
//#pragma comment(lib,"Qarp.lib")

void dectobin(unsigned int n);
QString Getgateway(QString ip,QString mask);
SetNetwork::SetNetwork(void):m_type(0)
{
}


SetNetwork::~SetNetwork(void)
{
	if (QThread::isRunning())
	{
		m_type=3;
		msleep(10);
	}
}

int SetNetwork::SetNetworkInfo( const QString &sDeviceID, const QString &sAddress, const QString &sMask, const QString &sGateway, const QString &sMac, const QString &sPort, const QString &sUsername, const QString &sPassword )
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"SetNetwork is running";
		return 1;
	}else{
		m_netinfo.sDeviceID=sDeviceID;
		m_netinfo.sAddress=sAddress;
		m_netinfo.sMask=sMask;
		m_netinfo.sGateway=sGateway;
		m_netinfo.sMac=sMac;
		m_netinfo.sPort=sPort;
		m_netinfo.sUsername=sUsername;
		m_netinfo.sPassword=sPassword;
		m_type=1;
		QThread::start();
		return 0;
	}
}

int SetNetwork::AutoSetNetworkInfo( QDomNodeList itemlist )
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"SetNetwork is running";
		return 1;
	}else{
		m_itemlist=itemlist;
		m_type=2;
		QThread::start();
		return 0;
	}
}

void SetNetwork::AutoSetNetworkInfo()
{

}

void SetNetwork::run()
{
	IDeviceNetModify *pSetNetInfo=NULL;
	pcomCreateInstance(CLSID_WinIpcSearch,NULL,IID_IDeviceNetModify,(void**)&pSetNetInfo);
	if (NULL==pSetNetInfo)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"build DeviceNetModify module fail";
		return ;
	}else{
		//keep going
	}
	bool mbstop=true;
	while (mbstop)
	{
		switch(m_type){
		case 0:{
			//do nothing
			m_type=3;
			   }
			   break;
		case 1:{
			//set one
			pSetNetInfo->SetNetworkInfo(m_netinfo.sDeviceID,m_netinfo.sAddress,m_netinfo.sMask,m_netinfo.sGateway,m_netinfo.sMac,m_netinfo.sPort,m_netinfo.sUsername,m_netinfo.sPassword);
			m_type=3;
			   }
			   break;
		case 2:{
			//step 0:获取本机的mac，gateway，mask
			//step 1:获取需分配的设备；
			//step 2:获取可分配ip；
			//step 3:设置设备的信息			
			bool mcast2stop=false;
			int nstep=0;
			while(mcast2stop==false){
				
				switch(nstep){
				case 0:{
					//step 0:获取本机的mac，gateway，mask
					GetHostNetInfo();
					m_devindex=0;
					nstep=1;
					   }
					   break;
				case 1:{
					//step 1:获取需分配的设备；
					if (true==GetDevInfo())
					{
						nstep=2;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"can not get any dev";
						nstep=4;
					}
					   }
					   break;
				case 2:{
					//step 2:获取可分配ip；
					GetUsableIp();
					if (m_usableIp.isEmpty())
					{
						qDebug()<<__FUNCTION__<<__LINE__<<"can not receive usable ip";
						nstep=4;
					}else{
						nstep=3;
						m_netinfo.sAddress=m_usableIp;
						qDebug()<<__FUNCTION__<<__LINE__<<m_usableIp;
						m_netinfo.sGateway=m_hostgateway;
						m_netinfo.sMask=m_hostmac;
					}
					   }
					   break;
				case 3:{
					//step 3:设置设备的信息 屏蔽此功能
					//pSetNetInfo->SetNetworkInfo(m_netinfo.sDeviceID,m_netinfo.sAddress,m_netinfo.sMask,m_netinfo.sGateway,m_netinfo.sMac,m_netinfo.sPort,m_netinfo.sUsername,m_netinfo.sPassword);
					nstep=1;
					   }
					   break;
				case 4:{
					mcast2stop=true;
					   }
					   break;
				}
			}
			m_type=3;
			   }
			   break;
		case 3:{
			mbstop=false;
			pSetNetInfo->Release();
			   }
			   break;
		}
	}
}

int SetNetwork::GetHostNetInfo()
{
	QList<QNetworkInterface> mHostInterface;
	mHostInterface=QNetworkInterface::allInterfaces();
	QList<QNetworkInterface>::const_iterator it;
	bool mbmatch=false;
	for (it=mHostInterface.constBegin();it!=mHostInterface.constEnd();it++)
	{
		if (it->hardwareAddress()!=NULL&&it->hardwareAddress().count()==17&&it->flags().testFlag(QNetworkInterface::IsLoopBack)!=true)
		{
			QList<QNetworkAddressEntry> mHostEntry=it->addressEntries();
			QList<QNetworkAddressEntry>::const_iterator itentry;
			for (itentry=mHostEntry.constBegin();itentry!=mHostEntry.constEnd();itentry++)
			{
				if (itentry->ip().protocol()==QAbstractSocket::IPv4Protocol)
				{
					m_hostip=itentry->ip().toString();
					m_hostsmask=itentry->netmask().toString();
					mbmatch=true;
					break;
				}
			}

		}
		if (mbmatch==true)
		{
			break;;
		}
	}
	if (mbmatch==true)
	{
		m_hostgateway=Getgateway(m_hostip,m_hostsmask);
		qDebug()<<__FUNCTION__<<__LINE__<<"GetHostNetInfo success";
		return 0;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"GetHostNetInfo fail";
		return 1;
	}
}
QString res;
void dectobin(unsigned int n){
	if (n>1)
	{
		dectobin(n/2);
	}
	res.append(QString::number(n%2));
};
void bintodec(unsigned int n){

}
QString Getgateway(QString ip,QString mask){
	QHostAddress mip(ip);
	QHostAddress mmask(mask);
	QString gateway= QHostAddress(mip.toIPv4Address()&mmask.toIPv4Address()).toString();
	gateway.replace(".0",".1");
	return QHostAddress(gateway).toString();
}
bool SetNetwork::GetUsableIp()
{
	//
	int step =0;
	bool stop=false;
	unsigned int dstip=QHostAddress(m_hostgateway).toIPv4Address()+1;
	int ncout=0;
	m_usableIp.clear();
	while(stop==false&&ncout<256){
		ncout++;
		switch (step){
		case 0:{
			//开始 检测的ip	
			QList<unsigned int>::const_iterator it;
			bool buseless=false;
			for (it=m_uselessIP.constBegin();it!=m_uselessIP.constEnd();it++)
			{
				if (*it==dstip)
				{
					//uselessip ;
					buseless=true;
					break;
				}
			}
			if (buseless==true)
			{
				dstip=dstip+1;
				step=0;
			}else{
				step =1;
			}
			   }
			   break;
		case 1:
			{
				QHostAddress mtestip(dstip);
				QString tonet=mtestip.toString();
				QStringList tonetlsit=tonet.split(".");
				tonet.clear();
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
					//this ip is not been used;
					step =2;
					m_uselessIP.append(dstip);
				}else{
					//this ip had been used;
					step =0;
					m_uselessIP.append(dstip);
					dstip=dstip+1;
				}
			}
			break;
		case 2:
			{
			//found
				m_usableIp=QHostAddress(dstip).toString();
				stop=true;
			}
			break;
		}
	}
	if (ncout<256)
	{
		return true;
	}else{
		return false;
	}
}

bool SetNetwork::GetDevInfo()
{
	if (0==m_itemlist.size())
	{
		return false;
	}else{
		//keep going
	}
	if (m_devindex<m_itemlist.count())
	{
		QDomNode itemDev;
		itemDev=m_itemlist.at(m_devindex);
		m_netinfo.sDeviceID=itemDev.toElement().attribute("sDeviceID");
		m_netinfo.sAddress=itemDev.toElement().attribute("sAddress");
		m_netinfo.sGateway=itemDev.toElement().attribute("sGateway");
		m_netinfo.sMask=itemDev.toElement().attribute("sMask");
		m_netinfo.sMac=itemDev.toElement().attribute("sMac");
		m_netinfo.sPort=itemDev.toElement().attribute("sPort");
		m_netinfo.sUsername=itemDev.toElement().attribute("sUsername");
		m_netinfo.sPassword=itemDev.toElement().attribute("sPassword");
		m_devindex++;
		return true;
	}else{
		return false;
	}

}


