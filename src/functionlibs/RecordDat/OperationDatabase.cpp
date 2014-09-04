#include "OperationDatabase.h"
#include <guid.h>
#include "netlib.h"
#pragma comment(lib,"netlib.lib")
typedef struct __tagMgrDataBaseInfo{
	QString sDatabaseName;
	QSqlDatabase *pDatabase;
	int nCount;
	QList<quintptr *> tThis;
}tagMgrDataBaseInfo;
QMultiMap<QString ,tagMgrDataBaseInfo> g_tMgrDataBase;
QSqlDatabase *initMgrDataBase(QString sDatabaseName,quintptr *nThis){
	if (g_tMgrDataBase.contains(sDatabaseName))
	{
		if (g_tMgrDataBase.find(sDatabaseName).value().tThis.contains(nThis))
		{
			//do nothing
		}else{
			g_tMgrDataBase.find(sDatabaseName).value().nCount++;
			g_tMgrDataBase.find(sDatabaseName).value().tThis.append(nThis);
		}
		return g_tMgrDataBase.find(sDatabaseName).value().pDatabase;
	}else{
		tagMgrDataBaseInfo tDataBaseInfo;
		tDataBaseInfo.sDatabaseName=sDatabaseName;
		tDataBaseInfo.nCount=1;
		tDataBaseInfo.tThis.append(nThis);

		QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE",sDatabaseName);
		tDataBaseInfo.pDatabase=new QSqlDatabase(db);
		tDataBaseInfo.pDatabase->setDatabaseName(sDatabaseName);
		if (tDataBaseInfo.pDatabase->open())
		{
			//do nothing
			QSqlQuery _query(*tDataBaseInfo.pDatabase);
			QString sCommand="pragma journal_mode =off";
			if (_query.exec(sCommand))
			{
			}else{
				tDataBaseInfo.pDatabase->close();
				delete tDataBaseInfo.pDatabase;
				tDataBaseInfo.pDatabase=NULL;
				printf("exec cmd fail:pragma journal_mode =off,in initMgrDataBase function /n");
				return NULL;
			}
		}else{
			printf("open database fail,in initMgrDataBase function/n");
			return NULL;
		}
		g_tMgrDataBase.insert(sDatabaseName,tDataBaseInfo);
		return tDataBaseInfo.pDatabase;
	}
}
void deInitMgrDataBase(quintptr *nThis){
	QMultiMap<QString,tagMgrDataBaseInfo>::iterator it;
	QStringList sDeleteList;
	for (it=g_tMgrDataBase.begin();it!=g_tMgrDataBase.end();it++)
	{
		if (it.value().tThis.contains(nThis))
		{
			it.value().nCount--;
			if (it.value().nCount==0)
			{
				it.value().pDatabase->close();
				delete it.value().pDatabase;
				it.value().pDatabase=NULL;
				sDeleteList.append(it.value().sDatabaseName);
				QSqlDatabase::removeDatabase(it.value().sDatabaseName);
			}else{
				it.value().tThis.removeOne(nThis);
			}
		}else{
			//keep going
		}
	}
	for (int i=0;i<sDeleteList.size();i++)
	{
		g_tMgrDataBase.remove(sDeleteList.at(i));
	}
}
OperationDatabase::OperationDatabase(void):m_pDisksSetting(NULL)
{
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IDiskSetting,(void**)&m_pDisksSetting);
	if (NULL==m_pDisksSetting)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_pDisksSetting should not be null";
		abort();
	}else{
		//do nothing
	}
	m_tSystemDatabaseInfo.uiDiskReservedSize=5400;
	m_tSystemDatabaseInfo.bIsRecover=false;
}


