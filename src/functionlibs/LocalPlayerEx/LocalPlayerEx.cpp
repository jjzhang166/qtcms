#include "LocalPlayerEx.h"
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <guid.h>
#include "IDisksSetting.h"
//#include "vld.h"


LocalPlayerEx::LocalPlayerEx()
	:m_nRef(0),
	m_i32GroupNum(0),
	m_i32Types(0),
	m_uiStartSec(0),
	m_uiEndSec(0),
	m_uiPlayTime(0),
	m_pCurAudioWnd(NULL),
	m_pFileData(NULL)
{
	//set event list
	m_eventList<<"GetRecordFileEx"<<"ThrowException";

	//get usable disk list
	IDisksSetting *pDiskSet = NULL;
	pcomCreateInstance(CLSID_CommonLibPlugin,NULL,IID_IDiskSetting,(void**)&pDiskSet);
	if (pDiskSet)
	{
		pDiskSet->getEnableDisks(m_disklst);//get usable disks
		pDiskSet->Release();
	}

	//create thread for read file
	m_pFileData = new QFileData();
	connect(m_pFileData, SIGNAL(sigStopPlay()), this, SLOT(onStopPlayMgr()));

	//create thread for playing
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		PlayMgr *pplayMgr = new PlayMgr();
		m_arrPlayInfo[i32Loop].pPlayMgr = pplayMgr;
		m_arrPlayInfo[i32Loop].i32WndId = NO_WINDOW_ID;
// 		connect(m_pFileData, SIGNAL(sigSkipTime(uint)), pplayMgr, SLOT(onSkipTime(uint)));
		connect(m_pFileData, SIGNAL(sigStartPlay(uint)), this, SLOT(onStartPlayMgr(uint)));
	}
}

LocalPlayerEx::~LocalPlayerEx()
{
	//clear read file thread
	if (m_pFileData)
	{
		if (m_pFileData->isRunning())
		{
			m_pFileData->stopThread();
		}
// 		m_pFileData->deleteLater();
		delete m_pFileData;
		m_pFileData = NULL;
	}
	//clear play thread
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		if (m_arrPlayInfo[i32Loop].pPlayMgr->isRunning())
		{
			m_arrPlayInfo[i32Loop].pPlayMgr->stop();
		}
// 		m_arrPlayInfo[i32Loop].pPlayMgr->deleteLater();
		delete m_arrPlayInfo[i32Loop].pPlayMgr;
		m_arrPlayInfo[i32Loop].pPlayMgr = NULL;
	}
	deInitDataBase();
}

QSqlDatabase * LocalPlayerEx::initDataBase( QString sDatabaseName )
{
	if (!m_dbMap.contains(sDatabaseName))
	{
		QString connectId = sDatabaseName + QString::number((int)this);
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectId);
		QSqlDatabase *pdb = new QSqlDatabase(db);
		pdb->setDatabaseName(sDatabaseName);
		if (!pdb->open())
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"open database error!";
			return NULL;
		}
		m_dbMap.insert(sDatabaseName, pdb);
		return pdb;
	}
	else
	{
		return m_dbMap.value(sDatabaseName);
	}
}
sqlite3* LocalPlayerEx::initDataBase(char *dbPath)
{
	if (!m_sqlMap.contains(QString(dbPath)))
	{
		sqlite3 *pdb = NULL;
		int ret = sqlite3_open(dbPath, &pdb);
		if (ret != SQLITE_OK)
		{
			return NULL;
		}
		else
		{
			//check if database has a table
			char **ppDbRet = NULL;
			int nRow = 0, nColumn = 0;
			sqlite3_get_table(pdb, "select count(*) from sqlite_master where type='table';", &ppDbRet, &nRow, &nColumn, NULL);
			if (!strcmp(ppDbRet[nColumn], "0"))
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"no table: search_record in"<<dbPath;
				sqlite3_close(pdb);
				return NULL;
			}
			m_sqlMap.insert(QString(dbPath), pdb);
			return pdb;
		}
	}
	else
	{
		return m_sqlMap.value(QString(dbPath));
	}
}
void LocalPlayerEx::deInitDataBase()
{
	QMap<QString, QSqlDatabase*>::iterator iter = m_dbMap.begin();
	while (iter != m_dbMap.end())
	{
		(*iter)->close();
		delete *iter;
		QSqlDatabase::removeDatabase(iter.key() + QString::number((int)this));
		++iter;
	}
	m_dbMap.clear();

	QMap<QString, sqlite3*>::iterator it = m_sqlMap.begin();
	while (it != m_sqlMap.end())
	{
		sqlite3_close(*it++);
	}
	m_sqlMap.clear();
}

