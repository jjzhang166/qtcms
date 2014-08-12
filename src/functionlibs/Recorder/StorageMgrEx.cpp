#include "StorageMgrEx.h"
#include <guid.h>
#include "netlib.h"
#pragma comment(lib,"netlib.lib")
typedef struct __tagMgrDataBaseInfo{
	QString sDatabaseName;
	QSqlDatabase *pDatabase;
	int nCount;
	QList<int *> tThis;
}tagMgrDataBaseInfo;
QMultiMap<QString ,tagMgrDataBaseInfo> g_tMgrDataBase;
QString g_sMgrSearchRecord="C:/CMS_RECORD/search_record.db";
QSqlDatabase *initMgrDataBase(QString sDatabaseName,int *nThis){
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
		}else{
			printf("open database fail,in initDataBase function/n");
			return NULL;
		}
		g_tMgrDataBase.insert(sDatabaseName,tDataBaseInfo);
		return tDataBaseInfo.pDatabase;
	}
}
void deInitMgrDataBase(int *nThis){
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
StorageMgrEx::StorageMgrEx(void):m_bStop(false),
	m_bIsExecute(false),
	m_bExecuteFlag(false),
	m_bIsBlock(false),
	m_iSleepSwitch(0),
	m_pDisksSetting(NULL)
{
	connect(&m_tCheckBlockTimer,SIGNAL(timeout()),this,SLOT(slCheckBlock()));
	m_tCheckBlockTimer.start(1000);
	for(int i=0;i<64;i++){
		m_tCurrentUseRecordId.append(-1);
	}
}


StorageMgrEx::~StorageMgrEx(void)
{
}

void StorageMgrEx::run()
{
	int iRunStep=MGR_UPDATASYSTEMDATABASE;
	bool bRunStop=false;
	int iUpdateConunt=0;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IDiskSetting,(void**)&m_pDisksSetting);
	if (m_pDisksSetting==NULL)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"run fail as m_pDisksSetting is null,please checkout";
		return;
	}else{
		//keep going
	}
	//创建搜索表
	if (createSearchTable())
	{
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"run fail as createSearchTable fail,please checkout";
		return;
	}
	while(bRunStop==false){
		switch(iRunStep){
		case MGR_APPLYDISKSPACE:{
			if (priApplyDiskSpace())
			{
				m_bExecuteFlag=true;
			}else{
				m_bExecuteFlag=false;
			}
			m_bIsExecute=true;
			iRunStep=MGR_DEFAULT;
								}
								break;
		case MGR_CREATERECORDITEM:{
			if (priCreateRecordItem())
			{
				m_bExecuteFlag=true;
			}else{
				m_bExecuteFlag=false;
			}
			m_bIsExecute=true;
			iRunStep=MGR_DEFAULT;
								  }
								  break;
		case MGR_CREATESEARCHITEM:{
			if (priCreateSearchItem())
			{
				m_bExecuteFlag=true;
			}else{
				m_bExecuteFlag=false;
			}
			m_bIsExecute=true;
			iRunStep=MGR_DEFAULT;
								  }
								  break;
		case MGR_UPDATASYSTEMDATABASE:{
			if (priUpdateSystemDataBaseData())
			{
				//do nothing
			}else{
				//do nothing
				qDebug()<<__FUNCTION__<<__LINE__<<"priUpdateSystemDataBaseData fail ,please checkout!!!";
			}
			iRunStep=MGR_DEFAULT;
									  }
									  break;
		case MGR_DEFAULT:{
			iUpdateConunt++;
			//两分钟更新一次system数据库中的值
			if (iUpdateConunt<120000)
			{
				if (m_tStepCode.size()>0)
				{
					iRunStep=m_tStepCode.dequeue();
				}else{
					msleep(1);
					iRunStep=MGR_DEFAULT;
				}
			}else{
				iUpdateConunt=0;
				iRunStep=MGR_UPDATASYSTEMDATABASE;
			}

			if (m_bStop)
			{
				iRunStep=MGR_END;
			}else{
				//keep going
			}
						 }
						 break;
		case MGR_END:{
			bRunStop=true;
					 }
					 break;
		}
	}
}

