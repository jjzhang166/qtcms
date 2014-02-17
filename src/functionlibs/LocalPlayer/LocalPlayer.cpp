#include "LocalPlayer.h"
#include <QtCore/QDateTime>
#include <QtCore/QDir>

#include <guid.h>
#include "avilib.h"


LocalPlayer::LocalPlayer() :
m_nRef(0),
m_nGroupNum(4),
m_bIsGroupPlaying(false)
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

	QMap<QWidget*, PrePlay>::iterator iter;
	for (iter = m_GroupMap.begin(); iter != m_GroupMap.end(); iter++)
	{
		if (NULL != iter->pPlayMgr)
		{
			delete iter->pPlayMgr;
			iter->pPlayMgr = NULL;
		}
	}
	m_GroupMap.clear();
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
		if (num < sltChannels.size())
		{
			temp = sltChannels[num] + ";";
		}
		else
		{
			break;
		}
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
	int aviFileLength = 0;
	int totalFrames = 0;
	int frameRate = 0;
	avi_t *aviFile = NULL;

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
					if (0 == sltFileList[k].size())
					{
						continue;
					}

					aviFile = AVI_open_input_file(filePath.toLatin1().data(), 0);
					totalFrames = AVI_video_frames(aviFile);
					frameRate = AVI_frame_rate(aviFile);
					aviFileLength = totalFrames/frameRate;//the length of avi file playing time

					AVI_close(aviFile);

					
					QVariantMap fileInfo;
					fileInfo.insert("filename", fileName);
					fileInfo.insert("filepath", filePath);
					fileInfo.insert("filesize", QString("%1").arg(fileSize));
					fileInfo.insert("channelnum", sltChannels[j]);
					date.setTime(fileTime);
					fileInfo.insert("startTime", date);
					fileInfo.insert("stopTime", date.addSecs(aviFileLength));

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

int LocalPlayer::checkFileExist(QStringList const fileList, const QDateTime& startTime, const QDateTime& endTime)
{
	QString filePath;
	QString dateStr;
	QFileInfo fileInfo(filePath);
	QDateTime dateTime;
	QDate date;
	QTime time;
	QRegExp rx("([0-9]{4}-[0-9]{2}-[0-9]{2})");
	int aviFileLength = 0;
	int totalFrames = 0;
	int frameRate = 0;
	avi_t *aviFile = NULL;

	for(int pos = 0; pos < fileList.size(); pos++)
	{
		filePath = fileList[pos];
		if (-1 != rx.indexIn(filePath,0))
		{
			dateStr = rx.cap(1);
		}
		fileInfo.setFile(filePath);
		QString baseName = fileInfo.baseName();
		date = QDate::fromString(dateStr, "yyyy-MM-dd");
		time = QTime::fromString(baseName, "hhmmss");
		dateTime.setDate(date);
		dateTime.setTime(time);

		if (0 == fileInfo.size())
		{
			return -1;
		}

		aviFile = AVI_open_input_file(filePath.toLatin1().data(), 0);
		totalFrames = AVI_video_frames(aviFile);
		frameRate = AVI_frame_rate(aviFile);
		aviFileLength = totalFrames/frameRate;//the length of avi file playing time

		AVI_close(aviFile);

		if (!((dateTime >= endTime) || (dateTime.addSecs(aviFileLength) <= startTime)))
		{
			return pos;
		}
	}

	return -1;
}

bool LocalPlayer::checkChannelInFileList(QStringList const filelist)
{
	QString channel;
	QString tempCh;
	QRegExp rx("CHL([0-9]{2})");

	for (int i = 0; i < filelist.size(); i++)
	{
		if ( -1 != rx.indexIn(filelist[i], 0))
		{
			tempCh = rx.cap(1);
		}

		if (channel.isEmpty())
		{
			channel = tempCh;
		}
		else if (channel != tempCh)
		{
			return false;
		}
	}

	return true;
}

