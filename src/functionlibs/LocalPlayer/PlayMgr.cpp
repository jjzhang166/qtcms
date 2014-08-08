#include "PlayMgr.h"
#include <guid.h>
#include "IEventRegister.h"
#include "avilib.h"
#include <QElapsedTimer>
#include <QDebug>

#define qDebug() qDebug()<<"======="<<__FUNCTION__<<(int)this

QMutex g_mtxPause;
QWaitCondition g_waitConPause;

uint PlayMgr::m_playingTime = 0;
bool PlayMgr::m_bIsChange = false;
void* PlayMgr::m_who = NULL;
bool PlayMgr::m_bIsPickThread = false;
IAudioPlayer* PlayMgr::m_pAudioPlayer = NULL;
bool PlayMgr::m_bIsSkiped = false;

PlayMgr::PlayMgr(void):
	m_pRenderWnd(NULL),
	m_pVedioDecoder(NULL),
	m_pVedioRender(NULL),
	m_pcbThrowExp(NULL),
	m_nInitHeight(0),
	m_nInitWidth(0),
	m_nSpeedRate(0),
	m_nStartPos(0),
	m_nAudioChl(1),
	m_nSampleRate(0),
	m_nSampleWidth(0),
	m_bIsAudioOpen(false),
	m_bPlaying(false),
	m_bStop(false),
	m_bPause(false)
{
	//ÉêÇë½âÂëÆ÷½Ó¿Ú
// 	pcomCreateInstance(CLSID_h264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pVedioDecoder);
	pcomCreateInstance(CLSID_HiH264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pVedioDecoder);
	//ÉêÇëäÖÈ¾Æ÷½Ó¿Ú
	pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_pVedioRender);

}


PlayMgr::~PlayMgr(void)
{
	m_bStop = true;
// 	if(this->isRunning())
// 	{
// 		wait(1000);
// 	}

// 	qDebug()<<"start sleep m_bStop:"<<m_bStop;

	while(QThread::isRunning())
	{
		msleep(10);
	}
	m_pVedioDecoder->Release();
	m_pVedioDecoder = NULL;
	m_pVedioRender->Release();
	m_pVedioRender = NULL;

// 	qDebug()<<"end sleep";
}

int PlayMgr::initCb()
{
	IEventRegister *pEventRegister = NULL;
	m_pVedioDecoder->QueryInterface(IID_IEventRegister, (void**)&pEventRegister);
	if (NULL == pEventRegister)
	{
		return 1;
	}
	QString eventName = "DecodedFrame";
	pEventRegister->registerEvent(eventName, cbDecodedFrame, this);

// 	qDebug()<<"render:"<<m_pVedioRender;
// 	qDebug()<<"init wnd;"<<m_pRenderWnd;
// 
	m_pVedioRender->setRenderWnd(m_pRenderWnd);
	pEventRegister->Release();
	return 0;
}

void PlayMgr::setCbTimeChange(pcbTimeChange pro, void* pUser)
{
	if (NULL != pro && NULL != pUser)
	{
		m_pcbTimeChg = pro;
		m_pUser = pUser;
	}
}

void PlayMgr::setCbThreowExcepion( pcbThreowException pro, void* pUser )
{
	if (pro && pUser)
	{
		if (m_pUser != pUser)
		{
			m_pUser = pUser;
		}
		m_pcbThrowExp = pro;
	}
}

void PlayMgr::setFileInfo(QMap<QString, PeriodTime> fileInfoMap)
{
	if (!fileInfoMap.isEmpty())
	{
		m_filePeriodMap = fileInfoMap;
	}
}
void PlayMgr::setParamter(QStringList &fileList, QWidget* wnd, QDateTime &start, QDateTime &end, int &startPos, QVector<PeriodTime> &skipTime)
{
	if (fileList.isEmpty() || NULL == wnd)
	{
		return;
	}

	m_lstfileList = fileList;
	m_pRenderWnd = wnd;
	m_startTime = start;
	m_endTime = end;
	m_nStartPos = startPos;
	m_skipTime = skipTime;

	initCb();
}

