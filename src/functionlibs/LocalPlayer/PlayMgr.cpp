#include "PlayMgr.h"
#include <guid.h>
#include "IEventRegister.h"
#include "avilib.h"
#include <QElapsedTimer>
#include <QDebug>

QMutex g_mtxPause;
QWaitCondition g_waitConPause;

uint PlayMgr::m_playingTime = 0;
bool PlayMgr::m_bIsChange = false;
void* PlayMgr::m_who = NULL;
bool PlayMgr::m_bIsPickThread = false;
IAudioPlayer* PlayMgr::m_pAudioPlayer = NULL;

PlayMgr::PlayMgr(void):
	m_pRenderWnd(NULL),
	m_pVedioDecoder(NULL),
	m_pVedioRender(NULL),
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
	if(this->isRunning())
	{
		wait(1000);
	}

	m_pVedioDecoder->Release();
	m_pVedioDecoder = NULL;
	m_pVedioRender->Release();
	m_pVedioRender = NULL;
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
	QString fileName;
	QString fileDate;
	QRegExp rx;
	QDateTime fileStartTime;
	QDateTime currentPlayTime;
	QDateTime tempTime = m_startTime;
	QDate date;
	QTime time;
	bool isPlayInMid = false;
	bool isFirstKeyFrame = false;
	qint64 timeOffset = 0;
	int skipPos = 0;

	m_bPlaying = true;
	for (int i = m_nStartPos; i < m_lstfileList.size() && !m_bStop && currentPlayTime < m_endTime; i++)
	{
		//open file
		filePath = m_lstfileList[i];
		rx = QRegExp("([0-9]{4}-[0-9]{2}-[0-9]{2})");
		if (-1 != rx.indexIn(filePath,0))
		{
			fileDate = rx.cap(1);
		}

		rx = QRegExp("([0-9]{6}).avi");
		if (-1 != rx.indexIn(filePath, 0))
		{
			fileName = rx.cap(1);
		}

		//get start time from file path
		date = QDate::fromString(fileDate, "yyyy-MM-dd");
		time = QTime::fromString(fileName, "hhmmss");
		fileStartTime.setDate(date);
		fileStartTime.setTime(time);

		timeOffset = fileStartTime.toMSecsSinceEpoch() - tempTime.toMSecsSinceEpoch();
		if (!m_bIsChange)
		{
			m_who = (void*)this;
			m_bIsChange = true;
		}
		for (int j = skipPos; j < m_skipTime.size(); ++j)
		{
			if (fileStartTime.toTime_t() >= m_skipTime[j].end)
			{
				timeOffset -= (m_skipTime[j].end - m_skipTime[j].start)*1000;
				if (timeOffset < 0)
				{
					timeOffset = 0;
				}
				if (m_who == (void*)this)
				{
					m_playingTime += m_skipTime[j].end - m_skipTime[j].start;
				}
			}
			else
			{
				skipPos = j;
				break;
			}
		}
		if (timeOffset > 0)
		{
			m_mutex.lock();
			m_waitForPlay.wait(&m_mutex, timeOffset - 100);
			m_mutex.unlock();
		}
		else
		{
			isPlayInMid = true;
		}

		avi_t *file = AVI_open_input_file(filePath.toLatin1().data(),1);
		if (NULL != file)
		{
			int frameRate = AVI_frame_rate(file);

			int isKeyFrame = 0;
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
			long bytes = 0;

			//find start point
			if (AVI_seek_start(file))
			{
				AVI_close(file);
				continue;
			}
			//start frame
			int startframe = 0;
			long startBytes = 0;
			if (isPlayInMid)
			{
				startframe = qAbs(timeOffset)*frameRate/1000;
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
					int waitmilliSeconds = 0;
					waitmilliSeconds = 1000000/frameRate + m_nSpeedRate*10*1000;
					qint64 before = frameTimer.nsecsElapsed()/1000;
					if (waitmilliSeconds - spend > 0)
					{
						usleep(waitmilliSeconds - spend);
					}
					spend = frameTimer.nsecsElapsed()/1000 - before - waitmilliSeconds;

					m_pVedioDecoder->decode(vedioBuff, length);
					frameTimer.start();

					//count current play time;
					currentPlayTime = fileStartTime.addSecs(frame/frameRate);

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
					}

					if (NULL != m_pcbTimeChg)
					{
						m_pcbTimeChg(m_playingTime, m_pUser);
					}
				}
				nRet = AVI_read_data(file, vedioBuff, sizeof(vedioBuff), audioBuff, sizeof(audioBuff), &length);		
			}

			tempTime = currentPlayTime;
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
