#include "DeviceSearchWindows.h"
#include "guid.h"
#include <QtXml\qdom.h>
#include <QtCore\QFile>
#include <QtCore\QIODevice>
#include <QtCore\QCoreApplication>

int __cdecl DeviceSearchProc(QString sEventName,QVariantMap dvrItem,void * pUser);
int __cdecl DeviceSetNetInfoProc(QString sEventName, QVariantMap ipcItem, void *pUser);

DeviceSearchWindows::DeviceSearchWindows(QWidget *parent)
: QTableWidget(parent),
m_pDeviceNetModify(NULL),
QWebPluginFWBase(this)
{
	// clear search component
	m_deviceList.clear();

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
				pEventInstance->registerEvent("SearchDeviceSuccess",DeviceSearchProc,this);

				if (QString("winipcsearch") == sItemType)
				{
					p_deviceSearch->QueryInterface(IID_IDeviceNetModify, (void**)&m_pDeviceNetModify);
					pEventInstance->registerEvent("SettingStatus", DeviceSetNetInfoProc, this);

				}
				pEventInstance->Release();

				m_deviceList.insert(m_deviceList.size(),p_deviceSearch);
			}
		}
	}

	file->close();
	delete file;

	connect(this,SIGNAL(addItemToUI(QVariantMap)),this,SLOT(sendToHtml(QVariantMap)));
}

DeviceSearchWindows::~DeviceSearchWindows(void)
{
	for (QList<IDeviceSearch*>::iterator iter=m_deviceList.begin();iter != m_deviceList.end(); iter++)
	{
		(*iter)->Stop();
		(*iter)->Release();
	}
	if (m_pDeviceNetModify!=NULL)
	{
		m_pDeviceNetModify->Release();
		m_pDeviceNetModify=NULL;
	}
}

int DeviceSearchWindows::Start()
{
	m_DeviceItemMutex.lock();
	m_DeviceItem.clear();
	m_DeviceItemMutex.unlock();
	for (QList<IDeviceSearch*>::iterator iter=m_deviceList.begin();iter != m_deviceList.end(); iter++)
	{
		(*iter)->Start();
	}

	return 0;

}

int DeviceSearchWindows::Stop()
{
	for (QList<IDeviceSearch*>::iterator iter=m_deviceList.begin();iter != m_deviceList.end(); iter++)
	{
		(*iter)->Stop();
	}
	return 0;
}

int DeviceSearchWindows::Flush()
{
	m_DeviceItemMutex.lock();
	m_DeviceItem.clear();
	m_DeviceItemMutex.unlock();
	for (QList<IDeviceSearch*>::iterator iter=m_deviceList.begin();iter != m_deviceList.end(); iter++)
	{
		(*iter)->Flush();
	}
	return 0;
}
int DeviceSearchWindows::setInterval(int nInterval)
{
	for (QList<IDeviceSearch*>::iterator iter=m_deviceList.begin();iter != m_deviceList.end(); iter++)
	{
		(*iter)->setInterval(nInterval);
	}
	return 0;
}