long __stdcall LocalPlayerEx::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_ILocalRecordSearchEx == iid)
	{
		*ppv = static_cast<ILocalRecordSearchEx *>(this);
	}
	else if (IID_ILocalPlayerEx == iid)
	{
		*ppv = static_cast<ILocalPlayerEx *>(this);
	}
	else if (IID_IEventRegister == iid)
	{
		*ppv = static_cast<IEventRegister *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IVideoDisplayOption == iid)
	{
		*ppv = static_cast<IVideoDisplayOption *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall LocalPlayerEx::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall LocalPlayerEx::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef-- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

QStringList LocalPlayerEx::eventList()
{
	return m_eventList;
}

int LocalPlayerEx::queryEvent( QString eventName,QStringList &eventParams )
{
	if (!m_eventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	if ("GetRecordFileEx" == eventName)
	{
		eventParams<<"wndId"<<"type"<<"startTime"<<"endTime";
	}
	if ("ThrowException" == eventName)
	{
		eventParams<<"file"<<"expCode"<<"pWnd";
	}

	return IEventRegister::OK;
}

int LocalPlayerEx::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
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

void LocalPlayerEx::eventProcCall( QString sEvent,QVariantMap param )
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

QString LocalPlayerEx::getTypeList( int nTypes )
{
	QStringList typeList;
	int pos = 1;
	while (nTypes > 0)
	{
		if (nTypes & 1)
		{
			typeList<<"nRecordType=" + QString::number(pos);
		}
		pos++;
		nTypes = nTypes>>1;
	}

	return typeList.join(" or ");
}

int LocalPlayerEx::searchVideoFileEx( const int & nWndId, const QString & sDate, const QString & sStartTime, const QString & sEndTime, const int & nTypes )
{
	//get start seconds and end seconds
	QString sStart = sDate + " " + sStartTime;
	QString sEnd = sDate + " " + sEndTime;
	QDateTime dtStart = QDateTime::fromString(sStart, "yyyy-MM-dd hh:mm:ss");
	QDateTime dtEnd = QDateTime::fromString(sEnd, "yyyy-MM-dd hh:mm:ss");
	QString sqlCmd = QString("select nRecordType, nStartTime, nEndTime from search_record where nWndId=%1 and nEndTime>'%2' and nStartTime<'%3' and (%4) and nStartTime!=nEndTime order by nStartTime").arg(nWndId).arg(dtStart.toTime_t()).arg(dtEnd.toTime_t()).arg(getTypeList(nTypes));
	QStringList diskList = m_disklst.split(":", QString::SkipEmptyParts);
	foreach(QString disk, diskList)
	{
		QString sDbName = disk + ":/recEx/record.db";
		if (!QFile::exists(sDbName))
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"file "<<sDbName<<" didn't exist";
			continue;
		}
// 		QSqlDatabase *pdb = initDataBase(sDbName);
// 		if (!pdb)
// 		{
// 			qDebug()<<__FUNCTION__<<__LINE__<<"get database point error!";
// 			continue;
// 		}
// 		QSqlQuery query(*pdb);
// // 		query.exec(sqlCmd);
// 		exceCommand(query, sqlCmd);
// 
// 		while (query.next())
// 		{
// 			int recordType = query.value(0).toInt();
// 			uint start = query.value(1).toUInt();
// 			uint end = query.value(2).toUInt();
// 
// 			QVariantMap info;
// 			info.insert("wndId", nWndId);
// 			info.insert("type", recordType);
// 
// 			start = qMax(dtStart.toTime_t(), start);
// 			end = qMin(dtEnd.toTime_t(), end);
// 			info.insert("start", QDateTime::fromTime_t(start).toString("yyyy-MM-dd hh:mm:ss"));
// 			info.insert("end", QDateTime::fromTime_t(end).toString("yyyy-MM-dd hh:mm:ss"));
// 
// 			eventProcCall(QString("GetRecordFileEx"), info);
// 		}
// 		query.finish();

		sqlite3 *db = NULL;
		db = initDataBase(sDbName.toLatin1().data());
		if (!db)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<nWndId<<" open db error";
			continue;
		}

		char **ppDbRet = NULL, *pErrMsg = NULL;
		int nRow = 0, nColumn = 0, index = 0, count = 0, ret = -1;
		while ((ret = sqlite3_get_table(db, sqlCmd.toLatin1().data(), &ppDbRet, &nRow, &nColumn, &pErrMsg)) != SQLITE_OK && count < 50)
		{
			//wait 3 milliseconds
			QEventLoop tEventLoop;
			QTimer::singleShot(3,&tEventLoop,SLOT(quit()));
			tEventLoop.exec();
			++count;
		}
		if (count>0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<nWndId<<"repeat times: "<<count;
		}
		if (ret == SQLITE_OK)
		{
			uint uiStart = dtStart.toTime_t();
			uint uiEnd = dtEnd.toTime_t();
			QVariantMap info;
			index = nColumn;
			for (int i = 0; i < nRow; ++i)
			{
				int recordType = atoi(ppDbRet[index++]);
				uint start = atoi(ppDbRet[index++]);
				uint end = atoi(ppDbRet[index++]);
				
				info.clear();
	 			info.insert("wndId", nWndId);
	 			info.insert("type", recordType);
	 
	 			start = qMax(uiStart, start);
	 			end = qMin(uiEnd, end);
	 			info.insert("start", QDateTime::fromTime_t(start).toString("yyyy-MM-dd hh:mm:ss"));
	 			info.insert("end", QDateTime::fromTime_t(end).toString("yyyy-MM-dd hh:mm:ss"));

	 			eventProcCall(QString("GetRecordFileEx"), info);
			}
		}
		else
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"exec ret = "<<ret<<" msg: "<<pErrMsg;
		}
		sqlite3_free_table(ppDbRet);
// 		sqlite3_close(db);
	}
	return ILocalRecordSearchEx::OK;
}


