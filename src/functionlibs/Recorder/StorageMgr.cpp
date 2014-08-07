
#include "StorageMgr.h"
#include "guid.h"
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include "netlib.h"
#pragma comment(lib,"netlib.lib")

#include <QMultiMap>
#include <stdio.h>
#define qDebug() qDebug()<<"this:"<<(int)this

typedef struct __tagDataBaseInfo{
	QString sDatabaseName;
	QSqlDatabase *pDatabase;
	int nCount;
	QList<int *> tThis;
}tagDataBaseInfo;
QMultiMap<QString ,tagDataBaseInfo> g_tDataBase;
QString g_sSearchRecord="C:/CMS_RECORD/search_record.db";
QSqlDatabase * initDataBase(QString sDatabaseName,int *nThis){
	//检测sDatabaseName 是否存在
	if (g_tDataBase.contains(sDatabaseName))
	{
		if (g_tDataBase.find(sDatabaseName).value().tThis.contains(nThis))
		{
			//do noting
		}else{
			g_tDataBase.find(sDatabaseName).value().nCount++;
			g_tDataBase.find(sDatabaseName).value().tThis.append(nThis);
		}
		return g_tDataBase.find(sDatabaseName).value().pDatabase;
	}else{
		tagDataBaseInfo tDataBaseInfo;
		tDataBaseInfo.sDatabaseName=sDatabaseName;
		tDataBaseInfo.nCount=1;
		tDataBaseInfo.tThis.append(nThis);
		
		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", sDatabaseName);
		tDataBaseInfo.pDatabase=new QSqlDatabase(db);
		tDataBaseInfo.pDatabase->setDatabaseName(sDatabaseName);
		if (tDataBaseInfo.pDatabase->open())
		{
			//do nothing
		}else{
			printf("open database fail,in initDataBase function/n");
			return NULL;
		}
		g_tDataBase.insert(sDatabaseName,tDataBaseInfo);
		return tDataBaseInfo.pDatabase;
	}
}
void deInitDataBase(int * nThis){
	QMultiMap<QString,tagDataBaseInfo>::iterator it;
	QStringList sDeleteList;
	for (it=g_tDataBase.begin();it!=g_tDataBase.end();it++)
	{
		if (it.value().tThis.contains(nThis))
		{
			//
			it.value().nCount--;
			if (it.value().nCount==0)
			{
				it.value().pDatabase->close();
				delete it.value().pDatabase;
				it.value().pDatabase=NULL;
				sDeleteList.append(it.value().sDatabaseName);
				QSqlDatabase::removeDatabase(it.value().sDatabaseName);
			}else{
				//do nothing
				it.value().tThis.removeOne(nThis);
			}
		}else{
			//keep going
		}
	}
	for(int i=0;i<sDeleteList.size();i++){
		g_tDataBase.remove(sDeleteList.at(i));
	}
}
//QMutex StorageMgr::m_schRecLock;
QMutex StorageMgr::m_sLock;
QMutex StorageMgr::m_dblock;
QList<int > StorageMgr::m_insertIdList;
StorageMgr::StorageMgr(void):
	m_insertId(-1),
	m_searchRecordId(-1),
	m_nPosition(0),
	m_pDisksSetting(NULL)
	//m_db(NULL)
/*	m_currdisk('0')*/
{
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IDiskSetting,(void **)&m_pDisksSetting);
	if (!m_pDisksSetting)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"create m_pDisksSetting fail,please check";
	}
	/* 使用只打开一次数据库的方法
	QDateTime curTime = QDateTime::currentDateTime();
	m_connectId = QString::number(curTime.toTime_t()) + QString::number((int)this);
	
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectId);
	
	m_dblock.lock();
	m_db = new QSqlDatabase(db);
	if (NULL == m_db)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"init database failed!";
	}
	m_dblock.unlock()
	QDateTime curTime = QDateTime::currentDateTime();
	m_connectSearchId = QString::number((int)this) + QString::number(curTime.toTime_t());
	QSqlDatabase dbSch = QSqlDatabase::addDatabase("QSQLITE", m_connectSearchId);
	m_schRecLock.lock();
	m_dbSearch = new QSqlDatabase(dbSch);
	if (NULL == m_dbSearch)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"init search database failed!";
	}
	m_schRecLock.unlock();;*/
}


StorageMgr::~StorageMgr(void)
{
	if (m_pDisksSetting)
	{
		m_pDisksSetting->Release();
	}
	m_nPosition=__LINE__;
	/* 使用只打开一次数据库的方法
	m_dblock.lock();
	m_db->close();
	delete m_db;
	m_db = NULL;
	QSqlDatabase::removeDatabase(m_connectId);
	m_dblock.unlock();
	
	m_nPosition=__LINE__;
	m_schRecLock.lock();
	m_dbSearch->close();
	delete m_dbSearch;
	m_dbSearch = NULL;
	QSqlDatabase::removeDatabase(m_connectSearchId);
	
	m_schRecLock.unlock();
	*/
	m_dblock.lock();
	deInitDataBase((int *)this);
	m_dblock.unlock();
	
}

