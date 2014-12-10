#include "autoSearchDevice.h"
#include <QtXml\qdom.h>
#include <QtCore\QFile>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkAddressEntry>
#include "guid.h"
#include <QDebug>
#include "qarplib.h"
#include <QtCore\QCoreApplication>
int __cdecl autoDeviceSearchProc(QString sEventName,QVariantMap dvrItem,void * pUser);
int __cdecl autoDeviceSetNetInfoProc(QString sEventName, QVariantMap ipcItem, void *pUser);
autoSearchDevice::autoSearchDevice(void):m_bStop(false),
	m_pDeviceNetModify(NULL)
{
	m_sEventList<<"autoSearchDevice";
}


autoSearchDevice::~autoSearchDevice(void)
{
}

void autoSearchDevice::startSearch()
{
	QThread::start();
	m_bStop=false;
}

void autoSearchDevice::stopSearch()
{
	m_bStop=true;
	while(QThread::isRunning()){
		msleep(10);
	}
}

void autoSearchDevice::run()
{
	bool bRunStop=false;
	tagAutoSearchDeviceStep tRunStep=AutoSearchDeviceStep_Start;
	while(bRunStop==false){
		switch(tRunStep){
		case AutoSearchDeviceStep_Start:{
			startVendorSearch();
			tRunStep=AutoSearchDeviceStep_NetworkConfig;
										}
										break;
		case AutoSearchDeviceStep_NetworkConfig:{
			if (getNetworkConfig())
			{
				tRunStep=AutoSearchDeviceStep_Default;
			}else{
				tRunStep=AutoSearchDeviceStep_End;
			}
												}
												break;
		case AutoSearchDeviceStep_TestAndSet:{
			m_tQueueLock.lock();
			m_tCurrentDeviceItem=m_tWaitForTestDeviceItem.dequeue();
			checkAndSetConfig();
			m_tQueueLock.unlock();
			tRunStep=AutoSearchDeviceStep_Default;
											 }
											 break;
		case AutoSearchDeviceStep_Default:{
			if (!m_bStop)
			{
				if (m_tWaitForTestDeviceItem.isEmpty())
				{
					msleep(10);
				}else{
					tRunStep=AutoSearchDeviceStep_TestAndSet;
				}
			}else{
				tRunStep=AutoSearchDeviceStep_End;
			}
										  }
										  break;
		case AutoSearchDeviceStep_End:{
			for (int i=0;i<m_tDeviceList.size();i++)
			{
				m_tDeviceList[i]->Stop();
				m_tDeviceList[i]->Release();
			}
			if (m_pDeviceNetModify!=NULL)
			{
				m_pDeviceNetModify->Release();
				m_pDeviceNetModify=NULL;
			}
			m_tDeviceList.clear();
			m_tDeviceItem.clear();
			m_tWaitForTestDeviceItem.clear();
			m_tHadBeenUseIp.clear();
			bRunStop=true;
									  }
									  break;
		}
	}
}

void autoSearchDevice::startVendorSearch()
{
	// clear search component
	for (int i=0;i<m_tDeviceList.size();i++)
	{
		m_tDeviceList[i]->Stop();
		m_tDeviceList[i]->Release();
	}
	if (m_pDeviceNetModify!=NULL)
	{
		m_pDeviceNetModify->Release();
		m_pDeviceNetModify=NULL;
	}
	m_tDeviceList.clear();
	m_tDeviceItem.clear();
	m_tWaitForTestDeviceItem.clear();
	// install all search component
	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);

	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	int n;
	for (n = 0; n < itemList.count(); n++)
	{
		QDomNode item = itemList.at(n);
		QString sItemName = item.toElement().attribute("name");
		QString sItemType = item.toElement().attribute("type");

		if (sItemName.left(strlen("protocols.")) == QString("protocols."))
		{
			IDeviceSearch* p_deviceSearch = NULL;
			CLSID DeviceSearchClsid = pcomString2GUID(item.toElement().attribute("clsid"));
			pcomCreateInstance(DeviceSearchClsid,NULL,IID_IDeviceSearch,(void **)&p_deviceSearch);
			if (NULL != p_deviceSearch)
			{
				IEventRegister* pEventInstance = p_deviceSearch->QueryEventRegister();
				pEventInstance->registerEvent("SearchDeviceSuccess",autoDeviceSearchProc,this);

				if (QString("winipcsearch") == sItemType)
				{
					p_deviceSearch->QueryInterface(IID_IDeviceNetModify, (void**)&m_pDeviceNetModify);
					pEventInstance->registerEvent("SettingStatus", autoDeviceSetNetInfoProc, this);

				}
				pEventInstance->Release();

				m_tDeviceList.insert(m_tDeviceList.size(),p_deviceSearch);
			}
		}
	}

	file->close();
	delete file;

	for (QList<IDeviceSearch*>::iterator iter=m_tDeviceList.begin();iter != m_tDeviceList.end(); iter++)
	{
		(*iter)->Start();
	}

}

