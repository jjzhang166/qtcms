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
		tWndInfo.uiCurrentRecordType=0;
		tWndInfo.uiHistoryRecordType=0;
		tWndInfo.uiChannelInDatabaseId=0;
		tWndInfo.uiPreFrame=0;
		tWndInfo.uiPreIFrame=0;
		m_tFileInfo.tWndInfo.insert(i,tWndInfo);
	}
	m_pDataBuffer1=(char*)malloc(BUFFERSIZE*1024*1024);
	m_pDataBuffer2=(char*)malloc(BUFFERSIZE*1024*1024);
	if (m_pDataBuffer2==NULL||m_pDataBuffer1==NULL)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"malloc fail";
		abort();
	}else{
		//do nothing
	}
	memset(m_pDataBuffer1,0,BUFFERSIZE*1024*1024);
	memset(m_pDataBuffer2,0,BUFFERSIZE*1024*1024);
	connect(&m_tCheckIsBlockTimer,SIGNAL(timeout()),this,SLOT(slcheckBlock()));
	connect(&m_tWriteDiskTimer,SIGNAL(timeout()),this,SLOT(slsetWriteDiskFlag()));
	m_tCheckIsBlockTimer.start(5000);//5s
	
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
		free(m_pDataBuffer2);
		m_pDataBuffer2=NULL;
	}else{
		//do nothing
	}
	if (NULL!=m_pDataBuffer1)
	{
		free(m_pDataBuffer1);
		m_pDataBuffer1=NULL;
	}else{
		//do nothing
	}
	m_tCheckIsBlockTimer.stop();
	m_pDataBuffer=NULL;
}

