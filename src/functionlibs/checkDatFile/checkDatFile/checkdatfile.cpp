#include "checkdatfile.h"
#include <QMessageBox>

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
checkDatFile::checkDatFile(QWidget *parent):m_pPushButton(NULL),
	m_pLayout(NULL),
	m_pText(NULL)
{
	m_pLayout=new QVBoxLayout(this);
	m_pPushButton=new QPushButton;
	m_pText=new QTextEdit;
	m_pLayout->addWidget(m_pPushButton);
	m_pLayout->addWidget(m_pText);
	connect(m_pPushButton,SIGNAL(pressed()),this,SLOT(slCheckFile()));
	m_pPushButton->setText(tr("checkDatFile"));
	m_tRecordType.append(MOTIONRECORD);
	m_tRecordType.append(MANUALRECORD);
	m_tRecordType.append(TIMERECORD);
}

checkDatFile::~checkDatFile()
{
	deInitMgrDataBase((quintptr*)this);
	if (NULL!=m_pPushButton)
	{
		delete m_pPushButton;
		m_pPushButton=NULL;
	}
	if (NULL!=m_pLayout)
	{
		delete m_pLayout;
		m_pLayout=NULL;
	}
	if (NULL!=m_pText)
	{
		delete m_pText;
		m_pText=NULL;
	}
}

void checkDatFile::slCheckFile()
{
	qDebug()<<__FUNCTION__<<__LINE__<<"=======start=======";

	deInitMgrDataBase((quintptr*)this);
	QString sAllDisk=getRecordDisk();
	QStringList tDiskLisk=sAllDisk.split(",");
	foreach(QString sDiskItem,tDiskLisk){
		checkDiskItem(sDiskItem);
		builtSearch_recordItem(sDiskItem);
	}
	qDebug()<<__FUNCTION__<<__LINE__<<"=======end=======";
}

QString checkDatFile::getRecordDisk()
{
	return "D,E,F,G,H";
}

void checkDatFile::checkDiskItem( QString sDiskTtem )
{
	//D:/recEx/0000/0000/0000/0001.dat
	//D:/recEx/i/j/k/l.dat
	QString sDirPath=sDiskTtem+":/recEx";
	QDir tDir;
	if (tDir.exists(sDirPath))
	{
		//i 表示一级目录
		m_sDatabasePath=sDiskTtem+":/recEx/record_rebuilt.db";
		if (createRecordDatabase(m_sDatabasePath))
		{
			//keep going
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"abort the thread as createRecordDatabase fail";
			abort();
			return;
		}
		for (int i=0;i<DIRSIZE;i++)
		{
			sDirPath=sDiskTtem+":/recEx/0000";
			QString sI=QString::number(i);
			QString sFirstStepDir=sDirPath.replace(sDirPath.size()-sI.size(),sI.size(),sI);
			if (tDir.exists(sFirstStepDir))
			{
				//j 表示二级目录
				for (int j=0;j<DIRSIZE;j++)
				{
					QString sSecondStepDir=sFirstStepDir+"/0000";
					QString sI=QString::number(j);
					sSecondStepDir=sSecondStepDir.replace(sSecondStepDir.size()-sI.size(),sI.size(),sI);
					if (tDir.exists(sSecondStepDir))
					{
						//k 表示 三级目录
						for (int k=0;k<DIRSIZE;k++)
						{
							QString sThirdStepDir=sSecondStepDir+"/0000";
							QString sI=QString::number(k);
							sThirdStepDir=sThirdStepDir.replace(sThirdStepDir.size()-sI.size(),sI.size(),sI);
							if (tDir.exists(sThirdStepDir))
							{
								//文件层
								bool bCreateRecordFileStatusItem=false;
								for (int n=255;n>=0;n--)
								{
									QString sDatFilePath=sThirdStepDir+"/0000.dat";
									QString sFileNum=QString::number(n);
									sDatFilePath=sDatFilePath.replace(sDatFilePath.size()-4-sFileNum.size(),sFileNum.size(),sFileNum);
									QFile tDatFile;
									tDatFile.setFileName(sDatFilePath);
									if (tDatFile.exists())
									{
										if (bCreateRecordFileStatusItem==false)
										{
											for (int m=n;m>=0;m--)
											{
												QString sRecordFileStatusPath=sThirdStepDir+"/0000.dat";
												QString sRecordFileStatusNum=QString::number(m);
												sRecordFileStatusPath=sRecordFileStatusPath.replace(sRecordFileStatusPath.size()-4-sRecordFileStatusNum.size(),sRecordFileStatusNum.size(),sRecordFileStatusNum);
												quint64 uiFileNum=256*(256*(256*i+j)+k)+m;
												if (createRecordFileStatusItem(uiFileNum,sRecordFileStatusPath))
												{
													//keep going
												}else{
													abort();
												}
											}
											bCreateRecordFileStatusItem=true;
										}else{
											//do nothing
										}
										//解析文件
										analysisDatFile(sDatFilePath);
									}else{
										//do nothing
									}
								}
							}else{
								//do nothing
							}
						}
					}else{
						//do nothing
					}
				}
			}else{
				//keep going
			}
		}
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"this disk:"<<sDiskTtem<<"do no exists any record";
	}
}