int LocalPlayerEx::AddFileIntoPlayGroupEx( const int & nWndId, const QWidget * pWnd, const QDate& date, const QTime & startTime, const QTime & endTime, const int & nTypes )
{
	qint32 i32Loop = 0;
	qint32 i32WndNum = 0;
	while (i32Loop < MAX_PLAY_THREAD)
	{
		//wndid is repeat
		if ( nWndId == m_arrPlayInfo[i32Loop].i32WndId)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"wndId: "<<nWndId<<" has in group already";
			return 1;
		}
		if (NO_WINDOW_ID != m_arrPlayInfo[i32Loop++].i32WndId)
		{
			++i32WndNum;
		}
	}
	//group full
	if ( MAX_PLAY_THREAD == i32WndNum)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"wndId: "<<nWndId<<" group is full!";
		return 1;
	}
	//set start and end seconds
	uint uiStart = QDateTime(date, startTime).toTime_t();
	uint uiEnd = QDateTime(date, endTime).toTime_t();
	if (m_uiStartSec != uiStart && m_uiEndSec != uiEnd)
	{
		m_uiStartSec = uiStart;
		m_uiEndSec = uiEnd;
	}
	if (m_i32Types != nTypes)
	{
		m_i32Types = nTypes;
	}
	//set para for play thread
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		if (NO_WINDOW_ID == m_arrPlayInfo[i32Loop].i32WndId)
		{
			m_arrPlayInfo[i32Loop].i32WndId = nWndId;
			m_arrPlayInfo[i32Loop].pPlayMgr->setParamter(const_cast<QWidget*>(pWnd), m_uiStartSec, m_uiEndSec);
			m_arrPlayInfo[i32Loop].pPlayMgr->setCbTimeChange(cbTimeChange, this);
			m_pFileData->setBuffer(nWndId, m_arrPlayInfo[i32Loop].pPlayMgr->getBufferPointer());
			break;
		}
	}

	return 0;
}

