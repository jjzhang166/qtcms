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

				if (QString("ipcsearch") == sItemType)
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
}

int DeviceSearchWindows::Start()
{
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
	if (sDeviceID.isEmpty() && sAddress.isEmpty() && sMask.isEmpty() && sGateway.isEmpty() && sMac.isEmpty() && sPort.isEmpty())
	{
		return 1;
	}

	if (NULL == m_pDeviceNetModify)
	{
		return 1;
	}
	int nRet = m_pDeviceNetModify->SetNetworkInfo( sDeviceID, sAddress, sMask, sGateway, sMac, sPort, sUsername, sPassword);
	if (nRet != 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void DeviceSearchWindows::addItemMap(QVariantMap item)
{
	emit addItemToUI(item);
}

void DeviceSearchWindows::sendToHtml(QVariantMap item)
{
	

	EventProcCall("SearchDeviceSuccess",item);
}

void DeviceSearchWindows::sendInfoToUI(QVariantMap item)
{
	EventProcCall("SettingStatus",item);
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
