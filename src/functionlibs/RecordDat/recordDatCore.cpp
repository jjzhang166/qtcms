#include "recordDatCore.h"


recordDatCore::recordDatCore(void):m_bStop(true),
	m_nPosition(0),
	m_pDataBuffer(NULL),
	m_pDataBuffer1(NULL),
	m_pDataBuffer2(NULL)
{
	for(int i=0;i<WNDMAXSIZE;i++)
	{
		tagRecordDatCoreWndInfo tWndInfo;
		m_tFileInfo.tWndInfo.insert(i,tWndInfo);
	}
	m_pDataBuffer1=(char*)malloc(BUFFERSIZE*1024*1024);
	m_pDataBuffer2=(char*)malloc(BUFFERSIZE*1024*1024);
	connect(&m_tCheckIsBlockTimer,SIGNAL(timeout()),this,SLOT(slcheckBlock()));
	m_tCheckIsBlockTimer.start(5000);
}


recordDatCore::~recordDatCore(void)
{
	m_bStop=true;
	int nCount=0;
	while(QThread::isRunning()){
		msleep(10);
		nCount++;
		if (nCount>10000&&nCount%100==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"stop the thread had cost over 10s";
		}
	}
	if (NULL!=m_pDataBuffer2)
	{
	}
}

void recordDatCore::run()
{
	bool bRunStop=false;
	int nRunStep=recordDat_init;
	int nWriteType=2;//0:覆盖写；1：续写文件；2：没有文件可写
	QString sWriteFilePath;
	while(bRunStop==false){
		switch(nRunStep){
		case recordDat_init:{
			//各项参数初始化
			nRunStep=recordDat_filePath;
							}
							break;
		case recordDat_filePath:{
			//查找写文件的路径
			nWriteType=obtainFilePath(sWriteFilePath);
			if (nWriteType!=OVERWRITE&&nWriteType!=ADDWRITE)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as nWriteType mode is unable";
				nRunStep=recordDat_error;
			}else{
				nRunStep=recordDat_initMemory;
			}
								}
								break;
		case recordDat_initMemory:{
			//初始化内存块
			if (nWriteType==OVERWRITE)
			{

			}else{
				//nWriteType==ADDWRITE
			}
			nRunStep=recordDat_default;
								  }
								  break;
		case recordDat_writeMemory:{
			//数据帧写到内存块里，数据库条目更新
								   }
								   break;
		case recordDat_writeDisk:{
			//内存块写到磁盘
								 }
								 break;
		case recordDat_default:{
			//检测是否需要写到磁盘，检测是否有数据帧到达
							   }
							   break;
		case recordDat_error:{
			//出错
							 }
							 break;
		case recordDat_end:{
			//结束
						   }
						   break;
		}
	}
}

bool recordDatCore::setBufferQueue( int nWnd,BufferQueue &tBufferQueue )
{
	m_tBufferQueueMapLock.lock();
	m_tBufferQueueMap.insert(nWnd,&tBufferQueue);
	m_tBufferQueueMapLock.unlock();
	return true;
}

bool recordDatCore::removeBufferQueue( int nWnd )
{
	m_tBufferQueueMapLock.lock();
	m_tBufferQueueMap.remove(nWnd);
	m_tBufferQueueMapLock.unlock();
	return true;
}

void recordDatCore::registerEvent( QString sEventName,int(__cdecl *proc)(QString,QVariantMap,void*),void *pUser )
{
	if (m_tEventNameList.contains(sEventName))
	{
		tagRecordDatCoreProcInfo tProcInfo;
		tProcInfo.proc=proc;
		tProcInfo.pUser=pUser;
		m_tEventMap.insert(sEventName,tProcInfo);
		return;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"registerEvent fail as m_tEventNameList do not contains :"<<sEventName;
		return;
	}
}

