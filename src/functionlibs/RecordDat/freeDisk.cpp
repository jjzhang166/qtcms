#include "freeDisk.h"
#include "netlib.h"
#pragma comment(lib,"netlib.lib")
QString g_sMgrSearchRecord="C:/CMS_RECORD/search_record.db";
typedef struct __tagFreeDiskDataBaseInfo{
	QString sDatabaseName;
	QSqlDatabase *pDatabase;
	QString sConnectDatabaseId;
	int nCount;
	QList<quintptr *> tThis;
}tagFreeDiskDataBaseInfo;
quint64 g_uiDatabaseFreeDiskId=0;
QMultiMap<QString ,tagFreeDiskDataBaseInfo> g_tFreeDiskDataBase;
QSqlDatabase *initFreeDiskDataBase(QString sDatabaseName,quintptr *nThis){
	if (g_tFreeDiskDataBase.contains(sDatabaseName))
	{
		if (g_tFreeDiskDataBase.find(sDatabaseName).value().tThis.contains(nThis))
		{
			//do nothing
		}else{
			g_tFreeDiskDataBase.find(sDatabaseName).value().nCount++;
			g_tFreeDiskDataBase.find(sDatabaseName).value().tThis.append(nThis);
		}
		return g_tFreeDiskDataBase.find(sDatabaseName).value().pDatabase;
	}else{
		tagFreeDiskDataBaseInfo tDataBaseInfo;
		tDataBaseInfo.sDatabaseName=sDatabaseName;
		tDataBaseInfo.nCount=1;
		tDataBaseInfo.tThis.append(nThis);

		QDateTime tCurrentTime=QDateTime::currentDateTime();
		g_uiDatabaseFreeDiskId++;
		QString sDatabaseId=QString::number(tCurrentTime.toTime_t())+QString::number((quint64)nThis)+QString::number(g_uiDatabaseFreeDiskId);
		while(QSqlDatabase::connectionNames().contains(sDatabaseId)){
			g_uiDatabaseFreeDiskId++;
			sDatabaseId=QString::number(tCurrentTime.toTime_t())+QString::number((quint64)nThis)+QString::number(g_uiDatabaseFreeDiskId);
		}
		QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE",sDatabaseId);
		tDataBaseInfo.pDatabase=new QSqlDatabase(db);
		tDataBaseInfo.pDatabase->setDatabaseName(sDatabaseName);
		tDataBaseInfo.sConnectDatabaseId=sDatabaseId;
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
		g_tFreeDiskDataBase.insert(sDatabaseName,tDataBaseInfo);
		return tDataBaseInfo.pDatabase;
	}
}
void deInitFreeDiskDataBase(quintptr *nThis){
	QMultiMap<QString,tagFreeDiskDataBaseInfo>::iterator it;
	QStringList sDeleteList;
	for (it=g_tFreeDiskDataBase.begin();it!=g_tFreeDiskDataBase.end();it++)
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
				QSqlDatabase::removeDatabase(it.value().sConnectDatabaseId);
			}else{
				it.value().tThis.removeOne(nThis);
			}
		}else{
			//keep going
		}
	}
	for (int i=0;i<sDeleteList.size();i++)
	{
		g_tFreeDiskDataBase.remove(sDeleteList.at(i));
	}
}

freeDisk::freeDisk(void)
{
}


freeDisk::~freeDisk(void)
{
	deInitFreeDiskDataBase((quintptr *)this);
}

