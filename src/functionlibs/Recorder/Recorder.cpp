#include "Recorder.h"
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include "avilib.h"
#include "h264wh.h"

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
m_bFinish(true)
{
	m_nFrameCount = 0;
	m_nLastTicket = 0;
}

Recorder::~Recorder()
{

}

int Recorder::Start()
{
	if (! QThread::isRunning())
	{
		m_bFinish = false;
		start();
		
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
int Recorder::InputFrame(FrameInfo frameinfo)
{
	int type = frameinfo.type;
	if ((type != 0x00 && type !=  0x01 && type != 0x02) || frameinfo.uiDataSize<=0 )
	{
		return IRecorder::E_PARAMETER_ERROR;
	}
	if (!m_bFinish)
	{
		//FrameInfo *frame = (FrameInfo *)cbuf;
		RecBufferNode bufTemp;
		bufTemp.dwDataType = type;
		bufTemp.dwBufferSize = frameinfo.uiDataSize;
		bufTemp.dwTicketCount = frameinfo.uiTimeStamp;
		bufTemp.nChannel = 0;
		bufTemp.Buffer = new char[frameinfo.uiDataSize];
		memcpy(bufTemp.Buffer,frameinfo.pData,frameinfo.uiDataSize);
		m_dataRef.lock();
		m_dataqueue.enqueue(bufTemp);
		m_dataRef.unlock();

		if (0x01 == type || 0x02 == type)
		{
			if (0x01 == type)
			{
				GetWidthHeight(bufTemp.Buffer,bufTemp.dwBufferSize,&m_nRecWidth,&m_nRecHeight);
			}
			m_nFrameCount ++;
			if ( 0 == m_nLastTicket )
			{
				m_nLastTicket = frameinfo.uiTimeStamp;
			}
			else
			{
				if (frameinfo.uiTimeStamp - m_nLastTicket >= 1000)
				{
					if (m_nFrameCount < 31)
					{
						m_nFrameCountArray[m_nFrameCount] ++;
					}
						m_nFrameCount = 0;
					m_nLastTicket = frameinfo.uiTimeStamp;
				}
			}
		}

	}

	return IRecorder::OK;
}
int Recorder::SetDevInfo(const QString& devname,int nChannelNum)
{
	m_devname = devname;
	if (nChannelNum<1 || nChannelNum >16)
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
	//char sSavePath[1024];
	QString sSavePath;
	int nRecStep = 0;
	avi_t * AviFile = NULL;
	int nLoopCount = 0;
	bool bAudioBeSet;
	bool bThreadRunning = true;
	while (bThreadRunning)
	{
		msleep(10);

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
					msleep(1000);
					break;
				}

				// Create save path
				qDebug("Prepare to record,read path from profile:%s\n",sSavePath);

				bAudioBeSet = false;

				memset(m_nFrameCountArray,0,sizeof(m_nFrameCountArray));
				m_nFrameCount = 0;

				nRecStep = 1;
			}
			break;
		case 1:// wait for an I frame
			{
				m_dataRef.lock();
				if (m_dataqueue.size() > 0)
				{
					RecBufferNode NodeTemp = m_dataqueue.front();
					if (AVENC_IDR == NodeTemp.dwDataType)
					{
						qDebug("Get an I frame\n");
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

					QDateTime timeCur = QDateTime::currentDateTime();
					// Save path read from private profile

					// Create path
					CreateDir(sFilePathName);

					// Write file head
					AviFile = AVI_open_output_file(sFilePathName.toAscii().data());
					// Create file failed
					if (NULL == AviFile)
					{
						nRecStep = 0;
						m_dataRef.unlock();
						break;
					}
					AVI_set_video(AviFile,m_nRecWidth,m_nRecHeight,16,"X264");
					qDebug("Set pic resolution:%d * %d\n",m_nRecWidth,m_nRecHeight);
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
						AudioBufAttr * AudioHead = (AudioBufAttr *)node.Buffer;
						if (!bAudioBeSet)
						{
							int AudioFormat = WAVE_FORMAT_ALAW;
							if (!strcmp(AudioHead->encode,"g711a"))
							{
								qDebug("Audio format g711a\n");
								AudioFormat = WAVE_FORMAT_ALAW;
							}
							AVI_set_audio(AviFile, 1, AudioHead->samplerate, AudioHead->samplewidth, AudioFormat, 64);
							bAudioBeSet = true;
						}
						AVI_write_audio(AviFile,node.Buffer + sizeof(AudioBufAttr),AudioHead->entries * AudioHead->packsize);
					}


					delete node.Buffer;
					node.Buffer = NULL;
					m_dataqueue.pop_front();
				}
				m_dataRef.unlock();
				long nWrittenSize = AVI_bytes_written(AviFile);
				if (nWrittenSize > 1024 * 1024 * m_StorageMgr.getFilePackageSize())
				{
					qDebug("File over 128M\n");
					nRecStep = 4;
				}

				QString sdisk = sSavePath.left(2);
				quint64 FreeByteAvailable;
				quint64 TotalNumberOfBytes;
				quint64 TotalNumberOfFreeBytes;
				m_StorageMgr.GetDiskFreeSpaceEx(sdisk.toAscii().data(),&FreeByteAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);

				if( TotalNumberOfFreeBytes<= (quint64)m_StorageMgr.getFreeSizeForDisk() * 1024 * 1024)
				{ // not enough free space

					qDebug("Not enough free space\n");

					// close the file & terminate
					nRecStep = 4;
				}

			/*	if (m_bPackage)
				{
					m_bPackage = false;
					nRecStep = 4;
				}*/
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
						qDebug("Get an I frame,skip\n");
						nRecStep = 5;
					}
					else if (AVENC_PSLICE == node.dwDataType)
					{
						qDebug("Over frame\n");
						AVI_write_frame(AviFile,node.Buffer,node.dwBufferSize,(AVENC_IDR == node.dwDataType));

						delete node.Buffer;
						node.Buffer = NULL;
						m_dataqueue.pop_front();
					}
					else if (AVENC_AUDIO == node.dwDataType)
					{
						AudioBufAttr * AudioHead = (AudioBufAttr *)node.Buffer;
						AVI_write_audio(AviFile,node.Buffer + sizeof(AudioBufAttr),AudioHead->entries * AudioHead->packsize);

						delete node.Buffer;
						node.Buffer = NULL;
						m_dataqueue.pop_front();
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

					qDebug("Close file\n");
					AVI_close(AviFile);
					AviFile = NULL;
				}
				if (m_bFinish)
				{
					qDebug("Terminate thread\n");
					bThreadRunning = false;
				}
				else
				{
					qDebug("Next file\n");
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
		//创建文件
		QFile file(fullname);
		file.open(QIODevice::ReadOnly);
		file.close();

		if (!file.exists())
		    return false;
	}

	return true;
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