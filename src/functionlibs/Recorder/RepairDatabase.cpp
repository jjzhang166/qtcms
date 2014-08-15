#include "RepairDatabase.h"
#include <guid.h>
#include <IDisksSetting.h>
#include <QStringList>
#include <QDebug>
#include <QtSql>
#include <QFile>
typedef struct __tagRepairDataBaseInfo{
	QString sDatabaseName;
	QSqlDatabase *pDatabase;
	int nCount;
	QList<int *> tThis;
}tagRepairDataBaseInfo;
QMultiMap<QString ,tagRepairDataBaseInfo> g_tRepairDataBase;
QString g_sRepairSearchRecord="C:/CMS_RECORD/search_record.db";
QSqlDatabase *initRepairDataBase(QString sDatabaseName,int *nThis){
	if (g_tRepairDataBase.contains(sDatabaseName))
	{
		if (g_tRepairDataBase.find(sDatabaseName).value().tThis.contains(nThis))
		{
			//do nothing
		}else{
			g_tRepairDataBase.find(sDatabaseName).value().nCount++;
			g_tRepairDataBase.find(sDatabaseName).value().tThis.append(nThis);
		}
		return g_tRepairDataBase.find(sDatabaseName).value().pDatabase;
	}else{
		tagRepairDataBaseInfo tDataBaseInfo;
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
		g_tRepairDataBase.insert(sDatabaseName,tDataBaseInfo);
		return tDataBaseInfo.pDatabase;
	}
}
void deInitRepairDataBase(int *nThis){
	QMultiMap<QString,tagRepairDataBaseInfo>::iterator it;
	QStringList sDeleteList;
	for (it=g_tRepairDataBase.begin();it!=g_tRepairDataBase.end();it++)
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
		g_tRepairDataBase.remove(sDeleteList.at(i));
	}
}
RepairDatabase::RepairDatabase(void)
{
}


RepairDatabase::~RepairDatabase(void)
{
}

int RepairDatabase::fixExceptionalData()
{
	//0:成功，1：没有进行修复行为，2：存在修复失败的条目
	//对于录像表，删除开始时间等于结束时间的条目，并删除对于的文件，最多丢失崩溃前一分钟的录像
	//对于搜索表，删除开始时间等于结束时间的条目,最多丢失崩溃前一分钟中的搜索记录
	int nRet1=repairRecordDatabase();
	int nRet2=repairSearchDatabase();
	deInitRepairDataBase((int *)this);
	if (nRet2==0&&nRet1==0)
	{
		return 0;
	}else if(nRet1==2||nRet2==2){
		return 2;
	}else{
		return 1;
	}
}

int RepairDatabase::repairRecordDatabase()
{
	//0:成功，1：没有进行修复行为，2：存在修复失败的条目
	//对于录像表，删除开始时间等于结束时间的条目，并删除对于的文件，最多丢失崩溃前一分钟的录像
	IDisksSetting *pDisksSetting=NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		QString sAllDisks;
		if (0==pDisksSetting->getUseDisks(sAllDisks))
		{
			QStringList sDiskLisk=sAllDisks.split(":");
			if (sDiskLisk.size()!=0)
			{
				foreach(QString sDiskItem,sDiskLisk){
					QString sDataBasePath=sDiskItem+":/REC/record.db";
					if (QFile::exists(sDataBasePath))
					{
						QSqlDatabase *pDataBase=initRepairDataBase(sDataBasePath,(int* )this);
						if (pDataBase!=NULL)
						{
							QSqlQuery _query(*pDataBase);
							QString sCommand=QString("select id,path from local_record where end_time<=start_time");
							if (_query.exec(sCommand))
							{
								while(_query.next()){
									int nId=_query.value(0).toInt();
									QString sFilePath=_query.value(1).toString();
									//删除记录
									QSqlQuery _query2(*pDataBase);
									sCommand.clear();
									sCommand=QString("delete from local_record where id=%1").arg(nId);
									if (_query2.exec(sCommand))
									{
									}else{
										qDebug()<<__FUNCTION__<<__LINE__<<"repairRecordDatabase fail as exec cmd fail:"<<sCommand;
									}
									//删除文件
									if (QFile::exists(sFilePath))
									{
										if (QFile::remove(sFilePath))
										{
										}else{
											qDebug()<<__FUNCTION__<<__LINE__<<"repairRecordDatabase fail as remove file fail::"<<sFilePath;
										}				
									}else{
										//do nothing
									}
								}
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"repairRecordDatabase fail as exec cmd fail:"<<sCommand;
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"repairRecordDatabase fail as initRepairDataBase fail:"<<sDataBasePath;
							pDisksSetting->Release();
							pDisksSetting=NULL;
							return 1;
						}
					}else{
						//do nothing
					}
				}
			}else{
				//do nothing
			}
			pDisksSetting->Release();
			pDisksSetting=NULL;
			return 0;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"repairRecordDatabase fail as getUseDisks fail";
			pDisksSetting->Release();
			pDisksSetting=NULL;
			return 1;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"repairRecordDatabase fail as pDisksSetting is null";
		return 1;
	}
}

int RepairDatabase::repairSearchDatabase()
{
	//0:成功，1：没有进行修复行为，2：存在修复失败的条目
	//对于搜索表，删除开始时间等于结束时间的条目,最多丢失崩溃前一分钟中的搜索记录
	if (QFile::exists(g_sRepairSearchRecord))
	{
		QSqlDatabase *pDataBase=initRepairDataBase(g_sRepairSearchRecord,(int* )this);
		if (pDataBase!=NULL)
		{
			QSqlQuery _query(*pDataBase);
			QString sCommmad=QString("delete from search_record where start_time >=end_time");
			if (_query.exec(sCommmad))
			{
				return 0;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"repairSearchDatabase fail as exec cmd fail:"<<sCommmad;
				return 1;
				}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"repairSearchDatabase fail as initRepairDataBase fail ::"<<g_sRepairSearchRecord;
			return 1;
		}
	}else{
		return 0;
	}
}