void recordDatCore::run()
{
	bool bRunStop=false;
	int nRunStep=recordDat_init;
	int nSleepCount=0;
	int nWriteType=2;//0:覆盖写；1：续写文件；2：没有文件可写
	QString sWriteFilePath;
	m_tWriteDiskTimer.start(60000);
	while(bRunStop==false){
		switch(nRunStep){
		case recordDat_init:{
			//各项参数初始化
			m_tToDiskType=recordDatToDiskType_null;
			m_nWriteMemoryChannel=0;
			nRunStep=recordDat_filePath;
			m_pDataBuffer=m_pDataBuffer1;
							}
							break;
		case recordDat_filePath:{
			//查找写文件的路径
			//step1:创建文件（或者获取到已有的文件）
			//step2:数据库中锁定文件
			nWriteType=obtainFilePath(sWriteFilePath);
			if (nWriteType!=OVERWRITE&&nWriteType!=ADDWRITE)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as nWriteType mode is unable";
				m_tResetType=recordDatReset_outOfDisk;
				nRunStep=recordDat_reset;
			}else{
				nRunStep=recordDat_initMemory;
			}
								}
								break;
		case recordDat_initMemory:{
			//初始化内存块
			if (nWriteType==OVERWRITE)
			{
				//覆盖写文件
				memset(m_pDataBuffer,0,BUFFERSIZE*1024*1024);
			}else{
				//nWriteType==ADDWRITE
				//把原文件的内容读到内存中，进行续写
				memset(m_pDataBuffer,0,BUFFERSIZE*1024*1024);
				QFile tFile;
				tFile.setFileName(sWriteFilePath);
				if (tFile.open(QIODevice::ReadOnly))
				{
					m_tFileHead.clear();
					m_tFileHead=tFile.read(sizeof(tagFileHead));
					if (m_tFileHead.size()==sizeof(tagFileHead))
					{
						tagFileHead *pFileHead=(tagFileHead*)m_tFileHead.data();
						if (m_tFileHead.contains("JUAN"))
						{
							if (pFileHead->uiIndex<=BUFFERSIZE*1024*1024)
							{
								(QByteArray)m_pDataBuffer=tFile.read(pFileHead->uiIndex);
							}else{
								//do nothing
								qDebug()<<__FUNCTION__<<__LINE__<<"this file is out of length,i will over write it";
							}		
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"this file is undefined type,i will over write it";
							//do nothing
						}
					}else{
						//do nothing
						qDebug()<<__FUNCTION__<<__LINE__<<"this file is undefined type,i will over write it";
					}
					tFile.close();
				}else{
					//出现文件打开错误，启动重启
					qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as open file fail";
					m_tResetType=recordDatReset_fileError;
					nRunStep=recordDat_reset;
					break;
				}
			}
			//写文件头到内存中
			tagFileHead *pFileHead=(tagFileHead*)m_pDataBuffer;
			char *pMagic="JUAN";
			if (memcmp(pMagic,pFileHead->ucMagic,4)==0)
			{
				//do nothing

			}else{
				memcpy(pFileHead->ucMagic,pMagic,4);
				pFileHead->uiChannels[0]=0;
				pFileHead->uiChannels[1]=0;
				pFileHead->uiEnd=0;
				pFileHead->uiStart=0;
				pFileHead->uiVersion=0;
				pFileHead->uiIndex=sizeof(tagFileHead);
				for(int i=0;i<sizeof(tagIFrameIndex);i++){
					pFileHead->tIFrameIndex.uiFirstIFrame[i]=0;
				}
				m_tFileInfo.tFristIFrameIndex.clear();
				m_tFileInfo.tHistoryFrameIndex.clear();
				m_tFileInfo.tHistoryIFrameIndex.clear();
			}
			//建立buffer内所有通道的位置索引
			unsigned int uiCurrentIndex=sizeof(tagFileHead);
			bool bStop=false;
			while(bStop==false){
				if (uiCurrentIndex<pFileHead->uiIndex)
				{
					tagFileFrameHead *pFileFrameHead=(tagFileFrameHead*)(pFileHead+uiCurrentIndex);
					int nChannel=pFileFrameHead->tFrameHead.uiChannel;

					if (pFileFrameHead->tFrameHead.uiType==FT_VideoConfig)
					{
						//通道的第一个I帧
						if (!m_tFileInfo.tFristIFrameIndex.contains(nChannel))
						{
							m_tFileInfo.tFristIFrameIndex.insert(nChannel,uiCurrentIndex);
						}else{
							//do nothing
						}
						//通道的最后一个I帧的位置
						m_tFileInfo.tHistoryIFrameIndex[nChannel]=uiCurrentIndex;
					}else{
						//do nothing
					}
					//通道最后一个帧的位置
					m_tFileInfo.tHistoryFrameIndex.insert(nChannel,uiCurrentIndex);

					uiCurrentIndex=uiCurrentIndex+sizeof(tagFileFrameHead)+pFileFrameHead->tFrameHead.uiLength-sizeof(pFileFrameHead->tFrameHead.pBuffer);
				}else{
					bStop=true;
				}
			}
			nRunStep=recordDat_default;
								  }
								  break;
		case recordDat_writeMemory:{
			//数据帧写到内存块里，数据库条目更新
			//step 1：查找出需要写的通道
			m_tBufferQueueMapLock.lock();
			QMap<int,BufferQueue*>::Iterator tItem=m_tBufferQueueMap.begin();
			int nMinChannel=m_nWriteMemoryChannel;
			int nMinChannelEx=0;
			bool bFlag=false;
			bool bFlagNext=false;
			while(tItem!=m_tBufferQueueMap.end()){
				BufferQueue *pBuffer=tItem.value();
				int nKey=tItem.key();
					bFlag=true;
					if (nKey>m_nWriteMemoryChannel)
					{
						if (bFlagNext==false)
						{
							nMinChannelEx=nKey;
						}else{
							//do nothing
						}
						bFlagNext=true;
						if (nKey<=nMinChannelEx)
						{
							nMinChannelEx=nKey;
						}else{
							//keep going
						}
					}else{
						if (nKey<=nMinChannel)
						{
							nMinChannel=nKey;
						}else{
							//keep going
						}
					}
				tItem++;
			}
			int nWriteToBuffer=0;
			if (bFlag)
			{
				if (!bFlagNext)
				{
					m_nWriteMemoryChannel=nMinChannel;
				}else{
					//do nothing
					m_nWriteMemoryChannel=nMinChannelEx;
				}
				//把数据写到buffer中
				nWriteToBuffer=writeToBuffer(m_nWriteMemoryChannel,sWriteFilePath);
				if (nWriteToBuffer==0)
				{
					//00：buffer未满&&没写入buffer
					nRunStep=recordDat_default;
				}else if (nWriteToBuffer==1)
				{
					//01：buffer未满&&写入buffer 
					nRunStep=recordDat_default;
					nSleepCount=0;
				}else if (nWriteToBuffer==2)
				{
					//10：buffer已满&&未写入buffer；
					m_tToDiskType=recordDatToDiskType_bufferFull;
					nRunStep=recordDat_default;
				}else if (nWriteToBuffer==3)
				{
					//11：buffer已满&&写入buffer
					m_tToDiskType=recordDatToDiskType_bufferFull;
					nRunStep=recordDat_default;
					nSleepCount=0;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as nWriteToBuffer is undefined";
					abort();
				}
			}else{
				nRunStep=recordDat_default;
				sleepEx(10);
			}
			m_tBufferQueueMapLock.unlock();
			nSleepCount++;
			if (nSleepCount>64)
			{
				sleepEx(10);
				nSleepCount=0;
			}else{
				//do nothing
			}
								   }
								   break;
		case recordDat_writeDisk:{
			//内存块写到磁盘
			//step1:解锁文件(数据库置位)--仅仅用于recordDatToDiskType_bufferFull类型
			//step2:更新搜索表
			//step3:更新录像表
			//step4:通知线程写磁盘
			//step5:切换buffer指针--跳转到recordDat_default，buffer内容拷贝到里一个buffer指针（recordDatToDiskType_outOfTime）；切换buffer指针--跳转到recordDat_filePath（recordDatToDiskType_bufferFull）
			bool bFlags=false;
			if (m_tToDiskType==recordDatToDiskType_outOfTime||m_tToDiskType==recordDatToDiskType_bufferFull)
			{
				//step2:更新搜索表
				if (updateSearchDatabase())
				{
					//step3:更新录像表
					if (updateRecordDatabase())
					{
						//step4:通知线程写磁盘
						if (writeTodisk())
						{
							if (m_tToDiskType==recordDatToDiskType_outOfTime)
							{
								//step5:切换buffer指针--跳转到recordDat_default，buffer内容拷贝到里一个buffer指针
								if (m_pDataBuffer!=m_pDataBuffer1)
								{
									memcpy(m_pDataBuffer1,m_pDataBuffer,BUFFERSIZE*1024*1024);
									m_pDataBuffer=m_pDataBuffer1;
								}else{
									memcpy(m_pDataBuffer2,m_pDataBuffer,BUFFERSIZE*1024*1024);
									m_pDataBuffer=m_pDataBuffer2;
								}
								nRunStep=recordDat_default;
							}else{
								//step5:切换buffer指针--跳转到recordDat_filePath
								if (m_pDataBuffer!=m_pDataBuffer1)
								{
									m_pDataBuffer=m_pDataBuffer1;
								}else{
									m_pDataBuffer=m_pDataBuffer2;
								}
								//step1:解锁文件(数据库置位)
								m_tOperationDatabase.setFileIsLock(sWriteFilePath,false);
								nRunStep=recordDat_filePath;
							}
							bFlags=true;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"writeTodisk fail";
							m_tResetType=rrecordDatReset_writeToDisk;
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"updateRecordDatabase fail";
						m_tResetType=rrecordDatReset_recordDatabase;
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"updateSearchRecord fail";
					m_tResetType=recordDatReset_searchDatabase;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as m_tToDiskType is undefined";
				abort();
			}
			m_tToDiskType=recordDatToDiskType_null;
			m_nWriteDiskTimeCount=0;
			if (bFlags)
			{
				//do nothing
			}else{
				nRunStep=recordDat_reset;
			}
								 }
								 break;
		case recordDat_default:{
			//检测是否需要写到磁盘，检测是否有数据帧到达
			//step 1:记录定时写到硬盘的计数
			//step 2:检测是否需要把文件写到磁盘
			//step 3:检测是否有新的buffer需要写到内存
			if (m_bWriteDiskTimeFlags)
			{
				m_nWriteDiskTimeCount++;
				m_bWriteDiskTimeFlags=false;
			}else{
				//do nothing
			}
			if (m_nWriteDiskTimeCount>3)
			{
				m_tToDiskType=recordDatToDiskType_outOfTime;
			}else{
				//do nothing
			}
			if (m_tToDiskType==recordDatToDiskType_null)
			{
				//step frist:检测移出录像的通道，回写数据库
				//step second:写buffer
				m_tBufferQueueMapLock.lock();
				for (int i=0;i<m_tDatabaseInfo.tRemoveChannel.size();i++)
				{
					int nChannel=m_tDatabaseInfo.tRemoveChannel.at(i);
					//回写检索表，录像表
					if (m_tDatabaseInfo.tChannelInRecordDatabaseId.contains(nChannel))
					{
						m_tOperationDatabase.updateRecordDatabase(m_tDatabaseInfo.tChannelInRecordDatabaseId.value(nChannel));
						m_tDatabaseInfo.tChannelInRecordDatabaseId.remove(nChannel);
					}else{
						//do nothing
					}
					if (m_tDatabaseInfo.tChannelInSearchDatabaseId.contains(nChannel))
					{
						m_tOperationDatabase.updateSearchDatabase(m_tDatabaseInfo.tChannelInSearchDatabaseId.value(nChannel));
						m_tDatabaseInfo.tChannelInSearchDatabaseId.remove(nChannel);
					}else{
						//do nothing
					}
					m_tFileInfo.tWndInfo[nChannel].uiHistoryRecordType=0;
				}
				m_tBufferQueueMapLock.unlock();
				nRunStep=recordDat_writeMemory;
			}else
			{
				//把buffer写到硬盘中
				nRunStep=recordDat_writeDisk;
			}
			if (m_bStop)
			{
				nRunStep=recordDat_end;
			}else{
				//do nothing
			}
							   }
							   break;
		case recordDat_reset:{
			//出错后重启
			if (m_tResetType==recordDatReset_fileError)
			{
				//fix me
			}else if (m_tResetType==recordDatReset_outOfDisk)
			{
				//fix me
			}else{
				//fix me
			}
			if (m_bStop)
			{
				nRunStep=recordDat_end;
			}else{
				//do nothing
			}
							 }
							 break;
		case recordDat_error:{
			//不可恢复的出错
							 }
							 break;
		case recordDat_end:{
			//结束
						   }
						   break;
		}
	}
	m_tWriteDiskTimer.stop();
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
	if (!m_tDatabaseInfo.tRemoveChannel.contains(nWnd))
	{
		m_tDatabaseInfo.tRemoveChannel.append(nWnd);
	}else{
		//do nothing
	}
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
	m_tBufferQueueMapLock.lock();
	int nHisRecordType=m_tFileInfo.tWndInfo.value(nWnd).uiHistoryRecordType;
	int nTotal=MANUALRECORD+TIMERECORD+MOTIONRECORD;
	if (nType==MANUALRECORD||nType==MOTIONRECORD||TIMERECORD)
	{
		if (bFlags)
		{
			nHisRecordType=nType|nHisRecordType;
		}else{
			nHisRecordType=(nTotal-nType)&nHisRecordType;
		}
		m_tFileInfo.tWndInfo[nWnd].uiCurrentRecordType=nHisRecordType;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setRecordType Not working as nType is undefined";
	}
	m_tBufferQueueMapLock.unlock();
	return true;
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
			//判断是否覆盖录像
			bool bIsRecover=getIsRecover();
			if (bIsRecover)
			{
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

bool recordDatCore::getIsRecover()
{
	return false;
}

void recordDatCore::sleepEx( quint64 uiTime )
{
	if (m_nSleepSwitch<100)
	{
		msleep(uiTime);
		m_nSleepSwitch++;
	}else{
		QEventLoop eventloop;
		QTimer::singleShot(2, &eventloop, SLOT(quit()));
		eventloop.exec();
		m_nSleepSwitch=0;
	}
	return;
}
int recordDatCore::writeToBuffer( int nChannel,QString sFilePath )
{
	//返回值（按位）：00(0)：buffer未满&&没写入buffer；01(1)：buffer未满&&写入buffer；10(2)：buffer已满&&未写入buffer；11(3)：buffer已满&&写入buffer
	int nFlags=0;
	if (m_tBufferQueueMap.contains(nChannel))
	{
		BufferQueue *pBufferQueue=m_tBufferQueueMap.value(nChannel);
		pBufferQueue->enqueueDataLock();
		tagFileHead *pFileHead=(tagFileHead*)m_pDataBuffer;
		int nHistoryType=m_tFileInfo.tWndInfo.value(nChannel).uiHistoryRecordType;
		int nCurrentType=m_tFileInfo.tWndInfo.value(nChannel).uiCurrentRecordType;
		uint nRecType=0;
		quint64 uiTotalLength=BUFFERSIZE*1024*1024;
		quint64 uiFrameSize=0;
		uint nSearchItemId=0;
		uint nRecordItemId=0;
		int nStep=WriteToBuffer_Init;
		bool bStop=false;
		while(bStop==false){
			switch(nStep){
			case WriteToBuffer_Init:{
				if (nHistoryType==nCurrentType&&nCurrentType==0)
				{
					nStep=WriteToBuffer_00;
				}else if (nHistoryType==0&&nCurrentType!=0)
				{
					nStep=WriteToBuffer_01;
				}else if (nHistoryType!=0&&nCurrentType==0)
				{
					nStep=WriteToBuffer_10;
				}else if (nHistoryType==nCurrentType&&nCurrentType!=0)
				{
					nStep=WriteToBuffer_011;
				}else if (nHistoryType!=0&&nCurrentType!=0)
				{
					nStep=WriteToBuffer_111;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"undefined record type change,terminate the thread";
					abort();
				}
									}
									break;
			case WriteToBuffer_00:{
				//historyType==currentType==0,无任何操作
				nStep=WriteToBuffer_end;
				nFlags=0;
								  }
								  break;
			case WriteToBuffer_01:{
				//historyType==0,currentType!=0,开始录像，需要等待I帧
				//step 1:等待I帧
				//step 2:创建数据库条目
				//step 3:写帧
				bool bFindIFrame=false;
				RecBufferNode *pRecBufferNodeTemp=NULL;
				//step 1:等待I帧
				while(pBufferQueue->isEmpty()==false&&bFindIFrame==false){
					pRecBufferNodeTemp=pBufferQueue->front();
					if (NULL!=pRecBufferNodeTemp)
					{
						tagFrameHead *pFrameHead=NULL;
						pRecBufferNodeTemp->getDataPointer(&pFrameHead);
						if (NULL!=pFrameHead)
						{
							if (pFrameHead->uiType==IFRAME)
							{
								bFindIFrame=true;
							}else{
								RecBufferNode *pRecBufferNodeTakeOut=NULL;
								pRecBufferNodeTakeOut=pBufferQueue->dequeue();
								pRecBufferNodeTakeOut->release();
								pRecBufferNodeTakeOut=NULL;
							}
							pRecBufferNodeTemp->release();
							pFrameHead=NULL;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"there must exist a error,i will terminate the thread";
							abort();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"there must exist a error,i will terminate the thread";
						abort();
					}
				}
				//step 2:创建数据库条目
				if (bFindIFrame)
				{
					quint64 uiStartTime=QDateTime::currentDateTime().toTime_t();
					quint64 uiEndTime=uiStartTime;
					//step1:建立录像数据库条目
					//step2:建立搜索数据库条目
					if (createSearchDatabaseItem(nChannel,uiStartTime,uiEndTime,nCurrentType,nSearchItemId))
					{
						if (createRecordDatabaseItem(nChannel,uiStartTime,uiEndTime,nCurrentType,sFilePath,nRecordItemId))
						{
							m_tDatabaseInfo.tChannelInRecordDatabaseId.insert(nChannel,nRecordItemId);
							m_tDatabaseInfo.tChannelInSearchDatabaseId.insert(nChannel,nSearchItemId);
							m_tFileInfo.tWndInfo[nChannel].uiHistoryRecordType=nCurrentType;
							nHistoryType=nCurrentType;
							if (nHistoryType&MOTIONRECORD)
							{
								nRecType=MOTIONRECORD;
							}else if (nHistoryType&MANUALRECORD)
							{
								nRecType=MANUALRECORD;
							}else if (nHistoryType&TIMERECORD)
							{
								nRecType=TIMERECORD;
							}else{
								//do nothing
							}
							nStep=WriteToBuffer_Write;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"createRecordDatabaseItem fail,there must exist a error,i will terminate the thread";
							abort();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"createSearchDatabaseItem fail,there must exist a error,i will terminate the thread";
						abort();
					}
				}else{
					nFlags=0;
					nStep=WriteToBuffer_end;
				}
								  }
								  break;
			case WriteToBuffer_10:{
				//historyType!=0,currentType==0,停止录像
				//step 1:更新数据库
								  }
								  break;
			case WriteToBuffer_011:{
				//historyType==currentType!=0,类型没有转变，接着录像
				//step 1:写帧
								   }
								   break;
			case WriteToBuffer_111:{
				//historyType!=currentType!=0,类型转换，接着录像
				//step1：更新数据库
				//step 2:创建数据库条目
								   }
								   break;
			case WriteToBuffer_Write:{
				//写一帧数据
				bool bStopWrite=false;
				RecBufferNode *pRecBufferNodeTemp=NULL;
				while(bStopWrite==false&&pBufferQueue->isEmpty()==false){
					nCurrentType=m_tFileInfo.tWndInfo.value(nChannel).uiCurrentRecordType;
					if (nCurrentType==nHistoryType)
					{
						quint64 uiUnusedLength=uiTotalLength-pFileHead->uiIndex;
						pRecBufferNodeTemp=pBufferQueue->front();
						tagFrameHead *pFrameHead=NULL;
						pRecBufferNodeTemp->getDataPointer(&pFrameHead);
						if (NULL!=pFrameHead)
						{
							//step1:判断buffer的长度是否还够
							if (pFrameHead->uiType==IFRAME)
							{
								//补上配置帧
								uiFrameSize=sizeof(tagFileFrameHead)+pFrameHead->uiLength-sizeof(pFrameHead->pBuffer)+sizeof(tagFileFrameHead)+sizeof(tagVideoConfigFrame)-sizeof(pFrameHead->pBuffer);
							}else if (pFrameHead->uiType==PFRAME)
							{
								uiFrameSize=sizeof(tagFileFrameHead)+pFrameHead->uiLength-sizeof(pFrameHead->pBuffer);
							}else if (pFrameHead->uiType==AFRMAE)
							{
								//补上配置帧
								uiFrameSize=sizeof(tagFileFrameHead)+pFrameHead->uiLength-sizeof(pFrameHead->pBuffer)+sizeof(tagFileFrameHead)+sizeof(tagAudioConfigFrame)-sizeof(pFrameHead->pBuffer);
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"there is a undefined frame type,i will terminate the thread";
								abort();
							}
							//step 2;写帧
							if (uiUnusedLength<uiFrameSize)
							{
								//step1:写入配置帧			
								if (pFrameHead->uiType==FT_IFrame)
								{
									tagFileFrameHead *pFileFrameHead=(tagFileFrameHead*)(pFileHead+pFileHead->uiIndex);
									tagFrameHead *pFrameHeadBuffer=(tagFrameHead*)(pFileFrameHead);
									pFrameHeadBuffer->uiChannel=pFrameHead->uiChannel;
									pFrameHeadBuffer->uiExtension=pFrameHead->uiExtension;
									pFrameHeadBuffer->uiGentime=pFrameHead->uiGentime;
									pFrameHeadBuffer->uiLength=sizeof(tagVideoConfigFrame);
									pFrameHeadBuffer->uiPts=pFrameHead->uiPts;
									pFrameHeadBuffer->uiRecType=nRecType;
									pFrameHeadBuffer->uiSessionId=nRecordItemId;
									pFrameHeadBuffer->uiType=FT_VideoConfig;

									tagVideoConfigFrame *pVideoConfigFrame=(tagVideoConfigFrame*)(pFileFrameHead+sizeof(tagFileFrameHead)+sizeof(tagVideoConfigFrame)-sizeof(pFrameHeadBuffer->pBuffer));
									tagVideoConfigFrame *pVideoConfigFrameTemp=(tagVideoConfigFrame*)(pFrameHead+sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(char*));
									pVideoConfigFrame->uiHeight=pVideoConfigFrameTemp->uiHeight;
									pVideoConfigFrame->uiWidth=pVideoConfigFrameTemp->uiWidth;
									memcpy(pVideoConfigFrame->ucReversed,pVideoConfigFrameTemp->ucReversed,4);
									memcpy(pVideoConfigFrame->ucVideoDec,pVideoConfigFrameTemp->ucVideoDec,4);
									//tFristIFrameIndex
									if (!m_tFileInfo.tFristIFrameIndex.contains(pFrameHead->uiChannel))
									{
										m_tFileInfo.tFristIFrameIndex.insert(pFrameHead->uiChannel,pFileHead->uiIndex);
										pFileHead->tIFrameIndex.uiFirstIFrame[pFrameHead->uiChannel]=pFileHead->uiIndex;
									}else{
										//do nothing
									}
									//tHistoryIFrameIndex
									//set uiPreIFrame
									//set uiNextIFrame
									if (!m_tFileInfo.tHistoryIFrameIndex.contains(nChannel))
									{
										m_tFileInfo.tHistoryIFrameIndex.insert(nChannel,pFileHead->uiIndex);
										pFileFrameHead->tPerFrameIndex.uiPreIFrame=0;
									}else{
										pFileFrameHead->tPerFrameIndex.uiPreIFrame=m_tFileInfo.tHistoryIFrameIndex.value(nChannel);
										m_tFileInfo.tHistoryIFrameIndex.insert(nChannel,pFileHead->uiIndex);
										tagFileFrameHead *pLastFrameHead=(tagFileFrameHead*)(pFileHead+pFileFrameHead->tPerFrameIndex.uiPreIFrame);
										pLastFrameHead->tPerFrameIndex.uiNextIFrame=pFileHead->uiIndex;
									}
									//tHistoryFrameIndex
									//set uiPreFrame
									//set uiNextFrame
									if (!m_tFileInfo.tHistoryFrameIndex.contains(nChannel))
									{
										m_tFileInfo.tHistoryFrameIndex.insert(nChannel,pFileHead->uiIndex);
										pFileFrameHead->tPerFrameIndex.uiPreFrame=0;
									}else{
										pFileFrameHead->tPerFrameIndex.uiPreFrame=m_tFileInfo.tHistoryFrameIndex.value(nChannel);
										m_tFileInfo.tHistoryFrameIndex.insert(nChannel,pFileHead->uiIndex);
										tagFileFrameHead *pLastFrameHead=(tagFileFrameHead*)(pFileHead+pFileFrameHead->tPerFrameIndex.uiPreFrame);
										pLastFrameHead->tPerFrameIndex.uiNextFrame=pFileHead->uiIndex;
									}

									pFileHead->uiIndex=pFileHead->uiIndex+sizeof(tagFileFrameHead)+sizeof(tagVideoConfigFrame)-sizeof(pFileFrameHead->tFrameHead.pBuffer);
								}else if (pFrameHead->uiType==FT_Audio)
								{

								}else if (pFrameHead->uiType==FT_PFrame)
								{
									//do nothing
								}else{
									qDebug()<<__FUNCTION__<<__LINE__<<"there is a undefined frame type,i will terminate the thread";
									abort();
								}
								//step2:写入帧

								//step 2:写入帧数据
								tagFileFrameHead *pFileFrameHead=(tagFileFrameHead*)(pFileHead+pFileHead->uiIndex);
								tagFrameHead *pFrameHeadBuffer=(tagFrameHead*)(pFileFrameHead);
								pFrameHeadBuffer->uiChannel=pFrameHead->uiChannel;
								pFrameHeadBuffer->uiExtension=pFrameHead->uiExtension;
								pFrameHeadBuffer->uiGentime=pFrameHead->uiGentime;
								pFrameHeadBuffer->uiLength=pFrameHead->uiLength;
								pFrameHeadBuffer->uiPts=pFrameHead->uiPts;
								pFrameHeadBuffer->uiRecType=nRecType;
								pFrameHeadBuffer->uiSessionId=nRecordItemId;
								pFrameHeadBuffer->uiType=pFrameHead->uiType;
								memcpy(pFrameHeadBuffer->pBuffer,pFrameHead->pBuffer,pFrameHead->uiLength);

								//set uiPreIFrame
								//set uiNextIFrame
								if (m_tFileInfo.tHistoryIFrameIndex.contains(nChannel))
								{
									pFileFrameHead->tPerFrameIndex.uiPreIFrame=m_tFileInfo.tHistoryIFrameIndex.value(nChannel);
								}else{
									pFileFrameHead->tPerFrameIndex.uiPreIFrame=0;
								}
								pFileFrameHead->tPerFrameIndex.uiNextIFrame=0;

								//set uiPreFrame
								//set uiNextFrame
								if (m_tFileInfo.tHistoryFrameIndex.contains(nChannel))
								{
									pFileFrameHead->tPerFrameIndex.uiPreFrame=m_tFileInfo.tHistoryFrameIndex.value(nChannel);
									tagFileFrameHead *pLastFileFrameHead=(tagFileFrameHead*)(pFileHead+pFileFrameHead->tPerFrameIndex.uiPreFrame);
									pLastFileFrameHead->tPerFrameIndex.uiNextFrame=pFileHead->uiIndex;
								}else{
									pFileFrameHead->tPerFrameIndex.uiPreFrame=0;
								}
								pFileFrameHead->tPerFrameIndex.uiNextFrame=0;
								m_tFileInfo.tHistoryFrameIndex.insert(nChannel,pFileHead->uiIndex);

								pFileHead->uiIndex=pFileHead->uiIndex+sizeof(tagFileFrameHead)+pFileFrameHead->tFrameHead.uiLength-sizeof(pFrameHead->pBuffer);
								nFlags=nFlags|1;

								//删除节点数据
								RecBufferNode *pRecBufferNodeToken=pBufferQueue->dequeue();
								pRecBufferNodeToken->release();
								pRecBufferNodeToken=NULL;
							}else{
								nStep=WriteToBuffer_end;
								bStopWrite=true;
								nFlags=nFlags|2;
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"there must exist a error,i will terminate the thread";
							abort();
						}
						pRecBufferNodeTemp->release();
						pRecBufferNodeTemp=NULL;
					}else{
						nStep=WriteToBuffer_end;
						bStopWrite=true;
					}
				}
									 }
									 break;
			case WriteToBuffer_end:{

								   }
			}
		}
		return nFlags;
	}else{
		return nFlags;
	}
}
int recordDatCore::writeToBufferEx( int nChannel ,QString sFilePath)
{
	//返回值（按位）：00(0)：buffer未满&&没写入buffer；01(1)：buffer未满&&写入buffer；10(2)：buffer已满&&未写入buffer；11(3)：buffer已满&&写入buffer
	int nHistoryType=m_tFileInfo.tWndInfo.value(nChannel).uiHistoryRecordType;
	int nCurrentType=m_tFileInfo.tWndInfo.value(nChannel).uiCurrentRecordType;
	uint nRecType=0;
	if (m_tBufferQueueMap.contains(nChannel))
	{
		BufferQueue *pBufferQueue=m_tBufferQueueMap.value(nChannel);
		pBufferQueue->enqueueDataLock();
		tagFileHead *pFileHead=(tagFileHead*)m_pDataBuffer;
		quint64 uiTotalLength=BUFFERSIZE*1024*1024;
		quint64 uiFrameSize=0;
		if (nHistoryType==nCurrentType&&nHistoryType==0)
		{
			//historyType==currentType==0,无任何操作
			pBufferQueue->enqueueDataUnLock();
			return 0;
		}else if (nHistoryType==0&&nCurrentType!=0)
		{
			//historyType==0,currentType!=0,开始录像，需要等待I帧
			//如果等到I帧，则进行状态转换，建立录像数据库条目和搜索数据库条目
			int nStep=0;
			bool bStop=false;
			int nFlags=0;
			RecBufferNode *pRecBufferNodeTemp=NULL;
			while(!pBufferQueue->isEmpty()&&bStop==false){
				switch(nStep){
				case 0:{
					//去掉第一个I帧前的P帧
					pRecBufferNodeTemp=pBufferQueue->front();
					if (pRecBufferNodeTemp!=NULL)
					{
						tagFrameHead *pFrameHead=NULL;
						pRecBufferNodeTemp->getDataPointer(&pFrameHead);
						if (pFrameHead!=NULL)
						{
							if (pFrameHead->uiType==IFRAME)
							{
								nStep=1;
							}else{
								RecBufferNode *pRecBufferNodeTakeOut=NULL;
								pRecBufferNodeTakeOut=pBufferQueue->dequeue();
								pRecBufferNodeTakeOut->release();
								pRecBufferNodeTakeOut=NULL;
								nStep=0;
							}
							pRecBufferNodeTemp->release();
							pFrameHead=NULL;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"there must exist a error,i will terminate the thread";
							abort();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"there must exist a error,i will terminate the thread";
						abort();
					}
					   }
					   break;
				case 1:{
					//找到第一个I帧
					//step1:建立录像数据库条目
					//step2:建立搜索数据库条目
					uint nSearchItemId=0;
					uint nRecordItemId=0;
					quint64 uiStartTime=QDateTime::currentDateTime().toTime_t();
					quint64 uiEndTime=uiStartTime;
					if (createSearchDatabaseItem(nChannel,uiStartTime,uiEndTime,nCurrentType,nSearchItemId))
					{
						if (createRecordDatabaseItem(nChannel,uiStartTime,uiEndTime,nCurrentType,sFilePath,nRecordItemId))
						{
							m_tDatabaseInfo.tChannelInRecordDatabaseId.insert(nChannel,nRecordItemId);
							m_tDatabaseInfo.tChannelInSearchDatabaseId.insert(nChannel,nSearchItemId);
							m_tFileInfo.tWndInfo[nChannel].uiHistoryRecordType=nCurrentType;
							nHistoryType=nCurrentType;
							if (nHistoryType&MOTIONRECORD)
							{
								nRecType=MOTIONRECORD;
							}else if (nHistoryType&MANUALRECORD)
							{
								nRecType=MANUALRECORD;
							}else if (nHistoryType&TIMERECORD)
							{
								nRecType=TIMERECORD;
							}else{
								//do nothing
							}
							nStep=2;
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"there must exist a error,i will terminate the thread";
							abort();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"there must exist a error,i will terminate the thread";
						abort();
					}
					   }
					   break;
				case 2:{
					//写入帧
					nCurrentType=m_tFileInfo.tWndInfo.value(nChannel).uiCurrentRecordType;
					if (nCurrentType==nHistoryType)
					{
						//判断bubbfer长度是否还够写入一帧
						quint64 uiUnusedLength=uiTotalLength-pFileHead->uiIndex;
						pRecBufferNodeTemp=NULL;
						pRecBufferNodeTemp=pBufferQueue->front();
						tagFrameHead *pFrameHead=NULL;
						pRecBufferNodeTemp->getDataPointer(&pFrameHead);
						if (pFrameHead!=NULL)
						{
							if (pFrameHead->uiType==IFRAME)
							{
								//I帧前需要补上一个配置帧
								uiFrameSize=sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(pFrameHead->pBuffer)+sizeof(tagVideoConfigFrame)+sizeof(tagPerFrameIndex);
							}else if (pFrameHead->uiType==PFRAME)
							{
								uiFrameSize=sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(pFrameHead->pBuffer)+sizeof(tagPerFrameIndex);
							}else{
								//音频帧前需要补上一个配置帧
								uiFrameSize=sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(pFrameHead->pBuffer)+sizeof(tagAudioConfigFrame)+sizeof(tagPerFrameIndex);
							}
							
							if (uiFrameSize<uiUnusedLength)
							{
								//buffer的长度还足够
								pRecBufferNodeTemp->release();
								pRecBufferNodeTemp=NULL;
								pRecBufferNodeTemp=pBufferQueue->dequeue();
								pFrameHead=NULL;
								pRecBufferNodeTemp->getDataPointer(&pFrameHead);
								if (pFrameHead->uiType==IFRAME)
								{
									//step1:写入配置帧
									tagVideoConfigFrame *pVideoConfigFrame=(tagVideoConfigFrame*)(pFileHead+pFileHead->uiIndex);
									tagVideoConfigFrame *pVideoConfigFramTemp=(tagVideoConfigFrame*)(pFrameHead+sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(char*));
									pVideoConfigFrame->uiHeight=pVideoConfigFramTemp->uiHeight;
									pVideoConfigFrame->uiWidth=pVideoConfigFramTemp->uiWidth;

									//偏移文件最后写入位置的索引
									pFileHead->uiIndex=pFileHead->uiIndex+sizeof(tagVideoConfigFrame);
								}else if (pFrameHead->uiType==AFRMAE)
								{
									//step1:写入配置帧
									tagAudioConfigFrame *pAudioConfigFrame=(tagAudioConfigFrame*)(pFileHead+pFileHead->uiIndex);
									tagAudioConfigFrame *pAudioConfigFrameTemp=(tagAudioConfigFrame*)(pFrameHead+sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(char*));
									pAudioConfigFrame->uiChannels=pAudioConfigFrameTemp->uiChannels;
									pAudioConfigFrame->uiSamplebit=pAudioConfigFrameTemp->uiSamplebit;
									pAudioConfigFrame->uiSamplerate=pAudioConfigFrameTemp->uiSamplerate;

									//偏移文件最后写入位置的索引
									pFileHead->uiIndex=pFileHead->uiIndex+sizeof(tagAudioConfigFrame);
								}else {
									// do nothing
								}
								if (pFrameHead->uiType==IFRAME||pFrameHead->uiType==PFRAME||pFrameHead->uiType==AFRMAE)
								{				
									//step 2:写入帧数据
									tagFrameHead *pFrameHeadBuffer=(tagFrameHead*)(pFileHead+pFileHead->uiIndex);
									pFrameHeadBuffer->uiChannel=pFrameHead->uiChannel;
									pFrameHeadBuffer->uiGentime=pFrameHead->uiGentime;
									pFrameHeadBuffer->uiLength=pFrameHead->uiLength;
									pFrameHeadBuffer->uiPts=pFrameHead->uiPts;
									pFrameHeadBuffer->uiRecType=nRecType;
									pFrameHeadBuffer->uiSessionId=m_tDatabaseInfo.tChannelInRecordDatabaseId.value(nChannel);
									pFrameHeadBuffer->uiType=pFrameHead->uiType;
									memcpy(pFrameHeadBuffer->pBuffer,pFrameHead->pBuffer,pFrameHead->uiLength);

									//偏移文件最后写入位置的索引
									pFileHead->uiIndex=pFileHead->uiIndex+sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(char*);

									//step 3:写入帧之间的位置索引关系
									
								}else{
									qDebug()<<__FUNCTION__<<__LINE__<<"undefined frame type ,i will terminate the thread";
									abort();
								}
								nStep=2;
								nFlags=nFlags|1;
								pRecBufferNodeTemp->release();
								pRecBufferNodeTemp=NULL;
							}else{
								//buffer长度不足
								pRecBufferNodeTemp->release();
								pRecBufferNodeTemp=NULL;
								nStep=3;
								nFlags=nFlags|2;
							}
							
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"there must exist a error,i will terminate the thread";
							abort();
						}
					}else{
						nStep=3;
					}
					   }
					   break;
				case 3:{
					//结束
					   }
					   break;
				}
			}
		}else if (nHistoryType!=0&&nCurrentType==0)
		{
			//historyType!=0,currentType==0,停止录像
			//step1:回写录像数据库，回写搜索数据库
		}else if (nHistoryType==nCurrentType&&nCurrentType!=0)
		{
			//historyType==currentType!=0,类型没有转变，接着录像
		}else if (nHistoryType!=nCurrentType&&nCurrentType!=0&&nCurrentType!=0)
		{
			//historyType!=currentType!=0,类型转换，接着录像
			//step1:回写录像数据库，建立新的一条录像记录
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"terminate record as tagRecordDatTurnType is undefined";
			abort();
		}
	}else{
		return 0;
	}
	return 0;
}

void recordDatCore::slsetWriteDiskFlag()
{
	m_bWriteDiskTimeFlags=true;
}

bool recordDatCore::updateSearchDatabase()
{
	return false;
}

bool recordDatCore::updateRecordDatabase()
{
	return false;
}

bool recordDatCore::writeTodisk()
{
	return false;
}

bool recordDatCore::createSearchDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,uint &uiItemId )
{
	return m_tOperationDatabase.createSearchDatabaseItem(nChannel,uiStartTime,uiEndTime,uiType, uiItemId);
}

bool recordDatCore::createRecordDatabaseItem( int nChannel,quint64 uiStartTime,quint64 uiEndTime,uint uiType,QString sFileName,uint &uiItemId )
{
	return m_tOperationDatabase.createRecordDatabaseItem(nChannel,uiStartTime,uiEndTime,uiType,sFileName, uiItemId);
}