int LocalPlayerEx::SetSynGroupNum( int num )
{
	if (num < 0 || num > 64)
	{
		return 1;
	}
	m_i32GroupNum = (qint32)num;
	return 0;
}

int LocalPlayerEx::GroupPlay()
{
	//no wndId 
	if (NO_WINDOW_ID ==  m_arrPlayInfo->i32WndId)
	{
		return 1;
	}

	qint32 i32BufferNum = 0;
	qint32 i32StartPos = 0;
	QVector<PeriodTime> skipTime;
	QMap<uint, QVector<PeriodTime> > filePeriodMap;
	//get file list for playing
	QList<QString> fileList = getFileList(i32StartPos, filePeriodMap);
	if (fileList.isEmpty())
	{
		return 1;
	}
	//count skip time
	countSkipTime(filePeriodMap, skipTime);
	//set skip time and each file period for playing thread
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		uint wndId = m_arrPlayInfo[i32Loop].i32WndId;
		if (NO_WINDOW_ID != wndId && filePeriodMap.contains(wndId))
		{
			m_arrPlayInfo[i32Loop].pPlayMgr->setSkipTime(skipTime);
			m_arrPlayInfo[i32Loop].pPlayMgr->setFilePeriod(filePeriodMap.value(wndId));
		}
	}
	//set parameter into thread for reading 
	m_pFileData->setParamer(fileList, m_uiStartSec, m_uiEndSec, i32StartPos);
	m_pFileData->setSpeed(0);
	//start read file thread
	m_pFileData->startReadFile();

	return 0;
}

// int LocalPlayerEx::GroupPlayBack()
// {
// 	return 0;
// }

int LocalPlayerEx::GroupPause()
{
	PlayMgr::pause(true);
	return 0;
}

int LocalPlayerEx::GroupContinue()
{
	PlayMgr::pause(false);
	return 0;
}

int LocalPlayerEx::GroupStop()
{
	//stop read file thread
	m_pFileData->stopThread();
	//stop play thread
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		m_arrPlayInfo[i32Loop].pPlayMgr->stop();
		m_arrPlayInfo[i32Loop].i32WndId = NO_WINDOW_ID;
	}
	//clear read file buffer
// 	m_pFileData->clearBuffer();

	m_uiStartSec = 0;
	m_uiEndSec = 0;
	m_uiPlayTime = 0;
	return 0;
}

int LocalPlayerEx::GroupSpeedFast( int speed )
{
	//group is empty or speed isn't 2,4,or 8
	if (NO_WINDOW_ID == m_arrPlayInfo->i32WndId || (2 != speed && 4!= speed && 8 != speed))
	{
		return 1;
	}
	m_pFileData->setSpeed(0 - speed);
	//set play speed
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		if (NO_WINDOW_ID != m_arrPlayInfo[i32Loop].i32WndId)
		{
			m_arrPlayInfo[i32Loop].pPlayMgr->setSpeedRate(0 - speed);
		}
	}
	return 0;
}

int LocalPlayerEx::GroupSpeedSlow( int speed )
{
	//group is empty or speed isn't 2,4,or 8
	if (NO_WINDOW_ID == m_arrPlayInfo->i32WndId || (2 != speed && 4!= speed && 8 != speed))
	{
		return 1;
	}
	//set play speed
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		if (NO_WINDOW_ID != m_arrPlayInfo[i32Loop].i32WndId)
		{
			m_arrPlayInfo[i32Loop].pPlayMgr->setSpeedRate(speed);
		}
	}
	return 0;
}

int LocalPlayerEx::GroupSpeedNormal()
{
	//group is empty or speed isn't 2,4,or 8
	if (NO_WINDOW_ID == m_arrPlayInfo->i32WndId)
	{
		return 1;
	}
	//set play speed
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		if (NO_WINDOW_ID != m_arrPlayInfo[i32Loop].i32WndId)
		{
			m_arrPlayInfo[i32Loop].pPlayMgr->setSpeedRate(0);
		}
	}
	return 0;
}