bool freeDisk::freeDiskEx( QString sAllDisk ,quint64 uiDiskReservedSize)
{
	QStringList sDiskList=sAllDisk.split(":");
	if (sDiskList.size()!=0)
	{
		int iFreeDiskStep=0;
		bool bFreeDiskStop=false;
		bool bFreeSucceed=false;
		QMap<QDate,tagMgrRecInfo> tPreDeleteItem;
		while(bFreeDiskStop==false){
			switch(iFreeDiskStep){
			case 0:{
				//检测空间
				bool bFound=false;
				foreach(QString sDiskItem,sDiskList){
					quint64 uiFreeByteAvailable=0;
					quint64 uiTotalNumberOfBytes=0;
					quint64 uiTotalNumberOfFreeByte=0;
					QString sDiskEx=sDiskItem+":";
					if (GetDiskFreeSpaceExQ(sDiskEx.toAscii().data(),&uiFreeByteAvailable,&uiTotalNumberOfBytes,&uiTotalNumberOfFreeByte))
					{
						if (uiTotalNumberOfFreeByte/1024/1024>(quint64)uiDiskReservedSize)
						{
							bFound=true;
							break;
						}else{
							//keep going 
							qDebug()<<__FUNCTION__<<__LINE__<<sDiskEx<<"do not have enough space for recorder,find next disk";
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<sDiskEx<<"can not been gotten message,it may be a system disk";
					}
				}
				if (bFound)
				{
					iFreeDiskStep=1;
				}else{
					iFreeDiskStep=2;
				}
				   }
				   break;
			case 1:{
				//具有足够的空间
				iFreeDiskStep=3;
				bFreeSucceed=true;
				   }
				   break;
			case 2:{
				//空间不足 删除空间
				//step1:查找出最早的记录，不能删除当天，同时满足相隔3个小时，必须保证硬盘空间足够录像一天
				tPreDeleteItem.clear();
				QDate tEarlestDate;
				qDebug()<<__FUNCTION__<<__LINE__<<"findEarliestRecord in";
				foreach (QString sDiskEx,sDiskList)
				{
					QString sDataBasePath=sDiskEx+":/REC/record.db";
					QDate tDate;
					QMap<int ,QString> tMaxEndTimeMap;
					if (QFile::exists(sDataBasePath))
					{
						QStringList tList=findEarliestRecord(sDataBasePath,tDate,tMaxEndTimeMap);
						if (!tList.isEmpty())
						{
							tagMgrRecInfo tRecItem;
							tRecItem.sDbPath=sDataBasePath;
							tRecItem.tFileList=tList;
							tRecItem.tMaxEndTimeMap=tMaxEndTimeMap;
							tPreDeleteItem.insertMulti(tDate,tRecItem);
						}else{
							//keep going
							qDebug()<<__FUNCTION__<<__LINE__<<sDataBasePath<<"do not find any item for delete";
						}
					}else{
						//keep going
						qDebug()<<__FUNCTION__<<__LINE__<<"warn::"<<sDataBasePath<<"do not exists";
					}
				}
				qDebug()<<__FUNCTION__<<__LINE__<<"findEarliestRecord out";
				//step2:删除文件
				if (!tPreDeleteItem.isEmpty())
				{
					//查找各个磁盘之中最早的一天的录像
					qDebug()<<__FUNCTION__<<__LINE__<<"removeFile in";
					tEarlestDate=minDate(tPreDeleteItem.keys());
					QList<tagMgrRecInfo> tRecInfo=tPreDeleteItem.values(tEarlestDate);
					QStringList tRemoteItemList;
					for(int i=0;i<tRecInfo.size();i++){
						tagMgrRecInfo tEach=tRecInfo.at(i);
						QStringList tItemList=removeFile(tEach.tFileList);
						tRemoteItemList<<tItemList;
					}
					qDebug()<<__FUNCTION__<<__LINE__<<"removeFile out";
					//step3:删除录像表的条目
					qDebug()<<__FUNCTION__<<__LINE__<<"removeRecordDataBaseItem in";
					if (removeRecordDataBaseItem(tRemoteItemList,tRecInfo))
					{
						//keep going
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"removeRecordDataBaseItem fail ,please check";
					}
					qDebug()<<__FUNCTION__<<__LINE__<<"removeRecordDataBaseItem out";
					qDebug()<<__FUNCTION__<<__LINE__<<"removeSearchDataBaseItem in";
					//step4:删除搜索表中的条目
					if (removeSearchDataBaseItem(tRemoteItemList,tRecInfo,tEarlestDate.toString("yyyy-MM-dd")))
					{
						//keep going
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"removeSearchDataBaseItem fail ,please check";
					}
					qDebug()<<__FUNCTION__<<__LINE__<<"removeSearchDataBaseItem out";
					if (!(tRemoteItemList.isEmpty()&&tRecInfo.isEmpty()))
					{
						iFreeDiskStep=0;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"delete file fail,as the tRemoteItemList is empty";
						iFreeDiskStep=3;
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"there are not any item for delete";
					iFreeDiskStep=3;
				}
				   }
				   break;
			case 3:{
				//结束循环
				bFreeDiskStop=true;
				   }
				   break;
			}
		}
		if (bFreeSucceed)
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"freeDisk fail as it can not free enough space for recorder";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"freeDisk fail as there are no disk for recorder";
	}
	return false;
}

QStringList freeDisk::findEarliestRecord( QString tDbPath,QDate &tEarlestDate,QMap<int ,QString>&tMaxEndTimeMap )
{
	QStringList sPathItemList;
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initFreeDiskDataBase(tDbPath,(quintptr *)this);
	if (NULL!=pDataBase)
	{
		QString sEarlyDate;
		QSqlQuery _query(*pDataBase);
		QString sCommand=QString("select distinct date from local_record order by date limit 1");
		if (execCommand(_query,sCommand))
		{
			if (_query.next())
			{
				sEarlyDate=_query.value(0).toString();
				tEarlestDate=QDate::fromString(sEarlyDate,"yyyy-MM-dd");
				sCommand.clear();
				sCommand=QString("select date, path,id, win_id, end_time ,start_time from local_record where date='%1' order by start_time").arg(sEarlyDate);
				if (execCommand(_query,sCommand))
				{
					while(_query.next()){
							//确保将要删除的条目 与当前时间相隔 三个小时
							//fix me
							QDate tCurrentDate=QDate::currentDate();
							QDateTime tCurrentDateTime=QDateTime::currentDateTime();
							tCurrentDateTime.setDate(tCurrentDate);
							unsigned int iCurrentDate=tCurrentDateTime.toTime_t();
							QString sDateTime=_query.value(5).toString();
							QDateTime tDateTime=QDateTime::fromString(sDateTime,"hh:mm:ss");
							QDate tDate=QDate::fromString(sEarlyDate,"yyyy-MM-dd");
							tDateTime.setDate(tDate);
							unsigned int iDate=tDateTime.toTime_t();
							if ((iCurrentDate-iDate)>10)
							{
								int iWidId=_query.value(3).toInt();
								QString sEndTime=_query.value(4).toString();
								sPathItemList<<_query.value(1).toString();
								if (!tMaxEndTimeMap.contains(iWidId))
								{
									tMaxEndTimeMap.insert(iWidId,sEndTime);
								}else{
									QString sTemp=tMaxEndTimeMap.value(iWidId);
									if (sTemp<sEndTime)
									{
										tMaxEndTimeMap[iWidId]=sEndTime;
									}else{
										//do nothing
									}
								}
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"the item  can not be delete as start time within 5h"<<tDateTime.toString("hh:mm:ss");
							}
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"findEarliestRecord fail as exec the cmd::"<<sCommand<<"fail";
				}
			}else{
				//keep going 
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"findEarliestRecord fail as exec the cmd::"<<sCommand<<"fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"findEarliestRecord fail as pDataBase is null";
	}
	return sPathItemList;
}

bool freeDisk::removeRecordDataBaseItem( QStringList tRemoveFileItem,QList<tagMgrRecInfo> tRecInfo )
{
	//tRemoveFileItem :已删除的文件路径
	//tRecInfo:筛选出来最早一天的记录
	//this func need to test
	for (int i=0;i<tRecInfo.size();i++)
	{
		QString sDatabasePath=tRecInfo.value(i).sDbPath;
		QStringList sRemoveFile=tRecInfo.value(i).tFileList;
		if (sRemoveFile.size()>0)
		{
			QString sFileName=sRemoveFile.value(0);
			QSqlDatabase *pDataBase=NULL;
			pDataBase=initFreeDiskDataBase(sDatabasePath,(quintptr*)this);
			if (NULL!=pDataBase)
			{
				QSqlQuery _query(*pDataBase);
				QString sCommand=QString("select date from local_record where path='%1'").arg(sFileName);
				if (execCommand(_query,sCommand))
				{
					if (_query.next())
					{
						QString sDate=_query.value(0).toString();
						sCommand.clear();
						sCommand=QString("select max(id) from local_record where date='%1'").arg(sDate);
						if (execCommand(_query,sCommand))
						{
							if (_query.next())
							{
								int nId=_query.value(0).toInt();
								sCommand.clear();
								sCommand=QString("delete from local_record where id=%1 or id<%2").arg(nId).arg(nId);
								if (execCommand(_query,sCommand))
								{
								}else{
									qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
									abort();
								}
							}else{
								//do nothing
								qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
								abort();
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
							abort();
						}
					}else{
						//do nothing
						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
						abort();
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
					abort();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as pDataBase should not be null";
				abort();
			}
		}else{
			//do nothing
		}
	}
	return true;
	//使用上面的代码可以大大节省删除数据库的时间，下面的代码不再使用
}

bool freeDisk::removeSearchDataBaseItem( QStringList tRemoveFileItem,QList<tagMgrRecInfo> tRecInfo,QString sDate )
{
	//tRemoveFileItem:已删除文件的路径
	//tRecInfo:筛选出来最早一天的记录
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initFreeDiskDataBase(g_sMgrSearchRecord,(quintptr*)this);
	if (NULL!=pDataBase)
	{
		QSqlQuery _query(*pDataBase);
		for (int i=0;i<tRecInfo.size();i++)
		{
			tagMgrRecInfo tMgrRecInfo=tRecInfo.at(i);
			QMap<int ,QString>::iterator iter=tMgrRecInfo.tMaxEndTimeMap.begin();
			QString sCommond;
			while(iter!=tMgrRecInfo.tMaxEndTimeMap.end()){
				QString sMaxEndTime=iter.value();
				sCommond=QString("delete from search_record where (wnd_id=%1 and (date='%2' and end_time<='%3')) or date<'%4'").arg(iter.key()).arg(sDate).arg(iter.value()).arg(sDate);
				if (execCommand(_query,sCommond))
				{
					sCommond.clear();
					sCommond=QString("select id,start_time,end_time from search_record where date ='%1' and wnd_id =%2 order by start_time limit 1").arg(sDate).arg(iter.key());
					if (execCommand(_query,sCommond))
					{
						if (_query.next())
						{
							int iId=_query.value(0).toInt();
							QString sStartTime=_query.value(1).toString();
							QString sEndTime=_query.value(2).toString();
							if (sMaxEndTime>sStartTime&&sMaxEndTime<sEndTime)
							{
								sCommond.clear();
								sCommond=QString("update search_record set start_time='%1' where id='%2'").arg(sMaxEndTime).arg(iId);
								if (execCommand(_query,sCommond))
								{
									//do
								}else{
									qDebug()<<__FUNCTION__<<__LINE__<<"removeSearchDataBaseItem fail as exec cmd fail::"<<sCommond;
								}
							}else{
								//do nothing
							}
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"removeSearchDataBaseItem fail as exec cmd fail::"<<sCommond;
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"removeSearchDataBaseItem fail as exec cmd fail::"<<sCommond;
				}
				iter++;
			}
		}
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"removeSearchDataBaseItem fail as pDataBase is null";
		return false;
	}
}

QStringList freeDisk::removeFile( QStringList tItemList )
{
	QStringList tRemoteItemList;
	if (!tItemList.isEmpty())
	{
		foreach(QString sFilePath,tItemList){
			if (QFile::remove(sFilePath))
			{
				tRemoteItemList<<sFilePath;
				QString sDirPath=sFilePath.left(sFilePath.lastIndexOf("/"));
				QDir tDir(sDirPath);
				tDir.setFilter(QDir::Files);
				if (!tDir.count())
				{
					tDir.rmpath(sDirPath);
				}else{
					//do nothing
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"delete record file::"<<sFilePath<<"fail,please check";
			}
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"removeFile do not remove any file as tItemList is empty";
	}
	return tRemoteItemList;
}

QDate freeDisk::minDate( QList<QDate> tDateList )
{
	QDate tMinDate=tDateList.at(0);
	for(int i=1;i<tDateList.size();++i){
		tMinDate=qMin(tDateList.at(i),tMinDate);
	}
	return tMinDate;
}

bool freeDisk::execCommand( QSqlQuery & tQuery,QString sCommand )
{
	bool bflags=false;
	int nCount=0;
	while(bflags==false){
		if (nCount>1000)
		{
			bflags=true;
		}
		if (tQuery.exec(sCommand))
		{
			if (nCount!=0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"try=====as lock======success";
			}
			return true;
		}else{
			if ("database is locked"==tQuery.lastError().databaseText())
			{
				msleep(1);
				nCount++;
				qDebug()<<__FUNCTION__<<__LINE__<<"try:"<<nCount<<"as lock";
			}else{
				return false;
			}
		}
	}
	return false;
}
