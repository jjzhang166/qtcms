#include "Recorder.h"
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include "avilib.h"
//#include "h264wh.h"

#include <guid.h>

#define qDebug() qDebug()<<"this:"<<(int)this
#define qWarning() qWarning()<<"this:"<<(int)this

#pragma pack(4)
typedef struct _tagAudioBufAttr{
	int entries;
	int packsize;
	unsigned long long pts;
	time_t * gtime;
	char encode[8];
	int samplerate;
	int samplewidth;
}AudioBufAttr;
#pragma pack()

Recorder::Recorder() :
m_nRef(0),
m_channelnum(1),
m_windId(0),
m_recordType(0),
m_nUpdateCount(0),
m_bUpdateEndTime(false),
m_bFinish(true),
m_bIsblock(false),
m_bcheckdiskfreesize(false)
{
	m_nFrameCount = 0;
	m_nLastTicket = 0;
	m_eventList<<"RecordState";
	connect(&m_checkdisksize, SIGNAL(timeout()), this, SLOT(checkdiskfreesize()));
	connect(&m_checkIsBlock,SIGNAL(timeout()),this,SLOT(checkIsBlock()));
	connect(this,SIGNAL(sgBackToMainThread(QVariantMap)),this,SLOT(slBackToMainThread(QVariantMap)));
	m_checkIsBlock.start(3000);
}

Recorder::~Recorder()
{
	m_bFinish = true;
	int nCount=0;
	while(QThread::isRunning()){
		sleepEx(10);
		nCount++;
		if (nCount>500&&nCount%100==0)
		{
			qDebug()<<__FUNCTION__<<__LINE__<<m_devname<<nCount/100<<"there must exist some error ,thread block at :"<<m_nPosition;
		}
	}
	cleardata();
	/*m_checkdisksize.stop();*/
	m_checkIsBlock.stop();
}

int Recorder::Start()
{
	if (! QThread::isRunning())
	{
		m_bFinish = false;
		start();
		connect(&m_updateSchRec, SIGNAL(timeout()), this, SLOT(updateSchRec()));
		m_updateSchRec.start(1000*60);
	}
	return IRecorder::OK;
}
int Recorder::Stop()
{
	if (QThread::isRunning())
	{
		m_bFinish = true;
	}else{
		//do nothing
		cleardata();
	}
	m_updateSchRec.stop();
	return IRecorder::OK;
}
int Recorder::InputFrame(QVariantMap& frameinfo)
{
	int type = frameinfo["frametype"].toInt();
	int datasize = frameinfo["length"].toInt();
	if ((type != 0x00 && type !=  0x01 && type != 0x02) || datasize<=0 )
	{
		return IRecorder::E_PARAMETER_ERROR;
	}
	if (!m_bFinish)
	{
		RecBufferNode bufTemp;
		bufTemp.dwDataType = type;
		bufTemp.dwBufferSize = datasize;
		bufTemp.dwTicketCount = (unsigned int)(frameinfo["pts"].toULongLong()/1000);
		bufTemp.nChannel = frameinfo["channel"].toInt();
		bufTemp.Buffer=NULL;
		bufTemp.Buffer = new char[datasize];
		if (!bufTemp.Buffer)
			return IRecorder::E_SYSTEM_FAILED;
		char* pdata = (char*)frameinfo["data"].toUInt();
		memcpy(bufTemp.Buffer,pdata,datasize);

		if (0x01 == type || 0x02 == type)
		{
			if (0x01 == type)
			{
				//GetWidthHeight(bufTemp.Buffer,bufTemp.dwBufferSize,&m_nRecWidth,&m_nRecHeight);
				m_nRecWidth = frameinfo["width"].toInt();
				m_nRecHeight = frameinfo["height"].toInt();
			}
			m_nFrameCount ++;
			if ( 0 == m_nLastTicket )
			{
				m_nLastTicket = bufTemp.dwTicketCount;
			}
			else
			{
				if (bufTemp.dwTicketCount - m_nLastTicket >= 1000)
				{
					if (m_nFrameCount < 31)
					{
						m_nFrameCountArray[m_nFrameCount] ++;
					}
						m_nFrameCount = 0;
					m_nLastTicket = bufTemp.dwTicketCount;
				}
			}
		}
		else if (0x00 == type)
		{
			bufTemp.samplerate = frameinfo["samplerate"].toInt();
			bufTemp.samplewidth = frameinfo["samplewidth"].toInt();
		}
		if (m_dataqueue.size()>10){
			msleep(m_dataqueue.size());	
			if (m_dataqueue.size()>20&&m_dataqueue.size()%10==0)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"size:"<<m_dataqueue.size()<<m_devname<<"record cause sleep!!!";
			}
		}
		
		m_dataRef.lock();
		m_dataqueue.enqueue(bufTemp);
		m_dataRef.unlock();
	}

	return IRecorder::OK;
}
int Recorder::SetDevInfo(const QString& devname,int nChannelNum)
{
	m_devname = devname;
	if (nChannelNum<0)
	{
		return IRecorder::E_PARAMETER_ERROR;
	}
	else
	{
		m_channelnum = nChannelNum;
	}
	return IRecorder::OK;
}