int StorageMgr::getFilePackageSize()
{
	int filesize = 128;
	m_nPosition=__LINE__;
	if (m_pDisksSetting)
	{
		m_pDisksSetting->getFilePackageSize(filesize);
	}

	return filesize;
}
QString StorageMgr::getUseDisks()
{
	QString sDisks;
	m_nPosition=__LINE__;
	if (m_pDisksSetting)
	{
		m_pDisksSetting->getUseDisks(sDisks);
	}
	return sDisks;
}
bool StorageMgr::getLoopRecording()
{
	bool loop = false;
	m_nPosition=__LINE__;
	if (m_pDisksSetting)
	{
		loop = getLoopRecording();
	}
	return loop;
}
int StorageMgr::getFreeSizeForDisk()
{
	int spacereservedsize=4096;
	m_nPosition=__LINE__;
	if (m_pDisksSetting)
	{
		m_pDisksSetting->getDiskSpaceReservedSize(spacereservedsize);
	}
	return spacereservedsize;
}
QString StorageMgr::getFileSavePath( QString devname,int nChannelNum,int winId, int type, QTime &start )
{
	QString sFileSavePath="none";
	m_insertId=-1;
	m_nPosition=__LINE__;
	m_sLock.lock();
	QString sDisk=getUsableDisk();
	m_sLock.unlock();
	if ("0"!=sDisk)
	{
		sFileSavePath=sDisk+":/REC";
		m_nPosition=__LINE__;
		m_dblock.lock();
		//if database is not open, open it
		QSqlDatabase *pDataBase=NULL;
		pDataBase=initDataBase(sFileSavePath+"/record.db",(int *)this);
		if (pDataBase==NULL)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"getFileSavePath fail as open db fail";
			m_dblock.unlock();
			return "none";
		}else{
			//keep going
		}
		/* 使用只打开一次数据库的方法
		if (!m_db->isOpen())
		{
			m_db->setDatabaseName(sFileSavePath+"/record.db");
			if (m_db->open())
			{
				//keep going
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"getFileSavePath fail as open db fail";
				m_dblock.unlock();
				return "none";
			}
		}
		QSqlQuery _query(*m_db);
		*/
		//find the newest record in database and create path base on it
		QSqlQuery _query(*pDataBase);
		QString command = QString("select path, id, start_time, end_time, date from local_record where win_id=%1 order by id desc limit 1").arg(winId);
		_query.exec(command);

		if (_query.next())//has find it
		{
			QString path = _query.value(0).toString();
			int id = _query.value(1).toInt();
			QString start_time = _query.value(2).toString();
			QString end_time = _query.value(3).toString();
			QString date = _query.value(4).toString();

			if (start_time == end_time)//end_time is not updated
			{
				QFile::remove(path);
				deductPeriod(winId, date, start_time);
				_query.exec(QString("delete from local_record where id=%1").arg(id));
				sFileSavePath = path;
			}
			else
			{
				int pos = path.lastIndexOf("/");
				int fileNum = path.mid(pos + 1, 3).toInt();
				int dirNum = path.mid(pos - 3, 3).toInt();
				fileNum++;
				if (fileNum > 127)//Ensure that each directory only 128 files
				{
					dirNum++;
					fileNum = 0;
				}
				QString temp;
				temp = temp.sprintf("/WND%02d/%04d/%03d.avi", winId, dirNum, fileNum);
				sFileSavePath += temp;
			}
		}
		else
		{
			//use default path
			QString temp;
			sFileSavePath += temp.sprintf("/WND%02d/0000/000.avi",winId); 
		}

		//insert new data into database
		_query.prepare("insert into local_record(dev_name,dev_chl,win_id,date,start_time,end_time,record_type,path) values(:dev_name,:dev_chl,:win_id,:date,:start_time,:end_time,:record_type,:path)");
		_query.bindValue(":dev_name",devname);
		_query.bindValue(":dev_chl",nChannelNum + 1);//ensure chl num start with 1
		_query.bindValue(":win_id",winId);
		_query.bindValue(":date",QDate::currentDate().toString("yyyy-MM-dd"));
		start = QTime::currentTime();
		_query.bindValue(":start_time",start.toString("hh:mm:ss"));
		_query.bindValue(":end_time", start.toString("hh:mm:ss"));
		_query.bindValue(":record_type", type);
		_query.bindValue(":path", sFileSavePath);
		_query.exec();

		//save the id inserted just now
		command = QString("select max(id) from local_record");
		_query.exec(command);
		if (_query.next())
		{
			m_insertId = _query.value(0).toInt();
			m_insertIdList.append(m_insertId);
		}

		_query.finish();
		m_dblock.unlock();
		return sFileSavePath;
	}else{
		return sFileSavePath;
	}
}