bool autoSearchDevice::getNetworkConfig()
{
	QList<QNetworkInterface> tHostInterface;
	tHostInterface=QNetworkInterface::allInterfaces();
	QList<QNetworkInterface>::const_iterator it;
	bool bMatch=false;
	for (it=tHostInterface.constBegin();it!=tHostInterface.constEnd();it++)
	{
		if (it->hardwareAddress()!=NULL&&it->hardwareAddress().count()==17&&it->flags().testFlag(QNetworkInterface::IsLoopBack)!=true)
		{
			QList<QNetworkAddressEntry>tHostEntry=it->addressEntries();
			QList<QNetworkAddressEntry>::const_iterator tItem;
			for (tItem=tHostEntry.constBegin();tItem!=tHostEntry.constEnd();tItem++)
			{
				if (tItem->ip().protocol()==QAbstractSocket::IPv4Protocol)
				{
					m_tInterfaceInfo.sIp=tItem->ip().toString();
					m_tInterfaceInfo.sMask=tItem->netmask().toString();
					bMatch=true;
					qDebug()<<__FUNCTION__<<__LINE__<<m_tInterfaceInfo.sIp<<m_tInterfaceInfo.sMask;
					break;
				}
			}
		}
	}
	QHostAddress tIp(m_tInterfaceInfo.sIp);
	QHostAddress tMask(m_tInterfaceInfo.sMask);
	QString sGateway=QHostAddress(tIp.toIPv4Address()&tMask.toIPv4Address()).toString();
	sGateway.replace(".0",".1");
	m_tInterfaceInfo.sGateway=sGateway;
	m_tInterfaceInfo.uiLastTestIp=QHostAddress(m_tInterfaceInfo.sGateway).toIPv4Address()+1;
	qDebug()<<__FUNCTION__<<__LINE__<<m_tInterfaceInfo.sGateway;
	return true;
}

void autoSearchDevice::cbSearchDevice( QVariantMap item )
{
	m_tDeviceItemMutex.lock();
	QVariantMap tItem=item;
	if (!m_tDeviceItem.contains(tItem.value("SearchSendToUI_ID").toString()))
	{
		m_tDeviceItem.insert(tItem.value("SearchSendToUI_ID").toString(),tItem);
		tItem.remove("SearchSendToUI_ID");
		m_tQueueLock.lock();
		m_tWaitForTestDeviceItem.enqueue(tItem);
		m_tQueueLock.unlock();
	}
	m_tDeviceItemMutex.unlock();
	return ;
}

void autoSearchDevice::checkAndSetConfig()
{
	int nStep=0;
	bool bStop=false;
	while(bStop==false){
		switch(nStep){
		case 0:{
			//测试是否有Ip冲突
			if (isIpConflict())
			{
				qDebug()<<__FUNCTION__<<__LINE__<<m_tCurrentDeviceItem.value("SearchIP_ID").toString()<<"IpConflict";
				nStep=1;
			}else{
				nStep=4;
			}
			   }
			   break;
		case 1:{
			//测试是否是  juan Ipc
			if (isJuanIpc())
			{
				nStep=2;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<m_tCurrentDeviceItem.value("SearchIP_ID").toString()<<"IpConflict and it is not juan ipc";
				nStep=5;
			}
			   }
			   break;
		case 2:{
			//获取 可以使用的Ip
			if (getUseableIp())
			{
				nStep=3;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<m_tCurrentDeviceItem.value("SearchIP_ID").toString()<<"getUseableIp fail";
				nStep=5;
			}
			   }
			   break;
		case 3:{
			//设置ip
			if (setIpConfig())
			{
				nStep=4;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<m_tCurrentDeviceItem.value("SearchIP_ID").toString()<<"setIpConfig fail";
				nStep=5;
			}
			   }
			   break;
		case 4:{
			//可以抛出信息
			nStep=5;
			eventProcCall("autoSearchDevice",m_tCurrentDeviceItem);
			qDebug()<<__FUNCTION__<<__LINE__<<m_tCurrentDeviceItem.value("SearchIP_ID").toString();
			   }
			   break;
		case 5:{
			//退出
			bStop=true;
			   }
			   break;
		}
	}	
}

