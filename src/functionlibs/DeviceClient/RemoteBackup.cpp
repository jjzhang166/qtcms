#include "RemoteBackup.h"
#include <QtCore/QDir>
#include "netlib.h"
#include "guid.h"
//#include <QtCore/QElapsedTimer>

#define AVENC_IDR		0x01
#define AVENC_PSLICE	0x02
#define AVENC_AUDIO		0x00


int __cdecl cbGetStream(QString evName,QVariantMap evMap,void*pUser);

RemoteBackup::RemoteBackup(void):
m_pBackupConnect(NULL),
m_backuping(false),
m_bAudioBeSet(false),
m_bCheckDisk(false),
m_bCheckBlock(false),
m_nPosition(0),
m_progress(0.0f),
m_videoHeight(0),
m_videoWidth(0)
{
	m_backproc.backproc = NULL;
	m_backproc.pUser = NULL;

	m_nFrameCount = 0;
	m_nLastTicket = 0;
	m_firstgentime = 0;
	m_eventList<<"progress"<<"BackupStatusChange";
	connect(&m_tCheckBlock,SIGNAL(timeout()),this,SLOT(slCheckBlock()));
	connect(&m_tCheckDisk,SIGNAL(timeout()),this,SLOT(slCheckDisk()));
	m_tCheckDisk.start(3000);
	m_tCheckBlock.start(5000);
}


RemoteBackup::~RemoteBackup(void)
{
	m_backuping=false;
	wait();
	clearbuffer();
}

int RemoteBackup::StartByParam( const QString &sAddr,unsigned int uiPort,const QString &sEseeId, int nChannel, int nTypes, const QString &sDeviceName,const QDateTime & startTime, const QDateTime & endTime, const QString & sbkpath )
{
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"backup had been running,if you want reset please call Stop() first";
		return 2;
	}else{
		m_tBackUpInfo.sAddr=sAddr;
		m_tBackUpInfo.uiPort=uiPort;
		m_tBackUpInfo.sEeeId=sEseeId;
		m_tBackUpInfo.nChannel=nChannel;
		m_tBackUpInfo.nTypes=nTypes;
		m_tBackUpInfo.sDeviceName=sDeviceName;
		m_tBackUpInfo.startTime=startTime;
		m_tBackUpInfo.endTime=endTime;
		m_tBackUpInfo.sPath=sbkpath;
		start();
		return 0;
	}	
}
int RemoteBackup::Stop()
{
	m_backuping = false;
	wait();
	return 0;
}
float RemoteBackup::getProgress()
{
	return m_progress;
}
int RemoteBackup::WriteFrameData(QVariantMap &frameinfo)
{
	int type = frameinfo["frametype"].toInt();
	
	if (AVENC_AUDIO != type && AVENC_IDR != type && AVENC_PSLICE != type)
	{
		return 1;
	}
	if (m_backuping!=true)
	{
		return 1;
	}else{
		//keep going
	}
	RecFrame recframe;
	recframe.type = type;
	recframe.datasize = frameinfo["length"].toUInt();
	recframe.pts = (unsigned int)frameinfo["pts"].toULongLong()/1000;
	recframe.gentime = frameinfo["gentime"].toUInt();
	recframe.pdata = new char[recframe.datasize];
	if(NULL == recframe.pdata)
		return 2;
	char* pfdata = (char*)frameinfo["data"].toUInt();
	memcpy(recframe.pdata,pfdata,recframe.datasize);
	
	//Calculation the frame
	if (0x01 == recframe.type || 0x02 == recframe.type)
	{
		if(AVENC_IDR == recframe.type)
		{
			m_videoWidth = frameinfo["width"].toInt();
			m_videoHeight = frameinfo["height"].toInt();
		}
		m_nFrameCount ++;
		if ( 0 == m_nLastTicket )
		{
			m_nLastTicket = recframe.pts;
		}
		else
		{
			if (recframe.pts - m_nLastTicket >= 1000)
			{
				if (m_nFrameCount < 31)
				{
					m_nFrameCountArray[m_nFrameCount] ++;
				}
				m_nFrameCount = 0;
				m_nLastTicket = recframe.pts;
			}
		}
	}
	else if (0x00 == type)
	{
		m_samplerate = frameinfo["samplerate"].toInt();
		m_samplewidth = frameinfo["samplewidth"].toInt();
	}

	m_bufflock.lock();
	m_bufferqueue.enqueue(recframe);
	m_bufflock.unlock();

	if(0 == m_firstgentime)
		m_firstgentime = recframe.gentime;
	return 0;
}

