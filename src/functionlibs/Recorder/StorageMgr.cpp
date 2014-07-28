#include "StorageMgr.h"
#include "guid.h"
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include "netlib.h"
#pragma comment(lib,"netlib.lib")

#define qDebug() qDebug()<<"this:"<<(int)this


QMutex StorageMgr::m_schRecLock;
QMutex StorageMgr::m_sLock;
QMutex StorageMgr::m_dblock;
QList<int > StorageMgr::m_insertIdList;
StorageMgr::StorageMgr(void):
	m_insertId(-1),
	m_searchRecordId(-1),
	m_nPosition(0),
	m_pDisksSetting(NULL),
	m_db(NULL)
/*	m_currdisk('0')*/
{
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IDiskSetting,(void **)&m_pDisksSetting);
	if (!m_pDisksSetting)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"create m_pDisksSetting fail,please check";
	}
	QDateTime curTime = QDateTime::currentDateTime();
	m_connectId = QString::number(curTime.toTime_t()) + QString::number((int)this);
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectId);
	m_dblock.lock();
	m_db = new QSqlDatabase(db);
	if (NULL == m_db)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"init database failed!";
	}
	m_dblock.unlock();

	m_connectSearchId = QString::number((int)this) + QString::number(curTime.toTime_t());
	QSqlDatabase dbSch = QSqlDatabase::addDatabase("QSQLITE", m_connectSearchId);
	m_schRecLock.lock();
	m_dbSearch = new QSqlDatabase(dbSch);
	if (NULL == m_dbSearch)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"init search database failed!";
	}
	m_schRecLock.unlock();
}


StorageMgr::~StorageMgr(void)
{
	if (m_pDisksSetting)
	{
		m_pDisksSetting->Release();
	}
	m_nPosition=__LINE__;
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

		//find the newest record in database and create path base on it
		QSqlQuery _query(*m_db);
		QString command = QString("select path from local_record where win_id=%1 order by id desc limit 1").arg(winId);
		_query.exec(command);

		if (_query.next())//has find it
		{
			QString path = _query.value(0).toString();
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
		else
		{
			//use default path
			QString temp;
			sFileSavePath += temp.sprintf("/WND%02d/0000/000.avi",winId); 
		}

		//insert new data into database
		_query.prepare("insert into local_record(dev_name,dev_chl,win_id,date,start_time,record_type,path) values(:dev_name,:dev_chl,:win_id,:date,:start_time,:record_type,:path)");
		_query.bindValue(":dev_name",devname);
		_query.bindValue(":dev_chl",nChannelNum + 1);//ensure chl num start with 1
		_query.bindValue(":win_id",winId);
		_query.bindValue(":date",QDate::currentDate().toString("yyyy-MM-dd"));
		start = QTime::currentTime();
		_query.bindValue(":start_time",start.toString("hh:mm:ss"));
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

QString StorageMgr::getUsableDisk()//????'0'???÷?????????ú×???????????
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
								if (!m_curDisk.isEmpty() && m_curDisk != sGottenDisk && m_db->isOpen())
								{
									m_db->close();
								}
								QString dbpath = sdisk + "/REC";
								QDir dDir;
								m_nPosition=__LINE__;
								m_dblock.lock();
								if (dDir.exists(dbpath))
								{
									//do nothing
								}else{
									//create dir ::rec
									dDir.mkpath(dbpath);
								}
								dbpath+="/record.db";
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
								}
								m_dblock.unlock();
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
	m_dblock.lock();
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
				QStringList list = findEarlestRecord(path, date);
				if (list.isEmpty())
				{
					continue;//open database failed or no record in database
				}
				RecInfo re;
				re.dbPath = path;
				re.fileLsit = list;
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
				deleteFile(each.fileLsit);
				//deduct period from each window id in search_record table
				m_nPosition=__LINE__;
				deductPeriod(each.dbPath, earlestDate.toString("yyyy-MM-dd"));
				//delete record from database
				m_nPosition=__LINE__;
				deleteRecord(each.dbPath, earlestDate.toString("yyyy-MM-dd"));
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
	m_dblock.unlock();
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

void StorageMgr::deleteFile(const QStringList& fileList)
{
	if (!fileList.isEmpty())
	{
		//quint64 FreeByteAvailable = 0;
		//quint64 TotalNumberOfBytes = 0;
		//quint64 TotalNumberOfFreeBytes = 0;
		//int filesize = 0;
		//if(0 != m_pDisksSetting->getFilePackageSize(filesize))
		//	filesize = 128;
		//int freesizem = 0;
		//if(0 != m_pDisksSetting->getDiskSpaceReservedSize(freesizem))
		//	freesizem = 4096;

		m_nPosition=__LINE__;
		foreach(QString file, fileList)
		{
			QFile::remove(file);
			QString dirpath = file.left(file.lastIndexOf("/"));
			QDir dir(dirpath);
			dir.setFilter(QDir::Files);
			if (!dir.count())
			{
				dir.rmpath(dirpath);
			}
			//QString sdisk = file.left(2);
			//GetDiskFreeSpaceExQ(sdisk.toAscii().data(),&FreeByteAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);
			//qint64 freeSize = (qint64)TotalNumberOfFreeBytes - (qint64)freesizem*1024*1024;
			//qint64 fiveFile = filesize * 1024 * 1024 * 5;
			//if (freeSize > fiveFile)//Ensure that the free space can store 5 files
			//{
			//	break;
			//}
		}
	}
}

bool StorageMgr::GetDiskFreeSpaceEx(char* lpDirectoryName, quint64* lpFreeBytesAvailableToCaller, quint64* lpTotalNumberOfBytes, quint64* lpTotalNumberOfFreeBytes)
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
				if (deleteOldDir(sDiskList))
				{
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"freeDisk fail as deleteOldDir fail";
				}
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

bool StorageMgr::deleteRecord()
{
	bool bRet=false;
	m_nPosition=__LINE__;
	m_dblock.lock();
	if (m_insertId!=-1)
	{
		QSqlQuery _query(*m_db);
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
	}
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
	}
}

