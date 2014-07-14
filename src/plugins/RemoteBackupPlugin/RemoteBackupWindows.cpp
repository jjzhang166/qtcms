#include "RemoteBackupWindows.h"
#include "IEventRegister.h"
#include "guid.h"
#include <QtXml\qdom.h>
#include <QDebug>
#include <QtCore\QFile>
#include <QtCore\QIODevice>
#include <QtCore\QCoreApplication>
int __cdecl BackupStatusProc(QString sEventName,QVariantMap dvrItem,void * pUser);
int __cdecl BackupProgressProc(QString sEventName,QVariantMap item, void *pUser);

RemoteBackupWindows::RemoteBackupWindows(QWidget *parent)
: QWidget(parent),
QWebPluginFWBase(this),
m_pRemoteBackup(NULL)
{

	// install all search component
	LoadDeviceClient("DVR");
	m_dir.append(".");
	connect(this,SIGNAL(sendStatus(QVariantMap)),this,SLOT(sendToHtml(QVariantMap)));
}

RemoteBackupWindows::~RemoteBackupWindows(void)
{
	stopBackup();
	if (m_pRemoteBackup)
	{
		m_pRemoteBackup->Release();
	}
	
}

bool RemoteBackupWindows::LoadDeviceClient(QString vendor)
{
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
		QString sVendor = item.toElement().attribute("vendor");

		if (sItemName.left(strlen("device.")) == QString("device.") && sVendor == vendor)
		{
			CLSID RemoteBackupClsid = pcomString2GUID(item.toElement().attribute("clsid"));
			pcomCreateInstance(RemoteBackupClsid,NULL,IID_IRemoteBackup,(void **)&m_pRemoteBackup);
			if (NULL != m_pRemoteBackup)
			{
				IEventRegister* pEventInstance = NULL;
				m_pRemoteBackup->QueryInterface(IID_IEventRegister,(void**)&pEventInstance);
				pEventInstance->registerEvent("BackupStatusChange",BackupStatusProc,this);
				pEventInstance->registerEvent("progress",BackupProgressProc,this);
				pEventInstance->Release();

			}
		}
	}

	file->close();
	delete file;

	return (bool)m_pRemoteBackup;
}

int RemoteBackupWindows::startBackup(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,
	int nChannel,
	int nTypes,
	const QString &sDeviceName,
	const QString & startTime,
	const QString & endTime,
	const QString & sbkpath)
{
	int nRet = 0;
	QDateTime stime = QDateTime::fromString(startTime,"yyyy-MM-dd hh:mm:ss");
	QDateTime etime = QDateTime::fromString(endTime,"yyyy-MM-dd hh:mm:ss");
	QString dir=QString::fromLocal8Bit(sbkpath.toLocal8Bit());
	if (m_pRemoteBackup)
	{
		nRet = m_pRemoteBackup->startBackup(sAddr,uiPort,sEseeId,nChannel,nTypes,sDeviceName,stime,etime,sbkpath);
	}
	return nRet;
}
int RemoteBackupWindows::stopBackup()
{
	int nRet = 0;
	if (m_pRemoteBackup)
	{
		nRet = m_pRemoteBackup->stopBackup();
	}
	return nRet;
}
float RemoteBackupWindows::getProgress()
{
	float progress = 0.0f;
	if (m_pRemoteBackup)
	{
		progress = m_pRemoteBackup->getProgress();
	}
	return progress;
}

void RemoteBackupWindows::sendToHtml(QVariantMap item)
{
	QString sEventName=item.value("sEventName").toString();
	item.remove(sEventName);
	EventProcCall(sEventName,item);

}

void RemoteBackupWindows::procCallBack(QVariantMap item)
{
	emit sendStatus(item);
}

void RemoteBackupWindows::ChooseDir()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		m_dir,
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	QVariantMap item;
	m_dir=dir;
	dir.replace(QString("\\"),QString("/"));
	item.insert("path",dir);
	EventProcCall("RecordDirPath",item);
}

int __cdecl BackupStatusProc(QString sEventName,QVariantMap dvrItem,void * pUser)
{
	if (sEventName == "BackupStatusChange")
	{
		dvrItem.insert("sEventName","BackupStatusChange");
		((RemoteBackupWindows*)pUser)->procCallBack(dvrItem);
	}
	return 0;
}

int __cdecl BackupProgressProc( QString sEventName,QVariantMap item, void *pUser )
{
	if (sEventName == "progress")
	{
		item.insert("sEventName","progress");
		((RemoteBackupWindows*)pUser)->procCallBack(item);
	}
	return 0;
}