QDateTime LocalPlayerEx::GetNowPlayedTime()
{
	QDateTime datetime = QDateTime::fromTime_t(m_uiStartSec);
	qint32 diff = m_uiPlayTime - m_uiStartSec;
	QTime spend(diff/3600, diff%3600/60, diff%60);
	datetime.setTime(spend);
	return datetime;
}

bool LocalPlayerEx::GroupEnableAudio( bool bEnable )
{
	PlayMgr::audioSwitch(bEnable);
	return true;
}

int LocalPlayerEx::GroupSetVolume( unsigned int uiPersent, QWidget* pWnd )
{
	if (!pWnd)
	{
		return 1;
	}
	PlayMgr *pplayMgr = getPlayMgrPointer(pWnd);
	if (!pplayMgr)
	{
		return 1;
	}

	if (0xAECBCA == uiPersent)
	{
		if (!m_pCurAudioWnd)
		{
			m_pCurAudioWnd = pplayMgr;
			m_pCurAudioWnd->openAudio(true);
			return 0;
		}
		else
		{
			m_pCurAudioWnd->openAudio(false);
		}
		m_pCurAudioWnd = pplayMgr;
		m_pCurAudioWnd->openAudio(true);
	}
	else
	{
		pplayMgr->setVolume(uiPersent);
	}

	return 0;
}

void LocalPlayerEx::getWndIdList( QList<qint32> &wndList )
{
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		qint32 i32WndId = m_arrPlayInfo[i32Loop].i32WndId;
		if (NO_WINDOW_ID != i32WndId)
		{
			wndList<<i32WndId;
		}
	}
}

QString LocalPlayerEx::intToStr( QList<qint32> &wndList )
{
	QString wndStr;
	QList<qint32>::iterator it = wndList.begin();
	while (it != wndList.end())
	{
		wndStr += QString::number(*it++) + ",";
	}
	return wndStr.left(wndStr.size() - 1);
}