int Recorder::SetDevInfoEx(const int &nWindId, const int &nRecordType)
{
	if (nWindId < 0 || nRecordType > 3 || nRecordType < 0)
	{
		return IRecorderEx::E_PARAMETER_ERROR;
	}
	m_windId = nWindId;
	m_recordType = nRecordType;
	return IRecorderEx::OK;
}

void Recorder::run()
{
	qDebug()<<"----------------run start-----------------";

	QString sSavePath;
	int nRecStep=INIT;
	avi_t * AviFile = NULL;
	int nSleepTime=0;
	int nLoopCount = 0;
	int nFileMaxSize=124;
	quint64 qDiskReservedSize=1024;
	bool bAudioBeSet=false;
	bool bThreadRunning = true;
	bool bHasAddRecord = false;
	QVariantMap parm;
	parm.insert("RecordState",true);
	eventProcCall("RecordState",parm);
	QTime start;
	QVariantMap vCheckDisk;
	vCheckDisk.insert("checkdiskfreesize",true);
	emit sgBackToMainThread(vCheckDisk);
	while(bThreadRunning){
		if (m_dataqueue.size()<2||nSleepTime>100)
		{
			msleep(10);
			nSleepTime=0;
			m_nPosition=__LINE__;
		}
		nSleepTime++;

		switch(nRecStep){
		case INIT:{
			//新文件的各项参数初始化
			nFileMaxSize=m_StorageMgr.getFilePackageSize();
			qDiskReservedSize=(quint64)m_StorageMgr.getFreeSizeForDisk();
			bAudioBeSet=false;
			m_dataRef.lock();
			memset(m_nFrameCountArray,0,sizeof(m_nFrameCountArray));
			m_nFrameCount=0;
			m_nLastTicket=0;
			m_lnFirstPts=0;
			m_lnLastPts=0;
			m_dataRef.unlock();
			if (m_bFinish)
			{
				nRecStep=END;
			}else{
				nRecStep=FRIST_I_FRAME;
			}
			   }
			   break;
		case FRIST_I_FRAME:{
			//等待第一个I帧
			m_dataRef.lock();
			if (m_dataqueue.size()>0)
			{
				RecBufferNode nodeTemp=m_dataqueue.front();
				if (AVENC_IDR==nodeTemp.dwDataType)
				{
					m_lnFirstPts=nodeTemp.dwTicketCount;
					nRecStep=CREATE_PATH;
				}else{
					delete []nodeTemp.Buffer;
					nodeTemp.Buffer=NULL;
					m_dataqueue.pop_front();
					nRecStep=FRIST_I_FRAME;
				}
			}else{
				nRecStep=FRIST_I_FRAME;
			}

			if (m_bFinish)
			{
				nRecStep=END;    
			}else{
				//keep going
			}
			m_dataRef.unlock();
			  }
			   break;
		case CREATE_PATH:{
			//申请空间 并 创建文件路径
			qWarning()<<__FUNCTION__<<__LINE__<<"start create file";

			m_nPosition=__LINE__;
			m_bIsblock=true;
			if (CreateSavePath(sSavePath,start)&&CreateDir(sSavePath))
			{
				nRecStep=OPEN_FILE;
				//开始记录录像时间
				QString curDate = QDate::currentDate().toString("yyyy-MM-dd");
// 				if (!bHasAddRecord && !m_StorageMgr.addSearchRecord(m_windId, m_recordType, curDate, start.toString("hh:mm:ss"), QString("00:00:00")))
// 				{
// 					qDebug()<<__FUNCTION__<<__LINE__<<"add search record info failed!";
// 				}
// 				else
// 				{
// 					qDebug()<<__FUNCTION__<<__LINE__<<"add search record: wndId:"<<m_windId<<" start:"<<start.toString("hh:mm:ss")<<"end: 00:00:00";
// 					bHasAddRecord = true;
// 				}
				if (!bHasAddRecord)
				{
					if (m_StorageMgr.addSearchRecord(m_windId, m_recordType, curDate, start.toString("hh:mm:ss"), QString("00:00:00")))
					{
						bHasAddRecord = true;
						qDebug()<<__FUNCTION__<<__LINE__<<"add search record: wndId:"<<m_windId<<" start:"<<start.toString("hh:mm:ss")<<"end: 00:00:00";
					}
					else
					{
						qDebug()<<__FUNCTION__<<__LINE__<<"add search record info failed!";
					}
				}
			}else{
				// fix me : 处理建立路径时产生的资源
				qDebug()<<__FUNCTION__<<__LINE__<<"recorder fail as create path fail";
				nRecStep=END;
			}
			m_bIsblock=false;
			if (m_bFinish)
			{
				// fix me : 处理建立路径时产生的资源
				nRecStep=END;
			}else{
				//keep going 
			}
			   }

			qWarning()<<__FUNCTION__<<__LINE__<<"stop create file"<<sSavePath<<"step:"<<nRecStep;
			   break;
		case OPEN_FILE:{
			//打开文件
			qWarning()<<__FUNCTION__<<__LINE__<<"start open file"<<sSavePath;

			m_dataRef.lock();
			if (m_dataqueue.size()>0)
			{
				AviFile=AVI_open_output_file(sSavePath.toAscii().data());
				if (NULL!=AviFile)
				{
					nRecStep=SET_VIDEO_PARM;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"recorder fail as AVI_open_output_file fail";
					nRecStep=END;
				}
			}else{
				//keep going
			}
			m_dataRef.unlock();
			   }
		   qWarning()<<__FUNCTION__<<__LINE__<<"stop open file"<<sSavePath<<AviFile<<"step:"<<nRecStep;
			   break;
		case SET_VIDEO_PARM:{
			// 设置文件（视频）的各项参数
			m_dataRef.lock();
			AVI_set_video(AviFile,m_nRecWidth,m_nRecHeight,25,"X264");
			m_dataRef.unlock();
			nRecStep=WRITE_FRAME;
			   }
			qWarning()<<__FUNCTION__<<__LINE__<<"set parm"<<sSavePath<<"W: "<<m_nRecWidth<<" H: "<<m_nRecHeight<<"step:"<<nRecStep;
			   break;
		case SET_AUDIO_PARM:{
			//设置文件（音频）的各项参数
			   }
			   break;
		case WRITE_FRAME:{
			//写文件
			m_dataRef.lock();
			if (m_dataqueue.size()>0)
			{
				nSleepTime--;
				RecBufferNode node=m_dataqueue.front();
				if (AVENC_IDR==node.dwDataType||AVENC_PSLICE==node.dwDataType)
				{
					m_lnLastPts=node.dwTicketCount;
					AVI_write_frame(AviFile,node.Buffer,node.dwBufferSize,(AVENC_IDR==node.dwDataType));
				}else if (0x00==node.dwDataType)
				{
					if (!bAudioBeSet)
					{
						int AudioFormat=WAVE_FORMAT_ALAW;
						AVI_set_audio(AviFile,1,node.samplerate,node.samplewidth,AudioFormat,64);
						bAudioBeSet=true;
					}
					AVI_write_audio(AviFile,node.Buffer,node.dwBufferSize);
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"this is a undefined frame,please check";
				}
				delete [] node.Buffer;
				node.Buffer=NULL;
				m_dataqueue.pop_front();
			}else{
				//keep going
			}
			if (m_bFinish)
			{
				nRecStep=WAIT_FOR_PACK;
			}else{
				nRecStep=CHECK_DISK_SPACE;
			}
			m_dataRef.unlock();
			   }
			   break;
		case CHECK_DISK_SPACE:{

			//检测硬盘空间
			if (m_bcheckdiskfreesize)
			{
				//硬盘空间足够，下一步检测文件大小
				//硬盘空间不够，释放空间
				//释放成功，下一步检测文件大小，释放失败，跳转到等待I帧
				m_bcheckdiskfreesize=false;
				QString sDisk=sSavePath.left(2);
				quint64 FreeByteAvailable;
				quint64 TotalNumberOfBytes;
				quint64 TotalNumberOfFreeBytes;
				m_StorageMgr.GetDiskFreeSpace(sDisk.toAscii().data(),&FreeByteAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);
				if (TotalNumberOfFreeBytes<=qDiskReservedSize*1024*1024)
				{
					m_nPosition=__LINE__;
					m_bIsblock=true;
					if (m_StorageMgr.freeDisk())
					{
						//success
						nFileMaxSize=m_StorageMgr.getFilePackageSize();
						qDiskReservedSize=(quint64)m_StorageMgr.getFreeSizeForDisk();
						nRecStep=CHECK_FILE_SIZE;
					}else{
						//fail
						qDebug()<<__FUNCTION__<<__LINE__<<"recorder turn to pack as no enough space";
						nRecStep=WAIT_FOR_PACK;
					}
					m_bIsblock=false;
				}else{
					//do nothing
					nRecStep=CHECK_FILE_SIZE;
				}
			}else{
				//do nothing
				nRecStep=WRITE_FRAME;
			}
			   }
			   break;
		case CHECK_FILE_SIZE:{
			//检测文件大小
			//更新结束时间5min
			//文件不够大，接着录像 ，否则打包文件,跳转到等待I帧
			if (m_bUpdateEndTime)
			{
				m_bUpdateEndTime=false;
// 				m_StorageMgr.updateSearchRecord(QTime::currentTime().toString("hh:mm:ss"));
			}else{
				//do nothing
			}
			long nWritenSize=AVI_bytes_written(AviFile);
			if (nWritenSize>1024*1024*nFileMaxSize)
			{
				nRecStep=WAIT_FOR_PACK;
			}else{
				//keep going
				nRecStep=WRITE_FRAME;
			}
			   }
			   break;
		case WAIT_FOR_PACK:{
			qWarning()<<__FUNCTION__<<__LINE__<<"start wait for pack";

			//等待i帧过来，打包
			int nCount=0;
			bool nBwait=true;
			m_nPosition=__LINE__;
			m_bIsblock=true;
			while(nBwait&&nCount<300){
				m_dataRef.lock();
				if (m_dataqueue.size()>0)
				{
					RecBufferNode node=m_dataqueue.front();
					if (AVENC_IDR==node.dwDataType)
					{
						nBwait=false;
					}else if (AVENC_PSLICE==node.dwDataType)
					{
						AVI_write_frame(AviFile,node.Buffer,node.dwBufferSize,(AVENC_IDR==node.dwDataType));
						m_lnLastPts=node.dwTicketCount;
						delete[]node.Buffer;
						node.Buffer=NULL;
						m_dataqueue.pop_front();
					}else if (AVENC_AUDIO==node.dwDataType)
					{
						AVI_write_audio(AviFile,node.Buffer,node.dwBufferSize);
						delete[]node.Buffer;
						node.Buffer=NULL;
						m_dataqueue.pop_front();
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"there is a undefined frame ,please check";
					}
				}else{
					//do nothing
					msleep(10);
				}
				nCount++;
				m_dataRef.unlock();
			}
			m_bIsblock=false;
			nRecStep=PACK;
						   }
						   break;
		case PACK:{
			qWarning()<<__FUNCTION__<<__LINE__<<"start pack";

			//文件打包,打包是失败，跳转到end
			//打包并保存到数据库成功，如果失败，跳转到end
			//检测是否结束，如果结束，跳转到end，否则重头开始
			if (AviFile!=NULL)
			{
				int iMaxFramerate=0;
				int iMaxFramerateCount=0;
				int iFrameCount=0;
				int i;
				m_dataRef.lock();
				for (i=0;i<31;i++)
				{
					if (m_nFrameCountArray[i]!=0)
					{
						iMaxFramerate=i;
					}
				}
				for (i=iMaxFramerate-3>=0?iMaxFramerate-3:0;i<iMaxFramerate;i++)
				{
					if (m_nFrameCountArray[i]>(unsigned int)iMaxFramerateCount)
					{
						iMaxFramerateCount=m_nFrameCountArray[i];
						iFrameCount=i;
					}
				}
				if (iFrameCount==0)
				{
					int totalFrame=AVI_video_frames(AviFile);
					quint64 totalTime=0;
					totalTime=m_lnLastPts-m_lnFirstPts;
					if (totalTime/1000>0)
					{
						iFrameCount=totalFrame/(totalTime/1000);
						if (iFrameCount>25)
						{
							iFrameCount=25;
						}else{
							//do nothing
						}
					}else{
						if (totalFrame<25)
						{
							iFrameCount=totalFrame;
						}else{
							iFrameCount=25;
						}	
					}
				}else{
					//do nothing
				}
				AVI_set_video(AviFile,m_nRecWidth,m_nRecHeight,iFrameCount,"X264");
				AVI_close(AviFile);

				qDebug()<<__FUNCTION__<<__LINE__<<"pack file"<<sSavePath<<"W: "<<m_nRecWidth<<" H: "<<m_nRecHeight<<"frameRate: "<<iFrameCount;

				AviFile=NULL;
				m_dataRef.unlock();
				m_nPosition=__LINE__;
				m_bIsblock=true;
// 				QString sEndTime=getFileEndTime(sSavePath,start);
				QTime end = QTime::currentTime();
				QString sEndTime = end.toString("hh:mm:ss");
				if (!sEndTime.isEmpty())
				{
					m_nPosition=__LINE__;
					if (end < start)
					{
						sEndTime = "23:59:59";
					}
					if (m_StorageMgr.updateRecord(sEndTime,getFileSize(sSavePath)))
					{
						//keep going
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"recorder stop as updateRecord fail";
						nRecStep=END;
					}
					m_nPosition=__LINE__;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"record fail as it can not get file endTime";
					//删除此文件，并删除数据库中此文件的记录
					m_nPosition=__LINE__;
					QFile fRemove;
					if (fRemove.remove(sSavePath))
					{
						//删除数据库的记录
						m_nPosition=__LINE__;
						if (m_StorageMgr.deleteRecord(sSavePath))
						{
							//do nothing
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"deleteRecord fail ,please check";
						}
						m_nPosition=__LINE__;
					}else{
						qDebug()<<__FUNCTION__<<__LINE__<<"it can not remove the crash file ,please check";
					}			
					//keep going
				}
				m_bIsblock=false;
				if (m_bFinish)
				{
					qWarning()<<"m_bFinish: "<<m_bFinish<<"switch to step:END";
					nRecStep=END;
				}else{
					//keep going
					nRecStep=INIT;
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"recorder pack fail ,there must exist a error";
			}
			   }
			   break;
		case ERROR:{
			//错误
			qDebug()<<__FUNCTION__<<__LINE__<<"ERROR step";

				}
				break;
		case END:{
			qDebug()<<__FUNCTION__<<__LINE__<<"END step";

			//end
			bThreadRunning=false;
			m_bFinish=true;
			if (AviFile!=NULL)
			{
				AVI_close(AviFile);
				AviFile=NULL;
				qDebug()<<__FUNCTION__<<__LINE__<<"there must exist a error";
			}else{
				//do nothing
			}
			//更新录像结束时间
			m_nPosition=__LINE__;
			m_bIsblock=true;
			if (bHasAddRecord)
			{
				QTime endTime = QTime::currentTime();
				if (start < endTime)//当天的录像
				{
					if (start.secsTo(endTime) <= 3)
					{
						//录像时间不足3秒，认为录像失败，删除插入的数据
						m_StorageMgr.deleteSearchRecord();
					}
					else
					{
						QString endStr = endTime.toString("hh:mm:ss");
						m_StorageMgr.updateSearchRecord(endStr);

						qWarning()<<__FUNCTION__<<__LINE__<<"update search record wnd:"<<m_windId<<"endtime:"<<endStr;
					}
				}
				else
				{
					m_StorageMgr.updateSearchRecord(QString("23:59:59"));//跨天录像，从零点截断
					qWarning()<<__FUNCTION__<<__LINE__<<"update search record wnd:"<<m_windId<<"endtime:23：59：59";
				}
			}
			m_bIsblock=false;
			//删除无用文件
			m_nPosition=__LINE__;
			m_bIsblock=true;
			if (-1!=m_StorageMgr.getInsertId())
			{
				QFile fRemove;
				if (fRemove.exists(sSavePath))
				{
					fRemove.remove(sSavePath);
				}else{
					// do nothing
				}
				//删除数据库中的记录
				m_StorageMgr.deleteRecord(sSavePath);
			}else{
				// do nothing 
			}
			m_bIsblock=false;
			m_nPosition=__LINE__;
			m_bIsblock=true;
			cleardata();
			m_bIsblock=false;
				}
			   break;
		}
	}
	vCheckDisk.clear();
	vCheckDisk.insert("checkdiskfreesize",false);
	emit sgBackToMainThread(vCheckDisk);
	m_bFinish = true;
	parm.clear();
	parm.insert("RecordState",false);
	m_nPosition=__LINE__;
	m_bIsblock=true;
	eventProcCall("RecordState",parm);
	m_bIsblock=false;

	qDebug()<<"----------------run end-----------------";

}

void Recorder::cleardata()
{
	m_dataRef.lock();
	while(m_dataqueue.size()>0)
	{
		RecBufferNode bnode = m_dataqueue.dequeue();
		delete[] bnode.Buffer;
	}
	m_dataRef.unlock();
}

bool Recorder::CreateSavePath(QString& sSavePath, QTime &start)
{
	sSavePath = m_StorageMgr.getFileSavePath(m_devname,m_channelnum, m_windId, m_recordType, start);
	if (sSavePath=="none")
	{
		return false;
	}else{
		return true;
	}
}

bool Recorder::CreateDir(QString fullname)
{
	QFileInfo info(fullname);
	
	QString filepath = info.path();
	QString filename = info.fileName();
	if (filename.isEmpty())
	    return false;
	QFile file(fullname);
	if(!file.exists())
	{
		//创建路径
		QDir dir;
		if (!dir.exists(filepath))
			dir.mkpath(filepath);
	}
	return true;
}

unsigned int Recorder::getSeconds(QString &fileName)
{
	QRegExp rx;
	QTime t;
	rx.setPattern("([0-9]{6}).avi");
	if (-1 != rx.indexIn(fileName))
	{
		QString time = rx.cap(1);
		t = QTime::fromString(time, "hhmmss");
	}
	return t.hour()*3600 + t.minute()*60 + t.second();
}

QString Recorder::getModeName()
{
	return QString("Recorder");
}

long __stdcall Recorder::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IRecorder == iid)
	{
		*ppv = static_cast<IRecorder *>(this);
	}
	else if (IID_IRecorderEx == iid)
	{
		*ppv = static_cast<IRecorderEx *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IEventRegister==iid)
	{
		*ppv=static_cast<IEventRegister*>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall Recorder::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall Recorder::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

QStringList Recorder::eventList()
{
	return m_eventList;
}

int Recorder::queryEvent( QString eventName,QStringList& eventParams )
{
	if (!m_eventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	if ("RecordState" == eventName)
	{
		eventParams<<"RecordState";
	}
	return IEventRegister::OK;
}

int Recorder::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser )
{
	if (!m_eventList.contains(eventName))
	{
		return IEventRegister::E_EVENT_NOT_SUPPORT;
	}
	ProcInfoItem proInfo;
	proInfo.proc=proc;
	proInfo.puser=pUser;

	m_eventMap.insert(eventName,proInfo);
	return IEventRegister::OK;
}

void Recorder::eventProcCall( QString sEvent,QVariantMap parm )
{
	if (m_eventList.contains(sEvent))
	{
		ProcInfoItem eventDes=m_eventMap.value(sEvent);
		if (NULL!=eventDes.proc)
		{
			eventDes.proc(sEvent,parm,eventDes.puser);
		}
	}
}

void Recorder::checkdiskfreesize()
{
	m_bcheckdiskfreesize=true;
	m_nUpdateCount++;
	if (m_nUpdateCount>150)
	{
		m_nUpdateCount=0;
		m_bUpdateEndTime=true;
	}
}
QString Recorder::getFileEndTime( QString fileName, QTime start )
{
	QString sRet;
	sRet.clear();
	avi_t *aviFile=NULL;
	aviFile=AVI_open_input_file(fileName.toLatin1().data(),1);
	if (NULL!=aviFile)
	{
		int audioChunks=AVI_audio_chunks(aviFile);
		int audioRate=AVI_audio_rate(aviFile);
		int audioBlock=AVI_audio_size(aviFile,0);
		int aviFileLength=0;
		if (0 <= audioChunks && 0 <= audioRate && 0 <= audioBlock)
		{
			aviFileLength = audioChunks*audioBlock/audioRate;
		}else{
			int totalFrames = AVI_video_frames(aviFile);
			int frameRate = AVI_frame_rate(aviFile);
			if (0 == totalFrames || 0 == frameRate)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"getFileEndTime as file totalFrames==0 or frameRate ==0,this kind of file should be remove";
			}else{
				aviFileLength = totalFrames/frameRate;//the length of avi file playing time
			}
		}
		AVI_close(aviFile);
		if (aviFileLength==0)
		{
			return sRet;
		}else{
			return start.addSecs(aviFileLength).toString("hh:mm:ss");
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getFileEndTime fail as AVI_open_input_file fail";
	}
	return sRet;
}

qint64 Recorder::getFileSize( QString fileName )
{
	return m_StorageMgr.getFileSize(fileName);
}

void Recorder::checkIsBlock()
{
	if (m_bIsblock)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"block at:"<<m_nPosition<<"StorageMgr run to:"<<m_StorageMgr.getBlockPosition();
	}else{
		//do nothing
	}
}

void Recorder::slBackToMainThread( QVariantMap evMap )
{
	if (evMap.contains("checkdiskfreesize"))
	{
		if (evMap.value("checkdiskfreesize").toBool())
		{
			m_checkdisksize.start(2000);
		}else{
			m_checkdisksize.stop();
		}
	}
}

void Recorder::sleepEx( int time )
{
	if (m_nSleepSwitch<100)
	{
		msleep(time);
		m_nSleepSwitch++;
	}else{
		QEventLoop eventloop;
		QTimer::singleShot(2, &eventloop, SLOT(quit()));
		eventloop.exec();
		m_nSleepSwitch=0;
	}
	return;
}

int Recorder::FixExceptionalData()
{
	return m_StorageMgr.fixExceptionalData();
}

void Recorder::updateSchRec()
{
	m_StorageMgr.updateSearchRecord(QTime::currentTime().toString("hh:mm:ss"));
}