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
//QString OperationDatabase::getOldestItem( QString sDisk )
//{
//	QString sRecordDatabasePath=sDisk+"/recEx/record.db";
//	QFile tFile;
//	QString sOldestFile;
//	tFile.setFileName(sRecordDatabasePath);
//	if (tFile.exists())
//	{
//		QSqlDatabase *pDataBase=initMgrDataBase(sRecordDatabasePath,(quintptr*)this);
//		if (pDataBase!=NULL)
//		{
//			QSqlQuery _query(*pDataBase);
//			QString sCommand;
//			int nStep=0;
//			bool bStop=false;
//			quint64 uiCurrentDateTime=QDateTime::currentDateTime().toTime_t();
//			quint64 uiEndTime=0;
//			QList<QString> tAbandonFilePath;
//			while(bStop==false){
//				switch(nStep){
//				case 0:{
//					//查找最早的记录
//					sCommand.clear();
//					sCommand=QString("select id,sFilePath,nStartTime from record where nStartTime=(select min(nStartTime) from record where nStartTime <%1 and nStartTime>%2)").arg(uiCurrentDateTime).arg(uiEndTime);
//					if (_query.exec(sCommand))
//					{
//						QString sFilePathTemp;
//						bool bFound=false;
//						while(_query.next()&&bFound==false){
//							sFilePathTemp=_query.value(1).toString();
//							if (tAbandonFilePath.contains(sFilePathTemp))
//							{
//								//keep going
//								sFilePathTemp.clear();
//							}else{
//								bFound=true;
//							}
//						}
//						if (!sFilePathTemp.isEmpty())
//						{
//							sCommand.clear();
//							sCommand=QString("select max(nEndTime) from record where sFilePath='%1'").arg(sFilePathTemp);
//							if (_query.exec(sCommand))
//							{
//								if (_query.next())
//								{
//									uiEndTime=_query.value(0).toUInt();
//									sCommand.clear();
//									sCommand=QString("select id,nLock,nDamage,nInUse from RecordFileStatus where sFilePath='%1'").arg(sFilePathTemp);
//									if (_query.exec(sCommand))
//									{
//										if (_query.next())
//										{
//											int nLock=_query.value(1).toInt();
//											int nDamage=_query.value(1).toInt();
//											if (nLock==1||nDamage==1)
//											{
//												nStep=0;
//												tAbandonFilePath.append(sFilePathTemp);
//												sFilePathTemp.clear();
//											}else{
//												sOldestFile=sFilePathTemp;
//												nStep=1;
//											}
//										}else{
//											QFile tFile;
//											tFile.setFileName(sFilePathTemp);
//											if (tFile.exists())
//											{
//												nStep=0;
//											}else{
//												sOldestFile=sFilePathTemp;
//												nStep=1;
//											}
//										}
//									}else{
//										qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
//										abort();
//									}
//								}else{
//									qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail"<<sCommand;
//									abort();
//								}
//							}else{
//								qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
//								abort();
//							}
//						}else{
//							nStep=1;
//						}
//					}else{
//						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
//						abort();
//					}
//					   }
//					   break;
//				case 1:{
//					//end
//					bStop=true;
//					   }
//					   break;
//				}
//			}
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"getOldestItem fail as pDataBase is null";
//		}
//	}else{
//		//do nothing
//	}
//	return sOldestFile;
//}
//QString OperationDatabase::getLatestItem( QString sDisk )
//{
//	QString sRecordDatabasePath=sDisk+"/recEx/record.db";
//	QFile tFile;
//	tFile.setFileName(sRecordDatabasePath);
//	if (tFile.exists())
//	{
//		QSqlDatabase *pDataBase=initMgrDataBase(sRecordDatabasePath,(quintptr*)this);
//		if (NULL!=pDataBase)
//		{
//			QSqlQuery _query(*pDataBase);
//			QString sCommand=QString("select id,sFilePath from record where nStartTime=(select max(nStartTime) from record)");
//			if (_query.exec(sCommand))
//			{
//				if (_query.next())
//				{
//					QString sFilePath=_query.value(1).toString();
//					sCommand.clear();
//					sCommand=QString("select id,nLock,nDamage from RecordFileStatus where sFilePath='%1'").arg(sFilePath);
//					if (_query.exec(sCommand))
//					{
//						if (_query.next())
//						{
//							if (_query.value(1).toInt()==1||_query.value(2).toInt()==1)
//							{
//								return "";
//							}else{
//								//do nothing
//							}
//						}else{
//							//do nothing
//						}
//					}else{
//						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
//						abort();
//					}
//					return sFilePath;
//				}else{
//					qDebug()<<__FUNCTION__<<__LINE__<<"the record.db is empty";
//				}
//			}else{
//				qDebug()<<__FUNCTION__<<__LINE__<<"getLatestItem fail exec sCommand fail :"<<sCommand;
//			}
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"getLatestItem fail as pDataBase is null";
//		}
//	}else{
//		qDebug()<<__FUNCTION__<<__LINE__<<sRecordDatabasePath<<"do not exist";
//	}
//	return "";
//}

//QString OperationDatabase::createLatestItem( QString sDisk )
//{
//	//用于磁盘还有空间，递增模式
//	QString sFilePath;
//	QString sRecordDatabasePath=sDisk+"/recEx/record.db";
//	QFile tFile;
//	tFile.setFileName(sRecordDatabasePath);
//	if (tFile.exists())
//	{
//		QSqlDatabase *pDataBase=initMgrDataBase(sRecordDatabasePath,(quintptr*)this);
//		if (NULL!=pDataBase)
//		{
//			QSqlQuery _query(*pDataBase);
//			QString sCommandTest=QString("select id,sFilePath from RecordFileStatus where nInUse=0 and nDamage=0 and nLock=0");
//			if (_query.exec(sCommandTest))
//			{
//				if (_query.next())
//				{
//					sFilePath=_query.value(1).toString();
//				}else{
//					QString sCommand=QString("select id,nFileNum from RecordFileStatus where nFileNum=(select max(nFileNum) from RecordFileStatus)");
//					if (_query.exec(sCommand))
//					{
//						if (_query.next())
//						{
//							quint32 uiFileNum=_query.value(1).toUInt();
//							uiFileNum=uiFileNum+1;
//							QString sFilePathTemp=sDisk+"/recEx/0000/0000/0000/0000.dat";
//							int nConsult=0;
//							int nResidue=0;
//							nConsult=uiFileNum/MAXFILENUM;
//							nResidue=uiFileNum%MAXFILENUM;
//							uiFileNum=nConsult;
//							QString sResidue=QString::number(nResidue);
//							// 0000.dat"
//							sFilePathTemp.replace((sFilePathTemp.length()-4-sResidue.length()),sResidue.length(),sResidue);
//							if (nConsult!=0)
//							{
//								// 0000/0000.dat"
//								nConsult=uiFileNum/MAXFILENUM;
//								nResidue=uiFileNum%MAXFILENUM;
//								uiFileNum=nConsult;
//								sResidue=QString::number(nResidue);
//								sFilePathTemp.replace((sFilePathTemp.length()-4-5-sResidue.length()),sResidue.length(),sResidue);
//								if (nConsult!=0)
//								{
//									// 0000/0000/0000.dat"
//									nConsult=uiFileNum/MAXFILENUM;
//									nResidue=uiFileNum%MAXFILENUM;
//									uiFileNum=nConsult;
//									sResidue=QString::number(nResidue);
//									sFilePathTemp.replace((sFilePathTemp.length()-4-5-5-sResidue.length()),sResidue.length(),sResidue);
//									if (nConsult!=0)
//									{
//										// 0000/0000/0000/0000.dat"
//										nConsult=uiFileNum/MAXFILENUM;
//										nResidue=uiFileNum%MAXFILENUM;
//										uiFileNum=nConsult;
//										sResidue=QString::number(nResidue);
//										sFilePathTemp.replace((sFilePathTemp.length()-4-5-5-sResidue.length()),sResidue.length(),sResidue);
//										sFilePath=sFilePathTemp;
//									}else{
//										// do nothing
//									}
//								}else{
//									//do nothing
//								}
//							}else{
//								//do nothing
//							}
//							sFilePath=sFilePathTemp;
//						}else{
//							sFilePath=sDisk+"/recEx/0000/0000/0000/0000.dat";
//						}
//					}else{
//						qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem fail exec sCommand fail :"<<sCommand;
//						abort();
//					}
//				}
//			}else{
//				qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem fail exec sCommand fail :"<<sCommandTest;
//				abort();
//			}
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem fail as pDataBase is null";
//			abort();
//		}
//	}else{
//		//创建数据库
//		if (createRecordDatabase(sRecordDatabasePath))
//		{
//			sFilePath=sDisk+"/recEx/0000/0000/0000/0000.dat";
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem as createRecordDatabase fail";
//			abort();
//		}
//	}
//	return sFilePath;
//}