int DeviceSearchWindows::SetNetworkInfo(const QString &sDeviceID,
	const QString &sAddress,
	const QString &sMask,
	const QString &sGateway,
	const QString &sMac,
	const QString &sPort,
	const QString &sUsername,
	const QString &sPassword)
{
	qDebug()<<"SetNetworkInfo"<<sDeviceID<<sAddress<<sMask<<sGateway<<sMac<<sPort<<sUsername<<sPassword;
	if (sDeviceID.isEmpty() && sAddress.isEmpty() && sMask.isEmpty() && sGateway.isEmpty() && sMac.isEmpty() && sPort.isEmpty())
	{
		return 1;
	}

	if (NULL == m_pDeviceNetModify)
	{
		return 1;
	}
	int nRet = m_pDeviceNetModify->SetNetworkInfo( sDeviceID, sAddress, sMask, sGateway, sMac, sPort, sUsername, sPassword);
	if (nRet==IDeviceNetModify::OK)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void DeviceSearchWindows::addItemMap(QVariantMap item)
{
	if (!m_DeviceItem.contains(item.value("SearchIP_ID").toString()))
	{
		m_DeviceItemMutex.lock();
		m_DeviceItem.insert(item.value("SearchIP_ID").toString(),item);
		m_DeviceItemMutex.unlock();
		emit addItemToUI(item);
	}
}

void DeviceSearchWindows::sendToHtml(QVariantMap item)
{
	EventProcCall("SearchDeviceSuccess",item);
}

void DeviceSearchWindows::sendInfoToUI(QVariantMap item)
{

	/*EventProcCall("SettingStatus",item);*/
}

int DeviceSearchWindows::AutoSetNetworkInfo()
{
	QString lSAddress;
	QString lSNetmask;
	if (0==__GetNetworkInfo(lSAddress,lSNetmask))
	{
		__GetInitAddress(lSAddress,lSNetmask);
		//读入数据
		QVariant lDevNetworkInfoFile=__QueryValue("AutoSetNetworkInfoID");
		QDomDocument lConfFile;
		lConfFile.setContent(lDevNetworkInfoFile.toString());
		QDomNode lDevNetworkInfoNode=lConfFile.elementsByTagName("devnetworkInfo").at(0);
		QDomNodeList itemList=lDevNetworkInfoNode.childNodes();
		if (0==itemList.count())
		{
			return 0;
		}
		else{
			for (int n=0;n<itemList.count();n++){
				QDomNode itemDev;
				itemDev=itemList.at(n);
				QString sDeviceID=itemDev.toElement().attribute("sDeviceID");
				QString sAddress=itemDev.toElement().attribute("sAddress");
				QString sGateway=itemDev.toElement().attribute("sGateway");
				QString sMask=itemDev.toElement().attribute("sMask");
				QString sMac=itemDev.toElement().attribute("sMac");
				QString sPort=itemDev.toElement().attribute("sPort");
				QString sUsername=itemDev.toElement().attribute("sUsername");
				QString sPassword=itemDev.toElement().attribute("sPassword");
				
				QString lSNewAddress;
				if (1==__ApplyAddress(lSNewAddress,lSAddress))
				{
					return 1;
				}
				qDebug()<<"lSNewAddress"<<lSNewAddress;
				qDebug()<<"lSNetmask"<<lSNetmask;
				// Set sAddress
				sAddress.clear();
				sAddress=lSNewAddress;
				//设置子网掩码
				sMask.clear();
				sMask=lSNetmask;
				//设置网关
				sGateway.clear();
				sGateway=QString().append(m_HistoryGateWay.IpPart1).append(".").append(m_HistoryGateWay.IpPart2).append(".").append(m_HistoryGateWay.IpPart3).append(".").append(m_HistoryGateWay.IpPart4);
				qDebug()<<"sGateway"<<sGateway;
				//Call SetNetworkInfo

				//SetNetworkInfo(sDeviceID,sAddress,sMask,sGateway,sMac,sPort,sUsername,sPassword);
			}
		}
	}else{
		return 1;
	}
}

int DeviceSearchWindows::__GetNetworkInfo( QString &address,QString &netmask )
{
	//限于单网卡的环境下进行设置
	//获取可用ip和掩码
	QList<QNetworkInterface> lHostInterface;
	lHostInterface=QNetworkInterface::allInterfaces();
	QList<QNetworkInterface>::const_iterator it;
	bool lBMatch=false;
	for (it=lHostInterface.constBegin();it!=lHostInterface.constEnd();it++)
	{
		//step1：获取可用的网卡
		
		if (it->hardwareAddress()!=NULL&&it->hardwareAddress().count()==17&&it->flags().testFlag(QNetworkInterface::IsLoopBack)!=true)
		{
			QList<QNetworkAddressEntry> lHostEntry=it->addressEntries();
			QList<QNetworkAddressEntry>::const_iterator itEntry;
			for(itEntry=lHostEntry.constBegin();itEntry!=lHostEntry.constEnd();itEntry++){
				if (itEntry->ip().protocol()==QAbstractSocket::IPv4Protocol)
				{
					address=itEntry->ip().toString();
					netmask=itEntry->netmask().toString();

					lBMatch=true;
					break;
				}
			}
		}
		if (lBMatch==true)
		{
			break;
		}
	}
	if (lBMatch==true)
	{
		return 0;
	}
	else{
		return 1;
	}
}

QVariant DeviceSearchWindows::__QueryValue( QString sElementId )
{
	QWidget *pa=this->parentWidget();
	QWebElement elementTemp=((QWebView*)pa)->page()->mainFrame()->findFirstElement(QString("#")+sElementId);
	return elementTemp.evaluateJavaScript("document.getElementById('" + sElementId + "').value");
}

int DeviceSearchWindows::__GetInitAddress(QString lSAddress,QString lSNetmask)
{
	DevNetworkInfo lSAddressInfo;
	DevNetworkInfo lSNetmaskInfo;
	lSAddressInfo.IpPart1=lSAddress.section(".",0,0);
	lSAddressInfo.IpPart2=lSAddress.section(".",1,1);
	lSAddressInfo.IpPart3=lSAddress.section(".",2,2);
	lSAddressInfo.IpPart4=lSAddress.section(".",3,3);

	lSNetmaskInfo.IpPart1=lSNetmask.section(".",0,0);
	lSNetmaskInfo.IpPart2=lSNetmask.section(".",1,1);
	lSNetmaskInfo.IpPart3=lSNetmask.section(".",2,2);
	lSNetmaskInfo.IpPart4=lSNetmask.section(".",3,3);
	
	m_HistoryNetMask.IpPart1=lSNetmask.section(".",0,0);
	m_HistoryNetMask.IpPart2=lSNetmask.section(".",1,1);
	m_HistoryNetMask.IpPart3=lSNetmask.section(".",2,2);
	m_HistoryNetMask.IpPart4=lSNetmask.section(".",3,3);
	if (lSNetmaskInfo.IpPart1.toInt()!=0)
	{
		m_HistoryAddress.IpPart1=lSAddressInfo.IpPart1;
	}else{
		m_HistoryAddress.IpPart1="1";
	}
	if (lSNetmaskInfo.IpPart2.toInt()!=0)
	{
		m_HistoryAddress.IpPart2=lSAddressInfo.IpPart2;
	}else{
		m_HistoryAddress.IpPart2="1";
	}
	if (lSNetmaskInfo.IpPart3.toInt()!=0)
	{
		m_HistoryAddress.IpPart3=lSAddressInfo.IpPart3;
	}else{
		m_HistoryAddress.IpPart3="1";
	}
	if (lSNetmaskInfo.IpPart4.toInt()!=0)
	{
		m_HistoryAddress.IpPart4=lSAddressInfo.IpPart4;
	}else{
		m_HistoryAddress.IpPart4="1";
	}
	m_HistoryGateWay.IpPart1=m_HistoryAddress.IpPart1;
	m_HistoryGateWay.IpPart2=m_HistoryAddress.IpPart2;
	m_HistoryGateWay.IpPart3=m_HistoryAddress.IpPart3;
	m_HistoryGateWay.IpPart4=m_HistoryAddress.IpPart4;
	return 0;
}

int DeviceSearchWindows::__flushAddress()
{
	if (m_HistoryAddress.IpPart4.toInt()<254)
	{
		m_HistoryAddress.IpPart4=QString("%1").arg(m_HistoryAddress.IpPart4.toInt()+1);
		goto FoundAddress;
	}else if(m_HistoryAddress.IpPart3.toInt()<255){
		m_HistoryAddress.IpPart4=QString("1");
		m_HistoryAddress.IpPart3=QString("%1").arg(m_HistoryAddress.IpPart3.toInt()+1);
		goto FoundAddress;
	}else if (m_HistoryAddress.IpPart2.toInt()<255)
	{
		m_HistoryAddress.IpPart3=QString("1");
		goto FoundAddress;
	}
	else{
		//没有可分配的ip
		return 1;
	}

FoundAddress:
	return 0;
}

int DeviceSearchWindows::__ApplyAddress(QString &lSNewAddress ,QString lSAddress )
{
	if (__flushAddress()==1)
	{
		//没有可用的ip
		return 1;
	}
	//判断该ip是否为主机ip
	lSNewAddress.append(m_HistoryAddress.IpPart1).append(".").append(m_HistoryAddress.IpPart2).append(".").append(m_HistoryAddress.IpPart3).append(".").append(m_HistoryAddress.IpPart4);
	//lSNewAddress.append(m_HistoryAddress.IpPart1).append(".").append(m_HistoryAddress.IpPart2).append(".").append("29").append(".").append(m_HistoryAddress.IpPart4);
	if (lSNewAddress==lSAddress)
	{
		if (__flushAddress()==1)
		{
			return 1;
		}
		lSNewAddress.clear();
		lSNewAddress.append(m_HistoryAddress.IpPart1).append(".").append(m_HistoryAddress.IpPart2).append(".").append(m_HistoryAddress.IpPart3).append(".").append(m_HistoryAddress.IpPart4);
		return 0;
	}
	return 0;
}

int __cdecl DeviceSearchProc(QString sEventName,QVariantMap dvrItem,void * pUser)
{
	/*qDebug("\nname:%s\ndev_mode:%s\nesee_id:%s\nip:%s\nnetmask:%s\ngetaway:%s\nport:%s\nchannelcnt:%s\nmac:%s\ndevid:%s\n",
	item.name,item.dev_model,item.esee_id,item.ip,item.netmask,item.gateway,item.port,item.channelcnt,item.mac,item.devid); */
	if (sEventName == "SearchDeviceSuccess")
	{
		((DeviceSearchWindows*)pUser)->addItemMap(dvrItem);
	}
	return 0;
}

int __cdecl DeviceSetNetInfoProc(QString sEventName, QVariantMap ipcItem, void *pUser)
{
	if ("SettingStatus" == sEventName)
	{
		((DeviceSearchWindows*)pUser)->sendInfoToUI(ipcItem);
	}
	return 0;
}