QStringList StorageMgr::findEarlestRecord( QString dbPath, QDate &earlestDate )
{
	QStringList pathList;
	pathList.empty();
	m_db->setDatabaseName(dbPath);
	if (!m_db->open())
	{
		qDebug()<<"open " + dbPath + "failed!";
		return pathList;
	}
	QSqlQuery _query(*m_db);
	QString command = QString("select date, path,id from local_record where date=(select date from local_record order by date limit 1) order by start_time");
	_query.exec(command);

	while (_query.next())
	{
		earlestDate = _query.value(0).toDate();
		if (m_insertIdList.contains(_query.value(2).toInt()))
		{
		}else{
			pathList<<_query.value(1).toString();
		}
	}
	_query.finish();
	m_db->close();

	return pathList;
}

void StorageMgr::deleteRecord( QString dbPath, QString date)
{
	m_db->setDatabaseName(dbPath);
	if (!m_db->open())
	{
		qDebug()<<"open " + dbPath + "failed!";
	}
	QSqlQuery _query(*m_db);
// 	foreach(QString path, filelist)
// 	{
// 		QString command = QString("delete from local_record where date = '%1' and path = '%2'").arg(date).arg(path);
// 		_query.exec(command);
// 	}
	QString command = QString("delete from local_record where date = '%1'").arg(date);
	_query.exec(command);

	_query.finish();
	m_db->close();
}



void StorageMgr::createTable()
{
	QSqlQuery _query(*m_db);
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
}

int StorageMgr::getInsertId()
{
	return m_insertId;
}

void StorageMgr::createSearchRecordTable()
{
	QSqlQuery _query(*m_dbSearch);
	QString command = "create table search_record(";
	command += "id integer primary key autoincrement,";
	command += "wnd_id integer,";
	command += "record_type integer,";
	command += "date char(32),";
	command += "start_time char(32),";
	command += "end_time char(32))";
	_query.exec(command);
}

bool StorageMgr::addSearchRecord( int wndId, int type, QString sDate, QString sStart, QString sEnd )
{
	if (wndId < 0 || sDate.isEmpty() || sStart.isEmpty() || sEnd.isEmpty())
	{
		return false;
	}
	m_nPosition=__LINE__;
	m_schRecLock.lock();
	//check database exists, 
// 	QString path = QCoreApplication::applicationDirPath() + "/search_record.db";
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
	}
	//insert data
	QSqlQuery _query(*m_dbSearch);
	QString command = QString("insert into search_record(wnd_id,record_type,date,start_time,end_time) values(%1,%2,'%3','%4','%5')").arg(wndId).arg(type).arg(sDate).arg(sStart).arg(sEnd);
	if (!_query.exec(command))
	{
		_query.finish();
		m_db->close();
		m_schRecLock.unlock();
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
	m_dbSearch->close();
	m_schRecLock.unlock();
	return true;
}

