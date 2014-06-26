#include "Recorder.h"
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include "avilib.h"
//#include "h264wh.h"

#include <guid.h>

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
m_bFinish(true),
m_bcheckdiskfreesize(false)
{
	m_nFrameCount = 0;
	m_nLastTicket = 0;
	m_eventList<<"RecordState";
}

Recorder::~Recorder()
{
	m_bFinish = true;
	wait();
	cleardata();
}

int Recorder::Start()
{
	if (! QThread::isRunning())
	{
		m_bFinish = false;
		start();
		connect(&m_checkdisksize, SIGNAL(timeout()), this, SLOT(checkdiskfreesize()));
		m_checkdisksize.start(1000);
	}
	return IRecorder::OK;
}
int Recorder::Stop()
{
	m_bFinish = true;
	wait();
	cleardata();
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
			msleep(10);	
			if (m_dataqueue.size()>20)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"size:"<<m_dataqueue.size()<<"record cause sleep!!!";
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
void Recorder::run()
{
	QString sSavePath;
	int nRecStep = 0;
	unsigned int startTick = 0;
	unsigned int endTick = 0;
	QString lastFileName;
	avi_t * AviFile = NULL;
	int nSleepTime=0;
	int nLoopCount = 0;
	bool bAudioBeSet;
	bool bThreadRunning = true;
	bool bFileStart = false;
	QVariantMap parm;
	parm.insert("RecordState",true);
	enventProcCall("RecordState",parm);

	while (bThreadRunning)
	{
		if (m_dataqueue.size()<1||nSleepTime>10)
		{
			msleep(10);
			nSleepTime=0;
		}
		nSleepTime++;
		if (m_bFinish)
		{
			nRecStep = 5;
		}


		switch(nRecStep)
		{
		case 0:// prepare for new file
			{
				bool bRet = CreateSavePath(sSavePath);
				if (!bRet)
				{
					/*msleep(1000);*/
					m_dataRef.lock();
					if (m_dataqueue.size()>0)
					{
						RecBufferNode NodeTemp=m_dataqueue.front();
						delete []NodeTemp.Buffer;
						NodeTemp.Buffer = NULL;
						m_dataqueue.pop_front();
					}
					m_dataRef.unlock();
					m_bFinish=true;
					break;
				}

				// Create save path

				bAudioBeSet = false;

				memset(m_nFrameCountArray,0,sizeof(m_nFrameCountArray));
				m_nFrameCount = 0;
				m_nLastTicket = 0;
				nRecStep = 1;
			}
			break;
		case 1:// wait for an I frame
			{
				m_dataRef.lock();
				if (m_dataqueue.size() > 0)
				{
					RecBufferNode NodeTemp = m_dataqueue.front();
					if (bFileStart)
					{
						startTick = NodeTemp.dwTicketCount;
					}
					if (AVENC_IDR == NodeTemp.dwDataType)
					{
						nRecStep = 2;
					}
					else
					{
						delete []NodeTemp.Buffer;
						NodeTemp.Buffer = NULL;
						m_dataqueue.pop_front();
					}
				}
				m_dataRef.unlock();
			}
			break;
		case 2://  write file head
			{
				QString sFilePathName = sSavePath;
				QString sFileName;
				m_dataRef.lock();
				if (m_dataqueue.size() > 0)
				{

					// Create path
					CreateDir(sFilePathName);

					// Write file head
					AviFile = AVI_open_output_file(sFilePathName.toAscii().data());
					// Create file failed
					if (NULL == AviFile)
					{
						qDebug()<<__FUNCTION__<<__LINE__<<sFilePathName<<"creat file fail";
						m_bFinish=true;
						m_dataRef.unlock();
						break;
					}
					AVI_set_video(AviFile,m_nRecWidth,m_nRecHeight,25,"X264");

					if (bFileStart && 0 == startTick - endTick)
					{
						unsigned int tick = getSeconds(sFilePathName);
						AVI_set_ticket(lastFileName.toLatin1().data(), tick);
					}

					nRecStep = 3;
				}
				m_dataRef.unlock();
			}
			break;
		case 3://  write frame
			{
				m_dataRef.lock();
				if (m_dataqueue.size() > 0)
				{
					RecBufferNode node = m_dataqueue.front();
					if (AVENC_IDR == node.dwDataType || AVENC_PSLICE == node.dwDataType)
					{
						AVI_write_frame(AviFile,node.Buffer,node.dwBufferSize,(AVENC_IDR == node.dwDataType));
					}// Ignor audio frame
					else if (0x00 == node.dwDataType)
					{
						if (!bAudioBeSet)
						{
							int AudioFormat = WAVE_FORMAT_ALAW;

							AVI_set_audio(AviFile, 1, node.samplerate, node.samplewidth, AudioFormat, 64);
							bAudioBeSet = true;
						}
						AVI_write_audio(AviFile,node.Buffer,node.dwBufferSize);

					}

					delete[] node.Buffer;
					node.Buffer = NULL;
					m_dataqueue.pop_front();
				}
				m_dataRef.unlock();


				if (m_bcheckdiskfreesize==true)
				{
					int fileMaxSize= m_StorageMgr.getFilePackageSize();
					quint64 diskReservedSize=(quint64)m_StorageMgr.getFreeSizeForDisk();
					long nWrittenSize = AVI_bytes_written(AviFile);
					if (nWrittenSize > 1024 * 1024 * fileMaxSize)
					{
						qDebug("File over 128M\n");
						nRecStep = 4;
					}

					m_bcheckdiskfreesize=false;
					QString sdisk = sSavePath.left(2);
					quint64 FreeByteAvailable;
					quint64 TotalNumberOfBytes;
					quint64 TotalNumberOfFreeBytes;
					m_StorageMgr.GetDiskFreeSpaceEx(sdisk.toAscii().data(),&FreeByteAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);

					if( TotalNumberOfFreeBytes<= diskReservedSize * 1024 * 1024)
					{
						// not enough free space
						if (m_StorageMgr.freeDisk())
						{
							//freedisk succeed,keep going
						}else{
							qDebug()<<__FUNCTION__<<__LINE__<<"Not enough free space\n";
							nRecStep = 4;
						}

					}
				}else{
					//do nothing
				}

			}
			break;
		case 4://  to be finish,find the next I frame
			{
				m_dataRef.lock();
				if (m_dataqueue.size() > 0)
				{
					RecBufferNode node = m_dataqueue.front();
					if (AVENC_IDR == node.dwDataType)
					{
						endTick = node.dwTicketCount;
						bFileStart = true;
						lastFileName = sSavePath;

						nRecStep = 5;
					}
					else if (AVENC_PSLICE == node.dwDataType)
					{
						AVI_write_frame(AviFile,node.Buffer,node.dwBufferSize,(AVENC_IDR == node.dwDataType));

						delete[] node.Buffer;
						node.Buffer = NULL;
						m_dataqueue.pop_front();
					}
					else if (AVENC_AUDIO == node.dwDataType)
					{
						AVI_write_audio(AviFile,node.Buffer,node.dwBufferSize);

						delete[] node.Buffer;
						node.Buffer = NULL;
						m_dataqueue.pop_front();
					}
					else{
						qDebug()<<__FUNCTION__<<__LINE__<<"lose control";
					}
				}
				m_dataRef.unlock();
			}
			break;
		case 5://  Close the file & more operatons
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
					AVI_set_video(AviFile,m_nRecWidth,m_nRecHeight,iFrameCount,"X264");

					qDebug()<<__FUNCTION__<<__LINE__<<"Close file";
					AVI_close(AviFile);
					AviFile = NULL;
				}
				if (m_bFinish)
				{
					qDebug()<<__FUNCTION__<<__LINE__<<"Terminate thread";
					bThreadRunning = false;
				}
				else
				{
					qDebug()<<__FUNCTION__<<__LINE__<<"Next file";
					nRecStep = 0;
				}
			}
			break;
		default:
			{

			}
			break;
		}
	}
	m_bFinish = true;
	parm.clear();
	parm.insert("RecordState",false);
	enventProcCall("RecordState",parm);
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

bool Recorder::CreateSavePath(QString& sSavePath)
{
	sSavePath = m_StorageMgr.getFileSavePath(m_devname,m_channelnum);
	return true;
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
		////创建文件
		//QFile file(fullname);
		//file.open(QIODevice::ReadOnly);
		//file.close();

		//if (!file.exists())
		//    return false;
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

void Recorder::enventProcCall( QString sEvent,QVariantMap parm )
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
	if (!QThread::isRunning())
	{
		m_checkdisksize.stop();
	}else{
		//keep going
	}
}