bool autoSearchDevice::isIpConflict()
{
	//return flase:表示没有ip冲突，true:表示有ip冲突
	QString sIp=m_tCurrentDeviceItem.value("SearchIP_ID").toString();
	QHostAddress mtestip(sIp);
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
	char *pMac=new char[6];
	char *pLastMac=new char[6];
	bool bFlags=false;
	QString sMac;
	for (int n=0;n<5;n++)
	{
		memset(pMac,0,6);
		if (qsendarpEx(QHostAddress(tonet).toIPv4Address(),*pMac))
		{
			//ping 不通，设备 有问题？
			//按照此设备 以断线，没有ip 冲突做处理
			bFlags=false;
			break;
		}else{
			// 可以ping 通
			if (n==0)
			{
				memcpy(pLastMac,pMac,6);
			}else{
				//keep going
			}
			if (memcmp(pLastMac,pMac,6)!=0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<pLastMac<<pMac;
				bFlags=true;
				break;
			}else{
				//keep going
			}
		}
	}
	delete pMac;
	pMac=NULL;
	delete pLastMac;
	pLastMac=NULL;
	m_tHadBeenUseIp.append(sIp);
	return bFlags;
}

bool autoSearchDevice::isJuanIpc()
{
	if (m_tCurrentDeviceItem.value("SearchVendor_ID").toString()=="IPC")
	{
		return true;
	}else{
		return false;
	}
}

bool autoSearchDevice::getUseableIp()
{
	bool bStop=false;
	int nStep=0;
	m_tInterfaceInfo.uiLastTestIp=m_tInterfaceInfo.uiLastTestIp+1;
	unsigned int uiGateway=QHostAddress(m_tInterfaceInfo.sGateway).toIPv4Address();
	bool bFlags=false;
	if (m_tInterfaceInfo.uiLastTestIp-uiGateway>252)
	{
		return false;
	}
	while(bStop==false){
		switch(nStep){
		case 0:{
			//检测是否已在使用过的队列中
			QString sCurrentIp=QHostAddress(m_tInterfaceInfo.uiLastTestIp).toString();
			if (m_tHadBeenUseIp.contains(sCurrentIp))
			{
				nStep=2;
			}else{
				nStep=1;
			}
			   }
			   break;
		case 1:{
			//检测是否在局域网中有
			QHostAddress mtestip(m_tInterfaceInfo.uiLastTestIp);
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
				nStep=3;
				m_tHadBeenUseIp.append(QHostAddress(m_tInterfaceInfo.uiLastTestIp).toString());
			}else{
				nStep=2;
				m_tHadBeenUseIp.append(QHostAddress(m_tInterfaceInfo.uiLastTestIp).toString());
			}
			   }
			   break;
		case 2:{
			//+1
			m_tInterfaceInfo.uiLastTestIp=m_tInterfaceInfo.uiLastTestIp+1;
			if (m_tInterfaceInfo.uiLastTestIp-uiGateway>242)
			{
				nStep=4;
			}else{
				nStep=0;
			}
			   }
			   break;
		case 3:{
			//获取到
			nStep=4;
			bFlags=true;
			   }
			   break;
		case 4:{
			//结束
			bStop=true;
			   }
		}
	}
	return bFlags;
}

bool autoSearchDevice::setIpConfig()
{
	QString sIp=QHostAddress(m_tInterfaceInfo.uiLastTestIp).toString();
	m_tCurrentDeviceItem["SearchIP_ID"]=sIp;
	m_pDeviceNetModify->SetNetworkInfo(m_tCurrentDeviceItem.value("SearchDeviceId_ID").toString(),sIp,m_tInterfaceInfo.sMask,m_tInterfaceInfo.sGateway,m_tCurrentDeviceItem.value("SearchMac_ID").toString(),m_tCurrentDeviceItem.value("SearchHttpport_ID").toString(),"admin","");
	return true;
}

int autoSearchDevice::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_sEventList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<eventName<<"is undefined";
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}else{
		tagautoSearchDeviceProInfo tProInfo;
		tProInfo.proc=proc;
		tProInfo.pUser=pUser;
		m_tEventMap.insert(eventName,tProInfo);
		return IEventRegister::OK;
	}
}

void autoSearchDevice::eventProcCall( QString sEvent,QVariantMap tInfo )
{
	if (m_sEventList.contains(sEvent))
	{
		tagautoSearchDeviceProInfo tProInfo=m_tEventMap.value(sEvent);
		if (NULL!=tProInfo.proc)
		{
			tProInfo.proc(sEvent,tInfo,tProInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is not register";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<sEvent<<"is  undefined";
	}
}

int __cdecl autoDeviceSearchProc( QString sEventName,QVariantMap dvrItem,void * pUser )
{
	if (sEventName == "SearchDeviceSuccess")
	{
		((autoSearchDevice*)pUser)->cbSearchDevice(dvrItem);
	}
	return 0;
}

int __cdecl autoDeviceSetNetInfoProc( QString sEventName, QVariantMap ipcItem, void *pUser )
{
	return 0;
}
