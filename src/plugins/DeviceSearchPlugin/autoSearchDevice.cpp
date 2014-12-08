#include "autoSearchDevice.h"
#include <QtXml\qdom.h>
#include <QtCore\QFile>
#include "guid.h"
#include <QDebug>
#include <QtCore\QCoreApplication>
int __cdecl autoDeviceSearchProc(QString sEventName,QVariantMap dvrItem,void * pUser);
int __cdecl autoDeviceSetNetInfoProc(QString sEventName, QVariantMap ipcItem, void *pUser);
autoSearchDevice::autoSearchDevice(void):m_bStop(false),
	m_pDeviceNetModify(NULL)
{
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
			QVariantMap tItem=m_tWaitForTestDeviceItem.dequeue();
			qDebug()<<__FUNCTION__<<__LINE__<<tItem.value("SearchIP_ID").toString();
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
