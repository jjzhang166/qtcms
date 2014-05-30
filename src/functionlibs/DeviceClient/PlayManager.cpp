#include "PlayManager.h"
#include <guid.h>
#include "IEventRegister.h"
#include <QElapsedTimer>
#include "GlobalSettings.h"

#include <QDebug>


IAudioPlayer* PlayManager::m_pAudioPlayer = NULL;
PlayManager* PlayManager::m_pCurView = NULL;
int PlayManager::m_nSampleRate = 0;
int PlayManager::m_nSampleWidth = 0;


PlayManager::PlayManager(void):
m_nInitWidth(0),
m_nInitHeight(0),
m_ui64TSP(0),
m_uiCurrentFrameTime(0),
m_pRenderWnd(NULL),
m_speed(SpeedNomal),
m_nSpeedRate(0),
m_bPause(false),
m_bFirstFrame(true),
m_bRendFinished(false),
m_bStop(false)
{
	//申请解码器接口
	pcomCreateInstance(CLSID_HiH264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pVedioDecoder);
//	pcomCreateInstance(CLSID_h264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pVedioDecoder);
	//申请渲染器接口
	pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_pVedioRender);

}


PlayManager::~PlayManager(void)
{
	m_bStop = true;
	if(this->isRunning())
	{
		wait(100);
	}

	m_pVedioDecoder->Release();
	m_pVedioDecoder = NULL;
	if (!m_bRendFinished)
	{
		msleep(10);
	}
	m_pVedioRender->Release();
	m_pVedioRender = NULL;
}

int PlayManager::initCb()
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

void PlayManager::setParamter(BufferManager *pBufferManager, QWidget* wnd)
{
	m_pBufferManager = pBufferManager;
	m_pRenderWnd = wnd;

	m_bStop = false;

	initCb();
}

void PlayManager::setPlaySpeed(int types, int speedRate)
{
	if (1 == types)
	{
		m_speed = SpeedFast;
		m_nSpeedRate = 0;
	}
	else if (2 == types)
	{
		m_speed = SpeedSlow;
		m_nSpeedRate = speedRate;
	}
	else
	{
		m_speed = SpeedNomal;
		m_nSpeedRate = 0;
	}
}

int PlayManager::getPlayTime()
{
	return m_uiCurrentFrameTime;
}

void PlayManager::pause(bool isPause)
{
	m_bPause = isPause;
}

void PlayManager::stop()
{
	m_bStop = true;

	m_nInitWidth = 0;
	m_nInitHeight = 0;
	m_ui64TSP = 0;
	m_uiCurrentFrameTime = 0;
	m_pRenderWnd = NULL;
	m_speed = SpeedNomal;
	m_nSpeedRate = 0;
	m_bPause = false;
	m_bFirstFrame = true;

}

void PlayManager::run()
{
	QElapsedTimer frameTimer;
	unsigned int nLength = 0;
	char * lpdata = NULL;
	qint64 spend = 0;

	while(!m_bStop)
	{
		if (m_bPause)
		{
// 			g_mutex.lock();
// 			g_pause.wait(&g_mutex);
// 			g_mutex.unlock();
			usleep(40);
			continue;
		}

		RecordStreamFrame recStream;
		memset(&recStream, 0, sizeof(RecordStreamFrame));

		if (1 == m_pBufferManager->readStream(recStream))
		{
			m_pBufferManager->removeItem(&recStream);
			continue;
		}

		if (0 == recStream.cFrameType && NULL != m_pAudioPlayer && m_pCurView == this)
		{
			int nSampleWidth = recStream.uiAudioDataWidth;
			int nSampleRate = recStream.uiAudioSampleRate;
			if (m_nSampleRate != nSampleRate || m_nSampleWidth != nSampleWidth)
			{
				m_nSampleWidth = nSampleWidth;
				m_nSampleRate = nSampleRate;
				m_pAudioPlayer->SetAudioParam(1, m_nSampleRate, m_nSampleWidth);
			}
			m_pAudioPlayer->Play(recStream.pData, recStream.uiLength);
			m_pBufferManager->removeItem(&recStream);
			continue;
		}
		if (NULL == recStream.pData)
		{
			continue;
		}

		nLength = recStream.uiLength;
		lpdata = recStream.pData;

		if (m_bFirstFrame)
		{
			m_ui64TSP = recStream.ui64TSP;

			if (NULL == m_pVedioDecoder)
			{
				return;
			}
			m_pVedioDecoder->decode(lpdata, nLength);//解码播放
			m_pBufferManager->removeItem(&recStream);

// 			delete lpdata;
// 			lpdata = NULL;

			m_bFirstFrame = false;

			frameTimer.start();
			continue;
		}

		m_uiCurrentFrameTime = recStream.uiGenTime;

		int offsets = 1000000/recStream.uiFrameRate;
		qint64 waitSeconds = recStream.ui64TSP - m_ui64TSP - frameTimer.nsecsElapsed()/1000 + m_nSpeedRate*offsets;
		qint64 before = frameTimer.nsecsElapsed()/1000;
		if (SpeedFast != m_speed && waitSeconds > 0)
		{
			qint64 sec = waitSeconds - frameTimer.nsecsElapsed()/1000 + before - spend;
			usleep( sec >= 0 ? sec : 0);
		}

		m_ui64TSP = recStream.ui64TSP;

		//解码播放
		if (NULL == m_pVedioDecoder)
		{
			return;
		}

		spend = frameTimer.nsecsElapsed()/1000 - before - waitSeconds;

		m_pVedioDecoder->decode(lpdata, nLength);
		frameTimer.start();
		m_pBufferManager->removeItem(&recStream);


	}

	m_bStop = false;
}

int PlayManager::prePlay(QVariantMap item)
{
	if (NULL == m_pVedioRender || m_bStop)
	{
		return 1;
	}

	m_bRendFinished = false;
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
	
	m_bRendFinished = true;

	return 0;
}

int cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser)
{
	if ("DecodedFrame" == evName)
	{
		((PlayManager*)pUser)->prePlay(evMap);
		return 0;
	}
	else
		return 1;
}

void PlayManager::AudioSwitch(bool enabled)
{
	if (enabled)
	{
		pcomCreateInstance(CLSID_AudioPlayer,NULL,IID_IAudioPlayer,(void **)&m_pAudioPlayer);
		if (NULL != m_pAudioPlayer)
		{
			m_pAudioPlayer->EnablePlay(true);
			m_nSampleRate = 0;
			m_nSampleWidth = 0;
// 			m_pCurView = this;
		}
	}
	else
	{
		if (NULL != m_pAudioPlayer)
		{
			m_pAudioPlayer->Stop();
			m_pAudioPlayer->Release();
			m_pAudioPlayer = NULL;
			m_pCurView = NULL;
			m_nSampleRate = 0;
			m_nSampleWidth = 0;
		}
	}
}

int PlayManager::setVolume(unsigned int &uiPersent)
{
	if (NULL == m_pAudioPlayer || uiPersent < 0)
	{
		return 1;
	}

	return m_pAudioPlayer->SetVolume(uiPersent);
}

void PlayManager::setCurAudioWnd(PlayManager* curWnd)
{
	if (NULL != curWnd)
	{
		m_pCurView = curWnd;
	}
}