QList<QString> LocalPlayerEx::getFileList( qint32 &i32Pos, QMap<uint, QVector<PeriodTime> >& filePeriodMap )
{
	qint32 insertPos = -1;
	QList<TimePath> tpList;

	QList<QString> fileList;
// 	QVector<uint> vecTime;
	QList<qint32> wndList;
	getWndIdList(wndList);
	if (wndList.isEmpty())
	{
		return fileList;
	}
// 	QDateTime dateTime = QDateTime::fromTime_t(m_uiStartSec);
// 	QDateTime startTime(dateTime.date(), QTime(0, 0, 0));
// 	QDateTime endTime(dateTime.date(), QTime(23, 59, 59));
	QString sqlCommand = QString("select nWndId, nStartTime, nEndTime, sFilePath from record where nWndId in(%1) and (%2) and nStartTime < %3 and nEndTime > %4 and nStartTime!=nEndTime order by nStartTime").arg(intToStr(wndList)).arg(getTypeList(m_i32Types)).arg(m_uiEndSec).arg(m_uiStartSec);
	QStringList diskList = m_disklst.split(":", QString::SkipEmptyParts);
	foreach(QString disk, diskList)
	{
		QString sDbName = disk + ":/recEx/record.db";
		if (!QFile::exists(sDbName))
		{
			qDebug()<<__FUNCTION__<<__LINE__<<sDbName<<" didn't exists!";
			continue;
		}
		sqlite3 *pdb = initDataBase(sDbName.toLatin1().data());
		if (!pdb)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"get database point error!";
			continue;
		}
		char **ppDbRet = NULL, *pErrMsg = NULL;
		int nRow = 0, nColumn = 0, index = 0;
		QStringList excludeFilelst;
		//find locked file or damaged file
		int ret = execCommand(pdb, "select sFilePath from RecordFileStatus where (nLock=1 or nDamage=1);", &ppDbRet, &nRow, &nColumn, &pErrMsg);
		if (SQLITE_OK == ret)
		{
			index = nColumn;
			for (int i = 0; i < nRow; ++i)
				excludeFilelst<<QString(ppDbRet[index++]);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"search RecordFileStatus failed. error: "<<pErrMsg;
		}
		ret = execCommand(pdb, sqlCommand.toLatin1().data(), &ppDbRet, &nRow, &nColumn, &pErrMsg);
		if (SQLITE_OK == ret)
		{
			index = nColumn;
			for (int i = 0; i < nRow; ++i)
			{
				uint wndId = atoi(ppDbRet[index++]);
				uint start = atoi(ppDbRet[index++]);
				uint end = atoi(ppDbRet[index++]);
				QString path = QString(ppDbRet[index++]);
				//exclude file from file list
				if (excludeFilelst.contains(path))
				{
					continue;
				}
				appendTimePath(tpList, start, path, insertPos);
				PeriodTime item = {start, end};
				appendPeriodTime(filePeriodMap[wndId], item);
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"search record failed. error: "<<pErrMsg;
		}
		sqlite3_free_table(ppDbRet);
// 		QSqlQuery query(*pdb);
// 		//find locked file or damaged file
// 		QString cmd = QString("select sFilePath from RecordFileStatus where (nLock=1 or nDamage=1)");
// 		if (exceCommand(query, cmd))
// 		{
// 			while (query.next())
// 			{
// 				excludeFilelst<<query.value(0).toString();
// 			}
// 		}
// 		query.exec(sqlCommand);
// 		exceCommand(query, sqlCommand);
// 		
// 		while (query.next())
// 		{
// 			uint wndId = query.value(0).toUInt();
// 			uint start = query.value(1).toUInt();
// 			uint end = query.value(2).toUInt();
// 			QString path = query.value(3).toString();
// 			//exclude file from file list
// 			if (excludeFilelst.contains(path))
// 			{
// 				continue;
// 			}
// 			appendTimePath(tpList, start, path, insertPos);
// 
// 			//file isn't in list or one file has two period time
// // 			if (!fileList.contains(path) || end - lastEnd > 60)
// // 			{
// // // 				fileList<<path;
// // 				appendFile(fileList, path, vecTime, end);
// // 			}
// // 			if(!find && end > m_uiStartSec)
// // 			{
// // 				startPath = path;
// // 				find = true;
// // 			}
// // 			if (!find && ((lastEnd < m_uiStartSec && m_uiStartSec < start) || (start <= m_uiStartSec && m_uiStartSec < end)))
// // 			{
// // 				startPath = path;
// // 				find = true;
// // 			}
// // 			lastEnd = end;
// 			PeriodTime item = {start, end};
// // 			filePeriodMap[wndId].append(item);
// 			appendPeriodTime(filePeriodMap[wndId], item);
// 		}
// 		query.finish();
	}

	//find start position
// 	for (qint32 i32loop = 0; i32loop < vecTime.size(); ++i32loop)
// 	{
// 		if (m_uiStartSec < vecTime[i32loop])
// 		{
// 			i32Pos = i32loop;
// 			break;
// 		}
// 	}
	//get file list
	foreach (TimePath tp, tpList)
	{
		if (!fileList.contains(tp.path))
		{
			fileList.push_back(tp.path);
		}
	}

	return fileList;
}

PlayMgr* LocalPlayerEx::getPlayMgrPointer( QWidget* pwnd )
{
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		PlayMgr *pplayMgr = m_arrPlayInfo[i32Loop].pPlayMgr->getPlayMgrPointer(pwnd);
		if (pplayMgr)
		{
			return pplayMgr;
		}
	}
	return NULL;
}

void LocalPlayerEx::setPlayTime(uint &playtime)
{
	m_uiPlayTime = playtime;
}

void LocalPlayerEx::onStartPlayMgr( uint wndId )
{
	PlayInfo* pplayInfo = m_arrPlayInfo;
	while (NO_WINDOW_ID != pplayInfo->i32WndId && pplayInfo - m_arrPlayInfo < MAX_PLAY_THREAD)
	{
		if (wndId == pplayInfo->i32WndId)
		{
			pplayInfo->pPlayMgr->startPlay();
		}
		++pplayInfo;
	}
}