bool checkDatFile::createRecordDatabase( QString sDatabasePath )
{
	QFile tFile;
	tFile.setFileName(sDatabasePath);
	if (tFile.exists())
	{
		if (tFile.remove())
		{
		}else{
			QMessageBox msgBox;
			QString sText=QString("please close file '%1'").arg(sDatabasePath);
			msgBox.setText(sText);
			msgBox.setInformativeText("if you had closed,click 'Ok' button,if not click 'Cancel' button");
			msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Ok);
			int ret = msgBox.exec();
			switch (ret) {
			case QMessageBox::Ok:
				// Save was clicked
				if (tFile.remove())
				{
				}else{
					abort();
				}
				break;
			case QMessageBox::Cancel:
				// Don't Save was clicked
				abort();
				break;
			default:
				// should never be reached
				break;
			}
		}
	}else{
		//do nothing
	}
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
				qDebug()<<__FUNCTION__<<__LINE__<<"create dirpath fail:"<<sDirPath;
				abort();
			}
		}else{
			//do nothing
		}
		if (tFile.open(QIODevice::ReadWrite))
		{
			tFile.close();
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"create file fail:"<<sDatabasePath;
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

void checkDatFile::analysisDatFile( QString sFilePath )
{
	qDebug()<<__FUNCTION__<<__LINE__<<"start analysisFile :"<<sFilePath;
	m_pText->setText("start analysisFile :");
	if (isJUANRecordDatFile(sFilePath))
	{
		//done
		printfFileData(sFilePath);
	}else{
		QString sKey="nDamage";
		int nFlags=1;
		if (setRecordFileStatus(sFilePath,sKey,nFlags))
		{
			//keep going
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"setRecordFileStatus fail";
		}
	}
	qDebug()<<__FUNCTION__<<__LINE__<<"stop analysisFile :"<<sFilePath;
}

bool checkDatFile::createRecordFileStatusItem( quint64 uiFileNum,QString sFilePath )
{
	QSqlDatabase *pDatabase=NULL;
	pDatabase=initMgrDataBase(m_sDatabasePath,(quintptr*)this);
	QSqlQuery _query(*pDatabase);
	if (NULL!=pDatabase)
	{
		QString sCommand=QString("insert into RecordFileStatus(sFilePath,nLock,nDamage,nInUse,nFileNum) values('%1',0,0,0,%2)").arg(sFilePath).arg(uiFileNum);
		if (_query.exec(sCommand))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
			abort();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null,terminate the thread";
		abort();
	}
	return false;
}

bool checkDatFile::setRecordFileStatus( QString sFilePath,QString sKey,int nFlags )
{
	QSqlDatabase *pDatabase=NULL;
	pDatabase=initMgrDataBase(m_sDatabasePath,(quintptr*)this);
	QSqlQuery _query(*pDatabase);
	if (NULL!=pDatabase)
	{
		QString sCommand=QString("update RecordFileStatus set '%1'=%2 where sFilePath ='%3'").arg(sKey).arg(nFlags).arg(sFilePath);
		if (_query.exec(sCommand))
		{
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
			abort();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null,terminate the thread";
		abort();
	}
	return false;
}

bool checkDatFile::isJUANRecordDatFile( QString sFilePath )
{
	QFile tFile;
	tFile.setFileName(sFilePath);
	if (tFile.exists())
	{
		if (tFile.open(QIODevice::ReadWrite))
		{
			m_tFileData.clear();
			m_tFileData=tFile.readAll();
			if (m_tFileData.length()>sizeof(tagFileHead))
			{
				tagFileHead *pFileHead=(tagFileHead*)m_tFileData.data();
				//test 1:检测文件中的每一帧
				//生成的信息有：
				//1.文件头信息
				//2.每一个通道录像数据库信息
				if (testPerFrame(pFileHead,sFilePath))
				{
					if (saveItemToDatabase(sFilePath))
					{
						//done
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"saveItemToDatabase fail ,i will abort the thread";
						abort();
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"testPerFrame fail,i will abort the thread";
					tFile.close();
					return false;
				}
			}else{
				//do nothing
			}
			tFile.close();
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<sFilePath<<" will set to nDamage,as it can not been open";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<sFilePath<<"is not exist";
		abort();
	}
	return false;
}

void checkDatFile::clearDatFileInfo()
{
	for (int i=0;i<64;i++)
	{
		m_tFileHeadInfo.tIFrameIndex.uiFirstIFrame[i]=0;
	}
	memset(m_tFileHeadInfo.ucMagic,0,4);
	m_tFileHeadInfo.uiVersion=0;
	m_tFileHeadInfo.uiChannels[0]=0;
	m_tFileHeadInfo.uiChannels[1]=0;
	m_tFileHeadInfo.uiStart=0;
	m_tFileHeadInfo.uiEnd=0;
	m_tFileHeadInfo.uiIndex=0;
	//
	m_tWndRecordItemInfo.clear();
}

bool checkDatFile::testPerFrame( tagFileHead *pFileHead ,QString sFilePath)
{
	//test 1:检测文件中的每一帧
	//生成的信息有：
	//1.文件头信息
	//2.每一个通道录像数据库信息
	clearDatFileInfo();
	char *pMagic="JUAN";
	if (memcmp(pMagic,pFileHead->ucMagic,sizeof(pFileHead->ucMagic))==0)
	{
		m_tFileHeadInfo.uiVersion=pFileHead->uiVersion;
		m_tFileHeadInfo.uiChannels[0]=pFileHead->uiChannels[0];
		m_tFileHeadInfo.uiChannels[1]=pFileHead->uiChannels[1];
		m_tFileHeadInfo.uiStart=pFileHead->uiStart;
		m_tFileHeadInfo.uiEnd=pFileHead->uiEnd;
		int nSize=sizeof(m_tFileHeadInfo.tIFrameIndex.uiFirstIFrame)/sizeof(unsigned int);
		for (int i=0;i<nSize;i++)
		{
			m_tFileHeadInfo.tIFrameIndex.uiFirstIFrame[i]=pFileHead->tIFrameIndex.uiFirstIFrame[i];
		}
		//step1:test uiStartTime and uiEndTime
		unsigned int uiCurrentIndex=sizeof(tagFileHead);
		if (uiCurrentIndex<pFileHead->uiIndex)
		{
			quint64 uiCurrentTime=QDateTime::currentDateTime().toTime_t();
			if (m_tFileHeadInfo.uiStart>uiCurrentTime||m_tFileHeadInfo.uiEnd>uiCurrentTime||m_tFileHeadInfo.uiStart>m_tFileHeadInfo.uiEnd||m_tFileHeadInfo.uiStart<=0||m_tFileHeadInfo.uiEnd<=0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"uiCurrentTime;"<<uiCurrentTime<<"uiStartTime:"<<m_tFileHeadInfo.uiStart<<"uiEndTime"<<m_tFileHeadInfo.uiEnd;
				abort();
			}else{
				//keep going
				QString sStartTime=QDateTime::fromTime_t(m_tFileHeadInfo.uiStart).toString("dd.MM.yyyy");
				QString sEndTime=QDateTime::fromTime_t(m_tFileHeadInfo.uiEnd).toString("dd.MM.yyyy");
				QString sCmpTime="01.01.2014";
				if (sEndTime<sCmpTime||sStartTime<sCmpTime)
				{
					qDebug()<<__FUNCTION__<<__LINE__<<"uiStartTime"<<sStartTime<<"uiEndTime"<<sEndTime;
					abort();
				}else{
					//keep going
				}
			}
		}else{
			//do nothing
		}
		//step 2: test frame
		
		unsigned int uiFileSize=DATFILESIZE*1024*1024;
		if (pFileHead->uiIndex>=uiFileSize)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<"pFileHead->uiIndex out of range";
			abort();
		}else{
			//keep going
		}

		while(uiCurrentIndex<uiFileSize&&uiCurrentIndex<pFileHead->uiIndex){
			tagFileFrameHead *pFileFrameHead=(tagFileFrameHead*)((char*)pFileHead+uiCurrentIndex);
			//step 2.1:检测 帧头
			if (testFrameHeadInfo(pFileFrameHead,sFilePath))
			{
				//step 2.2:检测每一帧的位置信息
				if (testPerFrameIndex(pFileFrameHead,sFilePath,uiCurrentIndex))
				{
					uiCurrentIndex=uiCurrentIndex+pFileFrameHead->tFrameHead.uiLength+sizeof(tagFileFrameHead)-sizeof(pFileFrameHead->tFrameHead.pBuffer);
				}else{
					abort();
				}
			}else{
				abort();
			}
		}
		return true;
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<sFilePath<<"can been analysis";
	}
	return false;
}

bool checkDatFile::testFrameHeadInfo( tagFileFrameHead* pFileFrameHead,QString sFilePath )
{
	if (pFileFrameHead->tFrameHead.uiType==FT_Audio||pFileFrameHead->tFrameHead.uiType==FT_IFrame||pFileFrameHead->tFrameHead.uiType==FT_PFrame||pFileFrameHead->tFrameHead.uiType==FT_AudioConfig||pFileFrameHead->tFrameHead.uiType==FT_VideoConfig)
	{
		if (pFileFrameHead->tFrameHead.uiLength>=0&&pFileFrameHead->tFrameHead.uiLength<FRAMEMAXLENGTH*1024*1024)
		{
			if (pFileFrameHead->tFrameHead.uiChannel>=0&&pFileFrameHead->tFrameHead.uiChannel<WNDNUM)
			{
				quint64 uiCurrentTime=QDateTime::currentDateTime().toTime_t();
				QDateTime tCmpTime=QDateTime::fromString("01.01.2014","dd.MM.yyyy");
				quint64 uiCmpTime=tCmpTime.toTime_t();
				if (pFileFrameHead->tFrameHead.uiGentime>uiCmpTime&&pFileFrameHead->tFrameHead.uiGentime<uiCurrentTime)
				{
					if (pFileFrameHead->tFrameHead.uiRecType==MOTIONRECORD||pFileFrameHead->tFrameHead.uiRecType==TIMERECORD||pFileFrameHead->tFrameHead.uiRecType==MANUALRECORD)
					{
						return true;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"pFileFrameHead->tFrameHead.uiRecType:"<<pFileFrameHead->tFrameHead.uiRecType<<"out of range";
						abort();
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"pFileFrameHead->tFrameHead.uiGentime:"<<pFileFrameHead->tFrameHead.uiGentime<<"out of range";
					abort();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"pFileFrameHead->tFrameHead.uiChannel:"<<pFileFrameHead->tFrameHead.uiChannel<<"out of range";
				abort();
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"pFileFrameHead->tFrameHead.uiLength:"<<pFileFrameHead->tFrameHead.uiLength<<"out of range";
			abort();
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"pFileFrameHead->tFrameHead.uiType :"<<pFileFrameHead->tFrameHead.uiType<<"undefined";
		abort();
	}
	return false;
}

bool checkDatFile::testPerFrameIndex( tagFileFrameHead* pFileFrameHead ,QString sFilePath ,quint64 uiCurrentIndex)
{
	//不检测帧的数据部分
	//step 1:检测文件头中 是否有这一帧的信息
	unsigned int uiWnd=pFileFrameHead->tFrameHead.uiChannel;
	int nFlags=0;
	if (uiWnd>31)
	{
		nFlags=m_tFileHeadInfo.uiChannels[1]&&(1<<(uiWnd-32));
	}else{
		nFlags=m_tFileHeadInfo.uiChannels[0]&&(1<<uiWnd);
	}
	if (nFlags==1)
	{
		//keep going
		if (pFileFrameHead->tFrameHead.uiType==FT_VideoConfig)
		{
			//I 帧
			if (m_tFileHeadInfo.tIFrameIndex.uiFirstIFrame[uiWnd]!=0)
			{
				if (m_tWndRecordItemInfo.contains(uiWnd))
				{
					//do nothing
				}else{
					tagWndRecordItemInfo tWndRecordItemInfo;
					tWndRecordItemInfo.uiWndId=uiWnd;
					tWndRecordItemInfo.uiHisRecordId=pFileFrameHead->tFrameHead.uiSessionId;
					tWndRecordItemInfo.uiFristIFrameIndex=0;
					tWndRecordItemInfo.uiHistoryFrameIndex=0;
					tWndRecordItemInfo.uiHistoryIFrameIndex=0;
					tWndRecordItemInfo.uiNextFrameIndex=uiCurrentIndex;
					tWndRecordItemInfo.uiNextIFrameIndex=uiCurrentIndex;
					tWndRecordItemInfo.sFilePath=sFilePath;
					tagRecordItemInfo tRecordItemInfo;
					tRecordItemInfo.uiRecordType=pFileFrameHead->tFrameHead.uiRecType;
					tRecordItemInfo.uiStartTime=pFileFrameHead->tFrameHead.uiGentime;
					tRecordItemInfo.uiEndTime=pFileFrameHead->tFrameHead.uiGentime;
					tWndRecordItemInfo.tRecordItemList.append(tRecordItemInfo);
					m_tWndRecordItemInfo.insert(uiWnd,tWndRecordItemInfo);
				}
				tagWndRecordItemInfo tWndRecordItemInfo=m_tWndRecordItemInfo.value(uiWnd);
				if (tWndRecordItemInfo.uiHistoryFrameIndex==pFileFrameHead->tPerFrameIndex.uiPreFrame)
				{
					if (tWndRecordItemInfo.uiHistoryIFrameIndex==pFileFrameHead->tPerFrameIndex.uiPreIFrame)
					{
						if ((tWndRecordItemInfo.uiNextIFrameIndex==uiCurrentIndex||tWndRecordItemInfo.uiNextIFrameIndex==0)&&tWndRecordItemInfo.uiNextFrameIndex==uiCurrentIndex)
						{
							tWndRecordItemInfo.uiHistoryFrameIndex=uiCurrentIndex;
							tWndRecordItemInfo.uiHistoryIFrameIndex=uiCurrentIndex;
							tWndRecordItemInfo.uiNextFrameIndex=pFileFrameHead->tPerFrameIndex.uiNextFrame;
							tWndRecordItemInfo.uiNextIFrameIndex=pFileFrameHead->tPerFrameIndex.uiNextIFrame;
							if (tWndRecordItemInfo.uiFristIFrameIndex==0)
							{
								//test uiFristIFrameIndex
								if (uiCurrentIndex==m_tFileHeadInfo.tIFrameIndex.uiFirstIFrame[uiWnd])
								{
									//keep going
									tWndRecordItemInfo.uiFristIFrameIndex=uiCurrentIndex;
								}else{
									qDebug()<<__FUNCTION__<<__LINE__<<"uiCurrentIndex!=m_tFileHeadInfo.tIFrameIndex.uiFirstIFrame[uiWnd]";
									abort();
								}
							}else{
								//do nothing
							}
							//done
							m_tWndRecordItemInfo.insert(uiWnd,tWndRecordItemInfo);
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"(tWndRecordItemInfo.uiNextFrameIndex!=uiCurrentIndex||tWndRecordItemInfo.uiNextIFrameIndex!=uiCurrentIndex";
							abort();
						}
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"tWndRecordItemInfo.uiHistoryIFrameIndex!=pFileFrameHead->tPerFrameIndex.uiPreIFrame";
						abort();
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"tWndRecordItemInfo.uiHistoryFrameIndex!=pFileFrameHead->tPerFrameIndex.uiPreFrame";
					abort();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"m_tFileHeadInfo.tIFrameIndex.uiFirstIFrame[uiWnd] should not been 0";
				abort();
				return false;
			}
		}else{
			// 普通帧
			if (m_tWndRecordItemInfo.contains(uiWnd))
			{
				//do nothing
			}else{
				tagWndRecordItemInfo tWndRecordItemInfo;
				tWndRecordItemInfo.uiWndId=uiWnd;
				tWndRecordItemInfo.uiHisRecordId=pFileFrameHead->tFrameHead.uiSessionId;
				tWndRecordItemInfo.sFilePath=sFilePath;
				tWndRecordItemInfo.uiFristIFrameIndex=0;
				tWndRecordItemInfo.uiHistoryFrameIndex=0;
				tWndRecordItemInfo.uiHistoryIFrameIndex=0;
				tWndRecordItemInfo.uiNextFrameIndex=uiCurrentIndex;
				tWndRecordItemInfo.uiNextIFrameIndex=0;
				tagRecordItemInfo tRecordItemInfo;
				tRecordItemInfo.uiRecordType=pFileFrameHead->tFrameHead.uiRecType;
				tRecordItemInfo.uiStartTime=pFileFrameHead->tFrameHead.uiGentime;
				tRecordItemInfo.uiEndTime=pFileFrameHead->tFrameHead.uiGentime;
				tWndRecordItemInfo.tRecordItemList.append(tRecordItemInfo);
				m_tWndRecordItemInfo.insert(uiWnd,tWndRecordItemInfo);
			}
			tagWndRecordItemInfo tWndRecordItemInfo=m_tWndRecordItemInfo.value(uiWnd);
			if (tWndRecordItemInfo.uiHistoryFrameIndex==pFileFrameHead->tPerFrameIndex.uiPreFrame)
			{
				if (tWndRecordItemInfo.uiHistoryIFrameIndex==pFileFrameHead->tPerFrameIndex.uiPreIFrame)
				{
					if (tWndRecordItemInfo.uiNextFrameIndex==uiCurrentIndex)
					{
						tWndRecordItemInfo.uiHistoryFrameIndex=uiCurrentIndex;
						tWndRecordItemInfo.uiNextFrameIndex=pFileFrameHead->tPerFrameIndex.uiNextFrame;
						m_tWndRecordItemInfo.insert(uiWnd,tWndRecordItemInfo);
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"tWndRecordItemInfo.uiNextFrameIndex!=uiCurrentIndex";
						abort();
					}
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"tWndRecordItemInfo.uiHistoryIFrameIndex!=pFileFrameHead->tPerFrameIndex.uiPreIFrame";
					abort();
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"tWndRecordItemInfo.uiHistoryFrameIndex!=pFileFrameHead->tPerFrameIndex.uiPreFrame";
				abort();
			}
		}
		// 建立数据库信息
		tagWndRecordItemInfo tWndRecordItemInfo=m_tWndRecordItemInfo.value(uiWnd);
		if (tWndRecordItemInfo.uiHisRecordId==pFileFrameHead->tFrameHead.uiSessionId)
		{
			//更新 条目
			tWndRecordItemInfo.tRecordItemList.last().uiEndTime=pFileFrameHead->tFrameHead.uiGentime;
			m_tWndRecordItemInfo.insert(uiWnd,tWndRecordItemInfo);
		}else{
			//创建条目
			tagRecordItemInfo tRecordItemInfo;
			tRecordItemInfo.uiEndTime=pFileFrameHead->tFrameHead.uiGentime;
			tRecordItemInfo.uiStartTime=pFileFrameHead->tFrameHead.uiGentime;
			tRecordItemInfo.uiRecordType=pFileFrameHead->tFrameHead.uiRecType;
			tWndRecordItemInfo.uiHisRecordId=pFileFrameHead->tFrameHead.uiSessionId;
			tWndRecordItemInfo.tRecordItemList.append(tRecordItemInfo);
			m_tWndRecordItemInfo.insert(uiWnd,tWndRecordItemInfo);
		}
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"m_tFileHeadInfo.uiChannels do not contain this wnd I Frame ";
		abort();
	}
	return false;
}

bool checkDatFile::saveItemToDatabase(QString sFilePath)
{
	//step1:操作RecordFileStatus 表
	//step 2:操作 Record 表

	//step1:操作RecordFileStatus 表
	if (setRecordFileStatus(sFilePath,"nInUse",1))
	{
		//keep going
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"setRecordFileStatus fail";
		abort();
	}
	//step 2:操作 Record 表
	QSqlDatabase *pDatabase=NULL;
	pDatabase=initMgrDataBase(m_sDatabasePath,(quintptr*)this);
	if (NULL!=pDatabase)
	{
		QSqlQuery _query(*pDatabase);
		QMap<int,tagWndRecordItemInfo>::const_iterator tItem=m_tWndRecordItemInfo.constBegin();
		while(tItem!=m_tWndRecordItemInfo.constEnd()){
			tagWndRecordItemInfo tWndRecordItemInfo=tItem.value();
			QList<tagRecordItemInfo> tRecordItemInfoList=tWndRecordItemInfo.tRecordItemList;
			for (int i=0;i<tRecordItemInfoList.size();i++)
			{
				tagRecordItemInfo tRecordItemInfo=tRecordItemInfoList.value(i);
				QString sCommand=QString("insert into record (nWndId,nRecordType,nStartTime,nEndTime,sFilePath) values(%1,%2,%3,%4,'%5')").arg(tWndRecordItemInfo.uiWndId).arg(tRecordItemInfo.uiRecordType).arg(tRecordItemInfo.uiStartTime).arg(tRecordItemInfo.uiEndTime).arg(sFilePath);
				if (_query.exec(sCommand))
				{
					//keep going
				}else{
					abort();
					qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
				}
			}
			++tItem;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
		abort();
	}
	return true;
}

bool checkDatFile::builtSearch_recordItem( QString sDiskTtem )
{
	QString sDatabasePath=sDiskTtem+":/recEx/record_rebuilt.db";
	QFile tDatabaseFile;
	tDatabaseFile.setFileName(sDatabasePath);
	qDebug()<<__FUNCTION__<<__LINE__<<"builtSearch_recordItem start";
	if (tDatabaseFile.exists())
	{
		QSqlDatabase *pDatabase=NULL;
		pDatabase=initMgrDataBase(sDatabasePath,(quintptr*)this);
		if (NULL!=pDatabase)
		{
			m_tSearchRecordItem.clear();
			QSqlQuery _query(*pDatabase);
			for (int i=0;i<WNDNUM;i++)
			{
				//i 表示窗口号
				for (int j=0;j<m_tRecordType.size();j++)
				{
					//uiRecordType 表示录像类型
					unsigned int uiRecordType=m_tRecordType.value(j);
					QString sCommand=QString("select nStartTime,nEndTime from record where nWndId=%1 and nRecordType=%2 order by nStartTime").arg(i).arg(uiRecordType);
					if (_query.exec(sCommand))
					{
						bool bFlags=false;
						tagSearch_recordItem tSearchRecordItem;
						bool bSaveFlags=false;
						while(_query.next()){
							bSaveFlags=true;
							if (bFlags==false)
							{
								tSearchRecordItem.uiRecordType=uiRecordType;
								tSearchRecordItem.uiWnd=i;
								tSearchRecordItem.uiStartTime=_query.value(0).toUInt();
								tSearchRecordItem.uiEndTime=_query.value(1).toUInt();
								bFlags=true;
							}else{
								//do nothing
								if (_query.value(0).toUInt()-tSearchRecordItem.uiEndTime>=2)
								{
									//相差超过两秒，认为是新的条
									m_tSearchRecordItem.append(tSearchRecordItem);
									tSearchRecordItem.uiRecordType=uiRecordType;
									tSearchRecordItem.uiWnd=i;
									tSearchRecordItem.uiStartTime=_query.value(0).toUInt();
									tSearchRecordItem.uiEndTime=_query.value(1).toUInt();
								}else{
									//不过超过两秒，同一条
									tSearchRecordItem.uiEndTime=_query.value(1).toUInt();
								}
							}
						}
						if (bSaveFlags)
						{
							m_tSearchRecordItem.append(tSearchRecordItem);
						}else{
							//do nothing
						}
						for (int i=0;i<m_tSearchRecordItem.size();i++)
						{
							tagSearch_recordItem tSearchRecordItem=m_tSearchRecordItem.value(i);
							QString sCommand=QString("insert into search_record (nWndId,nRecordType,nStartTime,nEndTime) values(%1,%2,%3,%4)").arg(tSearchRecordItem.uiWnd).arg(tSearchRecordItem.uiRecordType).arg(tSearchRecordItem.uiStartTime).arg(tSearchRecordItem.uiEndTime);
							if (_query.exec(sCommand))
							{
								//keep going
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
								abort();
							}
						}
						m_tSearchRecordItem.clear();
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"exec cmd fail:"<<sCommand;
						abort();
					}
				}
			}
			QString sOldDatabasePath=sDiskTtem+":/recEx/record.db";
			QFile tOldDatabaseFile;
			tOldDatabaseFile.setFileName(sOldDatabasePath);
			if (tOldDatabaseFile.exists())
			{
				if (tOldDatabaseFile.remove())
				{
					//keep going
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"please close :"<<sOldDatabasePath;
					QMessageBox msgBox;
					QString sText=QString("please close file '%1'").arg(sOldDatabasePath);
					msgBox.setText(sText);
					msgBox.setInformativeText("if you had closed,click 'Ok' button,if not click 'Cancel' button");
					msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
					msgBox.setDefaultButton(QMessageBox::Ok);
					int ret = msgBox.exec();
					switch (ret) {
					case QMessageBox::Ok:
						// Save was clicked
						if (tOldDatabaseFile.remove())
						{
						}else{
							abort();
						}
						break;
					case QMessageBox::Cancel:
						// Don't Save was clicked
						abort();
						break;
					default:
						// should never be reached
						break;
					}
				}
			}else{
				//do nothing
			}
			tOldDatabaseFile.setFileName(sDatabasePath);
			if (tOldDatabaseFile.copy(sOldDatabasePath))
			{
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"copy :"<<sDatabasePath<<"to "<<sOldDatabasePath<<"fail";
				abort();
			}
			qDebug()<<__FUNCTION__<<__LINE__<<"builtSearch_recordItem end";
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"pDatabase should not been null";
			abort();
		}
	}else{
		//do nothing
		qDebug()<<__FUNCTION__<<__LINE__<<"builtSearch_recordItem end";
		return true;
	}
	return false;
}

void checkDatFile::setText( QTextEdit **tText )
{
	*tText=m_pText;
}

void checkDatFile::printfFileData(QString sFilePath)
{
	qDebug()<<__FUNCTION__<<__LINE__<<sFilePath<<":detection report start";
	//开始时间，结束时间
	//录像的通道
	//每个通道的录像条目
	QString sFileStartTime=QDateTime::fromTime_t(m_tFileHeadInfo.uiStart).toString("dd.MM.yyyy:hh:mm:ss");
	QString sFileEndTime=QDateTime::fromTime_t(m_tFileHeadInfo.uiEnd).toString("dd.MM.yyyy:hh:mm:ss");
	qDebug()<<__FUNCTION__<<__LINE__<<"file start time:"<<sFileStartTime;
	qDebug()<<__FUNCTION__<<__LINE__<<"file end time:"<<sFileEndTime;
	QMap<int,tagWndRecordItemInfo>::const_iterator tItem=m_tWndRecordItemInfo.constBegin();
	while(tItem!=m_tWndRecordItemInfo.constEnd()){
		qDebug()<<__FUNCTION__<<__LINE__<<"wnd id:"<<tItem.key();
		tagWndRecordItemInfo tWndRecordItemInfo=tItem.value();
		for (int i=0;i<tWndRecordItemInfo.tRecordItemList.size();i++)
		{
			tagRecordItemInfo tRecordItemInfo=tWndRecordItemInfo.tRecordItemList.value(i);
			QString sStartTime=QDateTime::fromTime_t(tRecordItemInfo.uiStartTime).toString("hh:mm:ss");
			QString sEndTime=QDateTime::fromTime_t(tRecordItemInfo.uiEndTime).toString("hh:mm:ss");
			qDebug()<<__FUNCTION__<<__LINE__<<"wnd id:"<<tItem.key()<<"uiRecordType"<<tRecordItemInfo.uiRecordType<<"startTime:"<<sStartTime<<"endTime:"<<sEndTime;
		}
		++tItem;
	}
	qDebug()<<__FUNCTION__<<__LINE__<<sFilePath<<":detection report end";
}