int RemoteBackup::SetBackupEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser)
{
	m_backproc.backproc = proc;
	m_backproc.pUser = pUser;
	if (!m_eventList.contains(eventName))
	{
		return -1;
	}else{
		evItem proInfo;
		proInfo.pUser=pUser;
		proInfo.backproc=proc;
		m_eventMap.insert(eventName,proInfo);
	}
	return 0;
}

bool RemoteBackup::connectToDevice(const QString &sAddr,unsigned int uiPort,const QString &sEseeId)
{
	int amount=3;
	while(amount>0){
		//尝试bubble协议连接
		if (m_backuping&&tryConnectProtocol(CLSID_BubbleProtocol,sAddr,uiPort,sEseeId))
			return true;
		//尝试穿透协议连接
		if (m_backuping&&tryConnectProtocol(CLSID_Hole,sAddr,uiPort,sEseeId))
			return true;
		//尝试转发协议链接
		if (m_backuping&&tryConnectProtocol(CLSID_Turn,sAddr,uiPort,sEseeId))
			return true;
		
		amount--;
	}
	return false;
}
bool RemoteBackup::tryConnectProtocol(CLSID clsid,const QString &sAddr,unsigned int uiPort,const QString &sEseeId)
{
	pcomCreateInstance(clsid,NULL,IID_IDeviceConnection,(void**)&m_pBackupConnect);
	if (m_pBackupConnect)
	{
		IRemotePlayback *pRemotePlayback=NULL;
		m_pBackupConnect->QueryInterface(IID_IRemotePlayback,(void**)&pRemotePlayback);
		if (pRemotePlayback)
		{
			QVariantMap ports;
			ports.insert("media",uiPort);
			m_pBackupConnect->setDeviceHost(sAddr);
			m_pBackupConnect->setDevicePorts(ports);
			m_pBackupConnect->setDeviceId(sEseeId);

			if (0 == m_pBackupConnect->connectToDevice())
			{
				pRemotePlayback->Release();
				pRemotePlayback=NULL;
				return true;
			}

			pRemotePlayback->Release();
			pRemotePlayback = NULL;
		}
		m_pBackupConnect->Release();
		m_pBackupConnect = NULL;
	}

	return false;
}

