#include "PlayManager.h"
#include <guid.h>
#include "IEventRegister.h"
#include <QElapsedTimer>
#include "GlobalSettings.h"

#include <QDebug>

PlayManager::PlayManager(void):
m_nInitWidth(0),
m_nInitHeight(0),
m_ui64TSP(0),
m_uiStartFrameTime(0),
m_uiCurrentFrameTime(0),
m_pRenderWnd(NULL),
m_speed(SpeedNomal),
m_nSpeedRate(1),
m_bPause(false),
m_bFirstFrame(true),
m_bStop(false)
{
	//申请解码器接口
	pcomCreateInstance(CLSID_h264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pVedioDecoder);
	//申请渲染器接口
	pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_pVedioRender);

}


PlayManager::~PlayManager(void)
{
	m_bStop = true;
	if(this->isRunning())
	{
		wait();
	}

	m_pVedioDecoder->Release();
	m_pVedioDecoder = NULL;
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
	}
	else if (2 == types)
	{
		m_speed = SpeedSlow;
	}
	else
	{
		m_speed = SpeedNomal;
	}

	m_nSpeedRate = speedRate;
}

int PlayManager::getPlayTime()
{
	return m_uiCurrentFrameTime - m_uiStartFrameTime;
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
	m_uiStartFrameTime = 0;
	m_uiCurrentFrameTime = 0;
	m_pRenderWnd = NULL;
	m_speed = SpeedNomal;
	m_bPause = false;
	m_bFirstFrame = true;

}

void PlayManager::run()
{
	RecordAudioStream recAuStream;
	RecordVedioStream recVeStream;
	QElapsedTimer frameTimer;
	unsigned int nLength = 0;
	char * lpdata = NULL;

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

		RecordVedioStream recVeStream;
		recVeStream.sData.clear();
		if (1 == m_pBufferManager->readVedioStream(recVeStream))
		{
			continue;
		}
		if (recVeStream.sData.isEmpty())
		{
			continue;
		}

		nLength = recVeStream.uiLength;
		lpdata = recVeStream.sData.data();

		if (m_bFirstFrame)
		{
			m_uiStartFrameTime = recVeStream.uiGenTime;
			m_ui64TSP = recVeStream.ui64TSP;

			if (NULL == m_pVedioDecoder)
			{
				return;
			}
			m_pVedioDecoder->decode(lpdata, nLength);//解码播放
			m_bFirstFrame = false;

			frameTimer.start();
			continue;
		}

		m_uiCurrentFrameTime = recVeStream.uiGenTime;
		int waitSeconds = 0;
		if (SpeedNomal == m_speed)
		{
			waitSeconds = recVeStream.ui64TSP - m_ui64TSP - frameTimer.nsecsElapsed()/1000;
			if (waitSeconds > 0)
			{
				usleep(waitSeconds);
			}
		}
		else if (SpeedSlow == m_speed)
		{
			int offsets = 1000000/recVeStream.uiFrameRate;
			waitSeconds = recVeStream.ui64TSP - m_ui64TSP - frameTimer.nsecsElapsed()/1000 + m_nSpeedRate*offsets;
			if (waitSeconds > 0)
			{
				usleep(waitSeconds);
			}
		}
		else
		{
			//fast play
		}

		m_ui64TSP = recVeStream.ui64TSP;

		//解码播放
		if (NULL == m_pVedioDecoder)
		{
			return;
		}
		m_pVedioDecoder->decode(lpdata, nLength);
		frameTimer.start();
	}

	m_bStop = false;
}

int PlayManager::prePlay(QVariantMap item)
{
	if (NULL == m_pVedioRender)
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