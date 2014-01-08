#include "LocalPlayer.h"
#include <QtCore/QDateTime>
#include <QtCore/QDir>

#include <guid.h>


LocalPlayer::LocalPlayer() :
m_nRef(0)
{
	m_eventList<<"GetRecordDate"<<"GetRecordFile"<<"SearchStop";

	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDiskSetting,(void**)&m_pDiskSetting);

}

LocalPlayer::~LocalPlayer()
{
	if (NULL != m_pDiskSetting)
	{
		m_pDiskSetting->Release();
	}
}

int LocalPlayer::checkUsedDisk(QString &strDisk)
{
	if (NULL == m_pDiskSetting)
	{
		return 1;
	}

	int nRet = m_pDiskSetting->getUseDisks(strDisk);
	return nRet;
}

int LocalPlayer::searchDateByDeviceName(const QString& sdevname)
{
	if (sdevname.isEmpty())
	{
		return ILocalRecordSearch::E_PARAMETER_ERROR;
	}

	QString sUsedDisks;
	if (1 == checkUsedDisk(sUsedDisks))
	{
		return ILocalRecordSearch::E_SYSTEM_FAILED;
	}

	if (sUsedDisks.isEmpty())
	{
		return ILocalRecordSearch::E_SYSTEM_FAILED;
	}

	QStringList sltUsedDisk = sUsedDisks.split(":", QString::SkipEmptyParts);
	for (int i = 0; i < sltUsedDisk.size(); i++)
	{
		QString rootDir = QString(sltUsedDisk[i] + ":/JAREC/");
		QDir dir = QDir(rootDir);
		dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

		QStringList sltSubDir = dir.entryList();
		for (int j = 0; j < sltSubDir.size(); j++)
		{
			QString strSubDir = rootDir + sltSubDir[j] + "/";
			QDir subDir = QDir(strSubDir);
			QStringList sltSubSubDir = subDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
			if (sltSubSubDir.contains(sdevname))
			{
				QDateTime dateTime = QDateTime::fromString(sltSubDir[j], "yyyy-MM-dd");
				QVariantMap dateInfo;

				dateInfo.insert("devname", sdevname);
				dateInfo.insert("date", dateTime);

				eventProcCall(QString("GetRecordDate"), dateInfo);
			}
		}
	}

	QVariantMap stopInfo;
	stopInfo.insert("stopevent", QString("GetRecordDate"));
	eventProcCall(QString("SearchStop"), stopInfo);

	return ILocalRecordSearch::OK;
}

bool LocalPlayer::checkChannel(const QString& schannellist)
{
	if (schannellist.isEmpty())
	{
		return false;
	}

	QStringList sltChannels = schannellist.split(";");
	QRegExp regTimeFormat("[0-9]{1,2};");
	int num = 0;
	QString temp = sltChannels[num] + ";";
	while(temp.contains(regTimeFormat))
	{
		num++;
		temp = sltChannels[num] + ";";
	}

	if (num == sltChannels.size() - 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int LocalPlayer::searchVideoFile(const QString& sdevname, const QString& sdate, const QString& sbegintime, const QString& sendtime, const QString& schannellist)
{
	if (sdevname.isEmpty() || !checkChannel(schannellist))
	{
		return ILocalRecordSearch::E_PARAMETER_ERROR;
	}
	QDateTime date = QDateTime::fromString(sdate,"yyyy-MM-dd");
	QTime timeStart = QTime::fromString(sbegintime,"hh:mm:ss");
	QTime timeEnd = QTime::fromString(sendtime,"hh:mm:ss");

	if (!date.isValid() || !timeStart.isValid() || !timeEnd.isValid())
	{
		return ILocalRecordSearch::E_PARAMETER_ERROR;
	}

	QString sUsedDisks;
	if (1 == checkUsedDisk(sUsedDisks))
	{
		return ILocalRecordSearch::E_SYSTEM_FAILED;
	}

	if (sUsedDisks.isEmpty())
	{
		return ILocalRecordSearch::E_SYSTEM_FAILED;
	}

	QStringList sltUsedDisk = sUsedDisks.split(":", QString::SkipEmptyParts);
	QStringList sltChannels = schannellist.split(";", QString::SkipEmptyParts);
	for (int i = 0; i < sltUsedDisk.size(); i++)
	{
		QString root = sltUsedDisk[i] + ":/JAREC/" + sdate + "/" + sdevname;
		for (int j = 0; j < sltChannels.size(); j++)
		{
			QString channel = sltChannels[j].toInt() > 9 ? sltChannels[j] : "0" + sltChannels[j];
			QString channelPath = "/CHL" + channel;
			QString subPath = root + channelPath;
			QDir subDir = QString(subPath);
			QFileInfoList sltFileList = subDir.entryInfoList(QDir::Files);
			for (int k = 0; k < sltFileList.size(); k++)
			{
				QString fileName = sltFileList[k].fileName();
				QTime fileTime = QTime::fromString(fileName.left(6), "hhmmss");
				if (fileTime >= timeStart && fileTime <= timeEnd)
				{
					QString filePath = subPath + "/" + fileName;
					qint64 fileSize = sltFileList[k].size()/1024/1024;
					
					QVariantMap fileInfo;
					fileInfo.insert("filename", fileName);
					fileInfo.insert("filepath", filePath);
					fileInfo.insert("filesize", QString("%1").arg(fileSize));
					fileInfo.insert("channelnum", sltChannels[j]);
					date.setTime(timeStart);
					fileInfo.insert("startTime", date);
					date.setTime(timeEnd);
					fileInfo.insert("stopTime", date);

					eventProcCall(QString("GetRecordFile"), fileInfo);
				}
			}
		}
	}

	QVariantMap stopInfo;
	stopInfo.insert("stopevent", QString("GetRecordFile"));
	eventProcCall(QString("SearchStop"), stopInfo);

	return ILocalRecordSearch::OK;
}

QStringList LocalPlayer::eventList()
{
	return m_eventList;
}
int LocalPlayer::queryEvent(QString eventName,QStringList& eventParams)
{
	if (!m_eventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	if ("GetRecordDate" == eventName)
	{
		eventParams<<"devname"<<"date";
	}
	if ("GetRecordFile" == eventName)
	{
		eventParams<<"filename"<<"filepath"<<"filesize"<<"channelnum"<<"startTime"<<"stopTime";
	}
	if ("SearchStop" == eventName)
	{
		eventParams<<"stopevent";
	}

	return IEventRegister::OK;
}
int LocalPlayer::registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
{
	if (!m_eventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}

	ProcInfoItem proInfo;
	proInfo.proc = proc;
	proInfo.puser = pUser;

	m_eventMap.insert(eventName, proInfo);
	return IEventRegister::OK;
}


long __stdcall LocalPlayer::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_ILocalRecordSearch == iid)
	{
		*ppv = static_cast<ILocalRecordSearch *>(this);
	}
	else if (IID_IEventRegister == iid)
	{
		*ppv = static_cast<IEventRegister *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall LocalPlayer::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall LocalPlayer::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

void LocalPlayer::eventProcCall( QString sEvent,QVariantMap param )
{
	if (m_eventList.contains(sEvent))
	{
		ProcInfoItem eventDes = m_eventMap.value(sEvent);
		if (NULL != eventDes.proc)
		{
			eventDes.proc(sEvent,param,eventDes.puser);
		}
	}
}