QString StorageMgr::getUsableDisk()
{
	QString sDisks;
	QString sGottenDisk="0";
	int nTryCount=3;
	int nFreeSize;
	bool bRecover;
	if (m_pDisksSetting!=NULL)
	{
		m_nPosition=__LINE__;
		bRecover=m_pDisksSetting->getLoopRecording();
		m_nPosition=__LINE__;
		if (0!=m_pDisksSetting->getDiskSpaceReservedSize(nFreeSize))
		{
			nFreeSize=124;
		}else{
			//do nothing
		}
		m_nPosition=__LINE__;
		if (0==m_pDisksSetting->getUseDisks(sDisks))
		{
			QStringList sDiskList = sDisks.split(":");
			if (sDiskList.size()!=0)
			{
				m_nPosition=__LINE__;
				while(nTryCount>0){
					foreach(QString stritem,sDiskList){
						quint64 FreeByteAvailable = 0;
						quint64 TotalNumberOfBytes = 0;
						quint64 TotalNumberOfFreeBytes = 0;
						QString sdisk = stritem+":";
						if (GetDiskFreeSpaceExQ(sdisk.toAscii().data(),&FreeByteAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes))
						{
							if (TotalNumberOfFreeBytes/1024/1024 > (quint64)nFreeSize){
								sGottenDisk = stritem;
								/*使用只打开一次数据库的方法
								if (!m_curDisk.isEmpty() && m_curDisk != sGottenDisk && m_db->isOpen())
								{
									m_db->close();
								}*/
								QString dbpath = sdisk + "/REC";
								QDir dDir;
								m_nPosition=__LINE__;
								if (dDir.exists(dbpath))
								{
									//do nothing
								}else{
									//create dir ::rec
									dDir.mkpath(dbpath);
								}
								dbpath+="/record.db";
								if (!QFile::exists(dbpath))
								{
									if (createTable(dbpath))
									{
										//keep going
									}else{
										qDebug()<<__FUNCTION__<<__LINE__<<"create table error in "<<dbpath;
									}
								}else{
									if (1 == checkTableExist(dbpath, QString("local_record")))
									{
										createTable(dbpath);
									}
								}
								/*使用只打开一次数据库的方法
								if (!QFile::exists(dbpath) && !m_db->isOpen())
								{
									m_db->setDatabaseName(dbpath);
									if (!m_db->open())
									{
										qDebug()<<__FUNCTION__<<__LINE__<<"open data base fail,please check";
									}else{

									}
									createTable();
								}else{
									//do nothing
								}*/
								m_curDisk = sGottenDisk;
								break;
							}else{
								// there is not enough space in this disk,please find anther or free disk
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<sdisk<<"can not been gotten message,it may be a system disk ";
						}
					}
					if (bRecover && "0" == sGottenDisk)
					{
						m_nPosition=__LINE__;
						if (deleteOldDir(sDiskList))
						{
							//keep going
						}else{
							//there is no need to continue
							nTryCount=0;
						}
					}else{
						//keep going
					}
					if (bRecover&&sGottenDisk=="0")
					{
						nTryCount--;
					}else{
						//there is no need to continue
						nTryCount=0;
					}
					
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"can not get usableDisk ,there may be exist a error in database";
			}
		}else{
			//
			qDebug()<<__FUNCTION__<<__LINE__<<"can not get usableDisk,please select at least one disk as record disk in device setting page";
		}
	}else{
		//
		qDebug()<<__FUNCTION__<<__LINE__<<"can not get usableDisk as m_pDisksSetting is null";
	}
	return sGottenDisk;
}


bool StorageMgr::deleteOldDir( const QStringList& diskslist )
{
	QDate earlestDate;
	QMap<QDate, RecInfo> result;
	quint64 FreeByteAvailable = 0;
	quint64 TotalNumberOfBytes = 0;
	quint64 TotalNumberOfFreeBytes = 0;
	int freesizem = 0;
	m_nPosition=__LINE__;
	if(0 != m_pDisksSetting->getDiskSpaceReservedSize(freesizem))
		freesizem = 4096;
	int nStep=0;
	bool nIsStop=false;
	bool bRet=false;
	m_nPosition=__LINE__;
	while(!nIsStop){
		switch(nStep){
		case 0:{
			//检测是否需要删除
			foreach(QString disk,diskslist){
				QString sdisk = disk + ":";
				GetDiskFreeSpaceExQ(sdisk.toAscii().data(),&FreeByteAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);
				if (TotalNumberOfFreeBytes/1024/1024 > (quint64)freesizem)
				{
					//有足够的空间
					bRet=true;
					nIsStop=true;
					break;
				}else{
					//keep going
					nStep=1;
				}
			}
			   }
			   break;
		case 1:{
			//查找最早的记录
			result.empty();
			foreach(QString disk, diskslist)
			{
				QString path = disk + ":/REC/record.db";
				QDate date;
				QMap<int, QString> maxEndTimeMap;
				QStringList list = findEarlestRecord(path, date, maxEndTimeMap);
				if (list.isEmpty())
				{
					continue;//open database failed or no record in database
				}
				RecInfo re;
				re.dbPath = path;
				re.fileLsit = list;
				re.maxEndTimeMap = maxEndTimeMap;
				result.insertMulti(date, re);
			}
			if (result.isEmpty())
			{
				//没有录像可以删除了
				qDebug()<<__FUNCTION__<<__LINE__<<"there is no record for free,please remove anther file from disk";
				bRet=false;
				nIsStop=true;
			}else{
				earlestDate = minDate(result.keys());
				nStep=2;
			}
			   }
			   break;
		case 2:{
			//删除
			QList<RecInfo> recInfo = result.values(earlestDate);
			for(int i = recInfo.size() - 1; i >= 0; i--)
			{
				RecInfo each = recInfo.at(i);
				//delete file from directory
				m_nPosition=__LINE__;
				QStringList hasDelete = deleteFile(each.fileLsit);
				qDebug()<<__LINE__<<"earlydate:"<<earlestDate.toString("yyyy-MM-dd")<<"files have deleted:"<<hasDelete;
				//deduct period from each window id in search_record table
				m_nPosition=__LINE__;
				deductPeriod(each.dbPath, each.maxEndTimeMap, earlestDate.toString("yyyy-MM-dd"));
				//delete record from database
				m_nPosition=__LINE__;
				deleteRecord(each.dbPath, earlestDate.toString("yyyy-MM-dd"), each.maxEndTimeMap);
			}
			nStep=0;
			   }
			   break;
		case 3:{
			//结束
			   }
			   break;
		}
	}
	return bRet;
}


QDate StorageMgr::minDate(QList<QDate> dateList)
{
	QDate minDate = dateList.at(0);
	for (int i = 1; i < dateList.size(); ++i)
	{
		minDate = qMin(dateList.at(i), minDate);
	}
	return minDate;
}

QStringList StorageMgr::deleteFile(const QStringList& fileList)
{
	QStringList hasDeleted;
	if (!fileList.isEmpty())
	{
		m_nPosition=__LINE__;
		foreach(QString file, fileList)
		{
			if (QFile::remove(file))
			{
				hasDeleted<<file;
				QString dirpath = file.left(file.lastIndexOf("/"));
				QDir dir(dirpath);
				dir.setFilter(QDir::Files);
				if (!dir.count())
				{
					dir.rmpath(dirpath);
				}
			}
			else
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"delete file:"<<file<<"error!";
			}
		}
	}
	return hasDeleted;
}

bool StorageMgr::GetDiskFreeSpace(char* lpDirectoryName, quint64* lpFreeBytesAvailableToCaller, quint64* lpTotalNumberOfBytes, quint64* lpTotalNumberOfFreeBytes)
{
	return GetDiskFreeSpaceExQ(lpDirectoryName,lpFreeBytesAvailableToCaller,lpTotalNumberOfBytes,lpTotalNumberOfFreeBytes);
}

bool StorageMgr::freeDisk()
{
	QString sDisks;
	if (m_pDisksSetting!=NULL)
	{
		if (0==m_pDisksSetting->getUseDisks(sDisks))
		{
			QStringList sDiskList=sDisks.split(":");
			if (sDiskList.size()>0)
			{
				m_nPosition=__LINE__;
				m_sLock.lock();
				if (deleteOldDir(sDiskList))
				{
					m_sLock.unlock();
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"freeDisk fail as deleteOldDir fail";
				}
				m_sLock.unlock();
			}else{
				//
				qDebug()<<__FUNCTION__<<__LINE__<<"freeDisk fail ,please check database ,there may be some error in database";
			}
		}else{
			//
			qDebug()<<__FUNCTION__<<__LINE__<<"freeDisk fail as getUseDisks fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"freeDisk fail as m_pDisksSetting is null";
	}
	return false;
}

bool StorageMgr::deleteRecord(QString sFilePath)
{
	bool bRet=false;
	m_nPosition=__LINE__;
	m_dblock.lock();
	if (m_insertId!=-1)
	{
		/*使用只打开一次数据库的方法
		QSqlQuery _query(*m_db);*/
		QSqlDatabase *pDataBase=NULL;
		int nPos=sFilePath.indexOf("/REC/");
		int nEndPos=nPos+5;
		QString sFind=sFilePath.left(nEndPos)+"record.db";
		pDataBase=initDataBase(sFind,(int *)this);
		if (NULL!=pDataBase)
		{
			QSqlQuery _query(*pDataBase);
			QString command=QString("delete from local_record where id=%1").arg(m_insertId);
			if (_query.exec(command))
			{
				bRet =true;
				if (m_insertIdList.contains(m_insertId))
				{
					int npos=m_insertIdList.indexOf(m_insertId);
					m_insertIdList.removeAt(npos);
				}else{

				}
				m_insertId=-1;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"deleteRecord fail as exec the command fail,please check the command or the id";
				bRet=false;
			}
		}else{
			bRet= false;
			qDebug()<<__FUNCTION__<<__LINE__<<"deleteRecord fail::"<<sFilePath<<"as open database fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"deleteRecord fail as the id ==-1";
		bRet=false;
	}
	m_dblock.unlock();
	return bRet;
}
bool StorageMgr::updateRecord( QString sEnd, int size )
{
	//update file end time and file size
	m_nPosition=__LINE__;
	m_dblock.lock();
	/*使用只打开一次数据库的方法
	if (!m_db->isOpen() && !m_db->open())
	{
		if ("0" == m_curDisk)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"current disk is empty!";
			m_dblock.unlock();
			return false;
		}
		QString path = m_curDisk + "/REC/record.db";
		m_db->setDatabaseName(path);
		if (!m_db->open())
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"open database error again!";
			m_dblock.unlock();
			return false;
		}
	}*/
	if ("0"==m_curDisk)
	{
		m_dblock.unlock();
		qDebug()<<__FUNCTION__<<__LINE__<<"updateRecord fail as current disk is empty";
		return false;
	}else{
		QString sPath=m_curDisk+":/REC/record.db";
		QSqlDatabase *pDataBase=NULL;
		pDataBase=initDataBase(sPath,(int *)this);
		if (pDataBase==NULL)
		{
			m_dblock.unlock();
			qDebug()<<__FUNCTION__<<__LINE__<<"updateRecord fail as open data base fail";
			return false;
		}else{
			QSqlQuery _query(*pDataBase);
			QString command = QString("update local_record set end_time = '%1', file_size = %2 where id = %3").arg(sEnd).arg(QString::number(size)).arg(m_insertId);
			if (_query.exec(command))
			{
				m_dblock.unlock();
				if (m_insertIdList.contains(m_insertId))
				{
					int npos=m_insertIdList.indexOf(m_insertId);
					m_insertIdList.removeAt(npos);
				}else{

				}
				m_insertId = -1;
				return true;
			}else{
				m_dblock.unlock();
				qDebug()<<__FUNCTION__<<__LINE__<<"m_insertId::"<<m_insertId<<"sEnd::"<<sEnd<<"size::"<<size<<"updateRecord fail ,please check the database or the command";
				return false;
			}
		}
	}
	/*使用只打开一次数据库的方法
	QSqlQuery _query(*m_db);
	QString command = QString("update local_record set end_time = '%1', file_size = %2 where id = %3").arg(sEnd).arg(QString::number(size)).arg(m_insertId);
	if (_query.exec(command))
	{
		m_dblock.unlock();
		if (m_insertIdList.contains(m_insertId))
		{
			int npos=m_insertIdList.indexOf(m_insertId);
			m_insertIdList.removeAt(npos);
		}else{

		}
		m_insertId = -1;
		return true;
	}else{
		m_dblock.unlock();
		qDebug()<<__FUNCTION__<<__LINE__<<"m_insertId::"<<m_insertId<<"sEnd::"<<sEnd<<"size::"<<size<<"updateRecord fail ,please check the database or the command";
		return false;
	}*/
}

QStringList StorageMgr::findEarlestRecord( QString dbPath, QDate &earlestDate, QMap<int, QString> &maxEndTimeMap )
{
	QStringList pathList;
	pathList.empty();
	/*使用只打开一次数据库的方法
	m_db->setDatabaseName(dbPath);
	if (!m_db->open())
	{
		qDebug()<<"open " + dbPath + "failed!";
		return pathList;
	}
	QSqlQuery _query(*m_db);
	*/
	QSqlDatabase *pDataBase=NULL;
	m_dblock.lock();
	pDataBase=initDataBase(dbPath,(int *)this);
	if (pDataBase==NULL)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"findEarlestRecord fail as open data base fail";
		m_dblock.unlock();
		return pathList;
	}else{
		//keep going
	}
	QString earlyDate;
	QSqlQuery _query(*pDataBase);
	QString command = QString("select distinct date from local_record order by date limit 1");
	_query.exec(command);
	if (_query.next())
	{
		earlyDate = _query.value(0).toString();//get earliest record;
	}
	else
	{
		_query.finish();
		m_dblock.unlock();
		return pathList;
	}
	if (earlyDate != QDate::currentDate().toString("yyyy-MM-dd"))
	{
		command = QString("select date, path,id, win_id, end_time from local_record where date='%1' order by start_time").arg(earlyDate);
	}
	else
	{
		command = QString("select date, path, id, win_id, end_time from local_record where date='%1' and end_time<=strftime('%H:%M:%S','now','localtime','-1 hour')").arg(earlyDate);
	}
	_query.exec(command);

	while (_query.next())
	{
		earlestDate = _query.value(0).toDate();
		if (m_insertIdList.contains(_query.value(2).toInt()))
		{
		}else{
			pathList<<_query.value(1).toString();
		}
		int winId = _query.value(3).toInt();
		QString endTime = _query.value(4).toString();
		if (!maxEndTimeMap.contains(winId))
		{
			maxEndTimeMap.insert(winId, endTime);
		}
		else
		{
			QString temp = maxEndTimeMap.value(winId);
			if (temp < endTime)
			{
				maxEndTimeMap[winId] = endTime;
			}
		}
	}
	_query.finish();
	/*使用只打开一次数据库的方法
	m_db->close();*/
	m_dblock.unlock();
	return pathList;
}

void StorageMgr::deleteRecord( QString dbPath, QString date, QMap<int, QString> &maxEndTimeMap )
{
	/*使用只打开一次数据库的方法
	m_db->setDatabaseName(dbPath);
	if (!m_db->open())
	{
		qDebug()<<"open " + dbPath + "failed!";
	}
	QSqlQuery _query(*m_db);
	*/
	QSqlDatabase *pDataBase=NULL;
	m_dblock.lock();
	pDataBase=initDataBase(dbPath,(int *)this);
	if (NULL==pDataBase)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"deleteRecord ::"<<dbPath<<"fail as open data base fail";
		m_dblock.unlock();
		return;
	}else{
		//keep going
	}
	QSqlQuery _query(*pDataBase);
	QMap<int, QString>::iterator iter = maxEndTimeMap.begin();
	while(iter != maxEndTimeMap.end())
	{
		QString command = QString("delete from local_record where date = '%1' and win_id=%2 and end_time<='%3'").arg(date).arg(iter.key()).arg(iter.value());
		_query.exec(command);
		iter++;
	}
	_query.finish();
	/*使用只打开一次数据库的方法
	m_db->close();*/
	m_dblock.unlock();
	return;
}



bool StorageMgr::createTable(QString sPath)
{
	/*使用只打开一次数据库的方法
	QSqlQuery _query(*m_db);*/
	m_dblock.lock();
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initDataBase(sPath,(int *)this);
	QSqlQuery _query(*pDataBase);
	if (NULL!=pDataBase)
	{
		QString command = "create table local_record(id integer primary key autoincrement,";
		command += "dev_name chr(32),";
		command += "dev_chl integer,";
		command += "win_id integer,";
		command += "date char(32),";
		command += "start_time char(32),";
		command += "end_time char(32),";
		command += "record_type integer,";
		command += "file_size integer,";
		command += "path char(64))";
		_query.exec(command);
		m_dblock.unlock();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"createTable fail as open database fail";
		m_dblock.unlock();
		return false;
	}
}

int StorageMgr::getInsertId()
{
	return m_insertId;
}

bool StorageMgr::createSearchRecordTable()
{
	/*QSqlQuery _query(*m_dbSearch);*/
	m_dblock.lock();
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initDataBase(g_sSearchRecord,(int *)this);
	QSqlQuery _query(*pDataBase);
	if (NULL!=pDataBase)
	{
		QString command = "create table search_record(";
		command += "id integer primary key autoincrement,";
		command += "wnd_id integer,";
		command += "record_type integer,";
		command += "date char(32),";
		command += "start_time char(32),";
		command += "end_time char(32))";
		_query.exec(command);
		m_dblock.unlock();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"createSearchRecordTable fail";
		m_dblock.unlock();
		return false;
	}
	
}

bool StorageMgr::addSearchRecord( int wndId, int type, QString sDate, QString sStart, QString sEnd )
{
	if (wndId < 0 || sDate.isEmpty() || sStart.isEmpty() || sEnd.isEmpty())
	{
		return false;
	}
	m_nPosition=__LINE__;
	//m_schRecLock.lock();
	m_sLock.lock();
	if (!QFile::exists(g_sSearchRecord))
	{
		QDir dbpath;
		QString dir = g_sSearchRecord.left(g_sSearchRecord.lastIndexOf("/"));
		if (!dbpath.exists(dir))
		{
			dbpath.mkpath(dir);
			QFile file(g_sSearchRecord);
			file.open(QIODevice::WriteOnly);
			file.close();
		}else{
			//do nothing
		}
		if (createSearchRecordTable())
		{
			//keep going 
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"addSearchRecord fail as createSearchRecordTable fail";
			m_sLock.unlock();
			return false;
		}
	}else{
		//keep going 
	}
	m_sLock.unlock();
	/*使用只打开一次数据库的方法
	QString path = "C:/CMS_RECORD/search_record.db";

	if (!QFile::exists(path))
	{
		QDir dbpath;
		QString dir = path.left(path.lastIndexOf("/"));
		if (!dbpath.exists(dir))
		{
			dbpath.mkpath(dir);
			QFile file(path);
			file.open(QIODevice::WriteOnly);
			file.close();
		}
		m_dbSearch->setDatabaseName(path);
		if (!m_dbSearch->open())
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"open data base fail,please check";
			m_schRecLock.unlock();
			return false;
		}
		m_nPosition=__LINE__;
		createSearchRecordTable();
	}
	else if (!m_dbSearch->isOpen())
	{
		m_dbSearch->setDatabaseName(path);
		if (!m_dbSearch->open())
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"open data base fail,please check";
			m_schRecLock.unlock();
			return false;
		}
	}*/

	//insert data
	m_dblock.lock();
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initDataBase(g_sSearchRecord,(int *)this);
	if (NULL!=pDataBase)
	{
		//keep going
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"addSearchRecord fail as open data base fail ::"<<g_sSearchRecord;
		m_dblock.unlock();
		return false;
	}
	QSqlQuery _query (*pDataBase);
	//QSqlQuery _query(*m_dbSearch);
	QString command = QString("insert into search_record(wnd_id,record_type,date,start_time,end_time) values(%1,%2,'%3','%4','%5')").arg(wndId).arg(type).arg(sDate).arg(sStart).arg(sEnd);
	if (!_query.exec(command))
	{
		_query.finish();
		//m_dbSearch->close();
		//m_schRecLock.unlock();
		m_dblock.unlock();
		return false;
	}

	//save the id inserted just now
	command = QString("select max(id) from search_record");
	_query.exec(command);
	if (_query.next())
	{
		m_searchRecordId = _query.value(0).toInt();
	}

	_query.finish();
	//m_dbSearch->close();
	//m_schRecLock.unlock();
	m_dblock.unlock();
	return true;
}

bool StorageMgr::updateSearchRecord( QString sEnd )
{
	if (sEnd.isEmpty())
	{
		return false;
	}
	m_nPosition=__LINE__;
	/*使用只打开一次数据库的方法
	m_schRecLock.lock();
	
	if (!m_dbSearch->isOpen())
	{
// 		QString path = QCoreApplication::applicationDirPath() + "/search_record.db";
		QString path = "C:/CMS_RECORD/search_record.db";
		m_dbSearch->setDatabaseName(path);
		if (!m_dbSearch->open())
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"open data base fail,please check";
			m_schRecLock.unlock();
			return false;
		}
	}

	QSqlQuery _query(*m_dbSearch);*/
	m_dblock.lock();
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initDataBase(g_sSearchRecord,(int *)this);
	if (pDataBase!=NULL)
	{
		QSqlQuery _query(*pDataBase);
		QString command = QString("update search_record set end_time='%1' where id=%2").arg(sEnd).arg(m_searchRecordId);
		bool ret = _query.exec(command);

		_query.finish();
		m_dblock.unlock();
		return ret;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"updateSearchRecord fail as open database fail";
		m_dblock.unlock();
		return false;
	}
}

bool StorageMgr::deleteSearchRecord()
{
	m_nPosition=__LINE__;
	/*
	m_schRecLock.lock();
	if (!m_dbSearch->open())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"open data base fail,please check";
		m_schRecLock.unlock();
		return false;
	}
	QSqlQuery _query(*m_dbSearch);*/
	m_dblock.lock();
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initDataBase(g_sSearchRecord,(int *)this);
	if (NULL!=pDataBase)
	{
		QSqlQuery _query(*pDataBase);
		QString command = QString("delete from serach_record where id=%2").arg(m_searchRecordId);
		bool ret = _query.exec(command);

		_query.finish();
		m_dblock.unlock();
		return ret;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"deleteSearchRecord fail as open database fail";
		m_dblock.unlock();
		return false;
	}
}

void StorageMgr::deductPeriod( QString dbpath, QMap<int, QString> &maxEndTimeMap, QString date )
{
	m_nPosition=__LINE__;
	/*
	m_schRecLock.lock();
	if (!m_dbSearch->isOpen())
	{
		QString path = "C:/CMS_RECORD/search_record.db";
		m_dbSearch->setDatabaseName(path);
		if (!m_dbSearch->open())
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"open data base fail,please check";
			m_schRecLock.unlock();
			return;
		}
	}

	QSqlQuery _query(*m_dbSearch);
	*/
	m_dblock.lock();
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initDataBase(g_sSearchRecord,(int *)this);
	if (NULL!=pDataBase)
	{
		QSqlQuery _query(*pDataBase);
		QString cmd;
		QMap<int, QString>::iterator iter = maxEndTimeMap.begin();
		while(iter != maxEndTimeMap.end())
		{
			QString maxEndTime = iter.value();
			cmd = QString("delete from search_record where (wnd_id=%1 and (date='%2' and end_time<='%3')) or date<'%4'").arg(iter.key()).arg(date).arg(iter.value()).arg(date);
			_query.exec(cmd);

			qDebug()<<__FUNCTION__<<__LINE__<<cmd;

			cmd = QString("select id, start_time, end_time from search_record where date='%1' and wnd_id=%2 order by start_time limit 1").arg(date).arg(iter.key());
			_query.exec(cmd);
			if (_query.next())
			{
				int id = _query.value(0).toInt();
				QString start = _query.value(1).toString();
				QString end = _query.value(2).toString();
				if (maxEndTime > start && maxEndTime < end)
				{
					cmd = QString("update search_record set start_time='%1' where id=%2").arg(maxEndTime).arg(id);
					_query.exec(cmd);
				}
			}
			iter++;
		}
		_query.finish();
		m_dblock.unlock();
		return;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"deductPeriod fail as open database fail";
		m_dblock.unlock();
		return ;
	}
}

void StorageMgr::deductPeriod( int wndId, QString date, QString newEnd )
{
	QSqlDatabase *pDateBase = initDataBase(g_sSearchRecord, (int*)this);
	if (NULL != pDateBase)
	{
		QString startTime;
		int id = -1;
		QSqlQuery _query(*pDateBase);
		QString cmd = QString("select id, start_time, end_time from search_record where date='%1' and wnd_id=%2 order by start_time desc limit 1").arg(date).arg(wndId);
		_query.exec(cmd);
		if (_query.next())
		{
			id = _query.value(0).toInt();
			startTime = _query.value(1).toString();
		}
		if (startTime == newEnd)
		{
			cmd = QString("delete from search_record where id=%1").arg(id);
			_query.exec(cmd);
		}
		else
		{
			cmd = QString("update search_record set end_time='%1' where date='%2' and wnd_id=%3");
			_query.exec(cmd);
		}
		_query.finish();
	}
	else
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"deduct period failed when open search database!";
	}
}