//void OperationDatabase::clearInfoInDatabase( QString sFilePath )
//{
//	//删除 record 表中记录
//	//删除 search_record表中记录
//	//更新 RecordFileStatus
//	QString sDatabasePath=sFilePath.left(1)+":/recEx/record.db";
//	QFile tDatabaseFile;
//	tDatabaseFile.setFileName(sDatabasePath);
//	if (tDatabaseFile.exists())
//	{
//		QSqlDatabase *pDatabase=NULL;
//		pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
//		if (NULL!=pDatabase)
//		{
//			QSqlQuery _query(*pDatabase);
//			QString sCommand;
//			//删除 search_record表中记录
//			qDebug()<<__FUNCTION__<<__LINE__<<"//删除 search_record表中记录 in";
//			sCommand=QString("select nWndId,id from record where sFilePath='%1'").arg(sFilePath);
//			QList<quint64 > tRecordIdList;
//			if (_query.exec(sCommand))
//			{
//				QList<int > tWndIdList;
//				while(_query.next()){
//					tWndIdList.append(_query.value(0).toInt());
//					tRecordIdList.append(_query.value(1).toUInt());
//				}
//				if (!tWndIdList.isEmpty())
//				{
//					sCommand.clear();
//					sCommand=QString("select max(nEndTime) from record where sFilePath ='%1'").arg(sFilePath);
//					if (_query.exec(sCommand))
//					{
//						if (_query.next())
//						{
//							quint64 uiEndTime=_query.value(0).toUInt();
//							sCommand=QString("delete from search_record where nEndTime<=%1").arg(uiEndTime);
//							if (_query.exec(sCommand))
//							{
//								QString sWndIdList;
//								for (int i=0;i<tWndIdList.size();i++)
//								{
//									if (i==0)
//									{
//										sWndIdList=QString::number(tWndIdList.value(i));
//									}else{
//										sWndIdList=sWndIdList+","+QString::number(tWndIdList.value(i));
//									}
//								}
//								sCommand=QString("update search_record set nStartTime=%1 where nStartTime<%2 and nWndId in ").arg(uiEndTime).arg(uiEndTime)+"("+sWndIdList+")";
//								if (_query.exec(sCommand))
//								{
//									//keep going
//								}else{
//									qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
//									abort();
//								}
//							}else{
//								qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
//								abort();
//							}
//						}else{
//							//do nothing
//						}
//					}else{
//						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
//						abort();
//					}
//				}else{
//					//do nothing
//				}
//			}else{
//				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
//				abort();
//			}
//			qDebug()<<__FUNCTION__<<__LINE__<<"//删除 search_record表中记录 out";
//			//删除 record 表中记录
//			qDebug()<<__FUNCTION__<<__LINE__<<"删除 record 表中记录 in";
//			if (tRecordIdList.size()>0)
//			{
//				QString sReoveRecordIdList;
//				for(int i=0;i<tRecordIdList.size();i++){
//					if (i==0)
//					{
//						sReoveRecordIdList=QString::number(tRecordIdList.value(i));
//					}else{
//						sReoveRecordIdList=sReoveRecordIdList+","+QString::number(tRecordIdList.value(i));
//					}
//				}
//				sCommand=QString("delete from record where id in ")+"("+sReoveRecordIdList+")";
//				if (_query.exec(sCommand))
//				{
//					//keep going
//				}else{
//					qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
//					abort();
//				}
//			}else{
//				//do nothing
//			}
//			qDebug()<<__FUNCTION__<<__LINE__<<"删除 record 表中记录 out";
//			//更新 RecordFileStatus
//			QVariantMap tInfo;
//			tInfo.insert("nInUse",0);
//			priSetRecordFileStatus(sFilePath,tInfo);
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
//			abort();
//		}
//	}else{
//		//do nothing
//	}
//}

//bool OperationDatabase::updateRecordDatabase( QList<int> tIdList,QVariantMap tInfo,QString sFilePath )
//{
//	//可被更新的条目：nEndTime
//	if (tIdList.size()==0)
//	{
//		return true;
//	}else{
//		//keep going
//	}
//	if (tInfo.contains("nEndTime"))
//	{
//		QString sDatabasePath=sFilePath.left(1)+":/recEx/record.db";
//		QFile tDatabaseFile;
//		tDatabaseFile.setFileName(sDatabasePath);
//		if (tDatabaseFile.exists())
//		{
//			QSqlDatabase *pDatabase=NULL;
//			pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
//			if (NULL!=pDatabase)
//			{
//				QList<QString> tKeyList=tInfo.keys();
//				QString sKey=tKeyList.at(0);
//				QSqlQuery _query(*pDatabase);
//				QString sIdList;
//				for (int i=0;i<tIdList.size();i++)
//				{
//					if (i==0)
//					{
//						sIdList=QString::number(tIdList.value(i));
//					}else{
//						sIdList=sIdList+","+QString::number(tIdList.value(i));
//					}	
//				}
//				QString sCommand="update record set "+sKey+QString("=%1").arg(tInfo.value(sKey).toInt())+" where id in "+"("+sIdList+")";
//				if (_query.exec(sCommand))
//				{
//					return true;
//				}else{
//					qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail"<<sCommand;
//					abort();
//				}
//			}else{
//				qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
//				abort();
//			}
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"updateRecordDatabase fail as tDatabaseFile is no exists"<<sDatabasePath;
//		}
//	}else{
//		qDebug()<<__FUNCTION__<<__LINE__<<"undefined updateRecord type,i will abort the thread";
//		abort();
//	}
//	return false;
//}


//bool OperationDatabase::updateSearchDatabase( QList<int> tIdList,QVariantMap tInfo,QString sFilePath )
//{
//	//可被更新的条目：nEndTime nStartTime 
//	if (tIdList.size()==0)
//	{
//		return true;
//	}else{
//		//keep going
//	}
//	if (tInfo.contains("nEndTime")||tInfo.contains("nStartTime"))
//	{
//		QString sDatabasePath=sFilePath.left(1)+":/recEx/record.db";
//		QFile tDatabaseFile;
//		tDatabaseFile.setFileName(sDatabasePath);
//		if (tDatabaseFile.exists())
//		{
//			QSqlDatabase *pDatabase=NULL;
//			pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
//			if (NULL!=pDatabase)
//			{
//				QList<QString> tKeyList=tInfo.keys();
//				QString sKey=tKeyList.at(0);
//				QSqlQuery _query(*pDatabase);
//				QString sIdList;
//				for (int i=0;i<tIdList.size();i++)
//				{
//					if (i==0)
//					{
//						sIdList=QString::number(tIdList.value(i));
//					}else{
//						sIdList=sIdList+","+QString::number(tIdList.value(i));
//					}	
//				}
//				QString sCommand="update search_record set "+sKey+QString("=%1").arg(tInfo.value(sKey).toInt())+" where id in "+"("+sIdList+")";
//				if (_query.exec(sCommand))
//				{
//					return true;
//				}else{
//					qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
//					abort();
//				}
//			}else{
//				qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
//				abort();
//			}
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"updateSearchDatabase fail as tDatabaseFile is no exists"<<sDatabasePath;
//		}
//	}else{
//		qDebug()<<__FUNCTION__<<__LINE__<<"undefined updateSearchDatabase type,i will abort the thread";
//		abort();
//	}
//	return false;
//}

//bool OperationDatabase::createSearchDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType ,QString sFilename,uint &uiItemId)
//{
//	if (uiStartTime>uiEndTime)
//	{
//		qDebug()<<__FUNCTION__<<__LINE__<<"createSearchDatabaseItem fail as uiStartTime>uiEndTime"<<uiStartTime<<uiEndTime;
//		abort();
//		return false;
//	}else{
//		//keep going
//	}
//	if (uiType==MOTIONRECORD||uiType==TIMERECORD||uiType==MANUALRECORD)
//	{
//		//keep going
//	}else{
//		qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as uiType is undefine;"<<uiType;
//		abort();
//	}
//	QString sDatabasePath=sFilename.left(1)+":/recEx/record.db";
//	QFile tDatabaseFile;
//	tDatabaseFile.setFileName(sDatabasePath);
//	if (tDatabaseFile.exists())
//	{
//		//keep going
//	}else{
//		//创建搜索表
//		if (createRecordDatabase(sDatabasePath))
//		{
//			//keep going
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"createSearchDatabaseItem fail as createRecordDatabase fail";
//			return false;
//		}
//	}
//	QSqlDatabase *pDatabase=NULL;
//	pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
//	if (NULL!=pDatabase)
//	{
//		QSqlQuery _query(*pDatabase);
//		QString sCommand=QString("insert into search_record(nWndId,nRecordType,nStartTime,nEndTime)values(%1,%2,%3,%4)").arg(nChannel).arg(uiType).arg(uiStartTime).arg(uiEndTime);
//		if (_query.exec(sCommand))
//		{
//			sCommand.clear();
//			sCommand=QString("select id from search_record where nWndId=%1 and nRecordType=%2 and nStartTime=%3 and nEndTime=%4").arg(nChannel).arg(uiType).arg(uiStartTime).arg(uiEndTime);
//			if (_query.exec(sCommand))
//			{
//				if (_query.next())
//				{
//					uiItemId=_query.value(0).toUInt();
//					return true;
//				}else{
//					qDebug()<<__FUNCTION__<<__LINE__<<"there must something error,it may exec cmd fail"<<sCommand;
//					abort();
//				}
//			}else{
//				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail"<<sCommand;
//				abort();
//			}
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail"<<sCommand;
//			abort();
//		}
//	}else{
//		qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
//		abort();
//	}
//	return false;
//}

