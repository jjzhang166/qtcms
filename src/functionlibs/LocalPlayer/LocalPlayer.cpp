#include "LocalPlayer.h"
#include <QtCore/QDateTime>
#include <QtCore/QDir>

#include <guid.h>
#include "avilib.h"


LocalPlayer::LocalPlayer() :
m_nRef(0),
m_nGroupNum(4),
m_playTime(0),
m_skipTime(86400),
m_lastPlayTime(0),
m_callTimes(0),
m_bIsGroupPlaying(false),
m_db(NULL),
m_pCurView(NULL)
{
	m_eventList<<"GetRecordDate"<<"GetRecordFile"<<"SearchStop";

	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDiskSetting,(void**)&m_pDiskSetting);

	QDateTime curTime = QDateTime::currentDateTime();
	m_connectId = QString::number(curTime.toTime_t()) + QString::number((int)this);
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectId);
	m_db = new QSqlDatabase(db);
	if (NULL == m_db)
	{
		qDebug()<<"init database failed!";
	}
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
// 			delete iter->pPlayMgr;
// 			iter->pPlayMgr = NULL;
			iter->pPlayMgr->deleteLater();
		}
	}
	m_GroupMap.clear();

	m_db->close();
	delete m_db;
	m_db = NULL;
	QSqlDatabase::removeDatabase(m_connectId);
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
// 	for (int i = 0; i < sltUsedDisk.size(); i++)
// 	{
// 		QString rootDir = QString(sltUsedDisk[i] + ":/REC/");
// 		QDir dir = QDir(rootDir);
// 		dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
// 
// 		QStringList sltSubDir = dir.entryList();
// 		for (int j = 0; j < sltSubDir.size(); j++)
// 		{
// 			QString strSubDir = rootDir + sltSubDir[j] + "/";
// 			QDir subDir = QDir(strSubDir);
// 			QStringList sltSubSubDir = subDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
// 			if (sltSubSubDir.contains(sdevname))
// 			{
// 				QDateTime dateTime = QDateTime::fromString(sltSubDir[j], "yyyy-MM-dd");
// 				QVariantMap dateInfo;
// 
// 				dateInfo.insert("devname", sdevname);
// 				dateInfo.insert("date", dateTime);
// 
// 				eventProcCall(QString("GetRecordDate"), dateInfo);
// 			}
// 		}
// 	}

	foreach(QString disk, sltUsedDisk)
	{
		QString dbPath = disk + ":/REC/record.db";
		m_db->setDatabaseName(dbPath);
		if (!m_db->open())
		{
			qDebug()<<"open " + dbPath + "failed!";
			continue;
		}
		QSqlQuery _query(*m_db);
		QString sqlCommand = QString("select dev_name, date from local_record where dev_name = '%1'").arg(sdevname);
		_query.exec(sqlCommand);
		while(_query.next())
		{
			QDate date = _query.value(1).toDate();
			QDateTime dateTime;
			dateTime.setDate(date);

			QVariantMap dateInfo;
			dateInfo.insert("devname", sdevname);
			dateInfo.insert("date", dateTime);
			eventProcCall(QString("GetRecordDate"), dateInfo);
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

bool LocalPlayer::checkFileFromLong(QString path, unsigned long &tick, QDateTime & endTime)
{
	if (tick <= 0 || path.isEmpty())
	{
		return false;
	}
	QRegExp rx("([0-9]{4}-[0-9]{2}-[0-9]{2})");
	QDate date;
	QTime time(tick/3600, tick%3600/60, tick%60);
	if (-1 != rx.indexIn(path, 0))
	{
		QString dateStr = rx.cap(1);
		date = QDate::fromString(dateStr, "yyyy-MM-dd");
	}
	endTime.setDate(date);
	endTime.setTime(time);
	QString filePath = path + "/" + endTime.toString("hhmmss") + ".avi";
	if (QFile::exists(filePath))
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
	QDate date = QDate::fromString(sdate,"yyyy-MM-dd");
	QTime timeStart = QTime::fromString(sbegintime,"hh:mm:ss");
	QTime timeEnd = QTime::fromString(sendtime,"hh:mm:ss");
// 	int aviFileLength = 0;
// 	int totalFrames = 0;
// 	int frameRate = 0;
// 	int audioChunks = 0;
// 	int audioRate = 0;
// 	int audioBlock = 0;
// 	avi_t *aviFile = NULL;

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

	if (!m_filePeriodMap.isEmpty())
	{
		m_filePeriodMap.clear();
	}

	QStringList sltUsedDisk = sUsedDisks.split(":", QString::SkipEmptyParts);
// 	QStringList sltChannels = schannellist.split(";", QString::SkipEmptyParts);
// 	for (int i = 0; i < sltUsedDisk.size(); i++)
// 	{
// 		QString root = sltUsedDisk[i] + ":/REC/" + sdate + "/" + sdevname;
// 		for (int j = 0; j < sltChannels.size(); j++)
// 		{
// 			QString channel = sltChannels[j].toInt() > 9 ? sltChannels[j] : "0" + sltChannels[j];
// 			QString channelPath = "/CHL" + channel;
// 			QString subPath = root + channelPath;
// 			QDir subDir = QString(subPath);
// 			QFileInfoList sltFileList = subDir.entryInfoList(QDir::Files);
// 			for (int k = 0; k < sltFileList.size(); k++)
// 			{
// 				QString fileName = sltFileList[k].fileName();
// 				QTime fileTime = QTime::fromString(fileName.left(6), "hhmmss");
// 				date.setTime(fileTime);
// 				if (fileTime >= timeStart && fileTime <= timeEnd)
// 				{
// 					QString filePath = subPath + "/" + fileName;
// 					qint64 fileSize = sltFileList[k].size()/1024/1024;
// 					if (0 == sltFileList[k].size())
// 					{
// 						continue;
// 					}
// 
// 					QDateTime endTime;
// 					bool fileExit = false;
// 					aviFile = AVI_open_input_file(filePath.toLatin1().data(), 1);
// 
// 					unsigned long ticket = 0;
// 					AVI_get_ticket(aviFile, &ticket);
// 					if (!(ticket > 0 && (fileExit = checkFileFromLong(subPath, ticket, endTime))))
// 					{
// 						audioChunks = AVI_audio_chunks(aviFile);
// 						audioRate = AVI_audio_rate(aviFile);
// 						audioBlock = AVI_audio_size(aviFile, 0);
// 						if (0 <= audioChunks && 0 <= audioRate && 0 <= audioBlock)
// 						{
// 							aviFileLength = audioChunks*audioBlock/audioRate;
// 						}
// 						else
// 						{
// 							totalFrames = AVI_video_frames(aviFile);
// 							frameRate = AVI_frame_rate(aviFile);
// 							if (0 == totalFrames || 0 == frameRate)
// 							{
// 								continue;
// 							}
// 							aviFileLength = totalFrames/frameRate;//the length of avi file playing time
// 						}			
// 					}
// 
// 					AVI_close(aviFile);
// 
// 					QVariantMap fileInfo;
// 					fileInfo.insert("filename", fileName);
// 					fileInfo.insert("filepath", filePath);
// 					fileInfo.insert("filesize", QString("%1").arg(fileSize));
// 					fileInfo.insert("channelnum", sltChannels[j]);
// 					fileInfo.insert("startTime", date);
// 					if (fileExit)
// 					{
// 						fileInfo.insert("stopTime", endTime);
// 					}
// 					else
// 					{
// 						fileInfo.insert("stopTime", date.addSecs(aviFileLength));
// 					}
// 
// 					PeriodTime item;
// 					item.start = fileInfo.value("startTime").toDateTime().toTime_t();
// 					item.end = fileInfo.value("stopTime").toDateTime().toTime_t();
// 					m_filePeriodMap.insert(filePath, item);
// 
// 					eventProcCall(QString("GetRecordFile"), fileInfo);
// 				}
// 			}
// 		}
// 	}

	QString chllist = schannellist.left(schannellist.size() - 1);
	QString sqlChl = "dev_chl=" + chllist.replace(";", " or dev_chl=");
	QString sqlCommand = QString("select dev_chl, start_time, end_time, file_size, path from local_record where dev_name='%1' and date='%2' and start_time >='%3' and end_time<='%4' and (%5) order by start_time").arg(sdevname).arg(sdate).arg(sbegintime).arg(sendtime).arg(sqlChl);

	foreach(QString disk, sltUsedDisk)
	{
		QString dbPath = disk + ":/REC/record.db";
		m_db->setDatabaseName(dbPath);
		if (!m_db->open())
		{
			qDebug()<<"open " + dbPath + "failed!";
			continue;
		}
		QSqlQuery _query(*m_db);
		_query.exec(sqlCommand);
		while (_query.next())
		{
			QString chl = _query.value(0).toString();
			QTime start = _query.value(1).toTime();
			QTime end = _query.value(2).toTime();
			QString size = _query.value(3).toString();
			QString path = _query.value(4).toString();

			QDateTime startTime;
			startTime.setDate(date);
			startTime.setTime(start);
			QDateTime endTime;
			endTime.setDate(date);
			endTime.setTime(end);
			if (startTime >= endTime || size.toInt() <=0)
			{
				continue;
			}

			PeriodTime item;
			item.start = startTime.toTime_t();
			item.end = endTime.toTime_t();
			m_filePeriodMap.insert(path, item);

			QVariantMap fileInfo;
			fileInfo.insert("filename", path.right(path.size() - path.lastIndexOf("/") - 1));
			fileInfo.insert("filepath", path);
			fileInfo.insert("filesize", size);
			fileInfo.insert("channelnum", chl);
			fileInfo.insert("startTime", startTime);
			fileInfo.insert("stopTime", endTime);

			eventProcCall(QString("GetRecordFile"), fileInfo);
		}

		m_db->close();
	}


	QVariantMap stopInfo;
	stopInfo.insert("stopevent", QString("GetRecordFile"));
	eventProcCall(QString("SearchStop"), stopInfo);

	return ILocalRecordSearch::OK;
}
QStringList LocalPlayer::sortFileList(QStringList const fileList)
{
	QStringList sortList;
	if (fileList.isEmpty())
	{
		return sortList;
	}
	sortList = fileList;
	for (int i = 0; i < sortList.size() - 1; i++)
	{
		uint minTime = m_filePeriodMap.value(sortList.at(i)).start;
		int minPos = i;
		for (int j = i + 1; j < sortList.size(); j++)
		{
			uint time = m_filePeriodMap.value(sortList.at(j)).start;
			if (time < minTime)
			{
				minTime = time;
				minPos = j;
			}
		}
		if (i != minPos)
		{
			sortList.swap(i, minPos);
		}
	}
	return sortList;
}
int LocalPlayer::checkFileExist(QStringList const fileList, const QDateTime& startTime, const QDateTime& endTime, QVector<PeriodTime> &perTimeVec)
{
	QString filePath;
// 	QString dateStr;
// 	QFileInfo fileInfo(filePath);
// 	QDateTime dateTime;
// 	QDateTime fileEndTime;
// 	QDate date;
// 	QTime time;
// 	QRegExp rx("([0-9]{4}-[0-9]{2}-[0-9]{2})");
	int firstFile = -1;
	bool find = false;
	PeriodTime perTime;
// 	int aviFileLength = 0;
// 	int totalFrames = 0;
// 	int frameRate = 0;
// 	avi_t *aviFile = NULL;

	if (fileList.isEmpty())
	{
		return -1;
	}
	for(int pos = 0; pos < fileList.size(); pos++)
	{
		filePath = fileList[pos];
// 		if (-1 != rx.indexIn(filePath,0))
// 		{
// 			dateStr = rx.cap(1);
// 		}
// 		fileInfo.setFile(filePath);
// 		QString baseName = fileInfo.baseName();
// 		date = QDate::fromString(dateStr, "yyyy-MM-dd");
// 		time = QTime::fromString(baseName, "hhmmss");
// 		dateTime.setDate(date);
// 		dateTime.setTime(time);
// 
// 		if (0 == fileInfo.size())
// 		{
// 			continue;
// 		}
// 
// 		aviFile = AVI_open_input_file(filePath.toLatin1().data(), 0);
// 		totalFrames = AVI_video_frames(aviFile);
// 		frameRate = AVI_frame_rate(aviFile);
// 		if (totalFrames==0||frameRate==0)
// 		{
// 			AVI_close(aviFile);
// 			continue;
// 		}
// 		aviFileLength = totalFrames/frameRate;//the length of avi file playing time
// 		AVI_close(aviFile);
// 
// 		fileEndTime = dateTime.addSecs(aviFileLength);

		PeriodTime item = m_filePeriodMap.value(filePath);

		if (!find && !((item.start >= endTime.toTime_t()) || (item.end <= startTime.toTime_t())))
		{
			firstFile = pos;
			find = true;
		}
		perTime.start = qMax(item.start, startTime.toTime_t());
		perTime.end = qMin(item.end, endTime.toTime_t());
		if (perTime.end > perTime.start)
		{
			perTimeVec.append(perTime);
		}
	}

	return firstFile;
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
// 	//fileList has different channel
// 	if (!checkChannelInFileList(filelist))
// 	{
// 		return 3;
// 	}
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
	//sort file list
	QStringList sortList = sortFileList(filelist);

	QVector<PeriodTime> vecPerTime;
	//there is no file between start time and end time
	int startPos = checkFileExist(sortList, start, end, vecPerTime);
	if (-1 == startPos)
	{
		return 3;
	}

	PrePlay prePlay;
	prePlay.pPlayMgr = new PlayMgr();
	prePlay.fileList = sortList;
	prePlay.startTime = start;
	prePlay.endTime = end;
	prePlay.startPos = startPos;
	prePlay.skipTime = vecPerTime;
	
	m_startTime = start.toTime_t();
	m_endTime = end.toTime_t();

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

int LocalPlayer::countSkipTime()
{
	if (m_GroupMap.isEmpty())
	{
		return 1;
	}

	QVector<PeriodTime> skipTime;
	PeriodTime perTime;
	QMap<QWidget*, PrePlay>::iterator iter = m_GroupMap.begin();
	while (iter != m_GroupMap.end())
	{
		for (int i = 0; i < iter->skipTime.size(); ++i)
		{
 			if (skipTime.isEmpty())
 			{
				skipTime.append(iter->skipTime[i]);
 				continue;
 			}
 			int j = 0;
 			while(j < skipTime.size())
 			{ 
 				if (iter->skipTime[i].start == skipTime[j].start && iter->skipTime[i].end == skipTime[j].end)
 				{
 					break;
 				}
 				if (iter->skipTime[i].end < skipTime[j].start)
 				{
					if ((j >= 1  && iter->skipTime[i].start > skipTime[j - 1].end))
					{
						skipTime.insert(j - 1, iter->skipTime[i]);
						break;
					}
					else if (0 == j)
					{
						skipTime.prepend(iter->skipTime[i]);
					}
 				}
 				if (iter->skipTime[i].start > skipTime[j].end)
 				{
 					if ((j + 1 < skipTime.size() && iter->skipTime[i].end < skipTime[j + 1].start) || (j + 1 == skipTime.size()))
 					{
 						skipTime.insert(j + 1, iter->skipTime[i]);
 						break;
 					}
 				}
				if ((iter->skipTime[i].start < skipTime[j].start && iter->skipTime[i].end >= skipTime[j].start)|| (iter->skipTime[i].end > skipTime[j].end) && (iter->skipTime[i].start <= skipTime[j].end))
				{
					skipTime[j].start = qMin(skipTime[j].start, iter->skipTime[i].start);
					skipTime[j].end = qMax(skipTime[j].end, iter->skipTime[i].end);
					break;
				}
 				++j;
 			}
		}
		++iter;
	}

	QVector<PeriodTime> result;
	for (int i = 0 ; i <= skipTime.size(); ++i)
	{
		if (0 == i && skipTime[i].start >= m_startTime)
		{
			perTime.start = m_startTime;
			perTime.end = skipTime[i].start;
		}
		else if (i == skipTime.size() && skipTime[i - 1].end != m_endTime)
		{
			perTime.start = skipTime[i - 1].end;
			perTime.end = m_endTime;
		}
		else if (i > 0 && i < skipTime.size())
		{
			perTime.start = skipTime[i - 1].end;
			perTime.end = skipTime[i].start;
		}
		else
		{
			//nothing
		}

		if (perTime.start < perTime.end)
		{
			result.append(perTime);
			perTime.start = 0;
			perTime.end = 0;
		}
	}

	iter = m_GroupMap.begin();
	while(iter != m_GroupMap.end())
	{
		iter->skipTime = result;
		++iter;
	}

	return 0;
}

int LocalPlayer::GroupPlay()
{
	if (m_GroupMap.isEmpty())
	{
		return 1;
	}

	countSkipTime();

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
		iter->pPlayMgr->setCbTimeChange(cbTimeChange, this);
		iter->pPlayMgr->setParamter(iter->fileList, iter.key(), iter->startTime, iter->endTime, iter->startPos, iter->skipTime);
		iter->pPlayMgr->setFileInfo(m_filePeriodMap);
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
/*		delete iter->pPlayMgr;*/
		iter->pPlayMgr->deleteLater();
	}

	m_bIsGroupPlaying = false;
	m_GroupMap.clear();
	m_playTime = 0;
	m_skipTime = 86400;
	m_callTimes = 0;
	m_lastPlayTime = 0;
	
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

	time.setDate(QDate::currentDate());
	time.setTime(secTime.addSecs(m_playTime));
	return time;
}
void LocalPlayer::setBaseTime(uint &baseTime)
{
	if (m_callTimes == m_GroupMap.size() - 1)
	{
		m_playTime += qMin(m_skipTime, baseTime);
		m_skipTime = 86400;
		m_callTimes = 0;
	}
	else
	{
		m_callTimes++;
		m_skipTime = qMin(m_skipTime, baseTime);
	}
}
void LocalPlayer::setPlayTime(uint &playTime)
{
	if (playTime < 0)
	{
		return;
	}

	if (m_lastPlayTime < playTime)
	{
		m_playTime += playTime - m_lastPlayTime;
	}
	m_lastPlayTime = playTime;
}

bool LocalPlayer::GroupEnableAudio(bool bEnable)
{
	if(m_GroupMap.isEmpty())
	{
		return false;
	}
	if (NULL!=m_pCurView)
	{
		QMap<QWidget*, PrePlay>::iterator iter = m_GroupMap.find(m_pCurView);
		if (m_GroupMap.end() == iter)
		{
			return false;
		}
		if (!bEnable)
		{
			iter->pPlayMgr->OpneAudio(true);
			m_pCurView = NULL;
		}
		iter->pPlayMgr->AudioSwitch(bEnable);
	}

	return bEnable;
}
int LocalPlayer::GroupSetVolume(unsigned int uiPersent, QWidget* pWnd)
{
	if (uiPersent < 0 || m_GroupMap.isEmpty())
	{
		return 1;
	}
	if (0xAECBCB==uiPersent&&m_pCurView==NULL)
	{
		m_pCurView=pWnd;
		return 0;
	}
	QMap<QWidget*, PrePlay>::iterator iter = m_GroupMap.find(pWnd);
	if (m_GroupMap.end() == iter)
	{
		return 1;
	}
	if (0xAECBCA == uiPersent)
	{
		if (NULL != m_pCurView)
		{
			QMap<QWidget*, PrePlay>::iterator it = m_GroupMap.find(m_pCurView);
			if (it != m_GroupMap.end())
			{
				it->pPlayMgr->OpneAudio(false);
			}
		}
		m_pCurView = pWnd;
		iter->pPlayMgr->OpneAudio(true);
	}
	else
	{
		iter->pPlayMgr->setVolume(uiPersent);
	}
	return 0;
}

void cbTimeChange(QString evName, uint playTime, void* pUser)
{
	if (playTime >= 0 && pUser != NULL)
	{
		if ("playingTime" == evName)
		{
			((LocalPlayer*)pUser)->setPlayTime(playTime);
		}
		if ("skipTime" == evName)
		{
			((LocalPlayer*)pUser)->setBaseTime(playTime);
		}
	}
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
	else if (IID_ILocalRecordSearchEx == iid)
	{
		*ppv = static_cast<ILocalRecordSearchEx *>(this);
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

int LocalPlayer::searchVideoFileEx( const QString &sDevName, const QString& sDate, const int& nTypes )
{
	QDate date = QDate::fromString(sDate,"yyyy-MM-dd");
	if (!date.isValid())
	{
		return ILocalRecordSearchEx::E_PARAMETER_ERROR;
	}

	//get available disk
	QString sUsedDisks;
	if (1 == checkUsedDisk(sUsedDisks))
	{
		return ILocalRecordSearchEx::E_SYSTEM_FAILED;
	}
	if (sUsedDisks.isEmpty())
	{
		return ILocalRecordSearchEx::E_SYSTEM_FAILED;
	}
	QStringList sltUsedDisk = sUsedDisks.split(":", QString::SkipEmptyParts);

	if (!m_filePeriodMap.isEmpty())
	{
		m_filePeriodMap.clear();//clear info last time remain
	}
	//create query command
	QString sqlType = getTypeList(nTypes);
	QString sqlCommand = QString("select dev_chl, start_time, end_time, file_size, path from local_record where dev_name='%1' and date='%2' and (%3) order by start_time").arg(sDevName).arg(sDate).arg(sqlType);
	//query
	foreach(QString disk, sltUsedDisk)
	{
		QString dbPath = disk + ":/REC/record.db";
		m_db->setDatabaseName(dbPath);
		if (!m_db->open())
		{
			qDebug()<<"open " + dbPath + " failed!"<<__LINE__;
			continue;
		}
		QSqlQuery _query(*m_db);
		_query.exec(sqlCommand);
		while(_query.next())
		{
			QString chl = _query.value(0).toString();
			QTime start = _query.value(1).toTime();
			QTime end = _query.value(2).toTime();
			QString size = _query.value(3).toString();
			QString path = _query.value(4).toString();

			QDateTime startTime;
			startTime.setDate(date);
			startTime.setTime(start);
			QDateTime endTime;
			endTime.setDate(date);
			endTime.setTime(end);
			if (startTime >= endTime || size.toInt() <=0)
			{
				continue;
			}
			//record file's start and end time
			PeriodTime item;
			item.start = startTime.toTime_t();
			item.end = endTime.toTime_t();
			m_filePeriodMap.insert(path, item);

			//send file info to up level
			QVariantMap fileInfo;
			fileInfo.insert("filename", path.right(path.size() - path.lastIndexOf("/") - 1));
			fileInfo.insert("filepath", path);
			fileInfo.insert("filesize", size);
			fileInfo.insert("channelnum", chl);
			fileInfo.insert("startTime", startTime);
			fileInfo.insert("stopTime", endTime);

			eventProcCall(QString("GetRecordFile"), fileInfo);
		}

		m_db->close();
	}

	QVariantMap stopInfo;
	stopInfo.insert("stopevent", QString("GetRecordFile"));
	eventProcCall(QString("SearchStop"), stopInfo);

	return ILocalRecordSearch::OK;
}

QString LocalPlayer::getTypeList( int nTypes )
{
	QString typeList;
	int pos = 0;
	while (nTypes > 0)
	{
		if (nTypes & 1)
		{
			typeList += (0 == pos) ? QString::number(pos) : ("*" + QString::number(pos));
		}
		pos++;
		nTypes = nTypes>>1;
	}

	typeList = "record_type=" + typeList.replace("*", " or record_type=");
	return typeList;
}