//QString StorageMgr::getNewestRecord( QString devname, int chl )
//{
//	QString endTimeStr;
//	m_nPosition=__LINE__;
//	m_dblock.lock();
//	QSqlQuery _query(*m_db);
//	QString command = QString("select end_time from local_record where dev_name='%1' and dev_chl=%2 order by id desc limit 1").arg(devname).arg(chl + 1);
//	_query.exec(command);
//
//	if (_query.next())
//	{
//		endTimeStr = _query.value(0).toString();
//	}
//	_query.finish();
//	m_dblock.unlock();
//
//	return endTimeStr;
//}

int StorageMgr::getBlockPosition()
{
	return m_nPosition;
}

int StorageMgr::fixExceptionalData()
{
	QMap<int, RecordInfo> recInfoMap;
	//get all record which need to fix
	getRecInfo(recInfoMap);
	if (recInfoMap.isEmpty())
	{
		return 0;//no exceptional data
	}

	//judge record is to delete or fix
	judgeFixMethod(recInfoMap);

	//delete or fix record
	processRecord(recInfoMap);
	return 0;
}

void StorageMgr::getRecInfo( QMap<int, RecordInfo> &recInfoMap )
{
	QString disks = getUseDisks();
	QStringList diskList = disks.split(":");
	foreach(QString disk, diskList)
	{
		QString dbPath = disk + ":/REC/record.db";
		if (!QFile::exists(dbPath))
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"file:"<<dbPath<<" is not exist";
			continue;
		}
		QSqlDatabase *pDataBase = initDataBase(dbPath, (int*)this);
		if (NULL == pDataBase)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"can't open database "<<dbPath;
			continue;
		}
		QSqlQuery query(*pDataBase);
		QString cmd = "select id, win_id, start_time, end_time, path from local_record where start_time=end_time";
		query.exec(cmd);

		while(query.next())
		{
			RecordInfo item;
			item.dbPath = dbPath;
			item.id = query.value(0).toInt();
			item.startTime = query.value(2).toString();
			item.endTime = query.value(3).toString();
			item.filePath = query.value(4).toString();
			item.fixMethod = 0;//init for delete
			recInfoMap.insert(query.value(1).toInt(), item);
		}
		query.finish();
	}
}