//bool OperationDatabase::createRecordDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName ,uint &uiItemId)
//{
//	if (uiStartTime>uiEndTime)
//	{
//		qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as uiStartTime>uiEndTime :"<<uiStartTime<<uiEndTime;
//		abort();
//		return false;
//	}else{
//		//keep going
//	}
//	if (uiType==MOTIONRECORD||uiType==TIMERECORD||uiType==MANUALRECORD)
//	{
//		//keep going
//	}else{
//		qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as uiType is undefine;"<<uiType;
//		abort();
//	}
//	QString sDatabasePath=sFileName.left(1)+":/recEx/record.db";
//	QFile tDatabaseFile;
//	tDatabaseFile.setFileName(sFileName);
//	if (tDatabaseFile.exists())
//	{
//		//keep going
//	}else{
//		if (createRecordDatabase(sDatabasePath))
//		{
//			//keep going
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as createRecordDatabase fail";
//			abort();
//			return false;
//		}
//	}
//	QSqlDatabase *pDatabase=NULL;
//	pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
//	if (NULL!=pDatabase)
//	{
//		QSqlQuery _query(*pDatabase);
//		QString sCommand=QString("insert into record(nWndId,nRecordType,nStartTime,nEndTime,sFilePath)values(%1,%2,%3,%4,'%5')").arg(nChannel).arg(uiType).arg(uiStartTime).arg(uiEndTime).arg(sFileName);
//		if (_query.exec(sCommand))
//		{
//			sCommand.clear();
//			sCommand=QString("select id from record where nWndId=%1 and nRecordType=%2 and nStartTime=%3 and nEndTime=%4").arg(nChannel).arg(uiType).arg(uiStartTime).arg(uiEndTime);
//			if (_query.exec(sCommand))
//			{
//				if (_query.next())
//				{
//					uiItemId=_query.value(0).toUInt();
//					return true;
//				}else{
//					qDebug()<<__FUNCTION__<<__LINE__<<"there must be something error as exec cmd"<<sCommand;
//					abort();
//					return false;
//				}
//			}else{
//				qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as exec cmd fail:"<<sCommand;
//				abort();
//				return false;
//			}
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as exec cmd fail:"<<sCommand;
//			abort();
//			return false;
//		}
//	}else{
//		qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as pDatabase should not been null";
//		abort();
//		return false;
//	}
//	return false;
//}

//QString OperationDatabase::getUsableDisk( QString &sDiskLisk )
//{
//	//返回值：有剩余空间可用的盘符；传进参数：录像盘符列表
//	bool bStop=false;
//	int nStep=0;
//	QString sDiskFound;
//	while(bStop==false){
//		switch(nStep){
//		case 0:{
//			//检测空间是否够
//			if (NULL!=m_pDisksSetting)
//			{
//				sDiskLisk=m_tSystemDatabaseInfo.sAllRecordDisk;
//				QStringList tDiskList=m_tSystemDatabaseInfo.sAllRecordDisk.split(":");
//				if (tDiskList.size()!=0)
//				{
//					quint64 uiFreeByteAvailable=0;
//					quint64 uiTotalNumberOfBytes=0;
//					quint64 uiTotalNumberOfFreeByte=0;
//					foreach(QString sDiskItem,tDiskList){
//						QString sDiskEx=sDiskItem+":";
//						if (GetDiskFreeSpaceExQ(sDiskEx.toAscii().data(),&uiFreeByteAvailable,&uiTotalNumberOfBytes,&uiTotalNumberOfFreeByte))
//						{
//							if (uiTotalNumberOfFreeByte/1024/1024>(quint64)m_tSystemDatabaseInfo.uiDiskReservedSize)
//							{
//								sDiskFound=sDiskEx;
//								break;
//							}else{
//								qDebug()<<__FUNCTION__<<__LINE__<<sDiskEx<<"do not have enough space for recorder,find next disk";
//							}
//						}else{
//							qDebug()<<__FUNCTION__<<__LINE__<<sDiskEx<<"can not been gotten message,it may be a system disk";
//						}
//					}
//				}else{
//					qDebug()<<__FUNCTION__<<__LINE__<<"there is no disk for record as tDiskList size==0";
//				}
//			}else{
//				qDebug()<<__FUNCTION__<<__LINE__<<"there is no disk for record as m_pDisksSetting is null";
//				// do nothing
//			}
//			if (sDiskFound.isEmpty())
//			{
//				nStep=1;
//			}else{
//				nStep=2;
//			}
//			   }
//			   break;
//		case 1:{
//			//删除1.1.13版本录像,按天删除
//			if ( m_tFreeDisk.freeDiskEx(m_tSystemDatabaseInfo.sAllRecordDisk,m_tSystemDatabaseInfo.uiDiskReservedSize))
//			{
//				nStep=0;
//			}else{
//				nStep=2;
//			}
//			   }
//			   break;
//		case 2:{
//			//end
//			bStop=true;
//			   }
//		}
//	}
//	return sDiskFound;
//}

//void OperationDatabase::reloadSystemDatabase()
//{
//	//获取录像磁盘，获取磁盘保留空间，是否循环录像
//	if (NULL!=m_pDisksSetting)
//	{
//		if (0==m_pDisksSetting->getUseDisks(m_tSystemDatabaseInfo.sAllRecordDisk))
//		{
//			m_tSystemDatabaseInfo.bIsRecover=m_pDisksSetting->getLoopRecording();
//			int nDiskReservedSize=5400;
//			m_pDisksSetting->getDiskSpaceReservedSize(nDiskReservedSize);
//			m_tSystemDatabaseInfo.uiDiskReservedSize=nDiskReservedSize;
//		}else{
//			qDebug()<<__FUNCTION__<<__LINE__<<"reloadSystemDatabase fail as getUseDisks fail";
//		}
//	}else{
//		qDebug()<<__FUNCTION__<<__LINE__<<"reloadSystemDatabase fail as m_pDisksSetting is null";
//	}
//}

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
			sCommand+="create table search_record(";
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
				return false;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not be null";
			abort();
			return false;
		}
	}else{
		return true;
	}
}

//void OperationDatabase::setRecordFileStatus( QString sFilePath,QVariantMap tInfo )
//{
//	priSetRecordFileStatus(sFilePath,tInfo);
//}

