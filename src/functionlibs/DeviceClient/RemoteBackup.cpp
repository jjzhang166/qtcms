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
m_pRemotePlayback(NULL),
m_backuping(false),
m_bFinish(false),
m_bAudioBeSet(false),
m_progress(0.0f),
AviFile(NULL),
m_videoHeight(0),
m_videoWidth(0)
{
	m_backproc.backproc = NULL;
	m_backproc.pUser = NULL;

	m_nFrameCount = 0;
	m_nLastTicket = 0;
	m_firstgentime = 0;
	m_eventList<<"progress"<<"BackupStatusChange";
}


RemoteBackup::~RemoteBackup(void)
{
	m_backuping=false;
	wait();
	stopConnect();
	closeFile();
	clearbuffer();
}


int RemoteBackup::StartByParam(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,
	int nChannel,
	int nTypes,
	const QDateTime & startTime,
	const QDateTime & endTime,
	const QString & sbkpath)
{
	QDir dir;
	

	if (nChannel<0 || (endTime.toTime_t() < startTime.toTime_t()) || !dir.exists(sbkpath))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"parm error";
		QVariantMap item;
		item.insert("types","fail");
		eventProcCall("BackupStatusChange",item);
		item.clear();
		item.insert("types","stopBackup");
		eventProcCall("BackupStatusChange",item);
		return 1;
	}
	if (QThread::isRunning())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"last backup still running";
		QVariantMap item;
		item.insert("types","fail");
		eventProcCall("BackupStatusChange",item);
		item.clear();
		item.insert("types","stopBackup");
		eventProcCall("BackupStatusChange",item);
		return 2;
	}
	m_stime = startTime;
	m_etime = endTime;
	m_nchannel = nChannel;
	m_savePath = sbkpath;
	m_devid = sEseeId;
	if (connectToDevice(sAddr,uiPort,sEseeId))
	{
		IEventRegister* pEventRegister = NULL;
		m_pBackupConnect->QueryInterface(IID_IEventRegister,(void**)&pEventRegister);
		pEventRegister->registerEvent("RecordStream",cbGetStream,this);
		pEventRegister->Release();

		//++start backup
		clearbuffer();
		if(0 == m_pRemotePlayback->getPlaybackStreamByTime(1<<nChannel,nTypes,startTime,endTime))
		{
			QVariantMap item;
			item.insert("types","startBackup");
			eventProcCall("BackupStatusChange",item);
			start();
			return 0;
		}	
	}
	qDebug()<<__FUNCTION__<<__LINE__<<"back up connect to device fail";
	QVariantMap item;
	item.insert("types","fail");
	eventProcCall("BackupStatusChange",item);
	item.clear();
	item.insert("types","stopBackup");
	eventProcCall("BackupStatusChange",item);
	return 2;
}
int RemoteBackup::Stop()
{
	stopConnect();
	m_backuping = false;

	wait();
	QVariantMap item;
	item.insert("types","stopBackup");
	eventProcCall("BackupStatusChange",item);
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
	stopConnect();
	int amount=3;
	while(amount>0){
		//尝试bubble协议连接
		if (tryConnectProtocol(CLSID_BubbleProtocol,sAddr,uiPort,sEseeId))
			return true;
		//尝试穿透协议连接
		if (tryConnectProtocol(CLSID_Hole,sAddr,uiPort,sEseeId))
			return true;
		//尝试转发协议链接
		if (tryConnectProtocol(CLSID_Turn,sAddr,uiPort,sEseeId))
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
		m_pBackupConnect->QueryInterface(IID_IRemotePlayback,(void**)&m_pRemotePlayback);
		if (m_pRemotePlayback)
		{
			QVariantMap ports;
			ports.insert("media",uiPort);
			m_pBackupConnect->setDeviceHost(sAddr);
			m_pBackupConnect->setDevicePorts(ports);
			m_pBackupConnect->setDeviceId(sEseeId);

			if (0 == m_pBackupConnect->connectToDevice())
			{
				return true;
			}

			m_pRemotePlayback->Release();
			m_pRemotePlayback = NULL;
		}
		m_pBackupConnect->Release();
		m_pBackupConnect = NULL;
	}

	return false;
}
void RemoteBackup::stopConnect()
{
	if (NULL!=m_pBackupConnect)
	{
		if (IDeviceConnection::CS_Connected==m_pBackupConnect->getCurrentStatus()||IDeviceConnection::CS_Connectting==m_pBackupConnect->getCurrentStatus())
		{
			m_pBackupConnect->disconnect();
		}
		m_pBackupConnect->Release();
		m_pBackupConnect = NULL;
	}
	if (NULL !=m_pRemotePlayback)
	{
		m_pRemotePlayback->Release();
		m_pRemotePlayback = NULL;
	}
}

