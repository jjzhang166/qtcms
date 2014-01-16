#include "RemoteBackup.h"
#include <QtCore/QDir>
#include "netlib.h"
#include "guid.h"
//#include <QtCore/QElapsedTimer>

#define AVENC_IDR		0x01
#define AVENC_PSLICE	0x02
#define AVENC_AUDIO		0x00

//#pragma pack(4)
//typedef struct _tagAudioBufAttr{
//	int entries;
//	int packsize;
//	unsigned long long pts;
//	time_t * gtime;
//	char encode[8];
//	int samplerate;
//	int samplewidth;
//}AudioBufAttr;
//#pragma pack()

int __cdecl cbGetStream(QString evName,QVariantMap evMap,void*pUser);

RemoteBackup::RemoteBackup(void):
m_pBackupConnect(NULL),
m_pRemotePlayback(NULL),
m_backuping(false),
m_bFinish(false),
m_bAudioBeSet(false),
m_progress(0.0f),
AviFile(NULL)
{
	m_backproc.backproc = NULL;
	m_backproc.pUser = NULL;

	m_nFrameCount = 0;
	m_nLastTicket = 0;
	m_firstgentime = 0;
}


RemoteBackup::~RemoteBackup(void)
{
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
	/*m_sttime = startTime.toTime_t();
	m_edtime = endTime.toTime_t();*/
	if (nChannel<0 || (endTime.toTime_t() < startTime.toTime_t()) || !dir.exists(sbkpath))
	{
		return 1;
	}
	if (QThread::isRunning())
	{
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
			start();
			callBackupStatus("startBackup");
			return 0;
		}	
	}
	return 2;
}
int RemoteBackup::Stop()
{
	stopConnect();
	m_backuping = false;

	wait();
	callBackupStatus("stopBackup");
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
void RemoteBackup::callBackupStatus(QString sstatus)
{
	if (m_backproc.backproc)
	{
		QVariantMap evMap;
		evMap.insert("types",sstatus);
		m_backproc.backproc("backupEvent",evMap,m_backproc.pUser);
	}
}
bool RemoteBackup::createFile()
{
	QDir dir;
	if (!dir.exists(m_savePath))
		return false;
	if (!m_backuping)
	{
		QString fullname = m_savePath ;
		char sChannelNum[3];
		sprintf(sChannelNum,"%02d",m_nchannel+1);
		fullname += "/" + m_devid
			+"_"+"CHL" + QString("%1").arg(QString(sChannelNum))
			+"_" + m_stime.toString("yyyy-MM-dd(hhmmss)") 
			+"_"+ m_etime.toString("yyyy-MM-dd(hhmmss)")+".avi";
	    
		AviFile = AVI_open_output_file(fullname.toAscii().data());
		if (NULL == AviFile) 
			return false;

		AVI_set_video(AviFile,m_videoWidth,m_videoHeight,25,"X264");
		memset(m_nFrameCountArray,0,sizeof(m_nFrameCountArray));
	}

	return true;
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

	if (createFile())
	{
		m_backuping = true;
	}
	int timeout = 0;
	RecFrame recframe;

	unsigned int sdtime = m_stime.toTime_t();
	unsigned int edtime = m_etime.toTime_t();
	while(m_backuping )
	{
		if (m_bufferqueue.size()>0)
		{
			timeout=0;
			m_bufflock.lock();
			recframe = m_bufferqueue.dequeue();
			m_bufflock.unlock();

			//Calculation the progress
			
			unsigned int curr_gentime = recframe.gentime;
			if(curr_gentime>=m_firstgentime)
				m_progress = (float)(curr_gentime - m_firstgentime)/(edtime - sdtime);

			
			//write the video file
			if (AVENC_IDR == recframe.type || AVENC_PSLICE == recframe.type)
			{
				AVI_write_frame(AviFile,recframe.pdata,recframe.datasize,(AVENC_IDR == recframe.type));
			}// first audio frame
			else if (0x00 == recframe.type)
			{
				if (!m_bAudioBeSet)
				{
					int AudioFormat = WAVE_FORMAT_ALAW;

					AVI_set_audio(AviFile, 1, m_samplerate, m_samplewidth, AudioFormat, 64);
					m_bAudioBeSet = true;
				}
				AVI_write_audio(AviFile,recframe.pdata ,recframe.datasize);
			}
		
			delete[] recframe.pdata;
			recframe.pdata = NULL;

			if (m_progress>=1.0f || AVI_bytes_written(AviFile)>1024*1024*1024)	
			{
				//完成备份
				callBackupStatus("backupFinished");
				break;
			}

			quint64 FreeByteAvailable;
			quint64 TotalNumberOfBytes;
			quint64 TotalNumberOfFreeBytes;
			GetDiskFreeSpaceExQ(m_savePath.left(2).toAscii().data(),&FreeByteAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);
			if (TotalNumberOfFreeBytes<1024*1024)
			{
				callBackupStatus("diskfull");
				break;
			}

		}
		else 
		{
			timeout++;
			if (timeout>50)
			{
			    callBackupStatus("noStream");
			    break;
			}
			msleep(100);
		}
	}

	closeFile();
	clearbuffer();
	m_backuping = false;
	m_firstgentime = 0;
}

int __cdecl cbGetStream(QString evName,QVariantMap evMap,void*pUser)
{
	
	if ("RecordStream"==evName)
	{
		((RemoteBackup*)pUser)->WriteFrameData(evMap);
	}
	
	return 0;
}