void OperationDatabase::priSetRecordFileStatus( QString sFilePath,QVariantMap tInfo )
{
	QString sDisk=sFilePath.left(1)+":/recEx/record.db";
	QFile tDatabaseFile;
	tDatabaseFile.setFileName(sDisk);
	if (tDatabaseFile.exists())
	{
		//keep going
	}else{
		if (createRecordDatabase(sDisk))
		{
			//keep going
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"priSetRecordFileStatus fail as file is not exist"<<sDisk;
		}
	}
	QSqlDatabase *pDatabase=NULL;
	pDatabase=initMgrDataBase(sDisk,(quintptr*)this);
	if (NULL!=pDatabase)
	{
		QSqlQuery _query(*pDatabase);
		QString sCommand;
		if (tInfo.contains("nLock")||tInfo.contains("nDamage")||tInfo.contains("nInUse"))
		{
			//判断是否有这个文件的条目，有的话，就更新，没的话就创建
			sCommand=QString("select id from RecordFileStatus where sFilePath='%1'").arg(sFilePath);
			if (_query.exec(sCommand))
			{
				if (_query.next())
				{
					//更新
					sCommand.clear();
					int nId=_query.value(0).toInt();
					QList<QString> tKeyList=tInfo.keys();
					QString sKey=tKeyList.at(0);
					sCommand="update RecordFileStatus set "+sKey+QString("=%1 where id=%2").arg(tInfo.value(sKey).toInt()).arg(nId);
					if (_query.exec(sCommand))
					{
						//done
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
						abort();
					}
				}else{
					//创建
					sCommand.clear();
					quint64 nFileNum=countFileNum(sFilePath);
					sCommand=QString("insert into RecordFileStatus(sFilePath,nLock,nDamage,nInUse,nFileNum)values('%1',0,0,0,%2)").arg(sFilePath).arg(nFileNum);
					if (_query.exec(sCommand))
					{
						//keep going
						QList<QString> tKeyList=tInfo.keys();
						QString sKey=tKeyList.at(0);
						sCommand.clear();
						sCommand="update RecordFileStatus set "+sKey+QString("=%1 where sFilePath='%2'").arg(tInfo.value(sKey).toInt()).arg(sFilePath);
						if (_query.exec(sCommand))
						{
							//done
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
							abort();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
						abort();
					}
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
				abort();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"undefined type";
			abort();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not be null";
		abort();
	}
}

bool OperationDatabase::getIsRecover()
{
	return m_tSystemDatabaseInfo.bIsRecover;
}

quint64 OperationDatabase::countFileNum( QString sFilePath )
{
	int nFrist=sFilePath.mid(sFilePath.length()-8-5-5-5,4).toInt();
	int nSecond=sFilePath.mid(sFilePath.length()-8-5-5,4).toInt();
	int nThird=sFilePath.mid(sFilePath.length()-8-5,4).toInt();
	int nFourth=sFilePath.mid(sFilePath.length()-8,4).toInt();
	quint64 uiNum=MAXFILENUM*(MAXFILENUM*(MAXFILENUM*nFrist+nSecond)+nThird)+nFourth;
	return uiNum;
}

tagSystemDatabaseInfo OperationDatabase::getSystemDatabaseInfo()
{
	return m_tSystemDatabaseInfo;
}

bool OperationDatabase::isDiskSpaceOverReservedSize()
{
	QString sDiskFound;
	QString sDiskLisk=m_tSystemDatabaseInfo.sAllRecordDisk;
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
	if (!sDiskFound.isEmpty())
	{
		return true;
	}else{
		return false;
	}
}

//bool OperationDatabase::isRecordDataExistItem()
//{
//	//1.1.13版本之前的数据库是否有条目
//	//1.1.13版本之后的数据是否有条目
//	QString sDiskFound;
//	QString sDiskLisk=m_tSystemDatabaseInfo.sAllRecordDisk;
//	QStringList tDiskList=m_tSystemDatabaseInfo.sAllRecordDisk.split(":");
//	if (tDiskList.size()!=0)
//	{
//		quint64 uiFreeByteAvailable=0;
//		quint64 uiTotalNumberOfBytes=0;
//		quint64 uiTotalNumberOfFreeByte=0;
//		foreach(QString sDiskItem,tDiskList){
//			QString sDiskEx=sDiskItem+":";
//			//step1:1.1.13版本之前的数据库是否有条目
//			QString sDatabasePath=sDiskEx+"rec/record.db";
//			QFile tDatabaseFile;
//			tDatabaseFile.setFileName(sDatabasePath);
//			if (tDatabaseFile.exists())
//			{
//				QSqlDatabase *pDatabase=NULL;
//				pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
//				if (NULL!=pDatabase)
//				{
//					QSqlQuery _query(*pDatabase);
//					QString sCommand=QString("select *from local_record");
//					if (_query.exec(sCommand))
//					{
//						if (_query.next())
//						{
//							return true;
//						}else{
//							// do nothing
//						}
//					}else{
//						qDebug()<<__FUNCTION__<<__LINE__<<"isRecordDataExistItem fail as exec cmd fail:"<<sCommand;
//						abort();
//					}
//				}else{
//					qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as pDatabase should not been null";
//					abort();
//				}
//			}else{
//				//do nothing
//			}
//			//step1:1.1.13版本之后的数据是否有条目
//			sDatabasePath.clear();
//			sDatabasePath=sDiskEx+"recEx/record.db";
//			tDatabaseFile.setFileName(sDatabasePath);
//			if (tDatabaseFile.exists())
//			{
//				QSqlDatabase *pDatabase=NULL;
//				pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
//				if (NULL!=pDatabase)
//				{
//					QSqlQuery _query(*pDatabase);
//					QString sCommand=QString("select *from record");
//					if (_query.exec(sCommand))
//					{
//						if (_query.next())
//						{
//							return true;
//						}else{
//							//do nothing
//						}
//					}else{
//						qDebug()<<__FUNCTION__<<__LINE__<<"isRecordDataExistItem fail as exec cmd fail:"<<sCommand;
//						abort();
//					}
//				}else{
//					qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as pDatabase should not been null";
//					abort();
//				}
//			}else{
//				//do nothing
//			}
//		}
//	}else{
//		qDebug()<<__FUNCTION__<<__LINE__<<"there is no disk for record as tDiskList size==0";
//	}
//	return false;
//}			

void OperationDatabase::run()
{
	bool bRunStop=false;
	int nStepCode=OperationDatabase_init;
	QVariantMap tCurrentInfo;
	while(bRunStop==false){
		switch(nStepCode){
		case OperationDatabase_init:{
			nStepCode=OperationDatabase_obtainFilePath;
									}
									break;
		case OperationDatabase_obtainFilePath:{
			QString sFilePath;
			int nReturn=priObtainFilePath(sFilePath);
			m_tObtainFilePathLock.lock();
			if (m_tObtainFilePathResult.isEmpty())
			{
			}else{
				m_tObtainFilePathResult.clear();
			}
			QVariantMap tInfo;
			tInfo.insert("nReturn",nReturn);
			tInfo.insert("sWriteFilePath",sFilePath);
			m_tObtainFilePathResult.enqueue(tInfo);
			m_tObtainFilePathLock.unlock();
			nStepCode=OperationDatabase_default;
											  }
											  break;
		case OperationDatabase_updateRecordDatabase:{
			QList<int >tIdList;
			QVariantMap tInfo;
			QString sFilePath;
			sFilePath=tCurrentInfo.value("sFilePath").toString();
			QString sIdList=tCurrentInfo.value("tIdList").toString();
			QString sInfo=tCurrentInfo.value("tInfo").toString();
			QStringList tIdListSplit=sIdList.split(",");
			for (int i=0;i<tIdListSplit.size();i++)
			{
				tIdList.append(tIdListSplit.value(i).toInt());
			}
			QStringList tInfoSplit=sInfo.split(",");
			QString sKey;
			for(int i=0;i<tInfoSplit.size();i++){
				if (i%2==0)
				{
					sKey=tInfoSplit.value(i);
				}else{
					tInfo.insert(sKey,tInfoSplit.value(i));
				}
			}
			if (priUpdateRecordDatabase(tIdList,tInfo,sFilePath))
			{
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"priUpdateRecordDatabase fail";
			}
			nStepCode=OperationDatabase_default;
													}
													break;
		case OperationDatabase_updateSearchDatabase:{
			QList<int >tIdList;
			QVariantMap tInfo;
			QString sFilePath;
			sFilePath=tCurrentInfo.value("sFilePath").toString();
			QString sIdList=tCurrentInfo.value("tIdList").toString();
			QString sInfo=tCurrentInfo.value("tInfo").toString();
			QStringList tIdListSplit=sIdList.split(",");
			for (int i=0;i<tIdListSplit.size();i++)
			{
				tIdList.append(tIdListSplit.value(i).toInt());
			}
			QStringList tInfoSplit=sInfo.split(",");
			QString sKey;
			for(int i=0;i<tInfoSplit.size();i++){
				if (i%2==0)
				{
					sKey=tInfoSplit.value(i);
				}else{
					tInfo.insert(sKey,tInfoSplit.value(i));
				}
			}
			if (priUpdateSearchDatabase(tIdList,tInfo,sFilePath))
			{
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"priUpdateRecordDatabase fail";
			}
			nStepCode=OperationDatabase_default;
													}
													break;
		case OperationDatabase_createSearchDatabaseItem:{
			int nChannel=tCurrentInfo.value("nChannel").toInt();
			quint64 uiStartTime=tCurrentInfo.value("uiStartTime").toUInt();
			quint64 uiEndTime=tCurrentInfo.value("uiEndTime").toUInt();
			uint uiType=tCurrentInfo.value("uiType").toUInt();
			QString sFileName=tCurrentInfo.value("sFileName").toString();
			quint64 uiItemId=tCurrentInfo.value("uiItemId").toUInt();
			if (priCreateSearchDatabaseItem(nChannel,uiStartTime,uiEndTime,uiType,sFileName,uiItemId))
			{
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"priCreateSearchDatabaseItem fail";
			}
			nStepCode=OperationDatabase_default;
														}
														break;
		case OperationDatabase_createRecordDatabaseItem:{
			int nChannel=tCurrentInfo.value("nChannel").toInt();
			quint64 uiStartTime=tCurrentInfo.value("uiStartTime").toUInt();
			quint64 uiEndTime=tCurrentInfo.value("uiEndTime").toUInt();
			uint uiType=tCurrentInfo.value("uiType").toUInt();
			QString sFileName=tCurrentInfo.value("sFileName").toString();
			quint64 uiItemId=tCurrentInfo.value("uiItemId").toUInt();
			if (priCreateRecordDatabaseItem(nChannel,uiStartTime,uiEndTime,uiType,sFileName,uiItemId))
			{
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"priCreateSearchDatabaseItem fail";
			}
			nStepCode=OperationDatabase_default;
														}
														break;
		case OperationDatabase_setRecordFileStatus:{
			QString sInfo=tCurrentInfo.value("tInfo").toString();
			QString sFilePath=tCurrentInfo.value("sFilePath").toString();
			QStringList tInfoSplit=sInfo.split(",");
			QString sKey;
			QVariantMap tInfo;
			for(int i=0;i<tInfoSplit.size();i++){
				if (i%2==0)
				{
					sKey=tInfoSplit.value(i);
				}else{
					tInfo.insert(sKey,tInfoSplit.value(i));
				}
			}
			priSetRecordFileStatusEx(sFilePath,tInfo);
			nStepCode=OperationDatabase_default;
												   }
												   break;
		case OperationDatabase_reloadSystemDatabase:{
			priReloadSystemDatabase();
			nStepCode=OperationDatabase_default;
													}
													break;
		case OperationDatabase_clearInfoInDatabase:{
			QString sFilePath=tCurrentInfo.value("sFilePath").toString();
			priClearInfoInDatabase(sFilePath);
			nStepCode=OperationDatabase_default;
												   }
												   break;
		case OperationDatabase_isRecordDataExistItem:{
			bool bFlags=false;
			bFlags=PriIsRecordDataExistItem();
			m_tIsRecordDataExistItemResult.enqueue(bFlags);
			nStepCode=OperationDatabase_default;
													 }
													 break;
		case OperationDatabase_getMaxDatabaseId:{
			quint64 uiMaxRecordId=0;
			quint64 uiMaxSearchId=0;
			QString sFilePath=tCurrentInfo.value("sFilePath").toString();
			if (PriGetMaxDatabaseId(uiMaxRecordId,uiMaxSearchId,sFilePath))
			{

			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"OperationDatabase_getMaxDatabaseId fail as PriGetMaxDatabaseId";
			}
			QVariantMap tInfo;
			tInfo.insert("uiMaxRecordId",uiMaxRecordId);
			tInfo.insert("uiMaxSearchId",uiMaxSearchId);
			m_tGetMaxDatabaseIdResult.enqueue(tInfo);
			nStepCode=OperationDatabase_default;
												}
												break;
		case OperationDatabase_default:{
			if (!m_tStepCode.isEmpty())
			{
				m_tStepCodeLock.lock();
				tagCodeWithParm tCodeWithParm=m_tStepCode.dequeue();
				m_tStepCodeLock.unlock();
				nStepCode=tCodeWithParm.tCode;
				tCurrentInfo=tCodeWithParm.tInfo;
			}else{
				if (m_bStop==true)
				{
					if (!m_tObtainFilePathResult.isEmpty())
					{
						QVariantMap tInfo;
						tInfo=m_tObtainFilePathResult.first();
						QString sFilePath=tInfo.value("sWriteFilePath").toString();
						tInfo.insert("nLock",0);
						priSetRecordFileStatusEx(sFilePath,tInfo);
					}else{
						//do nothing
					}

					bRunStop=true;
				}else{
					msleep(10);
				}
			}
									   }
									   break;
		}
	}
}

int OperationDatabase::obtainFilePath( QString &sWriteFilePath )
{
	//0:覆盖写；1：续写文件；2：没有文件可写
	int nReturn=2;
	if (QThread::isRunning())
	{
		int nSleepCount=0;
		while(m_tObtainFilePathResult.isEmpty()){
			msleep(10);
			nSleepCount++;
			if (nSleepCount>300)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"obtainFilePath fail";
				m_tStepCodeLock.lock();
				tagCodeWithParm tCodeWithParm;
				tCodeWithParm.tCode=OperationDatabase_obtainFilePath;
				m_tStepCode.enqueue(tCodeWithParm);
				m_tStepCodeLock.unlock();
				return nReturn;
			}
		}
		m_tObtainFilePathLock.lock();
		QVariantMap tInfo=m_tObtainFilePathResult.dequeue();
		m_tObtainFilePathLock.unlock();
		nReturn=tInfo.value("nReturn").toInt();
		sWriteFilePath=tInfo.value("sWriteFilePath").toString();
		//准备下一个文件路径
		m_tStepCodeLock.lock();
		tagCodeWithParm tCodeWithParm;
		tCodeWithParm.tCode=OperationDatabase_obtainFilePath;
		m_tStepCode.enqueue(tCodeWithParm);
		m_tStepCodeLock.unlock();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"obtainFilePath fail as database thread is not running";
	}
	return nReturn;
}

int OperationDatabase::priObtainFilePath( QString &sWriteFilePath)
{
	//0:覆盖写；1：续写文件；2：没有文件可写
	int nStep=obtainFilePath_getDrive;
	bool bStop=false;
	int bFlag=2;
	QString sDiskList;//数据库中可用的盘符集
	QString sUsableDiks;//可用的盘符
	QString sFilePath;//文件路径
	while(bStop==false){
		switch(nStep){
		case obtainFilePath_getDrive:{
			//获取可录像盘符
			sUsableDiks=getUsableDiskEx(sDiskList);
			if (!(sUsableDiks.isEmpty()&&sDiskList.isEmpty()))
			{
				if (sUsableDiks.isEmpty())
				{
					nStep=obtainFilePath_diskFull;
				}else{
					nStep=obtainFilePath_diskUsable;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"obtainFilePath fail as there is no disk for record";
				nStep=obtainFilePath_fail;
			}
									 }
									 break;
		case obtainFilePath_diskUsable:{
			//有剩余空间的可录像的盘符
			sFilePath=getLatestItemEx(sUsableDiks);
			bFlag=1;
			nStep=obtainFilePath_createFile;
									   }
									   break;
		case obtainFilePath_diskFull:{
			//判断是否覆盖录像
			if (m_tSystemDatabaseInfo.bIsRecover)
			{
				//每个盘符都已经录满
				QStringList sDiskListInDatabase=sDiskList.split(":");
				QList<QString> tFilePathList;
				foreach(QString sDiskEx,sDiskListInDatabase){
					sDiskEx=sDiskEx+":";
					QString sFilePathItem=getOldestItemEx(sDiskEx);
					if (!sFilePathItem.isEmpty())
					{
						tFilePathList.append(sFilePathItem);
					}else{
						//do nothing
					}
				}
				if (!tFilePathList.isEmpty())
				{
					//查找出几个盘符最早的文件
					//把查找到最早文件的数据库内的相关信息删除
					//fix me
					quint64 uiStartTime=QDateTime::currentDateTime().toTime_t();
					QString sOldestFile;
					QString sOldestFileTemp;
					for (int i=0;i<tFilePathList.size();i++)
					{
						QString sFilePathItem=tFilePathList.at(i);
						if (QFile::exists(sFilePathItem))
						{
							QFile tFile;
							tFile.setFileName(sFilePathItem);
							if (tFile.open(QIODevice::ReadOnly|QIODevice::Text))
							{
								m_tFileHead.clear();
								m_tFileHead=tFile.read(sizeof(tagFileHead));
								if (m_tFileHead.size()>=sizeof(tagFileHead))
								{
									tagFileHead *pFileHead=(tagFileHead*)m_tFileHead.data();
									if (NULL!=pFileHead)
									{
										if (m_tFileHead.contains("JUAN"))
										{
											if (pFileHead->uiStart<uiStartTime)
											{
												uiStartTime=pFileHead->uiStart;
												sOldestFile=sFilePathItem;
											}else{
												//do nothing
												sOldestFileTemp=sFilePathItem;
											}
										}else{
											//do nothing
											sOldestFileTemp=sFilePathItem;
										}
									}else{
										//do nothing
										qDebug()<<__FUNCTION__<<__LINE__<<"pFileHead should not been null";
										abort();
									}
									tFile.close();
								}else{
									sOldestFile=sFilePathItem;
									tFile.close();
									break;
								}
							}else{
								//do nothing
								QVariantMap tInfo;
								tInfo.insert("nDamage",1);
								priSetRecordFileStatusEx(sFilePathItem,tInfo);
							}
						}else{
							//do nothing
							sOldestFileTemp=sFilePathItem;
						}
					}
					if (sOldestFile.isEmpty())
					{
						sOldestFile=sOldestFileTemp;
					}else{
						//do nothing
					}
					if (sOldestFile.isEmpty())
					{
						qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as there is no disk space";
						nStep=obtainFilePath_fail;
					}else{
						sFilePath=sOldestFile;
						priClearInfoInDatabase(sFilePath);
						bFlag=0;
						nStep=obtainFilePath_createFile;
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as there is no disk space";
					nStep=obtainFilePath_fail;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as there is no disk space and bIsRecover is false";
				nStep=obtainFilePath_fail;
			}
									 }
									 break;
		case obtainFilePath_createFile:{
			//如果文件不存在，则创建文件
			if (createNewFile(sFilePath))
			{
				nStep=obtainFilePath_success;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as createNewFile fail";
				QVariantMap tInfo;
				tInfo.insert("nLock",1);
				priSetRecordFileStatusEx(sFilePath,tInfo);
				nStep=obtainFilePath_fail;
			}
									   }
									   break;
		case obtainFilePath_success:{
			//获取录像文件路径成功
			QVariantMap tInfo;
			tInfo.insert("nLock",1);
			priSetRecordFileStatusEx(sFilePath,tInfo);
			nStep=obtainFilePath_end;
									}
									break;
		case obtainFilePath_fail:{
			//获取录像文件路径失败
			bFlag=2;
			nStep=obtainFilePath_end;
								 }
								 break;
		case obtainFilePath_end:{
			bStop=true;
								}
								break;
		}
	}
	sWriteFilePath=sFilePath;
	return bFlag;
}

bool OperationDatabase::updateRecordDatabase( QList<int> tIdList,QVariantMap tInfo,QString sFilePath )
{
	if (QThread::isRunning())
	{
		tagCodeWithParm tCodeWithParmInfo;
		tCodeWithParmInfo.tCode=OperationDatabase_updateRecordDatabase;
		QString sIdList;
		for (int i=0;i<tIdList.size();i++)
		{
			if (i==0)
			{
				sIdList=QString::number(tIdList.value(i));
			}else{
				sIdList=sIdList+","+QString::number(tIdList.value(i));
			}
		}
		QString sInfo;
		QVariantMap::const_iterator tItem=tInfo.constBegin();
		while(tItem!=tInfo.constEnd()){
			if (tItem==tInfo.constBegin())
			{
				sInfo=tItem.key()+","+tItem.value().toString();
			}else{
				sInfo=sInfo+","+tItem.key()+","+tItem.value().toString();
			}	
			tItem++;
		}
		QVariantMap tCodeParm;
		tCodeParm.insert("tIdList",sIdList);
		tCodeParm.insert("tInfo",sInfo);
		tCodeParm.insert("sFilePath",sFilePath);
		tCodeWithParmInfo.tInfo=tCodeParm;
		m_tStepCodeLock.lock();
		m_tStepCode.enqueue(tCodeWithParmInfo);
		m_tStepCodeLock.unlock();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"updateRecordDatabase fail as database thread is not running";
		return false;
	}
}

bool OperationDatabase::updateSearchDatabase( QList<int> tIdList,QVariantMap tInfo,QString sFilePath )
{
	if (QThread::isRunning())
	{
		tagCodeWithParm tCodeWithParmInfo;
		tCodeWithParmInfo.tCode=OperationDatabase_updateSearchDatabase;
		QString sIdList;
		for (int i=0;i<tIdList.size();i++)
		{
			if (i==0)
			{
				sIdList=QString::number(tIdList.value(i));
			}else{
				sIdList=sIdList+","+QString::number(tIdList.value(i));
			}
		}
		QString sInfo;
		QVariantMap::const_iterator tItem=tInfo.constBegin();
		while(tItem!=tInfo.constEnd()){
			if (tItem==tInfo.constBegin())
			{
				sInfo=tItem.key()+","+tItem.value().toString();
			}else{
				sInfo=sInfo+","+tItem.key()+","+tItem.value().toString();
			}	
			tItem++;
		}
		QVariantMap tCodeParm;
		tCodeParm.insert("tIdList",sIdList);
		tCodeParm.insert("tInfo",sInfo);
		tCodeParm.insert("sFilePath",sFilePath);
		tCodeWithParmInfo.tInfo=tCodeParm;
		m_tStepCodeLock.lock();
		m_tStepCode.enqueue(tCodeWithParmInfo);
		m_tStepCodeLock.unlock();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"updateSearchDatabase fail as database thread is not running";
		return false;
	}
}

bool OperationDatabase::createSearchDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId )
{
	if (QThread::isRunning())
	{
		tagCodeWithParm tCodeWithParm;
		QVariantMap tInfo;
		tInfo.insert("nChannel",nChannel);
		tInfo.insert("uiStartTime",uiStartTime);
		tInfo.insert("uiEndTime",uiEndTime);
		tInfo.insert("uiType",uiType);
		tInfo.insert("sFileName",sFileName);
		tInfo.insert("uiItemId",uiItemId);
		tCodeWithParm.tCode=OperationDatabase_createSearchDatabaseItem;
		tCodeWithParm.tInfo=tInfo;
		m_tStepCodeLock.lock();
		m_tStepCode.enqueue(tCodeWithParm);
		m_tStepCodeLock.unlock();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"createSearchDatabaseItem fail as database thread is not running";
		return false;
	}
}

bool OperationDatabase::createRecordDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId )
{
	if (QThread::isRunning())
	{
		tagCodeWithParm tCodeWithParm;
		QVariantMap tInfo;
		tInfo.insert("nChannel",nChannel);
		tInfo.insert("uiStartTime",uiStartTime);
		tInfo.insert("uiEndTime",uiEndTime);
		tInfo.insert("uiType",uiType);
		tInfo.insert("sFileName",sFileName);
		tInfo.insert("uiItemId",uiItemId);
		tCodeWithParm.tCode=OperationDatabase_createRecordDatabaseItem;
		tCodeWithParm.tInfo=tInfo;
		m_tStepCodeLock.lock();
		m_tStepCode.enqueue(tCodeWithParm);
		m_tStepCodeLock.unlock();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as database thread is not running";
		return false;
	}
}

void OperationDatabase::setRecordFileStatus( QString sFilePath,QVariantMap tInfo )
{
	if (QThread::isRunning())
	{
		QString sInfo;
		QVariantMap::const_iterator tItem=tInfo.constBegin();
		while(tItem!=tInfo.constEnd()){
			if (tItem==tInfo.constBegin())
			{
				sInfo=tItem.key()+","+tItem.value().toString();
			}else{
				sInfo=sInfo+","+tItem.key()+","+tItem.value().toString();
			}	
			tItem++;
		}
		QVariantMap tInfoParm;
		tInfoParm.insert("tInfo",sInfo);
		tInfoParm.insert("sFilePath",sFilePath);
		tagCodeWithParm tCodeWithParm;
		tCodeWithParm.tCode=OperationDatabase_setRecordFileStatus;
		tCodeWithParm.tInfo=tInfoParm;
		m_tStepCodeLock.lock();
		m_tStepCode.enqueue(tCodeWithParm);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
	}
}

void OperationDatabase::reloadSystemDatabase()
{
	if (QThread::isRunning())
	{
		tagCodeWithParm tCodeWithParm;
		tCodeWithParm.tCode=OperationDatabase_reloadSystemDatabase;
		m_tStepCodeLock.lock();
		m_tStepCode.enqueue(tCodeWithParm);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
	}
}

void OperationDatabase::clearInfoInDatabaseEx( QString sFilePath )
{
	if (QThread::isRunning())
	{
		tagCodeWithParm tCodeWithParm;
		QVariantMap tInfo;
		tInfo.insert("sFilePath",sFilePath);
		m_tStepCodeLock.lock();
		tCodeWithParm.tCode=OperationDatabase_clearInfoInDatabase;
		tCodeWithParm.tInfo=tInfo;
		m_tStepCode.enqueue(tCodeWithParm);
		m_tStepCodeLock.unlock();
	}else{
		//do nothing
	}
}

bool OperationDatabase::startOperationDatabase()
{
	if (!QThread::isRunning())
	{
		QThread::start();
		m_bStop=false;
		int nSleepCount=0;
		while(!QThread::isRunning()){
			msleep(1);
			nSleepCount++;
			if (nSleepCount>1000&&nSleepCount%1000==0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"startOperationDatabase fail ,please check";
			}else{
				//do nothing
			}
		}
	}else{
		//do nothing
	}
	return true;
}

bool OperationDatabase::stopOperationDatabase()
{
	if (QThread::isRunning())
	{
		m_bStop=true;
		int nSleepCount=0;
		while(QThread::isRunning()){
			msleep(1);
			nSleepCount++;
			if (nSleepCount>4000&&nSleepCount%1000==0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"stopOperationDatabase fail ,please check";
			}else{
				//do nothing
			}
		}
	}else{
		//do nothing
	}
	return true;
}

bool OperationDatabase::isRecordDataExistItem()
{
	if (QThread::isRunning())
	{
		tagCodeWithParm tCodeWithParm;
		tCodeWithParm.tCode=OperationDatabase_isRecordDataExistItem;
		m_tIsRecordDataExistItemResult.clear();
		m_tStepCodeLock.lock();
		m_tStepCode.enqueue(tCodeWithParm);
		m_tStepCodeLock.unlock();
		int nSleepCount=0;
		while(m_tIsRecordDataExistItemResult.isEmpty()){
			msleep(10);
			nSleepCount++;
			if (nSleepCount>300)
			{
				return false;
			}
		}
		bool bFlag=m_tIsRecordDataExistItemResult.dequeue();
		return bFlag;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"isRecordDataExistItemEx fail as database thread is not runing";
		return false;
	}
}

bool OperationDatabase::getMaxDatabaseId( quint64 &uiMaxRecordId,quint64 &uiMaxSearchId ,QString sFilePath)
{
	if (QThread::isRunning())
	{
		tagCodeWithParm tCodeWithParm;
		tCodeWithParm.tCode=OperationDatabase_getMaxDatabaseId;
		QVariantMap tInfo;
		tInfo.insert("sFilePath",sFilePath);
		tCodeWithParm.tInfo=tInfo;
		m_tGetMaxDatabaseIdResult.clear();
		m_tStepCodeLock.lock();
		m_tStepCode.enqueue(tCodeWithParm);
		m_tStepCodeLock.unlock();
		int nSleepCount=0;
		while(m_tGetMaxDatabaseIdResult.isEmpty()){
			nSleepCount++;
			msleep(10);
			if (nSleepCount>500)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"getMaxDatabaseId fail as time cost 5s";
				return false;
			}
		}
		QVariantMap tInfoResult;
		tInfoResult=m_tGetMaxDatabaseIdResult.dequeue();
		uiMaxSearchId=tInfoResult.value("uiMaxSearchId").toUInt();
		uiMaxRecordId=tInfoResult.value("uiMaxRecordId").toUInt();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getMaxDatabaseId fail as database thread is not runing";
		return false;
	}
}

bool OperationDatabase::createNewFile( QString sFilePath )
{
	QFile tFile;
	tFile.setFileName(sFilePath);
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
				qDebug()<<__FUNCTION__<<__LINE__<<"mkpath fail"<<sDirPath;
				abort();
			}
		}else{
			//keep going
		}
		if (tFile.open(QIODevice::ReadWrite))
		{
			tFile.close();
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"new file fail"<<sFilePath;
			abort();
			return false;
		}
	}else{
		return true;
	}
}