void LocalPlayerEx::onStopPlayMgr()
{
	GroupStop();
}

qint32 LocalPlayerEx::countSkipTime( const QMap<uint, QVector<PeriodTime> >& filePeriodMap, QVector<PeriodTime> &totalSkipTime )
{
	if (filePeriodMap.isEmpty())
	{
		return 1;
	}

	//for test
// 	QMap<uint, QVector<PeriodTime> >::const_iterator it = filePeriodMap.constBegin();
// 	while (it != filePeriodMap.constEnd())
// 	{
// 		qDebug()<<"=================="<<it.key()<<"=====================";
// 		for (qint32 index = 0; index < it->size(); ++index)
// 		{
// 			qDebug()<<QDateTime::fromTime_t(it->at(index).start).toString("hh:mm:ss")<<"-----"<<QDateTime::fromTime_t(it->at(index).end).toString("hh:mm:ss");
// 		}
// 		++it;
// 	}


	QVector<PeriodTime> skipTime;
	QMap<uint, QVector<PeriodTime> >::iterator iter = filePeriodMap.begin();
	while (iter != filePeriodMap.end())
	{
		for (int i = 0; i < iter->size(); ++i)
		{
			if (skipTime.isEmpty())
			{
				skipTime.append(iter->at(i));
				continue;
			}
			int j = 0;
			while(j < skipTime.size())
			{ 
				if (iter->at(i).start == skipTime[j].start && iter->at(i).end == skipTime[j].end)
				{
					break;
				}
				if (iter->at(i).end < skipTime[j].start)
				{
					if ((j >= 1  && iter->at(i).start > skipTime[j - 1].end))
					{
						skipTime.insert(j - 1, iter->at(i));
						break;
					}
					else if (0 == j)
					{
						skipTime.prepend(iter->at(i));
					}
				}
				if (iter->at(i).start > skipTime[j].end)
				{
					if ((j + 1 < skipTime.size() && iter->at(i).end < skipTime[j + 1].start) || (j + 1 == skipTime.size()))
					{
						skipTime.insert(j + 1, iter->at(i));
						break;
					}
				}
				if ((iter->at(i).start < skipTime[j].start && iter->at(i).end >= skipTime[j].start)|| (iter->at(i).end > skipTime[j].end) && (iter->at(i).start <= skipTime[j].end))
				{
					skipTime[j].start = qMin(skipTime[j].start, iter->at(i).start);
					skipTime[j].end = qMax(skipTime[j].end, iter->at(i).end);
					break;
				}
				++j;
			}
		}
		++iter;
	}

	PeriodTime perTime;
	for (int i = 0 ; i <= skipTime.size(); ++i)
	{
		if (0 == i && skipTime[i].start >= m_uiStartSec)
		{
			perTime.start = m_uiStartSec;
			perTime.end = skipTime[i].start;
		}
		else if (i == skipTime.size() && skipTime[i - 1].end != m_uiEndSec)
		{
			perTime.start = skipTime[i - 1].end;
			perTime.end = m_uiEndSec;
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
			totalSkipTime.append(perTime);
			perTime.start = 0;
			perTime.end = 0;
		}
	}

// 	for (qint32 i = 0; i < totalSkipTime.size(); ++i)
// 	{
// 		qDebug()<<QDateTime::fromTime_t(totalSkipTime[i].start).toString("hh:mm:ss")<<"<------------>"<<QDateTime::fromTime_t(totalSkipTime[i].end).toString("hh:mm:ss");
// 	}


	return 0;

}

// void LocalPlayerEx::appendFile( QList<QString> &fileList, QString fileName, QVector<uint> &vecTime, uint time )
// {
// 	if (fileList.isEmpty())
// 	{
// 		fileList<<fileName;
// 		vecTime.append(time);
// 		return;
// 	}
// 	qint32 pos = fileList.size() - 1;
// 	if (vecTime[pos] < time)
// 	{
// 		fileList<<fileName;
// 		vecTime.append(time);
// 	}
// 	else
// 	{
// 		while (vecTime[pos] > time)
// 		{
// 			--pos;
// 			if (pos < 0)
// 			{
// 				return;
// 			}
// 		}
// 		fileList.insert(pos + 1, fileName);
// 		vecTime.insert(pos + 1, time);
// 	}
// }