OperationDatabase::~OperationDatabase(void)
{
	if (NULL!=m_pDisksSetting)
	{
		m_pDisksSetting->Release();
		m_pDisksSetting=NULL;
	}else{
		//do nothing
	}
	deInitMgrDataBase(( quintptr*)this);
}
QString OperationDatabase::getOldestItem( QString sDisk )
{
	QString sRecordDatabasePath=sDisk+"/recEx/record.db";
	QFile tFile;
	QString sOldestFile;
	tFile.setFileName(sRecordDatabasePath);
	if (tFile.exists())
	{
		QSqlDatabase *pDataBase=initMgrDataBase(sRecordDatabasePath,(quintptr*)this);
		if (pDataBase!=NULL)
		{
			QSqlQuery _query(*pDataBase);
			QString sCommand;
			int nStep=0;
			bool bStop=false;
			quint64 uiStartTime=QDateTime::currentDateTime().toTime_t();
			
			while(bStop==false){
				switch(nStep){
				case 0:{
					//查找最早的记录
					sCommand.clear();
					sCommand=QString("select id,sFilePath,nStartTime from record where nStartTime=(select min(nStartTime) from record where nStartTime <%1").arg(uiStartTime);
					if (_query.exec(sCommand))
					{
						if (_query.next())
						{
							QString sFilePathTemp=_query.value(1).toString();
							uiStartTime=_query.value(2).toUInt();
							sCommand.clear();
							sCommand=QString("select id,nLock,nDamage,nInUse from RecordFileStatus where sFilePath='%1'").arg(sFilePathTemp);
							if (_query.exec(sCommand))
							{
								if (_query.next())
								{
									int nLock=_query.value(1).toInt();
									int nDamage=_query.value(1).toInt();
									if (nLock==1||nDamage==1)
									{
										nStep=0;
									}else{
										sOldestFile=sFilePathTemp;
										nStep=1;
									}
								}else{
									QFile tFile;
									tFile.setFileName(sFilePathTemp);
									if (tFile.exists())
									{
										nStep=0;
									}else{
										sOldestFile=sFilePathTemp;
										nStep=1;
									}
								}
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
								abort();
							}
						}else{
							nStep=1;
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
						abort();
					}
					   }
					   break;
				case 1:{
					//end
					bStop=true;
					   }
					   break;
				}
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getOldestItem fail as pDataBase is null";
		}
	}else{
		//do nothing
	}
	return sOldestFile;
}
QString OperationDatabase::getLatestItem( QString sDisk )
{
	QString sRecordDatabasePath=sDisk+"/recEx/record.db";
	QFile tFile;
	tFile.setFileName(sRecordDatabasePath);
	if (tFile.exists())
	{
		QSqlDatabase *pDataBase=initMgrDataBase(sRecordDatabasePath,(quintptr*)this);
		if (NULL!=pDataBase)
		{
			QSqlQuery _query(*pDataBase);
			QString sCommand=QString("select id,sFilePath from record where nStartTime=(select max(nStartTime) from record)");
			if (_query.exec(sCommand))
			{
				if (_query.next())
				{
					QString sFilePath=_query.value(1).toString();
					sCommand.clear();
					sCommand=QString("id,nLock,nDamage from RecordFileStatus where sFilePath='%1'").arg(sFilePath);
					if (_query.exec(sCommand))
					{
						if (_query.next())
						{
							if (_query.value(1).toInt()==1|_query.value(2).toInt()==0)
							{
								return "";
							}else{
								//do nothing
							}
						}else{
							//do nothing
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
						abort();
					}
					return sFilePath;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"the record.db is empty";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"getLatestItem fail exec sCommand fail :"<<sCommand;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getLatestItem fail as pDataBase is null";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<sRecordDatabasePath<<"do not exist";
	}
	return "";
}

QString OperationDatabase::createLatestItem( QString sDisk )
{
	//用于磁盘还有空间，递增模式
	QString sFilePath;
	QString sRecordDatabasePath=sDisk+"/recEx/record.db";
	QFile tFile;
	tFile.setFileName(sRecordDatabasePath);
	if (tFile.exists())
	{
		QSqlDatabase *pDataBase=initMgrDataBase(sRecordDatabasePath,(quintptr*)this);
		if (NULL!=pDataBase)
		{
			QSqlQuery _query(*pDataBase);
			QString sCommandTest=QString("select id,sFilePath from RecordFileStatus where nInUse=0 and nDamage=0 and nLock=0");
			if (_query.exec(sCommandTest))
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem fail exec sCommand fail :"<<sCommandTest;
				if (_query.next())
				{
					sFilePath=_query.value(1).toString();
				}else{
					QString sCommand=QString("select id,nFileNum from RecordFileStatus where nFileNum=(select max(nFileNum) from RecordFileStatus");
					if (_query.exec(sCommand))
					{
						if (_query.next())
						{
							quint32 uiFileNum=_query.value(1).toUInt();
							uiFileNum=uiFileNum+1;
							QString sFilePathTemp=sDisk+"/recEx/0000/0000/0000/0000.dat";
							int nConsult=0;
							int nResidue=0;
							nConsult=uiFileNum/MAXFILENUM;
							nResidue=uiFileNum%MAXFILENUM;
							uiFileNum=nConsult;
							QString sResidue=QString::number(nResidue);
							// 0000.dat"
							sFilePathTemp.replace((sFilePathTemp.length()-4-sResidue.length()),sResidue.length(),sResidue);
							if (nConsult!=0)
							{
								// 0000/0000.dat"
								nConsult=uiFileNum/MAXFILENUM;
								nResidue=uiFileNum%MAXFILENUM;
								uiFileNum=nConsult;
								sResidue=QString::number(nResidue);
								sFilePathTemp.replace((sFilePathTemp.length()-4-5-sResidue.length()),sResidue.length(),sResidue);
								if (nConsult!=0)
								{
									// 0000/0000/0000.dat"
									nConsult=uiFileNum/MAXFILENUM;
									nResidue=uiFileNum%MAXFILENUM;
									uiFileNum=nConsult;
									sResidue=QString::number(nResidue);
									sFilePathTemp.replace((sFilePathTemp.length()-4-5-5-sResidue.length()),sResidue.length(),sResidue);
									if (nConsult!=0)
									{
										// 0000/0000/0000/0000.dat"
										nConsult=uiFileNum/MAXFILENUM;
										nResidue=uiFileNum%MAXFILENUM;
										uiFileNum=nConsult;
										sResidue=QString::number(nResidue);
										sFilePathTemp.replace((sFilePathTemp.length()-4-5-5-sResidue.length()),sResidue.length(),sResidue);
										sFilePath=sFilePathTemp;
									}else{
										// do nothing
									}
								}else{
									//do nothing
								}
							}else{
								//do nothing
							}
						}else{
							sFilePath=sDisk+"/recEx/0000/0000/0000/0000.dat";
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem fail exec sCommand fail :"<<sCommand;
						abort();
					}
				}
			}else{
				abort();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem fail as pDataBase is null";
			abort();
		}
	}else{
		//创建数据库
		if (createRecordDatabase(sRecordDatabasePath))
		{
			sFilePath=sDisk+"/recEx/0000/0000/0000/0000.dat";
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem as createRecordDatabase fail";
			abort();
		}
	}
	return sFilePath;
}


void OperationDatabase::clearInfoInDatabase( QString sFilePath )
{
	//删除 record 表中记录
	//删除 search_record表中记录
	//更新 RecordFileStatus
	QSqlDatabase *pDatabase=NULL;
	QString sDatabasePath=sFilePath.left(1)+":/recEx/record.db";
	QFile tDatabaseFile;
	tDatabaseFile.setFileName(sDatabasePath);
	if (tDatabaseFile.exists())
	{
		pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
		if (NULL!=pDatabase)
		{
			QSqlQuery _query(*pDatabase);
			QString sCommand;
			//删除 search_record表中记录
			sCommand=QString("select nWndId from record where sFilePath='%1'").arg(sFilePath);
			if (_query.exec(sCommand))
			{
				QList<int > tWndIdList;
				if (_query.next())
				{
					tWndIdList.append(_query.value(0).toInt());
				}else{
					//do nothing
				}
				if (!tWndIdList.isEmpty())
				{
					sCommand.clear();
					sCommand=QString("select max(nEndTime) from record where sFilePath ='%1'").arg(sFilePath);
					if (_query.exec(sCommand))
					{
						quint64 uiEndTime=_query.value(0).toUInt();
						sCommand=QString("delete from search_record where nEndTime<=%1").arg(uiEndTime);
						if (_query.exec(sCommand))
						{
							for (int i=0;i<tWndIdList.size();i++)
							{
								int nWnd=tWndIdList.value(i);
								sCommand=QString("update search_record set nStartTime=%1 where nStartTime<%2 and nWndId=%3").arg(uiEndTime).arg(uiEndTime).arg(nWnd);
								if (_query.exec(sCommand))
								{
									//keep going
								}else{
									qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
									abort();
								}
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
							abort();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
						abort();
					}
				}else{
					//do nothing
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
				abort();
			}
			//删除 record 表中记录
			sCommand=QString("delete from record where sFilePath ='%1'").arg(sFilePath);
			if (_query.exec())
			{
				//keep going
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
				abort();
			}
			//更新 RecordFileStatus
			QVariantMap tInfo;
			tInfo.insert("nInUse",0);
			priSetRecordFileStatus(sFilePath,tInfo);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
			abort();
		}
	}else{
		//do nothing
	}
}

bool OperationDatabase::updateRecordDatabase( int nId,QVariantMap tInfo )
{
	return false;
}

bool OperationDatabase::updateSearchDatabase( int nId ,QVariantMap tInfo)
{
	return false;
}

bool OperationDatabase::createSearchDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType ,uint &uiItemId)
{
	return false;
}

bool OperationDatabase::createRecordDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName ,uint &uiItemId)
{
	return false;
}

QString OperationDatabase::getUsableDisk( QString &sDiskLisk )
{
	//返回值：有剩余空间可用的盘符；传进参数：录像盘符列表
	bool bStop=false;
	int nStep=0;
	QString sDiskFound;
	while(bStop==false){
		switch(nStep){
		case 0:{
			//检测空间是否够
			if (NULL!=m_pDisksSetting)
			{
				sDiskLisk=m_tSystemDatabaseInfo.sAllRecordDisk;
				QStringList tDiskList=m_tSystemDatabaseInfo.sAllRecordDisk.split(":");
				if (tDiskList.size()!=0)
				{
					quint64 uiFreeByteAvailable=0;
					quint64 uiTotalNumberOfBytes=0;
					quint64 uiTotalNumberOfFreeByte=0;
					foreach(QString sDiskItem,tDiskList){
						QString sDiskEx=sDiskItem+":";
						if (GetDiskFreeSpaceExQ(sDiskEx.toAscii().data(),&uiFreeByteAvailable,&uiTotalNumberOfBytes,&uiTotalNumberOfFreeByte))
						{
							if (uiTotalNumberOfFreeByte/1024/1024>(quint64)m_tSystemDatabaseInfo.uiDiskReservedSize)
							{
								sDiskFound=sDiskEx;
								break;
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<sDiskEx<<"do not have enough space for recorder,find next disk";
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<sDiskEx<<"can not been gotten message,it may be a system disk";
						}
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"there is no disk for record as tDiskList size==0";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"there is no disk for record as m_pDisksSetting is null";
				// do nothing
			}
			if (sDiskFound.isEmpty())
			{
				nStep=1;
			}else{
				nStep=2;
			}
			   }
			   break;
		case 1:{
			//删除1.1.13版本录像
			if (freeDisk())
			{
				nStep=0;
			}else{
				nStep=2;
			}
			   }
			   break;
		case 2:{
			//end
			   }
		}
	}
	return sDiskFound;
}

void OperationDatabase::reloadSystemDatabase()
{
	//获取录像磁盘，获取磁盘保留空间，是否循环录像
	if (NULL!=m_pDisksSetting)
	{
		if (0==m_pDisksSetting->getUseDisks(m_tSystemDatabaseInfo.sAllRecordDisk))
		{
			m_tSystemDatabaseInfo.bIsRecover=m_pDisksSetting->getLoopRecording();
			int nDiskReservedSize=5400;
			m_pDisksSetting->getDiskSpaceReservedSize(nDiskReservedSize);
			m_tSystemDatabaseInfo.uiDiskReservedSize=nDiskReservedSize;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"reloadSystemDatabase fail as getUseDisks fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"reloadSystemDatabase fail as m_pDisksSetting is null";
	}
}

bool OperationDatabase::createRecordDatabase( QString sDatabasePath )
{
	QFile tFile;
	tFile.setFileName(sDatabasePath);
	if (!tFile.exists())
	{
		QFileInfo tFileInfo(tFile);
		QString sDirPath=tFileInfo.absolutePath();
		QDir tDir;
		if (!tDir.exists(sDirPath))
		{
			if (tDir.mkpath(sDirPath))
			{
				//keep going
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"create dirPath fail"<<sDirPath;
				abort();
			}
		}else{
			// do nothing
		}
		if (tFile.open(QIODevice::ReadWrite))
		{
			tFile.close();
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabase fail:"<<sDatabasePath;
			abort();
		}
		QSqlDatabase *pDatabase=NULL;
		pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
		if (NULL!=pDatabase)
		{
			QSqlQuery _query(*pDatabase);
			QString sCommand="create table RecordFileStatus(";
			sCommand+="id integer primary key autoincrement,";
			sCommand+="sFilePath char(64),";
			sCommand+="nLock integer,";
			sCommand+="nDamage integer,";
			sCommand+="nInUse integer,";
			sCommand+="nFileNum integer)";
			//create table RecordFileStatus
			if (_query.exec(sCommand))
			{
				//keep going
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
				abort();
			}
			//create table record
			sCommand.clear();
			sCommand+="create table record(";
			sCommand+="id integer primary key autoincrement,";
			sCommand+="nWndId integer,";
			sCommand+="nRecordType integer,";
			sCommand+="nStartTime integer,";
			sCommand+="nEndTime integer,";
			sCommand+="sFilePath char(64))";
			if (_query.exec(sCommand))
			{
				//keep going
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
				abort();
			}
			//create table search_record
			sCommand.clear();
			sCommand+="id integer primary key autoincrement,";
			sCommand+="nWndId integer,";
			sCommand+="nRecordType integer,";
			sCommand+="nStartTime integer,";
			sCommand+="nEndTime integer)";
			if (_query.exec(sCommand))
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
				abort();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not be null";
			abort();
		}
	}else{
		return true;
	}
}

void OperationDatabase::setRecordFileStatus( QString sFilePath,QVariantMap tInfo )
{
	priSetRecordFileStatus(sFilePath,tInfo);
}

void OperationDatabase::priSetRecordFileStatus( QString sFilePath,QVariantMap tInfo )
{

}

bool OperationDatabase::getIsRecover()
{
	return m_tSystemDatabaseInfo.bIsRecover;
}

bool OperationDatabase::freeDisk()
{
	return false;
}