QString OperationDatabase::getUsableDiskEx( QString &sDiskLisk )
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
			//删除1.1.13版本录像,按天删除
			if ( m_tFreeDisk.freeDiskEx(m_tSystemDatabaseInfo.sAllRecordDisk,m_tSystemDatabaseInfo.uiDiskReservedSize))
			{
				nStep=0;
			}else{
				nStep=2;
			}
			   }
			   break;
		case 2:{
			//end
			bStop=true;
			   }
		}
	}
	return sDiskFound;
}

QString OperationDatabase::getLatestItemEx( QString sDisk )
{
	//获取最新的文件路径
	//step1:查找数据库中最新的文件路径,没有的话，直接创建
	//step2:根据数据库最新的文件路径，判断文件是否写满，为满则续写，满的话则新起一个文件
	QString sFilePath=getLatestItemExx(sDisk);
	if (sFilePath.isEmpty())
	{
		sFilePath=createLatestItemEx(sDisk);
	}else{
		if (checkFileIsFull(sFilePath))
		{
			sFilePath=createLatestItemEx(sDisk);
		}else{
			//do nothing
		}
	}
	return sFilePath;
}

bool OperationDatabase::priUpdateRecordDatabase( QList<int> tIdList,QVariantMap tInfo,QString sFilePath )
{
	//可被更新的条目：nEndTime
	if (tIdList.size()==0)
	{
		return true;
	}else{
		//keep going
	}
	if (tInfo.contains("nEndTime"))
	{
		QString sDatabasePath=sFilePath.left(1)+":/recEx/record.db";
		QFile tDatabaseFile;
		tDatabaseFile.setFileName(sDatabasePath);
		if (tDatabaseFile.exists())
		{
			QSqlDatabase *pDatabase=NULL;
			pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
			if (NULL!=pDatabase)
			{
				QList<QString> tKeyList=tInfo.keys();
				QString sKey=tKeyList.at(0);
				QSqlQuery _query(*pDatabase);
				QString sIdList;
				for (int i=0;i<tIdList.size();i++)
				{
					if (i==0)
					{
						sIdList=QString::number(tIdList.value(i));
					}else{
						sIdList=sIdList+","+QString::number(tIdList.value(i));
					}	
				}
				QString sCommand="update record set "+sKey+QString("=%1").arg(tInfo.value(sKey).toInt())+" where id in "+"("+sIdList+")";
				if (_query.exec(sCommand))
				{
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail"<<sCommand;
					abort();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
				abort();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"updateRecordDatabase fail as tDatabaseFile is no exists"<<sDatabasePath;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"undefined updateRecord type,i will abort the thread";
		abort();
	}
	return false;
}

bool OperationDatabase::priUpdateSearchDatabase( QList<int> tIdList,QVariantMap tInfo,QString sFilePath )
{
	//可被更新的条目：nEndTime nStartTime 
	if (tIdList.size()==0)
	{
		return true;
	}else{
		//keep going
	}
	if (tInfo.contains("nEndTime")||tInfo.contains("nStartTime"))
	{
		QString sDatabasePath=sFilePath.left(1)+":/recEx/record.db";
		QFile tDatabaseFile;
		tDatabaseFile.setFileName(sDatabasePath);
		if (tDatabaseFile.exists())
		{
			QSqlDatabase *pDatabase=NULL;
			pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
			if (NULL!=pDatabase)
			{
				QList<QString> tKeyList=tInfo.keys();
				QString sKey=tKeyList.at(0);
				QSqlQuery _query(*pDatabase);
				QString sIdList;
				for (int i=0;i<tIdList.size();i++)
				{
					if (i==0)
					{
						sIdList=QString::number(tIdList.value(i));
					}else{
						sIdList=sIdList+","+QString::number(tIdList.value(i));
					}	
				}
				QString sCommand="update search_record set "+sKey+QString("=%1").arg(tInfo.value(sKey).toInt())+" where id in "+"("+sIdList+")";
				if (_query.exec(sCommand))
				{
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
					abort();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
				abort();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"updateSearchDatabase fail as tDatabaseFile is no exists"<<sDatabasePath;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"undefined updateSearchDatabase type,i will abort the thread";
		abort();
	}
	return false;
}

bool OperationDatabase::priCreateSearchDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId )
{
	if (uiStartTime>uiEndTime)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"createSearchDatabaseItem fail as uiStartTime>uiEndTime"<<uiStartTime<<uiEndTime;
		abort();
		return false;
	}else{
		//keep going
	}
	if (uiType==MOTIONRECORD||uiType==TIMERECORD||uiType==MANUALRECORD)
	{
		//keep going
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as uiType is undefine;"<<uiType;
		abort();
	}
	QString sDatabasePath=sFileName.left(1)+":/recEx/record.db";
	QFile tDatabaseFile;
	tDatabaseFile.setFileName(sDatabasePath);
	if (tDatabaseFile.exists())
	{
		//keep going
	}else{
		//创建搜索表
		if (createRecordDatabase(sDatabasePath))
		{
			//keep going
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"createSearchDatabaseItem fail as createRecordDatabase fail";
			return false;
		}
	}
	QSqlDatabase *pDatabase=NULL;
	pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
	if (NULL!=pDatabase)
	{
		QSqlQuery _query(*pDatabase);
		QString sCommand=QString("insert into search_record(id,nWndId,nRecordType,nStartTime,nEndTime)values(%1,%2,%3,%4,%5)").arg(uiItemId).arg(nChannel).arg(uiType).arg(uiStartTime).arg(uiEndTime);
		if (_query.exec(sCommand))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail"<<sCommand;
			abort();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
		abort();
	}
	return false;
}

bool OperationDatabase::priCreateRecordDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,quint64 &uiItemId )
{
	if (uiStartTime>uiEndTime)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as uiStartTime>uiEndTime :"<<uiStartTime<<uiEndTime;
		abort();
		return false;
	}else{
		//keep going
	}
	if (uiType==MOTIONRECORD||uiType==TIMERECORD||uiType==MANUALRECORD)
	{
		//keep going
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as uiType is undefine;"<<uiType;
		abort();
	}
	QString sDatabasePath=sFileName.left(1)+":/recEx/record.db";
	QFile tDatabaseFile;
	tDatabaseFile.setFileName(sFileName);
	if (tDatabaseFile.exists())
	{
		//keep going
	}else{
		if (createRecordDatabase(sDatabasePath))
		{
			//keep going
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as createRecordDatabase fail";
			abort();
			return false;
		}
	}
	QSqlDatabase *pDatabase=NULL;
	pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
	if (NULL!=pDatabase)
	{
		QSqlQuery _query(*pDatabase);
		QString sCommand=QString("insert into record(id,nWndId,nRecordType,nStartTime,nEndTime,sFilePath)values(%1,%2,%3,%4,%5,'%6')").arg(uiItemId).arg(nChannel).arg(uiType).arg(uiStartTime).arg(uiEndTime).arg(sFileName);
		if (_query.exec(sCommand))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as exec cmd fail:"<<sCommand;
			abort();
			return false;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail as pDatabase should not been null";
		abort();
		return false;
	}
	return false;
}

void OperationDatabase::priSetRecordFileStatusEx( QString sFilePath,QVariantMap tInfo )
{
	QString sDisk=sFilePath.left(1)+":/recEx/record.db";
	QFile tDatabaseFile;
	tDatabaseFile.setFileName(sDisk);
	if (tDatabaseFile.exists())
	{
		//keep going
	}else{
		if (createRecordDatabase(sDisk))
		{
			//keep going
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"priSetRecordFileStatus fail as file is not exist"<<sDisk;
		}
	}
	QSqlDatabase *pDatabase=NULL;
	pDatabase=initMgrDataBase(sDisk,(quintptr*)this);
	if (NULL!=pDatabase)
	{
		QSqlQuery _query(*pDatabase);
		QString sCommand;
		if (tInfo.contains("nLock")||tInfo.contains("nDamage")||tInfo.contains("nInUse"))
		{
			//判断是否有这个文件的条目，有的话，就更新，没的话就创建
			sCommand=QString("select id from RecordFileStatus where sFilePath='%1'").arg(sFilePath);
			if (_query.exec(sCommand))
			{
				if (_query.next())
				{
					//更新
					sCommand.clear();
					int nId=_query.value(0).toInt();
					QList<QString> tKeyList=tInfo.keys();
					QString sKey=tKeyList.at(0);
					sCommand="update RecordFileStatus set "+sKey+QString("=%1 where id=%2").arg(tInfo.value(sKey).toInt()).arg(nId);
					if (_query.exec(sCommand))
					{
						//done
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
						abort();
					}
				}else{
					//创建
					sCommand.clear();
					quint64 nFileNum=countFileNum(sFilePath);
					sCommand=QString("insert into RecordFileStatus(sFilePath,nLock,nDamage,nInUse,nFileNum)values('%1',0,0,0,%2)").arg(sFilePath).arg(nFileNum);
					if (_query.exec(sCommand))
					{
						//keep going
						QList<QString> tKeyList=tInfo.keys();
						QString sKey=tKeyList.at(0);
						sCommand.clear();
						sCommand="update RecordFileStatus set "+sKey+QString("=%1 where sFilePath='%2'").arg(tInfo.value(sKey).toInt()).arg(sFilePath);
						if (_query.exec(sCommand))
						{
							//done
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
							abort();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
						abort();
					}
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail :"<<sCommand;
				abort();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"undefined type";
			abort();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not be null";
		abort();
	}
}

void OperationDatabase::priReloadSystemDatabase()
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

void OperationDatabase::priClearInfoInDatabase( QString sFilePath )
{
	//删除 record 表中记录
	//删除 search_record表中记录
	//更新 RecordFileStatus
	QString sDatabasePath=sFilePath.left(1)+":/recEx/record.db";
	QFile tDatabaseFile;
	tDatabaseFile.setFileName(sDatabasePath);
	if (tDatabaseFile.exists())
	{
		QSqlDatabase *pDatabase=NULL;
		pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
		if (NULL!=pDatabase)
		{
			QSqlQuery _query(*pDatabase);
			QString sCommand;
			//删除 search_record表中记录
			sCommand=QString("select nWndId,id from record where sFilePath='%1'").arg(sFilePath);
			QList<quint64 > tRecordIdList;
			if (_query.exec(sCommand))
			{
				QList<int > tWndIdList;
				while(_query.next()){
					tWndIdList.append(_query.value(0).toInt());
					tRecordIdList.append(_query.value(1).toUInt());
				}
				if (!tWndIdList.isEmpty())
				{
					sCommand.clear();
					sCommand=QString("select max(nEndTime) from record where sFilePath ='%1'").arg(sFilePath);
					if (_query.exec(sCommand))
					{
						if (_query.next())
						{
							quint64 uiEndTime=_query.value(0).toUInt();
							sCommand=QString("delete from search_record where nEndTime<=%1").arg(uiEndTime);
							if (_query.exec(sCommand))
							{
								QString sWndIdList;
								for (int i=0;i<tWndIdList.size();i++)
								{
									if (i==0)
									{
										sWndIdList=QString::number(tWndIdList.value(i));
									}else{
										sWndIdList=sWndIdList+","+QString::number(tWndIdList.value(i));
									}
								}
								sCommand=QString("update search_record set nStartTime=%1 where nStartTime<%2 and nWndId in ").arg(uiEndTime).arg(uiEndTime)+"("+sWndIdList+")";
								if (_query.exec(sCommand))
								{
									//keep going
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
				}else{
					//do nothing
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
				abort();
			}
			//删除 record 表中记录
			if (tRecordIdList.size()>0)
			{
				QString sReoveRecordIdList;
				for(int i=0;i<tRecordIdList.size();i++){
					if (i==0)
					{
						sReoveRecordIdList=QString::number(tRecordIdList.value(i));
					}else{
						sReoveRecordIdList=sReoveRecordIdList+","+QString::number(tRecordIdList.value(i));
					}
				}
				sCommand=QString("delete from record where id in ")+"("+sReoveRecordIdList+")";
				if (_query.exec(sCommand))
				{
					//keep going
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
					abort();
				}
			}else{
				//do nothing
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

bool OperationDatabase::PriIsRecordDataExistItem()
{
	//1.1.13版本之前的数据库是否有条目
	//1.1.13版本之后的数据是否有条目
	QString sDiskFound;
	QString sDiskLisk=m_tSystemDatabaseInfo.sAllRecordDisk;
	QStringList tDiskList=m_tSystemDatabaseInfo.sAllRecordDisk.split(":");
	if (tDiskList.size()!=0)
	{
		quint64 uiFreeByteAvailable=0;
		quint64 uiTotalNumberOfBytes=0;
		quint64 uiTotalNumberOfFreeByte=0;
		foreach(QString sDiskItem,tDiskList){
			QString sDiskEx=sDiskItem+":";
			//step1:1.1.13版本之前的数据库是否有条目
			QString sDatabasePath=sDiskEx+"rec/record.db";
			QFile tDatabaseFile;
			tDatabaseFile.setFileName(sDatabasePath);
			if (tDatabaseFile.exists())
			{
				QSqlDatabase *pDatabase=NULL;
				pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
				if (NULL!=pDatabase)
				{
					QSqlQuery _query(*pDatabase);
					QString sCommand=QString("select *from local_record");
					if (_query.exec(sCommand))
					{
						if (_query.next())
						{
							return true;
						}else{
							// do nothing
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"isRecordDataExistItem fail as exec cmd fail:"<<sCommand;
						abort();
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as pDatabase should not been null";
					abort();
				}
			}else{
				//do nothing
			}
			//step1:1.1.13版本之后的数据是否有条目
			sDatabasePath.clear();
			sDatabasePath=sDiskEx+"recEx/record.db";
			tDatabaseFile.setFileName(sDatabasePath);
			if (tDatabaseFile.exists())
			{
				QSqlDatabase *pDatabase=NULL;
				pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
				if (NULL!=pDatabase)
				{
					QSqlQuery _query(*pDatabase);
					QString sCommand=QString("select *from record");
					if (_query.exec(sCommand))
					{
						if (_query.next())
						{
							return true;
						}else{
							//do nothing
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"isRecordDataExistItem fail as exec cmd fail:"<<sCommand;
						abort();
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"terminate the thread as pDatabase should not been null";
					abort();
				}
			}else{
				//do nothing
			}
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"there is no disk for record as tDiskList size==0";
	}
	return false;
}

bool OperationDatabase::PriGetMaxDatabaseId( quint64 &uiMaxRecordId,quint64 &uiMaxSearchId,QString sFilePath )
{
	QString sRecordDatabasePath=sFilePath.left(1)+":/recEx/record.db";
	QFile tFile;
	tFile.setFileName(sRecordDatabasePath);
	if (tFile.exists())
	{
		QSqlDatabase *pDatabase=initMgrDataBase(sRecordDatabasePath,(quintptr*)this);
		if (NULL!=pDatabase)
		{
			QSqlQuery _query(*pDatabase);
			QString sCommand;
			sCommand=QString("select max(id) from record");
			if (_query.exec(sCommand))
			{
				if (_query.next())
				{
					uiMaxRecordId=_query.value(0).toUInt();
				}else{
					uiMaxRecordId=0;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
				abort();
			}
			sCommand.clear();
			sCommand=QString("select max(id) from search_record");
			if (_query.exec(sCommand))
			{
				if (_query.next())
				{
					uiMaxSearchId=_query.value(0).toUInt();
				}else{
					uiMaxSearchId=0;
				}	
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
				abort();
			}
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getOldestItem fail as pDataBase is null";
			abort();
		}
	}else{
		uiMaxSearchId=0;
		uiMaxRecordId=0;
		return true;
	}
	return false;
}

QString OperationDatabase::getOldestItemEx( QString sDisk )
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
			quint64 uiCurrentDateTime=QDateTime::currentDateTime().toTime_t();
			quint64 uiEndTime=0;
			QList<QString> tAbandonFilePath;
			while(bStop==false){
				switch(nStep){
				case 0:{
					//查找最早的记录
					sCommand.clear();
					sCommand=QString("select id,sFilePath,nStartTime from record where nStartTime=(select min(nStartTime) from record where nStartTime <%1 and nStartTime>%2)").arg(uiCurrentDateTime).arg(uiEndTime);
					if (_query.exec(sCommand))
					{
						QString sFilePathTemp;
						bool bFound=false;
						while(_query.next()&&bFound==false){
							sFilePathTemp=_query.value(1).toString();
							if (tAbandonFilePath.contains(sFilePathTemp))
							{
								//keep going
								sFilePathTemp.clear();
							}else{
								bFound=true;
							}
						}
						if (!sFilePathTemp.isEmpty())
						{
							sCommand.clear();
							sCommand=QString("select max(nEndTime) from record where sFilePath='%1'").arg(sFilePathTemp);
							if (_query.exec(sCommand))
							{
								if (_query.next())
								{
									uiEndTime=_query.value(0).toUInt();
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
												tAbandonFilePath.append(sFilePathTemp);
												sFilePathTemp.clear();
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
									qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail"<<sCommand;
									abort();
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

QString OperationDatabase::getLatestItemExx( QString sDisk )
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
					sCommand=QString("select id,nLock,nDamage from RecordFileStatus where sFilePath='%1'").arg(sFilePath);
					if (_query.exec(sCommand))
					{
						if (_query.next())
						{
							if (_query.value(1).toInt()==1||_query.value(2).toInt()==1)
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

bool OperationDatabase::checkFileIsFull( QString sFilePath )
{
	QFile tFile;
	tFile.setFileName(sFilePath);
	if (tFile.exists())
	{
		if (tFile.open(QIODevice::ReadWrite))
		{
			QByteArray tFilehead=tFile.read(sizeof(tagFileHead));
			if (tFilehead.size()<sizeof(tagFileHead))
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"undefined file,it can not bee use";
				QVariantMap tInfo;
				tInfo.insert("nDamage",1);
				priSetRecordFileStatusEx(sFilePath,tInfo);
				return true;
			}else{
				tagFileHead *pFileHead=(tagFileHead*)tFilehead.data();
				char *pChar="JUAN";
				if (memcmp(pChar,pFileHead->ucMagic,4)==0)
				{
					if (pFileHead->uiIndex<BUFFERSIZE*1024*1024-1024*1024)
					{
						return false;
					}else{
						return true;
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"undefined file,it can not bee use";
					QVariantMap tInfo;
					tInfo.insert("nDamage",1);
					priSetRecordFileStatusEx(sFilePath,tInfo);
					return true;
				}
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"open file fail,it can not been use, create new file"<<sFilePath;
			QVariantMap tInfo;
			tInfo.insert("nDamage",1);
			priSetRecordFileStatusEx(sFilePath,tInfo);
			return true;
		}
	}else{
		return false;
	}
}
QString OperationDatabase::createLatestItemEx( QString sDisk )
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
				if (_query.next())
				{
					sFilePath=_query.value(1).toString();
				}else{
					QString sCommand=QString("select id,nFileNum from RecordFileStatus where nFileNum=(select max(nFileNum) from RecordFileStatus)");
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
							sFilePath=sFilePathTemp;
						}else{
							sFilePath=sDisk+"/recEx/0000/0000/0000/0000.dat";
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem fail exec sCommand fail :"<<sCommand;
						abort();
					}
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"createLatestItem fail exec sCommand fail :"<<sCommandTest;
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