void LocalPlayerEx::appendPeriodTime( QVector<PeriodTime> &vecPeriod, const PeriodTime &per )
{
	if (vecPeriod.isEmpty())
	{
		vecPeriod.append(per);
		return;
	}
	qint32 pos = vecPeriod.size() - 1;
	if (vecPeriod[pos].start < per.start)
	{
		vecPeriod.append(per);
	}
	else
	{
		while (vecPeriod[pos].start > per.start)
		{
			--pos;
			if (pos < 0)
			{
				vecPeriod.insert(pos + 1, per);
				return;
			}
		}
		vecPeriod.insert(pos + 1, per);
	}
}

void LocalPlayerEx::appendTimePath( QList<TimePath> &tpList, const uint &start, const QString &path, qint32 &insertPos )
{
	TimePath tp = {start, path};
	qint32 end = tpList.size() - 1;
	if (tpList.isEmpty() || (insertPos == end && tpList.at(insertPos).start < start) || (insertPos < end && tpList.at(insertPos).start < start && tpList.at(insertPos + 1).start > start))
	{
		tpList.insert(++insertPos, tp);
	}
	else
	{
		qint32 pos = tpList.size() - 1;
		while (tpList.at(pos).start > start)
		{
			--pos;
			if (!pos)
			{
				tpList.insert(pos, tp);
				insertPos = pos;
				return;
			}
		}
		tpList.insert(pos + 1, tp);
		insertPos = pos + 1;
	}
}

bool LocalPlayerEx::exceCommand( QSqlQuery &queue, const QString &cmd )
{
	int nCount=0;
	while(true){
		if (nCount > 5)
		{
			return false;
		}
		if (queue.exec(cmd))
		{
			if (nCount)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"try=====as lock======success";
			}
			return true;
		}
		else
		{
			if ("database is locked"==queue.lastError().databaseText())
			{
				QEventLoop tEventLoop;
				QTimer::singleShot(1,&tEventLoop,SLOT(quit()));
				tEventLoop.exec();
				nCount++;
				qDebug()<<__FUNCTION__<<__LINE__<<"try:"<<nCount<<"as lock";
			}
			else
			{
				return false;
			}
		}
	}
}

void LocalPlayerEx::enableWindowStretch( QWidget * window,bool bEnable )
{
	int i;
	for (i = 0; i < MAX_PLAY_THREAD; i ++)
	{
		PlayMgr * pAct = m_arrPlayInfo[i].pPlayMgr->getPlayMgrPointer(window);
		if (pAct)
		{
			pAct->enableVideoStretch(bEnable);
			break;
		}
	}
}

bool LocalPlayerEx::getWindowStretchStatus( QWidget * window )
{
	int i;
	for (i = 0; i < MAX_PLAY_THREAD; i ++)
	{
		PlayMgr * pAct = m_arrPlayInfo[i].pPlayMgr->getPlayMgrPointer(window);
		if (pAct)
		{
			return pAct->getVideoStretchStatus();
		}
	}
	return true;
}

int LocalPlayerEx::execCommand( sqlite3 *pdb, const char* cmd, char*** pppRet, int* row, int* col, char** pMsg )
{
	int ret = -1;
	int count = 0;
	while ((ret = sqlite3_get_table(pdb, cmd, pppRet, row, col, pMsg)) != SQLITE_OK && count < 50)
	{
		//wait 3 milliseconds
		QEventLoop evLoop;
		QTimer::singleShot(3, &evLoop, SLOT(quit()));
		evLoop.exec();
		++count;
	}
	return ret;
}

void cbTimeChange(QString evName, uint playTime, void* pUser)
{
	if (pUser && "playingTime" == evName)
	{
		((LocalPlayerEx*)pUser)->setPlayTime(playTime);
	}
}
