#include "LocalPlayerEx.h"
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <guid.h>
#include "IDisksSetting.h"



LocalPlayerEx::LocalPlayerEx()
	:m_nRef(0),
	m_i32GroupNum(0),
	m_i32Types(0),
	m_uiStartSec(0),
	m_uiEndSec(0),
	m_uiPlayTime(0),
	m_uiLastPlayTime(0),
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

	//create thread for playing
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		PlayMgr *pplayMgr = new PlayMgr();
		m_arrPlayInfo[i32Loop].pPlayMgr = pplayMgr;
		m_arrPlayInfo[i32Loop].i32WndId = NO_WINDOW_ID;
		connect(m_pFileData, SIGNAL(sigSkipTime(uint)), pplayMgr, SLOT(onSkipTime(uint)));
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
		m_pFileData->deleteLater();
	}
	//clear play thread
	for (qint32 i32Loop = 0; i32Loop < MAX_PLAY_THREAD; ++i32Loop)
	{
		if (m_arrPlayInfo[i32Loop].pPlayMgr->isRunning())
		{
			m_arrPlayInfo[i32Loop].pPlayMgr->stop();
		}
		m_arrPlayInfo[i32Loop].pPlayMgr->deleteLater();
	}
	deInitDataBase();
}

QSqlDatabase * LocalPlayerEx::initDataBase( QString sDatabaseName )
{
	if (!m_dbMap.contains(sDatabaseName))
	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", sDatabaseName);
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

void LocalPlayerEx::deInitDataBase()
{
	QMap<QString, QSqlDatabase*>::iterator iter = m_dbMap.begin();
	while (iter != m_dbMap.end())
	{
		(*iter)->close();
		delete *iter;
		QSqlDatabase::removeDatabase(iter.key());
		++iter;
	}
	m_dbMap.clear();
}

long __stdcall LocalPlayerEx::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_ILocalRecordSearchEx == iid)
	{
		*ppv = static_cast<ILocalRecordSearchEx *>(this);
	}
	else if (IID_ILocalPlayer == iid)
	{
		*ppv = static_cast<ILocalPlayer *>(this);
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

int LocalPlayerEx::searchVideoFileEx( const QString &sDevName, const QString& sDate, const int& nTypes )
{
	//this interface is outdated
	return 0;
}

QString LocalPlayerEx::getTypeList( int nTypes )
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

	typeList = "nRecordType=" + typeList.replace("*", " or nRecordType=");
	return typeList;
}

int LocalPlayerEx::searchVideoFileEx( const int & nWndId, const QString & sDate, const QString & sStartTime, const QString & sEndTime, const int & nTypes )
{
	//get start seconds and end seconds
	QString sStart = sDate + " " + sStartTime;
	QString sEnd = sDate + " " + sEndTime;
	QDateTime dtStart = QDateTime::fromString(sStart, "yyyy-MM-dd hh:mm:ss");
	QDateTime dtEnd = QDateTime::fromString(sEnd, "yyyy-MM-dd hh:mm:ss");
	QString sqlCmd = QString("select nRecordType, nStartTime, nEndTime from search_record where nWndId='%1' and nEndTime>='%2' and nStartTime<='%3' and (%4) order by nStartTime").arg(nWndId).arg(dtStart.toTime_t()).arg(dtEnd.toTime_t()).arg(getTypeList(nTypes));
	QStringList diskList = m_disklst.split(":", QString::SkipEmptyParts);
	foreach(QString disk, diskList)
	{
		QString sDbName = disk + ":/recEx/record.db";
		if (!QFile::exists(sDbName))
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"file "<<sDbName<<" didn't exist";
			continue;
		}
		QSqlDatabase *pdb = initDataBase(sDbName);
		if (!pdb)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"get database point error!";
			continue;
		}
		QSqlQuery query(*pdb);
		query.exec(sqlCmd);

		while (query.next())
		{
			int recordType = query.value(0).toInt();
			uint start = query.value(1).toUInt();
			uint end = query.value(2).toUInt();

			QVariantMap info;
			info.insert("wndId", nWndId);
			info.insert("type", recordType);
			info.insert("start", QDateTime::fromTime_t(start).toString("yyyy-MM-dd hh:mm:ss"));
			info.insert("end", QDateTime::fromTime_t(end).toString("yyyy-MM-dd hh:mm:ss"));

			eventProcCall(QString("GetRecordFileEx"), info);
		}
		query.finish();
	}
	return ILocalRecordSearchEx::OK;
}

int LocalPlayerEx::AddFileIntoPlayGroup( QStringList const filelist,QWidget *wnd,const QDateTime &start,const QDateTime &end )
{
	//this interface is outdated
	return 0;
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
	qint32 i32BufferNum = 0;
	qint32 i32StartPos = 0;
	//get file list for playing
	QStringList fileList = getFileList(i32StartPos);
	//set parameter into thread for reading 
	m_pFileData->setParamer(fileList, m_uiStartSec, m_uiEndSec, i32StartPos);
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
	m_uiStartSec = 0;
	m_uiEndSec = 0;
	m_uiPlayTime = 0;
	m_uiLastPlayTime = 0;
	return 0;
}

int LocalPlayerEx::GroupSpeedFast( int speed )
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
/*	return QDateTime::fromTime_t(m_uiPlayTime);*/

	QDateTime datetime(QDate::currentDate(), QTime(0, 0, 0));
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

QStringList LocalPlayerEx::getFileList( qint32 &i32Pos )
{
	bool find = false;
	QString startPath;
	QStringList fileList;
	QList<qint32> wndList;
	getWndIdList(wndList);
	QString sqlCommand = QString("select nEndTime, sFilePath from record where nWndId in(%1) and (%2) and nStartTime!=nEndTime order by nStartTime").arg(intToStr(wndList)).arg(getTypeList(m_i32Types));
	QStringList diskList = m_disklst.split(":", QString::SkipEmptyParts);
	foreach(QString disk, diskList)
	{
		QString sDbName = disk + ":/recEx/record.db";
		QSqlDatabase *pdb = initDataBase(sDbName);
		if (!pdb)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"get database point error!";
			continue;
		}
		QSqlQuery query(*pdb);
		query.exec(sqlCommand);
		
		while (query.next())
		{
			QString path = query.value(1).toString();
			if (!fileList.contains(path))
			{
				fileList<<path;
			}
			if(!find && query.value(0).toUInt() > m_uiStartSec)
			{
				startPath = path;
				find = true;
			}
		}
		query.finish();
	}
	i32Pos = fileList.indexOf(startPath);

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

void LocalPlayerEx::setBaseTime( uint &baseTime )
{
	m_uiPlayTime += baseTime;
}

void LocalPlayerEx::setPlayTime(uint &playtime)
{
	if (m_uiLastPlayTime < playtime)
	{
		m_uiPlayTime += playtime - m_uiLastPlayTime;
	}
	m_uiLastPlayTime = playtime;
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

void cbTimeChange(QString evName, uint playTime, void* pUser)
{
	if (pUser && "skipTime" == evName)
	{
		((LocalPlayerEx*)pUser)->setBaseTime(playTime);
	}
	if (pUser && "playingTime" == evName)
	{
		((LocalPlayerEx*)pUser)->setPlayTime(playTime);
	}
}