void StorageMgr::judgeFixMethod( QMap<int, RecordInfo> &recInfoMap )
{
	QSqlDatabase *pDataBase = initDataBase(g_sSearchRecord, (int*)this);
	if (NULL == pDataBase)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"can't open database "<<g_sSearchRecord;
		return;
	}
	QSqlQuery query(*pDataBase);
	QMap<int, RecordInfo>::iterator iter = recInfoMap.begin();
	while(iter != recInfoMap.end())
	{
		QString cmd = QString("select id, end_time from search_record where wnd_id=%1 order by id desc limit 1").arg(iter.key());
		query.exec(cmd);
		if(query.next())
		{
			QString endTime = query.value(1).toString();
			if (endTime > iter->startTime)
			{
				iter->endTime = endTime;
				iter->fixMethod = 1;//this record is to fix
			}
			else
			{
				cmd = QString("delete from search_record where id=%1").arg(query.value(0).toInt());
				query.exec(cmd);
			}
		}
		iter++;
	}
}

void StorageMgr::processRecord( QMap<int, RecordInfo> &recInfoMap )
{
	QMap<int, RecordInfo>::iterator iter = recInfoMap.begin();
	while(iter != recInfoMap.end())
	{
		QSqlDatabase *pDataBase = initDataBase(iter->dbPath, (int*)this);
		if (NULL == pDataBase)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"can't open database "<<iter->dbPath<<" wnd:"<<iter.key();
			continue;
		}
		QSqlQuery query(*pDataBase);
		if (iter->fixMethod)//can be fixed
		{
			QString cmd = QString("update local_record set end_time='%1', file_size=%2 where id=%3").arg(iter->endTime).arg(getFileSize(iter->filePath)).arg(iter->id);
			query.exec(cmd);
		}
		else//can't be fixed, then delete
		{
			QString cmd = QString("delete from local_record where id=%1").arg(iter->id);
			query.exec(cmd);
			if (QFile::remove(iter->filePath))
			{
				QString dirpath = iter->filePath.left(iter->filePath.lastIndexOf("/"));
				QDir dir(dirpath);
				dir.setFilter(QDir::Files);
				if (!dir.count())
				{
					dir.rmpath(dirpath);
				}
			}
		}
		iter++;
	}
}

quint64 StorageMgr::getFileSize( QString fileName )
{
	QFile file(fileName);
	if (file.exists())
	{
		return file.size();
	}
	else
		return 0;
}

int StorageMgr::checkTableExist( QString dbpath, QString table )
{
	m_dblock.lock();
	QSqlDatabase *pDataBase = initDataBase(dbpath, (int*)this);
	if (NULL != pDataBase)
	{
		QSqlQuery query(*pDataBase);
		QString cmd = QString("select count(*) from sqlite_master where type='table' and name='%1'").arg(table);
		query.exec(cmd);
		if (query.next())
		{
			if (query.value(0).toInt() > 0)
			{
				m_dblock.unlock();
				return 0;//table has find
			}
			else
			{
				m_dblock.unlock();
				return 1;//don't find table
			}
		}
		else
		{
			m_dblock.unlock();
			return 2;//sql does not execute properly
		}
	}
	else
	{
		m_dblock.unlock();
		return 3;//can't open database
	}
}