int LocalPlayer::AddFileIntoPlayGroup(QStringList const filelist,QWidget *wnd,const QDateTime &start,const QDateTime &end)
{
	if (filelist.isEmpty() || NULL == wnd || start >= end)
	{
		return 3;
	}
	//fileList has different channel
	if (!checkChannelInFileList(filelist))
	{
		return 3;
	}
	//group full
	if (m_GroupMap.size() >= m_nGroupNum)
	{
		return 1;
	}
	//Window is occupied
	if (m_GroupMap.contains(wnd))
	{
		return 2;
	}
	//there is no file between start time and end time
	int startPos = checkFileExist(filelist, start, end);
	if (-1 == startPos)
	{
		return 3;
	}

	PrePlay prePlay;
	prePlay.pPlayMgr = new PlayMgr();
	prePlay.fileList = filelist;
	prePlay.startTime = start;
	prePlay.endTime = end;
	prePlay.startPos = startPos;
	
	m_GroupMap.insert(wnd, prePlay);

	return 0;
}
int LocalPlayer::SetSynGroupNum(int num)
{
	if (num <= 0 || num > 64)
	{
		return 1;
	}
	m_nGroupNum = num;
	return 0;
}
int LocalPlayer::GroupPlay()
{
	if (m_GroupMap.isEmpty())
	{
		return 1;
	}

	if (m_bIsGroupPlaying)
	{
		GroupStop();
	}

	QMap<QWidget*, PrePlay>::iterator iter;
	for (iter = m_GroupMap.begin(); iter != m_GroupMap.end(); iter++)
	{
		if (NULL == iter->pPlayMgr)
		{
			continue;
		}
		iter->pPlayMgr->setParamter(iter->fileList, iter.key(), iter->startTime, iter->endTime, iter->startPos);
		if (iter->pPlayMgr->isRunning())
		{
			iter->pPlayMgr->quit();
		}
		iter->pPlayMgr->start();
	}

	m_bIsGroupPlaying = true;

	return 0;
}
int LocalPlayer::GroupPause()
{
	if (m_GroupMap.isEmpty())
	{
		return 1;
	}

	QMap<QWidget*, PrePlay>::iterator iter;
	for (iter = m_GroupMap.begin(); iter != m_GroupMap.end(); iter++)
	{
		if (NULL == iter->pPlayMgr)
		{
			continue;
		}
		iter->pPlayMgr->pause(true);
	}

	return 0;
}
int LocalPlayer::GroupContinue()
{
	if (m_GroupMap.isEmpty())
	{
		return 1;
	}

	QMap<QWidget*, PrePlay>::iterator iter;
	for (iter = m_GroupMap.begin(); iter != m_GroupMap.end(); iter++)
	{
		if (NULL == iter->pPlayMgr)
		{
			continue;
		}
		iter->pPlayMgr->pause(false);
		g_waitConPause.wakeOne();
	}

	return 0;
}
int LocalPlayer::GroupStop()
{
	if (m_GroupMap.isEmpty())
	{
		return 1;
	}

	QMap<QWidget*, PrePlay>::iterator iter;
	for (iter = m_GroupMap.begin(); iter != m_GroupMap.end(); iter++)
	{
		if (NULL == iter->pPlayMgr)
		{
			continue;
		}
		g_waitConPause.wakeOne();
		iter->pPlayMgr->stop();
		delete iter->pPlayMgr;
	}

	m_bIsGroupPlaying = false;
	m_GroupMap.clear();
	return 0;
}
int LocalPlayer::GroupSpeedFast(int speed)
{
	if (m_GroupMap.isEmpty() || (2 != speed && 4 != speed && 8 != speed))
	{
		return 1;
	}

	QMap<QWidget*, PrePlay>::iterator iter;
	for (iter = m_GroupMap.begin(); iter != m_GroupMap.end(); iter++)
	{
		if (NULL == iter->pPlayMgr)
		{
			continue;
		}
		iter->pPlayMgr->setPlaySpeed(0 - speed);
	}
	return 0;
}
int LocalPlayer::GroupSpeedSlow(int speed)
{
	if (m_GroupMap.isEmpty() || (2 != speed && 4 != speed && 8 != speed))
	{
		return 1;
	}

	QMap<QWidget*, PrePlay>::iterator iter;
	for (iter = m_GroupMap.begin(); iter != m_GroupMap.end(); iter++)
	{
		if (NULL == iter->pPlayMgr)
		{
			continue;
		}
		iter->pPlayMgr->setPlaySpeed(speed);
	}

	return 0;
}
int LocalPlayer::GroupSpeedNormal()
{
	if (m_GroupMap.isEmpty())
	{
		return 1;
	}

	QMap<QWidget*, PrePlay>::iterator iter;
	for (iter = m_GroupMap.begin(); iter != m_GroupMap.end(); iter++)
	{
		if (NULL == iter->pPlayMgr)
		{
			continue;
		}
		iter->pPlayMgr->setPlaySpeed(0);
	}

	return 0;
}

QDateTime LocalPlayer::GetNowPlayedTime()
{
	QDateTime time;
	QTime secTime(0, 0, 0);

	if (m_GroupMap.isEmpty())
	{
		return time;
	}

	QMap<QWidget*, PrePlay>::iterator it;
	it = m_GroupMap.begin();

	if (NULL == it->pPlayMgr)
	{
		return time;
	}

	int mSeconds = 0;
	mSeconds = it->pPlayMgr->getPlayTime();
	mSeconds=mSeconds/1000;
	//time.setDate(QDate::currentDate());
	//time.setTime(secTime.addMSecs(mSeconds));
	time=QDateTime::fromTime_t(mSeconds);
	return time;
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
	else if (IID_ILocalPlayer == iid)
	{
		*ppv = static_cast<ILocalPlayer *>(this);
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