bool StorageMgr::updateSearchRecord( QString sEnd )
{
	if (sEnd.isEmpty())
	{
		return false;
	}
	m_nPosition=__LINE__;
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

	QSqlQuery _query(*m_dbSearch);
	QString command = QString("update search_record set end_time='%1' where id=%2").arg(sEnd).arg(m_searchRecordId);
	bool ret = _query.exec(command);

	_query.finish();
	m_dbSearch->close();
	m_schRecLock.unlock();
	return ret;
}

bool StorageMgr::deleteSearchRecord()
{
	m_nPosition=__LINE__;
	m_schRecLock.lock();
	if (!m_dbSearch->open())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"open data base fail,please check";
		m_schRecLock.unlock();
		return false;
	}
	QSqlQuery _query(*m_dbSearch);
	QString command = QString("delete from serach_record where id=%2").arg(m_searchRecordId);
	bool ret = _query.exec(command);

	_query.finish();
	m_dbSearch->close();
	m_schRecLock.unlock();
	return ret;
}

void StorageMgr::deductPeriod( QString dbpath, QString date )
{
// 	QMap<int, QString> perMap = getDeletedPeriod(dbpath, deleteFile, date);

	m_nPosition=__LINE__;
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
			return;
		}
	}

	QSqlQuery _query(*m_dbSearch);
// 	QString cmd;
// 	QMap<int, QString>::iterator it = perMap.begin();
// 	while (it != perMap.end())
// 	{
// 		QTime maxEnd = QTime::fromString(it.value(), "hh:mm:ss");
// 		cmd = QString("delete from search_record where wnd_id=%1 and date='%2' and end_time<='%3'").arg(it.key()).arg(date).arg(it.value());
// 		_query.exec(cmd);
// 		
// 		qDebug()<<__LINE__<<"delete database sql:"<<cmd;
// 
// 		cmd = QString("select id, start_time, end_time from search_record where date='%1' and wnd_id=%2 order by start_time limit 1").arg(date).arg(it.key());
// 		_query.exec(cmd);
// 		if (_query.next())
// 		{
// 			int id = _query.value(0).toInt();
// 			QTime start = _query.value(1).toTime();
// 			QTime end = _query.value(2).toTime();
// 			if (maxEnd > start && maxEnd < end)
// 			{
// 				cmd = QString("update search_record set start_time='%1' where id=%2").arg(it.value()).arg(id);
// 				_query.exec(cmd);
// 			}
// 		}
// 		it++;
// 	}
	QString cmd = QString("delete from search_record where date='%1'").arg(date);
	_query.exec(cmd);
	_query.finish();
	m_dbSearch->close();
	m_schRecLock.unlock(); 
}

// QMap<int, QString> StorageMgr::getDeletedPeriod( QString dbpath, QStringList fileList, QString date )
// {
// 	QMap<int, QString> perMap;
// 	if (!m_db->isOpen())
// 	{
// 		m_db->setDatabaseName(dbpath);
// 		if (!m_db->open())
// 		{
// 			qDebug()<<__FUNCTION__<<__LINE__<<"open data base fail,please check";
// 			return perMap;
// 		}
// 	}
// 
// 	QString sqlpath = "'" + fileList.join("','") + "'";
// 	QSqlQuery _query(*m_db);
// 	QString command = QString("select win_id, end_time from local_record where date='%1' and path in (%2) order by win_id").arg(date).arg(sqlpath);
// 	_query.exec(command);
// 
// 	while(_query.next())
// 	{
// 		int wndId = _query.value(0).toInt();
// 		QString endStr = _query.value(1).toString();
// 		QTime end = QTime::fromString(endStr, "hh:mm:ss");
// 
// 		if (!perMap.contains(wndId))//map is empty
// 		{
// 			perMap.insert(wndId, endStr);
// 		}
// 		else
// 		{
// 			QTime temp = QTime::fromString(perMap[wndId], "hh:mm:ss");
// 			if (temp < end)
// 			{
// 				perMap[wndId] = endStr;
// 			}
// 		}
// 	}
// 	_query.finish();
// 	m_db->close();
// 
// 	return perMap;
// }

QString StorageMgr::getNewestRecord( QString devname, int chl )
{
	QString endTimeStr;
	m_nPosition=__LINE__;
	m_dblock.lock();
	QSqlQuery _query(*m_db);
	QString command = QString("select end_time from local_record where dev_name='%1' and dev_chl=%2 order by id desc limit 1").arg(devname).arg(chl + 1);
	_query.exec(command);

	if (_query.next())
	{
		endTimeStr = _query.value(0).toString();
	}
	_query.finish();
	m_dblock.unlock();

	return endTimeStr;
}

int StorageMgr::getBlockPosition()
{
	return m_nPosition;
}