bool RemoteBackup::createFile()
{
	QDir dir;
	if (!dir.exists(m_savePath)){
		qDebug()<<__FUNCTION__<<__LINE__<<m_savePath<<"is no exists";
		return false;
	}else{
		//keep going
	}

		QString fullname = m_savePath ;
		char sChannelNum[3];
		sprintf(sChannelNum,"%02d",m_nchannel+1);
		fullname += "/" + m_devid
			+"_"+"CHL" + QString("%1").arg(QString(sChannelNum))
			+"_" + m_stime.toString("yyyy-MM-dd(hhmmss)") 
			+"_"+ m_etime.toString("yyyy-MM-dd(hhmmss)")+".avi";
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
		AviFile = AVI_open_output_file(byAyFullName.data());
		if (NULL == AviFile) {
			qDebug()<<__FUNCTION__<<__LINE__<<fullname<<"open fail";
			return false;
		}else{
			//keep going
		}
		int timeout=0;
		bool btimeout=true;
		while(btimeout&&timeout<50){
			if (m_videoWidth==0)
			{
				timeout++;
				msleep(100);
			}else{
				AVI_set_video(AviFile,m_videoWidth,m_videoHeight,25,"X264");
				memset(m_nFrameCountArray,0,sizeof(m_nFrameCountArray));
				btimeout=false;
			}
		}
		if (btimeout)
		{
			AVI_close(AviFile);
			AviFile = NULL;
			QFile file;
			file.setFileName(fullname);
			if (file.exists())
			{
				file.remove();
			}else{
				//keep going
			}
			return false;
		}else{
			return true;
		}
}
int RemoteBackup::closeFile()
{

	if (AviFile != NULL)
	{
		// set video info
		int iMaxFramerate = 0;
		int iMaxFramerateCount = 0;
		int iFrameCount = 0;
		int i;
		// the max framerate
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
		AVI_set_video(AviFile,m_videoWidth,m_videoHeight,iFrameCount,"X264");
		AVI_close(AviFile);
		AviFile = NULL;
		qDebug("Close file\n");
	}

	m_nFrameCount = 0;
	m_nLastTicket = 0;
	m_bAudioBeSet = false;
	m_progress = 0.0f;
	return 0;
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
	m_backuping=true;
	int step=0;
	int timeout=0;
	RecFrame recframe;

	unsigned int sdtime = m_stime.toTime_t();
	unsigned int edtime = m_etime.toTime_t();
	bool m_firstflame=true;
	int lastproess=0;
	while(m_backuping){
		switch(step){
		case 0://创建文件
			{
				if (createFile())
				{
					step=1;//创建成功
				}else{
					step=5;//创建失败，结束
					qDebug()<<__FUNCTION__<<__LINE__<<"creat file fail";
					QVariantMap item;
					item.insert("types","fail");
					eventProcCall("BackupStatusChange",item);
				}
			}
			break;
		case 1://检测磁盘空间是否足够，至少大于2g
			{
				if (getUsableDisk(m_savePath))
				{
					step=2;
				}else{
					step=5;//磁盘空间不足
					qDebug()<<__FUNCTION__<<__LINE__<<"insufficient-disk";
					QVariantMap item;
					item.insert("types","insufficient-disk");
					eventProcCall("BackupStatusChange",item);
				}
			}
			break;
		case 2://是否有码流
			{
				if (m_bufferqueue.size()>0)
				{
					step=3;
					timeout=0;
				}else{
					step=2;
					msleep(100);//等待码流
					timeout++;
					if (timeout>50)
					{
						step=5;//超时没有码流,结束备份
						qDebug()<<__FUNCTION__<<__LINE__<<"time out ,no stream";
						QVariantMap item;
						item.insert("types","fail");
						eventProcCall("BackupStatusChange",item);
					}else{
						//do nothing
					}
				}
			}
			break;
		case 3:
			{
				m_bufflock.lock();
				recframe = m_bufferqueue.dequeue();
				m_bufflock.unlock();

				//Calculation the progress
				unsigned int curr_gentime = recframe.gentime;
				if (true==m_firstflame)
				{
					m_firstgentime=curr_gentime;
					m_firstflame=false;
				}
				if(curr_gentime>=m_firstgentime){
					m_progress = (float)(curr_gentime - m_firstgentime)/(edtime - sdtime);
					int progress=m_progress*1000;
					if (progress%10==0&&progress>9&&progress>lastproess)
					{
						lastproess=progress;
						QVariantMap item;
						item.insert("parm",progress/10);
						eventProcCall("progress",item);

					}else{
						//do nothing
					}
				}else{
					//do nothing;
				}
					
				if (AVENC_IDR==recframe.type||AVENC_PSLICE==recframe.type)//video
				{
					AVI_write_frame(AviFile,recframe.pdata,recframe.datasize,(AVENC_IDR==recframe.type));
				}else if (0x00==recframe.type)//audio
				{
					if (!m_bAudioBeSet)
					{
						int AudioFormat=WAVE_FORMAT_ALAW;
						AVI_set_audio(AviFile,1,m_samplerate,m_samplewidth,AudioFormat,64);
						m_bAudioBeSet=true;
					}else{
						//do nothing
					}

					AVI_write_audio(AviFile,recframe.pdata,recframe.datasize);
				}else{
					//do nothing;
				}

				delete[] recframe.pdata;
				recframe.pdata = NULL;
				step =4;
			}
			break;
		case 4://判断是否结束
			{
				if (AVI_bytes_written(AviFile)>1024*1024*1024)
				{
					qDebug()<<__FUNCTION__<<__LINE__<<"file size >1G,stop back up";

					QVariantMap item;
					item.insert("parm",100);
					eventProcCall("progress",item);
					item.clear();
					item.insert("types","backupFinished");
					eventProcCall("BackupStatusChange",item);
					step=5;//文件最大不超过1G
				}else if (m_progress>=1.0f)
				{
					qDebug()<<__FUNCTION__<<__LINE__<<"m_progress"<<m_progress<<"back up finish";
					QVariantMap item;
					item.insert("parm",100);
					eventProcCall("progress",item);
					item.clear();
					item.insert("types","backupFinished");
					eventProcCall("BackupStatusChange",item);
					step=5;//文件下载完毕
				}else if (m_progress>=0.9f&&timeout>20)
				{
					qDebug()<<__FUNCTION__<<__LINE__<<"m_progress"<<m_progress<<"back up finish";
					QVariantMap item;
					item.insert("parm",100);
					eventProcCall("progress",item);
					item.clear();
					item.insert("types","backupFinished");
					eventProcCall("BackupStatusChange",item);
					step =5;//文件下载完毕,修正进度条不能满0.99的情况
				}
				else{
					//keep backing up
					step =2;
				}
			}
			break;
		case 5://结束
			{
				closeFile();
				clearbuffer();
				m_backuping = false;
				m_firstgentime = 0;
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
	quint64 FreeByteAvailable;
	quint64 TotalNumberOfBytes;
	quint64 TotalNumberOfFreeBytes;
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



int __cdecl cbGetStream(QString evName,QVariantMap evMap,void*pUser)
{
	
	if ("RecordStream"==evName)
	{
		((RemoteBackup*)pUser)->WriteFrameData(evMap);
	}
	
	return 0;
}