void recordDatCore::eventCallBack( QString sEventName,QVariantMap tInfo )
{
	if (m_tEventNameList.contains(sEventName))
	{
		tagRecordDatCoreProcInfo tProcInfo=m_tEventMap.value(sEventName);
		if (NULL!=tProcInfo.proc)
		{
			tProcInfo.proc(sEventName,tInfo,tProcInfo.pUser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sEventName<<" event is not register";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"not support:"<<sEventName;
	}
}

bool recordDatCore::setRecordType( int nWnd,int nType,bool bFlags )
{
	return false;
}

void recordDatCore::slcheckBlock()
{
	if (m_bIsBlock)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"block at:"<<m_nPosition;
	}else{
		//do nothing
	}
}

int  recordDatCore::obtainFilePath(QString &sWriteFilePath)
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
			sUsableDiks=getUsableDisk(sDiskList);
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
			sFilePath=getLatestItem(sUsableDiks);
			bFlag=1;
			nStep=obtainFilePath_createFile;
									   }
									   break;
		case obtainFilePath_diskFull:{
			//每个盘符都已经录满
			QStringList sDiskListInDatabase=sDiskList.split(":");
			QList<QString> tFilePathList;
			foreach(QString sDiskEx,sDiskListInDatabase){
				QString sFilePathItem=m_tOperationDatabase.getLatestItem(sDiskEx);
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
				for (int i=0;i<tFilePathList.size();i++)
				{
					QString sFilePathItem=tFilePathList.at(i);
					if (QFile::exists(sFilePathItem))
					{
						QFile tFile;
						tFile.setFileName(sFilePathItem);
						if (tFile.open(QIODevice::ReadOnly|QIODevice::Text))
						{
							if (tFile.size()>=sizeof(tagFileHead))
							{
								m_tFileHead.clear();
								m_tFileHead=tFile.read(sizeof(tagFileHead));
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
										}
									}else{
										//do nothing
									}
								}else{
									//do nothing
								}
								tFile.close();
							}else{
								sOldestFile=sFilePathItem;
								tFile.close();
								break;
							}
						}else{
							//do nothing
						}
					}else{
						//do nothing
					}	
				}
				if (sOldestFile.isEmpty())
				{
					sOldestFile=tFilePathList.at(0);
				}else{
					//do nothing
				}
				sFilePath=sOldestFile;
				m_tOperationDatabase.clearInfoInDatabase(sFilePath);
				bFlag=0;
				nStep=obtainFilePath_createFile;
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as there is no disk space";
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
				nStep=obtainFilePath_fail;
			}
									   }
									   break;
		case obtainFilePath_success:{
			//获取录像文件路径成功
			m_tOperationDatabase.setFileIsLock(sFilePath,true);
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
			//结束
			bStop=true;
								}
		}
	}
	sWriteFilePath=sFilePath;
	return bFlag;
}

QString recordDatCore::getUsableDisk( QString &sDiskLisk )
{
	return "";
}

QString recordDatCore::getLatestItem(QString sDisk)
{
	//获取最新的文件路径
	//step1:查找数据库中最新的文件路径,没有的话，直接创建
	//step2:根据数据库最新的文件路径，判断文件是否写满，为满则续写，满的话则新起一个文件
	QString sFilePath=m_tOperationDatabase.getLatestItem(sDisk);
	if (sFilePath.isEmpty())
	{
		sFilePath=m_tOperationDatabase.createLatestItem(sDisk);
	}else{
		if (checkFileIsFull(sFilePath))
		{
			sFilePath=m_tOperationDatabase.createLatestItem(sDisk);
		}else{
			//do nothing
		}
	}
	return sFilePath;
}

bool recordDatCore::checkFileIsFull( QString sFilePath )
{
	return false;
}

bool recordDatCore::createNewFile( QString sFilePath )
{
	return false;
}

bool recordDatCore::startRecord()
{
	if (!QThread::isRunning())
	{
		m_bStop=false;
		QThread::start();
	}else{
		//do nothing
	}
	return false;
}