void StorageMgrEx::startMgr()
{
	m_tFuncLock.lock();
	if (!QThread::isRunning())
	{
		m_tStepCode.clear();
		QThread::start();
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread had been running,there is no need to call this func again";
	}
	m_tFuncLock.unlock();
}

void StorageMgrEx::stopMgr()
{
	m_tFuncLock.lock();
	if (QThread::isRunning())
	{
		m_bStop=true;
		int iCount=0;
		while(QThread::isRunning()){
			iCount++;
			sleepEx(10);
			if (iCount>500&&iCount%100==0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"the thread should been stop,it block at ::"<<m_iPosition;
			}
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"the thread had been stop ,there is  no need to call this func again";
	}
	m_tFuncLock.unlock();
}

bool StorageMgrEx::applyDiskSpace(QString &sApplyDisk)
{
	//申请空间，创建文件路径，创建文件夹
	m_tFuncLock.lock();
	if (QThread::isRunning())
	{
		m_bIsExecute=false;
		m_bExecuteFlag=false;
		m_tStepCode.enqueue(MGR_APPLYDISKSPACE);
		int iCount=0;
		while(m_bIsExecute==false&&QThread::isRunning()){
			sleepEx(10);
			if (iCount>500&&iCount%100==0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"the thread may be block at::"<<m_iPosition<<"or may be out of control";
			}
		}
		if (m_bExecuteFlag)
		{
			sApplyDisk=m_tStorageMgrExInfo.sApplyDisk;
			m_tFuncLock.unlock();
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"applyDiskSpace fail as m_bExecuteFlag is false";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"applyDiskSpace fail as the thread is not running";
	}
	m_tFuncLock.unlock();
	return false;
}

bool StorageMgrEx::createRecordItem( QString sDisk,QString sDevName,int iWindId,int iChannelNum,int iType,unsigned int &uiItem,QString &sFilePath )
{
	m_tFuncLock.lock();
	if (QThread::isRunning())
	{
		m_bIsExecute=false;
		m_bExecuteFlag=false;
		m_tStorageMgrExInfo.sCreateRecordItemDisk.clear();
		m_tStorageMgrExInfo.sCreateRecordItemDisk=sDisk;
		m_tStorageMgrExInfo.sCreateRecordItemDevName=sDevName;
		m_tStorageMgrExInfo.iCreateRecordItemChannelNum=iChannelNum;
		m_tStorageMgrExInfo.iCreateRecordItemWindId=iWindId;
		m_tStorageMgrExInfo.iCreateRecordItemType=iType;
		m_tStepCode.enqueue(MGR_CREATERECORDITEM);
		int iCount=0;
		while(m_bIsExecute==false&&QThread::isRunning()){
			sleepEx(10);
			if (iCount>500&&iCount%100==0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"the thread may be block at::"<<m_iPosition<<"or may be out of control";
			}
		}
		uiItem=m_tStorageMgrExInfo.uiRecordDataBaseId;
		sFilePath=m_tStorageMgrExInfo.sRecordFilePath;
		if (m_bExecuteFlag)
		{
			m_tFuncLock.unlock();
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"applyDiskSpace fail as m_bExecuteFlag is false";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"createRecordItem fail as the thread is not running";
	}
	m_tFuncLock.unlock();
	return false;
}

bool StorageMgrEx::createSearchItem( unsigned int &uiItem,int nWindId,int nType,QString sDate,QString sStartTime,QString sEndTime)
{
	m_tFuncLock.lock();
	if (QThread::isRunning())
	{
		m_bIsExecute=false;
		m_bExecuteFlag=false;
		m_tStorageMgrExInfo.sCreateSearchItemDate=sDate;
		m_tStorageMgrExInfo.iCreateSearchItemWindId=nWindId;
		m_tStorageMgrExInfo.iCreateSearchItemType=nType;
		m_tStorageMgrExInfo.sCreateSearchItemEndTime=sEndTime;
		m_tStorageMgrExInfo.sCreateSearchItemStartTime=sStartTime;
		m_tStepCode.enqueue(MGR_CREATESEARCHITEM);
		int iCount=0;
		while(m_bIsExecute==false&&QThread::isRunning()){
			sleepEx(10);
			if (iCount>500&&iCount%100==0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"the thread may be block at::"<<m_iPosition<<"or may be out of control";
			}
		}
		uiItem=m_tStorageMgrExInfo.uiSearchDataBaseId;
		if (m_bExecuteFlag)
		{
			m_tFuncLock.unlock();
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"applyDiskSpace fail as m_bExecuteFlag is false";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"createSearchItem fail as the thread is not running";
	}
	m_tFuncLock.unlock();
	return false;
}

void StorageMgrEx::sleepEx( int iTime )
{
	if (m_iSleepSwitch<100)
	{
		msleep(iTime);
		m_iSleepSwitch++;
	}else{
		m_iSleepSwitch=0;
		QEventLoop tEventLoop;
		QTimer::singleShot(2,&tEventLoop,SLOT(quit));
		tEventLoop.exec();
	}
}

bool StorageMgrEx::priApplyDiskSpace()
{
	//申请空间，创建文件路径，创建文件夹
	QString sDisk;
	if (freeDisk(sDisk))
	{
		m_tStorageMgrExInfo.sAllRecordDisks=sDisk;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"priApplyDiskSpace fail freeDisk fail";
	}
	return false;
}

bool StorageMgrEx::priCreateRecordItem()
{
	//创建录像表 ，创建记录
	QString sDbPath=m_tStorageMgrExInfo.sCreateRecordItemDisk+"/REC";
	QDir tDir;
	int nStep=0;
	bool bStop=false;
	bool bFlags=false;
	while(bStop==false){
		switch(nStep){
		case 0:{
			//创建数据库目录
			if (tDir.exists(sDbPath))
			{
				//do nothing
				nStep=1;
			}else{
				if (tDir.mkpath(sDbPath))
				{
					//keep going
					nStep=1;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"priCreateRecordItem fail as it can not mkpath::"<<sDbPath;
					nStep=3;
				}
			}
			   }
			   break;
		case 1:{
			//创建数据库
			sDbPath+="/record.db";
			if (!QFile::exists(sDbPath))
			{
				//创建录像表
				if (createRecordTable(sDbPath))
				{
					nStep=2;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"priCreateRecordItem fail as createRecordTable fail";
					nStep=3;
				}
			}else{
				//keep going
				if (checkRecordTable(sDbPath,"local_record"))
				{
					nStep=2;
				}else{
					if (createRecordTable(sDbPath))
					{
						nStep=2;
					}else{
						nStep=3;
						qDebug()<<__FUNCTION__<<__LINE__<<"priCreateRecordItem fail as createRecordTable fail";
					}
				}
			}
			   }
			   break;
		case 2:{
			//创建条目
			if (insertItemIntoRecordTable())
			{
				nStep=4;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"priCreateRecordItem fail as insertItemIntoRecordTable fail";
				nStep=3;
			}
			   }
			   break;
		case 3:{
			//失败
			nStep=5;
			   }
			   break;
		case 4:{
			//成功
			nStep=5;
			bFlags=true;
			   }
			   break;
		case 5:{
			//返回
			bStop=true;
			   }
			   break;
		}
	}
	if (bFlags)
	{
		return true;
	}else{
		return false;
	}
}

bool StorageMgrEx::priCreateSearchItem()
{
	if (!(m_tStorageMgrExInfo.iCreateSearchItemWindId<0||m_tStorageMgrExInfo.sCreateSearchItemDate.isEmpty()||m_tStorageMgrExInfo.sCreateSearchItemStartTime.isEmpty()||m_tStorageMgrExInfo.sCreateSearchItemEndTime.isEmpty()))
	{
		QSqlDatabase *pDataBase=initMgrDataBase(g_sMgrSearchRecord,(int *)this);
		if (NULL!=pDataBase)
		{
			QSqlQuery _query(*pDataBase);
			QString sCommand=QString("insert into search_record(wnd_id,record_type,date,start_time,end_time) values(%1,%2,'%3','%4','%5')").arg(m_tStorageMgrExInfo.iCreateSearchItemWindId).arg(m_tStorageMgrExInfo.iCreateSearchItemType).arg(m_tStorageMgrExInfo.sCreateSearchItemDate).arg(m_tStorageMgrExInfo.sCreateSearchItemStartTime).arg(m_tStorageMgrExInfo.sCreateSearchItemEndTime);
			if (_query.exec(sCommand))
			{
				sCommand.clear();
				sCommand=QString("select max(id) from search_record");
				if (_query.exec(sCommand))
				{
					if (_query.next())
					{
						m_tStorageMgrExInfo.uiSearchDataBaseId=_query.value(0).toInt();
						return true;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"priCreateSearchItem fail as there are not any item";
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"priCreateSearchItem fail as exec the cmd fail::"<<sCommand;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"priCreateSearchItem fail as exec the cmd fail::"<<sCommand;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"priCreateSearchItem fail as pDataBase is null";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"priCreateSearchItem fail as the input param is unCorrent";
	}
	return false;
}

void StorageMgrEx::slCheckBlock()
{

}

bool StorageMgrEx::priUpdateSystemDataBaseData()
{
	//获取录像磁盘，获取磁盘保留空间，获取文件建议打包大小,是否循环录像
	tagStorageMgrExInfo tMgrExInfo;
	if (NULL!=m_pDisksSetting)
	{
		//获取录像磁盘
		tMgrExInfo.sAllRecordDisks.clear();
		if (0==m_pDisksSetting->getUseDisks(tMgrExInfo.sAllRecordDisks))
		{
			//获取是否循环录像
			tMgrExInfo.bRecoverRecorder=m_pDisksSetting->getLoopRecording();
			//获取文件建议打包大小
			m_pDisksSetting->getFilePackageSize(tMgrExInfo.iFileMaxSize);
			//获取磁盘建议保留空间
			m_pDisksSetting->getDiskSpaceReservedSize(tMgrExInfo.iDiskReservedSize);
			m_tGetMgrInfoLock.lock();
			m_tStorageMgrExInfo.sAllRecordDisks.clear();
			m_tStorageMgrExInfo.sAllRecordDisks=tMgrExInfo.sAllRecordDisks;
			m_tStorageMgrExInfo.bRecoverRecorder=tMgrExInfo.bRecoverRecorder;
			m_tStorageMgrExInfo.iFileMaxSize=tMgrExInfo.iFileMaxSize;
			m_tStorageMgrExInfo.iDiskReservedSize=tMgrExInfo.iDiskReservedSize;
			m_tGetMgrInfoLock.unlock();
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"priUpdateSystemDataBaseData fail as getUseDisks fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"priUpdateSystemDataBaseData fail as m_pDisksSetting is null";
	}
	return false;
}

tagStorageMgrExInfo StorageMgrEx::getStorageMgrExInfo()
{
	m_tGetMgrInfoLock.lock();
	tagStorageMgrExInfo tMgrExInfo;
	tMgrExInfo.bRecoverRecorder=m_tStorageMgrExInfo.bRecoverRecorder;
	tMgrExInfo.iDiskReservedSize=m_tStorageMgrExInfo.iDiskReservedSize;
	tMgrExInfo.iFileMaxSize=m_tStorageMgrExInfo.iFileMaxSize;
	tMgrExInfo.sAllRecordDisks=m_tStorageMgrExInfo.sAllRecordDisks;
	tMgrExInfo.sRecordFilePath=m_tStorageMgrExInfo.sRecordFilePath;
	tMgrExInfo.uiRecordDataBaseId=m_tStorageMgrExInfo.uiRecordDataBaseId;
	tMgrExInfo.uiSearchDataBaseId=m_tStorageMgrExInfo.uiSearchDataBaseId;
	m_tGetMgrInfoLock.unlock();
	return tMgrExInfo;
}

bool StorageMgrEx::freeDisk(QString &sDisk)
{
	QStringList sDiskList=m_tStorageMgrExInfo.sAllRecordDisks.split(":");
	if (sDiskList.size()!=0)
	{
		int iFreeDiskStep=0;
		bool bFreeDiskStop=false;
		bool bFreeSucceed=false;
		QMap<QDate,tagMgrRecInfo> tPreDeleteItem;
		while (bFreeDiskStop==false)
		{
			switch (iFreeDiskStep)
			{
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
						if (uiTotalNumberOfFreeByte/1024/1024>(quint64)m_tStorageMgrExInfo.iDiskReservedSize)
						{
							bFound=true;
							sDisk=sDiskItem;
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
				tPreDeleteItem.empty();
				QDate tEarlestDate;
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
				//step2:删除文件
				if (!tPreDeleteItem.isEmpty())
				{
					//查找各个磁盘之中最早的一天的录像
					tEarlestDate=minDate(tPreDeleteItem.keys());
					QList<tagMgrRecInfo> tRecInfo=tPreDeleteItem.values(tEarlestDate);
					QStringList tRemoteItemList;
					for(int i=0;i<tRecInfo.size();i++){
						tagMgrRecInfo tEach=tRecInfo.at(i);
						QStringList tItemList=removeFile(tEach.tFileList);
						tRemoteItemList<<tItemList;
					}
					if (!tRemoteItemList.isEmpty())
					{
						//step3:删除录像表的条目
						if (removeRecordDataBaseItem(tRemoteItemList,tRecInfo))
						{
							//keep going
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"removeRecordDataBaseItem fail ,please check";
						}
						//step4:删除搜索表中的条目
						if (removeSearchDataBaseItem(tRemoteItemList,tRecInfo,tEarlestDate.toString("yyyy-MM-dd")))
						{
							//keep going
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"removeSearchDataBaseItem fail ,please check";
						}
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

QStringList StorageMgrEx::findEarliestRecord( QString tDbPath,QDate &tEarlestDate,QMap<int ,QString>&tMaxEndTimeMap )
{
	QStringList sPathItemList;
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initMgrDataBase(tDbPath,(int *)this);
	if (NULL!=pDataBase)
	{
		QString sEarlyDate;
		QSqlQuery _query(*pDataBase);
		QString sCommand=QString("select distinct date from local_record order by date limit 1");
		if (_query.exec(sCommand))
		{
			if (_query.next())
			{
				sEarlyDate=_query.value(0).toString();
				sCommand.clear();
				sCommand=QString("select date, path,id, win_id, end_time ,start_time from local_record where date='%1' order by start_time").arg(sEarlyDate);
				if (_query.exec(sCommand))
				{
					while(_query.next()){
						if (!m_tCurrentUseRecordId.contains(_query.value(2).toInt()))
						{
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
							if ((iCurrentDate-iDate)>18000)
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
						}else{
							//keep going
							qDebug()<<__FUNCTION__<<__LINE__<<"this item can not been delete,as it is be using"<<_query.value(1).toString();
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

QDate StorageMgrEx::minDate( QList<QDate> tDateList )
{
	QDate tMinDate=tDateList.at(0);
	for(int i=1;i<tDateList.size();++i){
		tMinDate=qMin(tDateList.at(i),tMinDate);
	}
	return tMinDate;
}

QStringList StorageMgrEx::removeFile( QStringList tItemList )
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

bool StorageMgrEx::removeRecordDataBaseItem( QStringList tRemoveFileItem,QList<tagMgrRecInfo> tRecInfo )
{
	//tRemoveFileItem :已删除的文件路径
	//tRecInfo:筛选出来最早一天的记录
	//this func need to test

	foreach(QString sItem,tRemoveFileItem){
		QSqlDatabase *pDataBase=NULL;
		int nPos=sItem.indexOf("/REC/");
		int nEndPos=nPos+5;
		QString sFind=sItem.left(nEndPos)+"record.db";
		pDataBase=initMgrDataBase(sFind,(int*)this);
		if (pDataBase!=NULL)
		{
			QSqlQuery _query(*pDataBase);
			QString sCommond=QString("delete from local_record where path ='").append(sItem).append("'");
			if (_query.exec(sCommond))
			{
				//do nothing
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"removeRecordDataBaseItem fail as exec the cmd fail::"<<sCommond;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"removeRecordDataBaseItem fail as pDataBase is null,i will keep going without any handle";
		}
	}
	return true;
}

bool StorageMgrEx::removeSearchDataBaseItem( QStringList tRemoveFileItem,QList<tagMgrRecInfo> tRecInfo ,QString sDate)
{
	//tRemoveFileItem:已删除文件的路径
	//tRecInfo:筛选出来最早一天的记录
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initMgrDataBase(g_sMgrSearchRecord,(int*)this);
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
				if (_query.exec(sCommond))
				{
					sCommond.clear();
					sCommond=QString("select id,start_time,end_time from search_record where date ='%1' and wnd_id =%2 order by start_time limit 1").arg(sDate).arg(iter.key());
					if (_query.exec(sCommond))
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
								if (_query.exec(sCommond))
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
			}
		}
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"removeSearchDataBaseItem fail as pDataBase is null";
		return false;
	}
}

bool StorageMgrEx::createRecordTable( QString sPath )
{
	QSqlDatabase *pDataBase=NULL;
	pDataBase=initMgrDataBase(sPath,(int*)this);
	if (pDataBase!=NULL)
	{
		QSqlQuery _query(*pDataBase);
		QString sCommand = "create table local_record(id integer primary key autoincrement,";
		sCommand += "dev_name char(32),";
		sCommand += "dev_chl integer,";
		sCommand += "win_id integer,";
		sCommand += "date char(32),";
		sCommand += "start_time char(32),";
		sCommand += "end_time char(32),";
		sCommand += "record_type integer,";
		sCommand += "file_size integer,";
		sCommand += "path char(64))";
		if (_query.exec(sCommand))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"createRecordTable fail as exec cmd fail::"<<sCommand;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"createRecordTable fail as pDataBase is null";
	}
	return false;
}

bool StorageMgrEx::checkRecordTable( QString sPath,QString sTable )
{
	QSqlDatabase *pDataBase=initMgrDataBase(sPath,(int *)this);
	if (NULL!=pDataBase)
	{
		QSqlQuery _query(*pDataBase);
		QString sCmd=QString("select count(*) from sqlite_master where type='table' and name='%1'").arg(sTable);
		if (_query.exec(sCmd))
		{
			if (_query.next())
			{
				if (_query.value(0).toInt()>0)
				{
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"checkRecordTable fail as record.db is empty";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"checkRecordTable fail as there is no any item";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"checkRecordTable fail as exec cmd fail::"<<sCmd;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"checkRecordTable fail as pDataBase is null";
	}
	return false;
}

bool StorageMgrEx::insertItemIntoRecordTable()
{
	QString sPath=m_tStorageMgrExInfo.sCreateRecordItemDisk+"/REC/record.db";
	QSqlDatabase *pDataBase=initMgrDataBase(sPath,(int *)this);
	QString sFileSavePath=m_tStorageMgrExInfo.sCreateRecordItemDisk+"/REC";
	if (NULL!=pDataBase)
	{
		QSqlQuery _query(*pDataBase);
		QString sCommand=QString("select path, id, start_time, end_time, date from local_record where win_id=%1 order by id desc limit 1").arg(m_tStorageMgrExInfo.iCreateRecordItemWindId);
		if (_query.exec(sCommand))
		{
			bool bFlags=false;
			if (_query.next())
			{
				QString sDirPath=_query.value(0).toString();
				int iId=_query.value(1).toInt();
				QString sStartTime=_query.value(2).toString();
				QString sEndTime=_query.value(3).toString();
				QString sDate=_query.value(4).toString();
				if (sStartTime==sEndTime)
				{
					//clear old file and database
					if (QFile::exists(sDirPath))
					{
						if (QFile::remove(sDirPath))
						{
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"remove file fail::"<<sDirPath<<"but i do not do anything";
						}
					}else{
						//keep going
					}
					//删除录像数据库本条记录
					QString sCmd=QString("delete from local_record where id=%1").arg(iId);
					if (_query.exec(sCmd))
					{
						//删除搜索表本条记录
						if (updateSearchItem(m_tStorageMgrExInfo.iCreateRecordItemWindId,sDate,sEndTime))
						{
							sFileSavePath=sDirPath;
							bFlags=true;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"insertItemIntoRecordTable fail as updateSearchItem fail";
						}

					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"insertItemIntoRecordTable fail as exec fail ::"<<sCmd;
					}
				}else{
					int nPos=sDirPath.lastIndexOf("/");
					int nFileNum=sDirPath.mid(nPos+1,3).toInt();
					int nDirNum=sDirPath.mid(nPos-3,3).toInt();
					nFileNum++;
					if (nFileNum>127)
					{
						nDirNum++;
						nFileNum=0;
					}else{
						//do nothing
					}
					QString sTemp;		
					sFileSavePath+=sTemp.sprintf("/WND%02d/%04d/%03d.avi",m_tStorageMgrExInfo.iCreateRecordItemWindId,nDirNum,nFileNum);
					bFlags=true;
				}
			}else{
				QString sTemp;
				sFileSavePath+=sTemp.sprintf("/WND%02d/0000/000.avi",m_tStorageMgrExInfo.iCreateRecordItemWindId);
				bFlags=true;
			}
			if (bFlags)
			{
				if (QFile::exists(sFileSavePath))
				{
					if (QFile::remove(sFileSavePath))
					{
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"remove file fail::"<<sFileSavePath<<"but i do not do anything";
					}
				}else{
					//keep going
				}
				_query.prepare("insert into local_record(dev_name,dev_chl,win_id,date,start_time,end_time,record_type,path) values(:dev_name,:dev_chl,:win_id,:date,:start_time,:end_time,:record_type,:path)");
				_query.bindValue(":dev_name",m_tStorageMgrExInfo.sCreateRecordItemDevName);
				_query.bindValue(":dev_chl",m_tStorageMgrExInfo.iCreateRecordItemChannelNum + 1);//ensure chl num start with 1
				_query.bindValue(":win_id",m_tStorageMgrExInfo.iCreateRecordItemWindId);
				_query.bindValue(":date",QDate::currentDate().toString("yyyy-MM-dd"));
				QTime tStartTime;
				tStartTime = QTime::currentTime();
				_query.bindValue(":start_time",tStartTime.toString("hh:mm:ss"));
				_query.bindValue(":end_time", tStartTime.toString("hh:mm:ss"));
				_query.bindValue(":record_type", m_tStorageMgrExInfo.iCreateRecordItemType);
				_query.bindValue(":path", sFileSavePath);
				m_tStorageMgrExInfo.sRecordFilePath=sFileSavePath;
				if (_query.exec())
				{
					QString sCmd=QString("select max(id) from local_record");
					if (_query.exec(sCmd))
					{
						if (_query.next())
						{
							m_tStorageMgrExInfo.uiRecordDataBaseId=_query.value(0).toInt();
							if (m_tStorageMgrExInfo.iCreateRecordItemWindId<64&&m_tStorageMgrExInfo.iCreateRecordItemWindId>=0)
							{
								m_tCurrentUseRecordId.replace(m_tStorageMgrExInfo.iCreateRecordItemWindId,m_tStorageMgrExInfo.uiRecordDataBaseId);
								return true;
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"insertItemIntoRecordTable fail as input param are unCorrent";
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"insertItemIntoRecordTable fail as there are not any item";
						}		
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"insertItemIntoRecordTable fail as exec cmd fail"<<sCmd;
					}	
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"insertItemIntoRecordTable fail as exec cmd fail";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"insertItemIntoRecordTable fail as bFlags is false";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"insertItemIntoRecordTable fail as exec cmd fail::"<<sCommand;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"insertItemIntoRecordTable fail as pDataBase is null";
	}
	return false;
}

bool StorageMgrEx::updateSearchItem( int nWindId,QString sDate,QString sNewEndTime )
{
	QSqlDatabase *pDataBase=initMgrDataBase(g_sMgrSearchRecord,(int *)this);
	if (NULL!=pDataBase)
	{
		QString sStartTime;
		int nId=-1;
		QSqlQuery _query(*pDataBase);
		QString sCmd=QString("select id, start_time, end_time from search_record where date='%1' and wnd_id=%2 order by start_time desc limit 1").arg(sDate).arg(nWindId);
		if (_query.exec(sCmd))
		{
			if (_query.next())
			{
				nId=_query.value(0).toInt();
				sStartTime=_query.value(1).toString();
				if (sStartTime==sNewEndTime)
				{
					sCmd.clear();
					sCmd = QString("delete from search_record where id=%1").arg(nId);
					if (_query.exec(sCmd))
					{
						return true;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"updateSearchItem fail as exec cmd fail::"<<sCmd;
						return false;
					}
				}else{
					sCmd.clear();
					sCmd = QString("update search_record set end_time='%1' where date='%2' and wnd_id=%3").arg(sNewEndTime).arg(sDate).arg(nWindId);
					if (_query.exec(sCmd))
					{
						return true;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"updateSearchItem fail as exec fail ::"<<sCmd;
						return false;
					}
				}
			}else{
				//do nothing
				return true;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"updateSearchItem fail as exec cmd fail::"<<sCmd;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"updateSearchItem fail as pDataBase is null";
	}
	return false;
}

bool StorageMgrEx::createSearchTable()
{
	if (!QFile::exists(g_sMgrSearchRecord))
	{
		QDir tDbPath;
		QString sDir=g_sMgrSearchRecord.left(g_sMgrSearchRecord.lastIndexOf("/"));
		if (!tDbPath.exists(sDir))
		{
			if (tDbPath.mkpath(sDir))
			{
				//keep going
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"createSearchTable fail as mkpath fail::"<<sDir;
				return false;
			}
		}else{
			//keep going
		}
		QFile file(g_sMgrSearchRecord);
		file.open(QIODevice::WriteOnly);
		file.close();
		QSqlDatabase *pDataBase=NULL;
		pDataBase=initMgrDataBase(g_sMgrSearchRecord,(int *)this);
		if (NULL!=pDataBase)
		{
			QSqlQuery _query(*pDataBase);
			QString command = "create table search_record(";
			command += "id integer primary key autoincrement,";
			command += "wnd_id integer,";
			command += "record_type integer,";
			command += "date char(32),";
			command += "start_time char(32),";
			command += "end_time char(32))";
			if (_query.exec(command))
			{
				return true;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"createSearchTable fail as exec cmd fail::"<<command;
				return false;
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"createSearchTable fail as pDataBase is null";
			return false;
		}
	}else{
		return true;
	}
}
