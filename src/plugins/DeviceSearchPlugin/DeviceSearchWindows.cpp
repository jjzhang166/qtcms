#include "DeviceSearchWindows.h"
#include "guid.h"
#include <QtXml\qdom.h>
#include <QtCore\QFile>
#include <QtCore\QIODevice>
#include <QtCore\QCoreApplication>

int __cdecl DeviceSearchProc(QString param1,QVariantMap dvrItem,void * pUser);

DeviceSearchWindows::DeviceSearchWindows(QWidget *parent)
: QTableWidget(parent),
QWebPluginFWBase(this)
{
	// 读取配置文件，将第一个读到的
	//p_deviceSearch = NULL;
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

		if (sItemName.left(strlen("protocols.")) == QString("protocols."))
		{
			//if (NULL != p_deviceSearch)
			//{
			//	p_deviceSearch->Release();
			//	p_deviceSearch = NULL;
			//}
			IDeviceSearch* p_deviceSearch = NULL;
			CLSID DeviceSearchClsid = pcomString2GUID(item.toElement().attribute("clsid"));
			pcomCreateInstance(DeviceSearchClsid,NULL,IID_IDeviceSearch,(void **)&p_deviceSearch);
			if (NULL != p_deviceSearch)
			{
				IEventRegister* pEventInstance = p_deviceSearch->QueryEventRegister();
				pEventInstance->registerEvent("SearchDeviceSuccess",DeviceSearchProc,this);
				pEventInstance->queryEvent("SearchDeviceSuccess",paramlist);
				pEventInstance->Release();

				deviceList.insert(sItemName,p_deviceSearch);
			}
			//break;
		}
	}

	file->close();
	delete file;

//	initUI();
	connect(this,SIGNAL(addItemToUI(QVariantMap)),this,SLOT(sendToHtml(QVariantMap)));
}

DeviceSearchWindows::~DeviceSearchWindows(void)
{
	for (QMap<QString,IDeviceSearch*>::Iterator iter=deviceList.begin();iter != deviceList.end(); iter++)
	{
		(*iter)->Stop();
		(*iter)->Release();
	}
	//p_deviceSearch->Release();
}

//void DeviceSearchWindows::initUI()
//{
//	clear();
//	setColumnCount(paramlist.size());
//	int i=0;
//	for(QStringList::Iterator iter=paramlist.begin();iter!=paramlist.end();iter++)
//	{
//		setHorizontalHeaderItem(i,new QTableWidgetItem(*iter));
//		i++;
//	}
//	setObjectName(QString::fromUtf8("tableWidget"));
//	setEnabled(true);
//	setRowCount(0);
//}

int DeviceSearchWindows::Start()
{
	for (QMap<QString,IDeviceSearch*>::Iterator iter=deviceList.begin();iter != deviceList.end(); iter++)
	{
		(*iter)->Start();
	}

	return 0;

}

int DeviceSearchWindows::Stop()
{
	for (QMap<QString,IDeviceSearch*>::Iterator iter=deviceList.begin();iter != deviceList.end(); iter++)
	{
		(*iter)->Stop();
	}
	return 0;
}

int DeviceSearchWindows::Flush()
{
	//clear();
	//setRowCount(0);
	//initUI();
	itemList.clear();
	for (QMap<QString,IDeviceSearch*>::Iterator iter=deviceList.begin();iter != deviceList.end(); iter++)
	{
		(*iter)->Flush();
	}
	//p_deviceSearch->Flush();
	return 0;
}
int DeviceSearchWindows::setInterval(int nInterval)
{
	for (QMap<QString,IDeviceSearch*>::Iterator iter=deviceList.begin();iter != deviceList.end(); iter++)
	{
		(*iter)->setInterval(nInterval);
	}
	return 0;
}

//int DeviceSearchWindows::setSearchMode( QString searchModeName )
//{
//	Stop();
//	QString sAppPath = QCoreApplication::applicationDirPath();
//	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
//	file->open(QIODevice::ReadOnly);
//	QDomDocument ConfFile;
//	ConfFile.setContent(file);
//
//	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
//	QDomNodeList itemList = clsidNode.childNodes();
//	int n;
//	bool bFound = false;
//	for (n = 0; n < itemList.count(); n++)
//	{
//		QDomNode item = itemList.at(n);
//		QString sItemName = item.toElement().attribute("name");
//
//		if (sItemName.left(strlen("protocols.")) == QString("protocols."))
//		{
//			QString searchMode = item.toElement().attribute("type");
//			if (searchModeName == searchMode)
//			{
//				bFound = true;
//				if (NULL != p_deviceSearch)
//				{
//					p_deviceSearch->Release();
//					p_deviceSearch = NULL;
//					itemList.clear();
//					paramlist.clear();
//				}
//				CLSID DeviceSearchClsid = pcomString2GUID(item.toElement().attribute("clsid"));
//				pcomCreateInstance(DeviceSearchClsid,NULL,IID_IDeviceSearch,(void **)&p_deviceSearch);
//				if (NULL != p_deviceSearch)
//				{
//					IEventRegister* pEventInstance = p_deviceSearch->QueryEventRegister();
//					pEventInstance->registerEvent("SearchDeviceSuccess",DeviceSearchProc,this);
//					pEventInstance->queryEvent("SearchDeviceSuccess",paramlist);
//					pEventInstance->Release();
//					initUI();
//				}
//			}
//		}
//	}
//
//	file->close();
//	delete file;
//
//	if (!bFound)
//	{
//		return IDeviceSearch::E_SYSTEM_FAILED;
//	}
//
//	return IDeviceSearch::OK;
//}

void DeviceSearchWindows::addItemMap(QVariantMap item)
{
	if(itemList.find(item["SearchIP_ID"].toString())==itemList.end())
	{
		itemList.insert(item["SearchIP_ID"].toString(),item);
		emit addItemToUI(item);
	}
}

//void DeviceSearchWindows::addItemToUI(QVariantMap item)
//{
//	//static int lasePage = Page;
//	//if (Page != lasePage)
//	//{
//	//	clear();
//	//	setRowCount(0);
//	//	lasePage = Page;
//
//	//}
//
//	int thisrow = rowCount();
//	int paramcount = paramlist.size();
//	insertRow(thisrow);
//	for (int i=0;i<paramcount;i++)
//	{
//		setItem(thisrow,i,new QTableWidgetItem(item[paramlist[i]].toString()));
//	}
//
//}

void DeviceSearchWindows::sendToHtml(QVariantMap item)
{
	EventProcCall("SearchDeviceSuccess",item);
	//int thisrow = rowCount();
	//int paramcount = paramlist.size();
	//insertRow(thisrow);
	//for (int i=0;i<paramcount;i++)
	//{
	//	setItem(thisrow,i,new QTableWidgetItem(item[paramlist[i]].toString()));
	//}
}

int __cdecl DeviceSearchProc(QString param1,QVariantMap dvrItem,void * pUser)
{
	/*qDebug("\nname:%s\ndev_mode:%s\nesee_id:%s\nip:%s\nnetmask:%s\ngetaway:%s\nport:%s\nchannelcnt:%s\nmac:%s\ndevid:%s\n",
	item.name,item.dev_model,item.esee_id,item.ip,item.netmask,item.gateway,item.port,item.channelcnt,item.mac,item.devid); */
	((DeviceSearchWindows*)pUser)->addItemMap(dvrItem);
	return 0;
}