void PlayMgr::run()
{
	QString filePath;
// 	QString fileName;
// 	QString fileDate;
// 	QRegExp rx;
// 	QDateTime fileStartTime;
	QDateTime currentPlayTime = m_startTime;
//	QDateTime tempTime;
// 	QDate date;
// 	QTime time;
//	bool isPlayInMid = false;
	bool isFirstKeyFrame = false;
	int timeOffset = 0;
	int skipPos = 0;
	unsigned int skipTime = 0;
	unsigned int start = 0;
	bool bSkip = false;

	m_bPlaying = true;
	for (int i = m_nStartPos; i < m_lstfileList.size() && !m_bStop && currentPlayTime < m_endTime; i++)
	{
		//open file
		filePath = m_lstfileList[i];
// 		rx = QRegExp("([0-9]{4}-[0-9]{2}-[0-9]{2})");
// 		if (-1 != rx.indexIn(filePath,0))
// 		{
// 			fileDate = rx.cap(1);
// 		}
// 
// 		rx = QRegExp("([0-9]{6}).avi");
// 		if (-1 != rx.indexIn(filePath, 0))
// 		{
// 			fileName = rx.cap(1);
// 		}
// 
// 		//get start time from file path
// 		date = QDate::fromString(fileDate, "yyyy-MM-dd");
// 		time = QTime::fromString(fileName, "hhmmss");
// 		fileStartTime.setDate(date);
// 		fileStartTime.setTime(time);

		PeriodTime per = m_filePeriodMap.value(filePath);
		start = per.start;

		qDebug()<<filePath;

		while (skipPos < m_skipTime.size() || m_skipTime.isEmpty())
		{
			timeOffset = currentPlayTime.toTime_t() - start;
			qDebug()<<"timeoff :"<<timeOffset;
			if (timeOffset >= 0)
			{
				break;
			}
			else
			{
				int waitSec = 0;
				if (m_skipTime[skipPos].start > start)
				{
					waitSec = start - currentPlayTime.toTime_t();
				}
				else
				{
					waitSec = m_skipTime[skipPos].start - currentPlayTime.toTime_t();
				}
				qDebug()<<"wait:"<<waitSec;
				if (waitSec > 0)
				{
					m_mutex.lock();
					m_waitForPlay.wait(&m_mutex, waitSec*1000);
					m_mutex.unlock();
					currentPlayTime = currentPlayTime.addSecs(waitSec);
				}
				else
				{
					if (!m_bIsSkiped)
					{
						bSkip = true;
						m_bIsSkiped = true;
					}
					skipTime = m_skipTime[skipPos].end - m_skipTime[skipPos].start;
					if (NULL != m_pcbTimeChg && NULL != m_pUser && bSkip)
					{
						qDebug()<<"skip:"<<skipTime;
						m_pcbTimeChg(QString("skipTime"), skipTime, m_pUser);
					}
					currentPlayTime = currentPlayTime.addSecs(skipTime);
					skipPos++;
				}
			}
		}


		avi_t *file = AVI_open_input_file(filePath.toLatin1().data(),1);
		if (NULL != file)
		{
			qDebug()<<(int)this<<" open "<<filePath;

// 			int frameRate = AVI_frame_rate(file);
			int totalFrames = AVI_video_frames(file);
			int frameRate = totalFrames/(per.end - per.start);
			if (!frameRate)//gave a tip when file has a exception
			{
				if (m_pcbThrowExp)
				{
					QVariantMap item;
					item.insert("filePath", filePath);
					item.insert("expCode", 1);
					item.insert("pWnd", (uint)this);

					m_pcbThrowExp(QString("ThrowException"), item, m_pUser);
				}
				continue;
			}

//			int isKeyFrame = 0;
			long length = 0;
			QElapsedTimer frameTimer;
			int lastTime = 0;
			bool bIsPlayTimeChg = false;
			qint64 spend = 0;

			char vedioBuff[1280*720];
			memset(vedioBuff, 0 , sizeof(vedioBuff));

			char audioBuff[1024*4];
			memset(audioBuff, 0, sizeof(audioBuff));

			//audio info
			int nAudioChl = AVI_audio_channels(file);
			int nSampleRate = AVI_audio_rate(file);
			int nSampleWidth = AVI_audio_bits(file);
//			long bytes = 0;

			//find start point
			if (AVI_seek_start(file))
			{
				AVI_close(file);
				continue;
			}
			//start frame
			int startframe = (per.end > per.start) ? (qAbs(timeOffset)*totalFrames/(per.end - per.start)) : 0;
			if (startframe > 0)
			{
				AVI_seek_pos(file, startframe);
			}
			int nRet = AVI_read_data(file, vedioBuff, sizeof(vedioBuff), audioBuff, sizeof(audioBuff), &length);
			while(0 != nRet && !m_bStop && currentPlayTime < m_endTime)
			{
				int frame = file->video_pos;
				if(m_bPause)
				{
					g_mtxPause.lock();
					g_waitConPause.wait(&g_mtxPause);
					g_mtxPause.unlock();
				}
				if (2 == nRet)//play audio
				{
					if (NULL != m_pAudioPlayer && m_bIsAudioOpen)
					{
						if (m_nAudioChl != nAudioChl || m_nSampleRate != nSampleRate || m_nSampleWidth != nSampleWidth)
						{
							m_nAudioChl = nAudioChl;
							m_nSampleRate = nSampleRate;
							m_nSampleWidth = nSampleWidth;
							m_pAudioPlayer->SetAudioParam(m_nAudioChl, m_nSampleRate, m_nSampleWidth);
						}
						m_pAudioPlayer->Play(audioBuff, (int)length);
					}
				}
				else if (1 == nRet)//play video
				{
					if (!isFirstKeyFrame)
					{
						m_pVedioDecoder->decode(vedioBuff, length);
						frameTimer.start();
						nRet = AVI_read_data(file, vedioBuff, sizeof(vedioBuff), audioBuff, sizeof(audioBuff), &length);
						isFirstKeyFrame = true;
						continue;
					}
// 					int waitmilliSeconds = 0;
// 					waitmilliSeconds = 1000000/frameRate + m_nSpeedRate*10*1000;
// 					qint64 before = frameTimer.nsecsElapsed()/1000;
// 					if (waitmilliSeconds - spend > 0)
// 					{
// 						usleep(waitmilliSeconds - spend);
// 					}
// 					spend = frameTimer.nsecsElapsed()/1000 - before - waitmilliSeconds;

					qint64 waitmilliSeconds = (qint64)1000000*(per.end - per.start)/totalFrames + m_nSpeedRate*10*1000 - frameTimer.nsecsElapsed()/1000;
					qint64 before = frameTimer.nsecsElapsed()/1000;
					qint64 sec = 0;
					if (waitmilliSeconds > 0)
					{
						sec = waitmilliSeconds - frameTimer.nsecsElapsed()/1000 + before - spend;
						usleep(sec > 0 ? sec : 0);
					}
					spend = frameTimer.nsecsElapsed()/1000 - before - sec;

					m_pVedioDecoder->decode(vedioBuff, length);
					frameTimer.start();

					if (!m_bIsPickThread)
					{
						m_bIsPickThread = true;
						bIsPlayTimeChg = true;
					}
					if (frame/frameRate - lastTime > 0)
					{
						lastTime = frame/frameRate;
						if (bIsPlayTimeChg)
						{
							++m_playingTime;
						}
						currentPlayTime = currentPlayTime.addSecs(1);
					}

					if (NULL != m_pcbTimeChg && !m_bStop)
					{
						m_pcbTimeChg(QString("playingTime"), m_playingTime, m_pUser);
					}
				}
				nRet = AVI_read_data(file, vedioBuff, sizeof(vedioBuff), audioBuff, sizeof(audioBuff), &length);		
			}
			if (currentPlayTime.toTime_t() < per.end && i < m_lstfileList.size() - 1)
			{
				QString nextFile = m_lstfileList[i + 1];
				currentPlayTime = QDateTime::fromTime_t(m_filePeriodMap.value(nextFile).start);
			}

			qDebug()<<(int)this<<"close "<<filePath;

			AVI_close(file);
			if (bIsPlayTimeChg)
			{
				m_bIsPickThread = false;
			}
		}
		else
		{
			continue;
		}
	}
	m_nSpeedRate = 0;
	m_bStop = false;
	m_bIsChange = false;

	if (bSkip)
	{
		m_bIsSkiped = false;
	}
}