void RemoteBackup::clearbuffer()
{
	m_bufflock.lock();
	while(m_bufferqueue.size()>0)
	{
		RecFrame recframe = m_bufferqueue.dequeue();
		if(recframe.pdata)
			delete[] recframe.pdata;
	}
	m_bufflock.unlock();
}
void RemoteBackup::run()
{
	avi_t *pAviFile=NULL;
	m_backuping=true;
	bool bStop=false;
	int nStep=INIT;
	int nLastProgess=0;
	bool bFirstFlame=true;
	int nSleepCount=0;
	QString fullname;
	bool bKeepGoing=false;
	int nPackPart=0;
	while(bStop==false){
		switch(nStep){
		case INIT:{
			//step1:抛出开始备份事件
			//step2:检测参数
			//step3:清空数据
			//step4:初始化备份参数
			
			QVariantMap item;
			item.insert("types","startBackup");
			eventProcCall("BackupStatusChange",item);

			m_bAudioBeSet=false;
			m_videoHeight=0;
			m_videoWidth=0;
			m_samplerate=0;
			m_samplewidth=0;

			m_nFrameCount=0;
			m_nLastTicket=0;
			memset(m_nFrameCountArray,0,sizeof(m_nFrameCountArray));
			m_progress = 0.0f;
			m_firstgentime=0;
			QDir dir;
			if (m_tBackUpInfo.nChannel<0||m_tBackUpInfo.endTime.toTime_t()<m_tBackUpInfo.startTime.toTime_t()||!dir.exists(m_tBackUpInfo.sPath))
			{
				nStep=FAIL;
				qDebug()<<__FUNCTION__<<__LINE__<<"back up fail as backup parm error";
			}else{
				m_bCheckBlock=true;
				m_nPosition=__LINE__;
				clearbuffer();
				m_bCheckBlock=false;
				nStep = CONNECTTODEVICE;
			}
			if (m_backuping==false)
			{
				nStep=FAIL;
			}else{
				//keep going
			}
			   }
			   break;
		case CONNECTTODEVICE:{
			bool bIsconnect=false;
			m_bCheckBlock=true;
			m_nPosition=__LINE__;
			bIsconnect=connectToDevice(m_tBackUpInfo.sAddr,m_tBackUpInfo.uiPort,m_tBackUpInfo.sEeeId);
			m_bCheckBlock=false;
			if (bIsconnect)
			{
				//step 0:注册回调函数
				//step 1申请码流
				int step=0;
				bool isStop=false;
				while(isStop==false){
					switch(step){
					case 0:{
						IEventRegister *pEventRegister=NULL;
						if (m_pBackupConnect!=NULL)
						{
							m_pBackupConnect->QueryInterface(IID_IEventRegister,(void**)&pEventRegister);
							if (pEventRegister!=NULL)
							{
								pEventRegister->registerEvent("RecordStream",cbGetStream,this);
								pEventRegister->Release();
								pEventRegister=NULL;
								step=1;
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"backup fail as pEventRegister is null";
								step=2;
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"backup fail as m_pBackupConnect is null";
							step=2;
						}
						   }
						   break;
					case 1:{
						if (m_pBackupConnect!=NULL)
						{
							IRemotePlayback *pRemotePlayback=NULL;
							m_pBackupConnect->QueryInterface(IID_IRemotePlayback,(void**)&pRemotePlayback);
							if (NULL!=pRemotePlayback)
							{
								int nRet=-1;
								m_bCheckBlock=true;
								m_nPosition=__LINE__;
								nRet=pRemotePlayback->getPlaybackStreamByTime(1<<m_tBackUpInfo.nChannel,m_tBackUpInfo.nTypes,m_tBackUpInfo.startTime,m_tBackUpInfo.endTime);
								m_bCheckBlock=false;
								if (0==nRet)
								{
									step=3;
								}else{
									qDebug()<<__FUNCTION__<<__LINE__<<"back up fail as getPlaybackStreamByTime fail";
									step=2;
								}
								m_bCheckBlock=true;
								m_nPosition=__LINE__;
								pRemotePlayback->Release();
								m_bCheckBlock=false;
								pRemotePlayback=NULL;
							}else{
								qDebug()<<__FUNCTION__<<__LINE__<<"back up fail as this module do not support IRemotePlayback interface";
								step=2;
							}
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"back up fail as m_pBackupConnect is null";
							step=2;
						}
						   }
						   break;
					case 2:{
						//失败
						nStep=FAIL;
						isStop=true;
						   }
						   break;
					case 3:{
						//成功
						nStep=INITPACK;
						isStop=true;
						if (m_backuping==false)
						{
							nStep=FAIL;
						}else{
							//keep going
						}
						   }
						   break;
					}
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"back up fail as connect to device fail";
				nStep=FAIL;
			}
							 }
			   break;
		case INITPACK:{
			m_nPosition=__LINE__;
			m_bAudioBeSet=false;
			nStep=FIRST_I_FRAME;
					  }
					  break;
		case FIRST_I_FRAME:{
			m_nPosition=__LINE__;
			m_bufflock.lock();
			if (m_bufferqueue.size()>0)
			{
				RecFrame nodeTemp=m_bufferqueue.front();
				if (AVENC_IDR==nodeTemp.type)
				{
					nStep=OPEN_FILE;
					nSleepCount=0;
				}else{
					delete []nodeTemp.pdata;
					nodeTemp.pdata=NULL;
					m_bufferqueue.pop_front();
					nStep=FIRST_I_FRAME;
					nSleepCount=0;
				}
			}else{
				nStep=FIRST_I_FRAME;
				msleep(10);
				nSleepCount++;
				if (nSleepCount>500)
				{
					nStep=FAIL;
					qDebug()<<__FUNCTION__<<__LINE__<<"pack up fail as there is no stream last 5s";
				}else{
					//keep going
				}
			}
			m_bufflock.unlock();
			if (m_backuping==false)
			{
				nStep=FAIL;
			}else{
				//keep going
			}
						   }
						   break;
		case OPEN_FILE:{
			//step 0:检测路径是否存在
			//step 1:检测磁盘空间大小
			//step 2:创建文件
			m_nPosition=__LINE__;
			int step=0;
			bool bOpenStop=false;
			while(bOpenStop==false){
				switch(step){
				case 0:{
					//step 0:检测路径是否存在
					QDir dir;
					if (!dir.exists(m_tBackUpInfo.sPath))
					{
						qDebug()<<__FUNCTION__<<__LINE__<<"back up fail as the backup path is not exists";
						step=3;
					}else{
						step=1;
					}
					   }
					   break;
				case 1:{
					//step 1:检测磁盘空间大小
					if (getUsableDisk(m_tBackUpInfo.sPath))
					{
						step=2;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"back up fail as there is no enough space in disk";
						step=3;
					}
					   }
					   break;
				case 2:{
					//step 2:创建文件
					fullname=m_tBackUpInfo.sPath;
					char sChannelNum[3];
					sprintf(sChannelNum,"%02d",m_tBackUpInfo.nChannel+1);
					fullname += "/" + m_tBackUpInfo.sDeviceName
						+"_"+"CHL" + QString("%1").arg(QString(sChannelNum))
						+"_" + m_tBackUpInfo.startTime.toString("yyyy-MM-dd(hhmmss)") 
						+"_"+ m_tBackUpInfo.endTime.toString("yyyy-MM-dd(hhmmss)")+"_"+QString::number(nPackPart)+".avi";
					QFile file;
					file.setFileName(fullname);
					if (file.exists())
					{
						file.remove();
					}else{
						//keep going
					}

					QTextCodec * codec = QTextCodec::codecForLocale();
					QByteArray byAyFullName = codec->fromUnicode(fullname);
					pAviFile = AVI_open_output_file(byAyFullName.data());
					if (NULL == pAviFile) {
						qDebug()<<__FUNCTION__<<__LINE__<<fullname<<"open fail";
						step=3;
					}else{
						//keep going
						step=4;
					}
					if (m_backuping==false)
					{
						step=3;
					}else{
						//keep going
					}
					   }
					   break;
				case 3:{
					//fail
					bOpenStop=true;
					nStep=FAIL;
					   }
					   break;
				case 4:{
					//success
					bOpenStop=true;
					nStep=SET_VIDEO;
					   }
					   break;
				}
			}
					   }
					   break;
		case SET_VIDEO:{
			m_nPosition=__LINE__;
			m_bufflock.lock();
			AVI_set_video(pAviFile,m_videoWidth,m_videoHeight,25,"X264");
			nStep=WRITE_FRAME;
			m_bufflock.unlock();
					   }
					   break;
		case WRITE_FRAME:{
			m_nPosition=__LINE__;
			m_bufflock.lock();
			if (m_bufferqueue.size()>0)
			{
				RecFrame nodeTemp=m_bufferqueue.dequeue();
				unsigned int uiCurrGentime=nodeTemp.gentime;
				if (true==bFirstFlame)
				{
					m_firstgentime=uiCurrGentime;
					bFirstFlame=false;
				}else{
					//do nothing
				}
				if (uiCurrGentime>=m_firstgentime)
				{
					m_progress=(float)(uiCurrGentime-m_firstgentime)/(m_tBackUpInfo.endTime.toTime_t()-m_tBackUpInfo.startTime.toTime_t());
					int nProgress=m_progress*1000;
					if (nProgress%10==0&&nProgress>9&&nProgress>nLastProgess)
					{
						nLastProgess=nProgress;
						QVariantMap item;
						item.insert("parm",nProgress/10);
						eventProcCall("progress",item);
					}
				}else{
					//do nothing
				}

				if (AVENC_IDR==nodeTemp.type||AVENC_PSLICE==nodeTemp.type)
				{
					AVI_write_frame(pAviFile,nodeTemp.pdata,nodeTemp.datasize,(AVENC_IDR==nodeTemp.type));
				}else if (0x00==nodeTemp.type)
				{
					if (!m_bAudioBeSet)
					{
						m_bAudioBeSet=true;
						AVI_set_audio(pAviFile,1,m_samplerate,m_samplewidth,WAVE_FORMAT_ALAW,64);
					}else{
						//do nothing
					}
					AVI_write_audio(pAviFile,nodeTemp.pdata,nodeTemp.datasize);
				}else{
					//do noting
					qDebug()<<__FUNCTION__<<__LINE__<<"undefined type ,please check";
				}
				delete [] nodeTemp.pdata;
				nodeTemp.pdata=NULL;
				nStep=CHECK_FILE_SIZE;
				nSleepCount=0;
			}else{
				//wait for frame
				nSleepCount++;
				msleep(10);
				if (nSleepCount>300)
				{
					nStep=WAIT_FOR_PACK;
				}else{
					//keep going
					nStep=CHECK_FILE_SIZE;
				}
			}
			m_bufflock.unlock();
			if (m_backuping==false)
			{
				nStep=WAIT_FOR_PACK;
			}else{
				//do nothing
			}
						 }
						 break;
		case CHECK_FILE_SIZE:{
			m_nPosition=__LINE__;
			if (m_progress>=1.0f)
			{
				//下载完成
				QVariantMap item;
				item.insert("parm",100);
				eventProcCall("progress",item);
				nStep=WAIT_FOR_PACK;
			}else if (m_progress>=0.9f&&nSleepCount>20)
			{
				//文件下载完毕，修正进度条不满0.99的情况
				QVariantMap item;
				item.insert("parm",100);
				eventProcCall("progress",item);
				nStep=WAIT_FOR_PACK;
			}else{
				nStep=CHECK_DISK_SPACK;
			}
							 }
							 break;
		case CHECK_DISK_SPACK:{
			m_nPosition=__LINE__;
			if (m_bCheckDisk)
			{
				m_bCheckDisk=false;
				if (getUsableDisk(m_tBackUpInfo.sPath))
				{
					//keep going
					if (256*1024*1024<AVI_bytes_written(pAviFile))
					{
						nStep=WAIT_FOR_PACK;
						nPackPart++;
						bKeepGoing=true;
						qDebug()<<__FUNCTION__<<__LINE__<<"as file size >256,it going to pack ";
					}else{
						nStep=WRITE_FRAME;
					}
					
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"back up fail as there is no enough space in disk";
					nStep=WAIT_FOR_PACK;
				}
			}else{
				nStep=WRITE_FRAME;
			}
							  }
							  break;
		case WAIT_FOR_PACK:{
			bool bWaitStop=false;
			int step=0;
			nSleepCount=0;
			m_nPosition=__LINE__;
			while(bWaitStop==false){
				switch(step){
				case 0:{
					//等待i帧
					m_bufflock.lock();
					if (m_bufferqueue.size()>0)
					{
						RecFrame node=m_bufferqueue.front();
						if (AVENC_IDR==node.type)
						{
							step=1;
						}else if (AVENC_PSLICE==node.type)
						{
							AVI_write_frame(pAviFile,node.pdata,node.datasize,(AVENC_IDR==node.type));
							delete[] node.pdata;
							node.pdata=NULL;
							m_bufferqueue.pop_front();
							step =0;
						}else if (0x00==node.type)
						{
							AVI_write_audio(pAviFile,node.pdata,node.datasize);
							delete[]node.pdata;
							node.pdata=NULL;
							m_bufferqueue.pop_front();
							step =0;
						}else{
							// do nothing
							qDebug()<<__FUNCTION__<<__LINE__<<"there may be a error ,please check";
							step =1;
						}
						nSleepCount=0;
					}else{
						nSleepCount++;
						msleep(10);
						if (nSleepCount>100||m_backuping==false)
						{
							step=1;
						}else{
							step=0;
						}
					}
					m_bufflock.unlock();
					   }
					   break;
				case 1:{
					//等待结束
					bWaitStop=true;
					   }
					   break;
				}
			}
						   nStep=PACK;
						   }
						   break;
		case PACK:{
			m_nPosition=__LINE__;
			if (pAviFile!=NULL)
			{
				m_bufflock.lock();
				// set video info
				int iMaxFramerate = 0;
				int iMaxFramerateCount = 0;
				int iFrameCount = 0;
				int i;
				// the max frame rate
				for (i = 0;i < 31;i++)
				{
					if (m_nFrameCountArray[i] != 0)
					{
						iMaxFramerate = i;
					}
				}
				// the frame rate 
				for (i = iMaxFramerate - 3 >= 0 ? iMaxFramerate - 3 : 0; i <= iMaxFramerate;i++)
				{
					if (m_nFrameCountArray[i] > (unsigned int)iMaxFramerateCount)
					{
						iMaxFramerateCount = m_nFrameCountArray[i];
						iFrameCount = i;
					}
				}
				AVI_set_video(pAviFile,m_videoWidth,m_videoHeight,iFrameCount,"X264");
				AVI_close(pAviFile);
				pAviFile = NULL;
				m_bufflock.unlock();
				if (bKeepGoing)
				{
					nStep=INITPACK;
					bKeepGoing=false;
				}else{
					if (m_progress<0.9f)
					{
						// 下载量少于90%认为下载失败，并删除文件
						nStep=FAIL;
						qDebug()<<__FUNCTION__<<__LINE__<<"back fail as the m_progress::"<<m_progress<<"less than 90%";
					}else{
						nStep=SUCCESS;
					}
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"back up fail as pAviFile is null";
				nStep=FAIL;
			}
				  }
				  break;
		case FAIL:{
			//抛出备份失败事件
			//删除失败的文件
			m_nPosition=__LINE__;
			if (pAviFile!=NULL)
			{
				AVI_set_video(pAviFile,m_videoWidth,m_videoHeight,25,"X264");
				AVI_close(pAviFile);
				pAviFile=NULL;
			}else{
				//do nothing
			}
			QFile file;
			file.setFileName(fullname);
			if (file.exists())
			{
				file.remove();
			}else{
				//keep going
			}

			QVariantMap item;
			item.insert("types","fail");
			eventProcCall("BackupStatusChange",item);
			nStep=END;
			   }
			   break;
		case SUCCESS:{
			m_nPosition=__LINE__;
			QVariantMap item;
			item.insert("types","backupFinished");
			eventProcCall("BackupStatusChange",item);
			nStep =END;
					 }
		case END:{
			//抛出备份结束事件
			//断开连接，释放连接的资源
			//清空缓存
			m_nPosition=__LINE__;
			bStop=true;
			m_backuping=false;
			if (NULL!=m_pBackupConnect)
			{
				m_bCheckBlock=true;
				m_nPosition=__LINE__;
				m_pBackupConnect->disconnect();
				m_bCheckBlock=false;
				m_bCheckBlock=true;
				m_nPosition=__LINE__;
				m_pBackupConnect->Release();
				m_bCheckBlock=false;
				m_pBackupConnect=NULL;
			}else{
				//do nothing
			}
			m_bCheckBlock=true;
			m_nPosition=__LINE__;
			clearbuffer();
			m_bCheckBlock=false;
			QVariantMap item;
			item.insert("types","stopBackup");
			eventProcCall("BackupStatusChange",item);
				 }
				 break;
		}
	}
}

bool RemoteBackup::getUsableDisk(QString sdisk)
{
	bool flags=false;
	int freesizem;
	QStringList distlist=sdisk.split(":");
	sdisk=distlist.at(0);
	sdisk.append(":");
		//使用默认大小
	freesizem=1024;
	quint64 FreeByteAvailable=0;
	quint64 TotalNumberOfBytes=0;
	quint64 TotalNumberOfFreeBytes=0;
	if (GetDiskFreeSpaceExQ(sdisk.toAscii().data(),&FreeByteAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes))
	{
		if (TotalNumberOfFreeBytes>(quint64)freesizem*1024*1024*2)
		{
			flags=true;
		}
	}
	return flags;
}



void RemoteBackup::eventProcCall( QString sEventName,QVariantMap parm )
{
	if (m_eventList.contains(sEventName))
	{
		evItem eventDec=m_eventMap.value(sEventName);
		if (NULL!=eventDec.backproc)
		{
			eventDec.backproc(sEventName,parm,eventDec.pUser);
		}
	}
}

void RemoteBackup::slCheckDisk()
{
	m_bCheckDisk=true;
}

void RemoteBackup::slCheckBlock()
{
	if(m_bCheckBlock){
		qDebug()<<__FUNCTION__<<__LINE__<<"block at position ::"<<m_nPosition;
	}else{
		// do nothing
	}
}


int __cdecl cbGetStream(QString evName,QVariantMap evMap,void*pUser)
{
	
	if ("RecordStream"==evName)
	{
		((RemoteBackup*)pUser)->WriteFrameData(evMap);
	}
	
	return 0;
}