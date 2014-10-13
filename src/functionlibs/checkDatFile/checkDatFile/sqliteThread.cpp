#include "sqliteThread.h"
#include <QtSql>
#include <QList>
#include <QSqlDatabase>
typedef struct __tagMgrDataBaseInfo{
	QString sDatabaseName;
	QSqlDatabase *pDatabase;
	int nCount;
	QList<quintptr *> tThis;
}tagMgrDataBaseInfo;
QMultiMap<QString ,tagMgrDataBaseInfo> g_tMgrTestDataBase;
QSqlDatabase *initMgrTestDataBase(QString sDatabaseName,quintptr *nThis){
	if (g_tMgrTestDataBase.contains(sDatabaseName))
	{
		if (g_tMgrTestDataBase.find(sDatabaseName).value().tThis.contains(nThis))
		{
			//do nothing
		}else{
			g_tMgrTestDataBase.find(sDatabaseName).value().nCount++;
			g_tMgrTestDataBase.find(sDatabaseName).value().tThis.append(nThis);
		}
		return g_tMgrTestDataBase.find(sDatabaseName).value().pDatabase;
	}else{
		tagMgrDataBaseInfo tDataBaseInfo;
		tDataBaseInfo.sDatabaseName=sDatabaseName;
		tDataBaseInfo.nCount=1;
		tDataBaseInfo.tThis.append(nThis);

		QDateTime tCurrentTime=QDateTime::currentDateTime();
		QString sDatabaseId=QString::number(tCurrentTime.toTime_t())+QString::number((quint64)nThis);
		QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE",sDatabaseId);
		tDataBaseInfo.pDatabase=new QSqlDatabase(db);
		tDataBaseInfo.pDatabase->setDatabaseName(sDatabaseName);
		if (tDataBaseInfo.pDatabase->open())
		{
			//do nothing
			//QSqlQuery _query(*tDataBaseInfo.pDatabase);
			//QString sCommand="pragma journal_mode =off";
			//if (_query.exec(sCommand))
			//{
			//}else{
			//	tDataBaseInfo.pDatabase->close();
			//	delete tDataBaseInfo.pDatabase;
			//	tDataBaseInfo.pDatabase=NULL;
			//	printf("exec cmd fail:pragma journal_mode =off,in initMgrDataBase function /n");
			//	return NULL;
			//}
		}else{
			printf("open database fail,in initMgrDataBase function/n");
			return NULL;
		}
		g_tMgrTestDataBase.insert(sDatabaseName,tDataBaseInfo);
		return tDataBaseInfo.pDatabase;
	}
}
void deInitMgrTestDataBase(quintptr *nThis){
	QMultiMap<QString,tagMgrDataBaseInfo>::iterator it;
	QStringList sDeleteList;
	for (it=g_tMgrTestDataBase.begin();it!=g_tMgrTestDataBase.end();it++)
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
		g_tMgrTestDataBase.remove(sDeleteList.at(i));
	}
}
sqliteThread::sqliteThread(void)
{
	
}


sqliteThread::~sqliteThread(void)
{
	QThread::quit();
	while (QThread::isRunning())
	{
		msleep(10);
	}
}

void sqliteThread::run()
{
	QString sDatabasePath="D:/testdatabase/record.db";
	QSqlDatabase *pDatabase=NULL;
	pDatabase=initMgrTestDataBase(sDatabasePath,(quintptr*)this);
	QSqlQuery _query(*pDatabase);
	if (NULL!=pDatabase)
	{
		QString sCommand;
		QSqlQueryModel model;
		if (m_nMode==0)
		{
			sCommand=QString("update RecordFileStatus set nInUse=0");
		}else{
			sCommand=QString("select nRecordType, nStartTime, nEndTime from search_record where nWndId='%1' and nEndTime>='%2' and nStartTime<='%3' and nStartTime!=nEndTime order by nStartTime").arg(2).arg(1413037440).arg(1423018056);
			
		}

		int nFlag=0;
		while(true){
			if (m_nMode==0)
			{
				if (nFlag==0)
				{
					nFlag=1;
				}else{
					nFlag=0;
				}
				sCommand=QString("update search_record set nEndTime=%1 where id in (1,2,3,4,5,6)").arg(nFlag);
			}

			if (m_nMode==0)
			{
				//if (_query.exec(sCommand))
				//{
				//	printf("update\n");
				//}else{
				//	qDebug()<<__FUNCTION__<<__LINE__<<sCommand<<_query.lastError();
				//	abort();
				//}
				if (execCommand(_query,sCommand))
				{
					printf("update\n");
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<sCommand<<_query.lastError();
					abort();
				}
			}else{
				if (execCommand(_query,sCommand))
				{
					printf("select\n");
					
				}else{
					abort();
					printf("lastError\n");
				}
			}
		}
	}else{
		abort();
	}
}

void sqliteThread::startThread()
{
	QThread::start();
}

void sqliteThread::setTestMode( int nMode )
{
	m_nMode=nMode;
}

bool sqliteThread::execCommand( QSqlQuery & tQuery,QString sCommand )
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
				printf("try===========success\n");
			}
			return true;
		}else{
			if ("database is locked"==tQuery.lastError().databaseText())
			{
				msleep(1);
				nCount++;
				printf("try\n");
			}else{
				return false;
			}
		}
	}
	return false;
}