void PlayMgr::setPlaySpeed(int speedRate)
{
	m_nSpeedRate = speedRate;
}

void PlayMgr::pause(bool isPause)
{
	m_bPause = isPause;
}

void PlayMgr::stop()
{
	m_bPlaying = false;
	m_bStop = true;
	m_waitForPlay.wakeOne();

	m_nInitWidth = 0;
	m_nInitHeight = 0;
	m_nStartPos = 0;
	m_nSpeedRate = 0;
	m_pRenderWnd = NULL;
	m_bPause = false;

	m_playingTime = 0;
	m_bIsChange = false;
	m_who = NULL;
	m_bIsPickThread = false;
	m_bIsSkiped = false;
	m_filePeriodMap.clear();
}

void PlayMgr::OpneAudio(bool bEnabled)
{
	m_bIsAudioOpen = bEnabled;
}
int PlayMgr::setVolume(unsigned int &uiPersent)
{
	if (NULL == m_pAudioPlayer || uiPersent < 0)
	{
		return 1;
	}
	return m_pAudioPlayer->SetVolume(uiPersent);
}
void PlayMgr::AudioSwitch(bool bOpen)
{
	if (bOpen)
	{
		pcomCreateInstance(CLSID_AudioPlayer,NULL,IID_IAudioPlayer,(void **)&m_pAudioPlayer);
		if (NULL != m_pAudioPlayer)
		{
			m_pAudioPlayer->EnablePlay(true);
		}
	}
	else
	{
		if (NULL != m_pAudioPlayer)
		{
			m_pAudioPlayer->Stop();
			m_pAudioPlayer->Release();
			m_pAudioPlayer = NULL;
			m_nAudioChl = 1;
			m_nSpeedRate = 0;
			m_nSampleWidth = 0;
		}
	}
}

int cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser)
{
	if ("DecodedFrame" == evName)
	{
		((PlayMgr*)pUser)->prePlay(evMap);
		return 0;
	}
	else
	{
		return 1;
	}
}

int PlayMgr::prePlay(QVariantMap item)
{
	if (NULL == m_pVedioRender || !m_bPlaying)
	{
		return 1;
	}

	char* pData=(char*)item.value("data").toUInt();
	char* pYdata=(char*)item.value("Ydata").toUInt();
	char* pUdata=(char*)item.value("Udata").toUInt();
	char* pVdata=(char*)item.value("Vdata").toUInt();
	int iWidth=item.value("width").toInt();
	int iHeight=item.value("height").toInt();
	int iYStride=item.value("YStride").toInt();
	int iUVStride=item.value("UVStride").toInt();
	int iLineStride=item.value("lineStride").toInt();
	QString iPixeFormat=item.value("pixelFormat").toString();
	int iFlags=item.value("flags").toInt();

	if (m_nInitHeight != iHeight || m_nInitWidth != iWidth)
	{
		m_pVedioRender->init(iWidth,iHeight);
		m_nInitHeight = iHeight;
		m_nInitWidth = iWidth;
	}

	m_pVedioRender->render(pData,pYdata,pUdata,pVdata,iWidth,iHeight,iYStride,iUVStride,iLineStride,iPixeFormat,iFlags);

	return 0;
}
