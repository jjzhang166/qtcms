#include "PlayMgr.h"
#include <guid.h>
#include <QElapsedTimer>
#include "IEventRegister.h"

#include <QDebug>
#include <QFile>

int _cdecl cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser);


bool PlayMgr::m_bPause = false;
QMutex PlayMgr::m_mxPause;
QWaitCondition PlayMgr::m_wcPause;
IAudioPlayer* PlayMgr::m_pAudioPlayer = NULL;

PlayMgr::PlayMgr( void )
	:m_uiStartSec(0),
	m_uiEndSec(0),
	m_uiCurrentGMT(0),
	m_uiSkipTime(0),
	m_i32SpeedRate(0),
	m_i32Width(0),
	m_i32Height(0),
	m_i32AudioChl(1),
	m_i32SmapleRate(0),
	m_i32SmapleWidth(0),
	m_bStop(false),
	m_bIsAudioOpen(false),
	m_pcbTimeChg(NULL),
	m_pUser(NULL),
	m_pVedioDecoder(NULL),
	m_pVedioRender(NULL),
	m_pRenderWnd(NULL)
{
	//get decoder interface
	pcomCreateInstance(CLSID_HiH264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pVedioDecoder);
	//get render interface
	pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_pVedioRender);

}

PlayMgr::~PlayMgr( void )
{
	m_bStop = true;
	while (isRunning())
	{
		msleep(10);
	}

	//clear buffer
	clearBuffer();
}

qint32 PlayMgr::initCbFuction()
{
	IEventRegister *pIEvReg = NULL;
	m_pVedioDecoder->QueryInterface(IID_IEventRegister, (void**)&pIEvReg);
	if (!pIEvReg)
	{
		return 1;
	}
	pIEvReg->registerEvent(QString("DecodedFrame"), cbDecodedFrame, this);
	m_pVedioRender->setRenderWnd(m_pRenderWnd);
	pIEvReg->Release();
	return 0;
}

void PlayMgr::setParamter( QWidget* pWnd, uint uiStartSec, uint uiEndSec )
{
	m_pRenderWnd = pWnd;
	m_uiStartSec = uiStartSec;
	m_uiEndSec = uiEndSec;

	if (initCbFuction())
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register callback function error!";
	}
}

void PlayMgr::setSpeedRate( qint32 i32Speed )
{
	m_i32SpeedRate = i32Speed;
}

QList<FrameData>* PlayMgr::getBufferPointer()
{
	return &m_quFrameBuffer;
}

void PlayMgr::setCbTimeChange( pcbTimeChange pro, void* pUser )
{
	if (pro && pUser)
	{
		m_pcbTimeChg = pro;
		m_pUser = pUser;
	}
}

void PlayMgr::pause( bool bIsPause )
{
	m_bPause = bIsPause;
	if (!bIsPause)
	{
		m_wcPause.wakeAll();
	}
}

void PlayMgr::startPlay()
{
	if (!isRunning())
	{
		m_bStop = false;
		start();
	}
}

void PlayMgr::stop()
{
	m_uiCurrentGMT = 0;
	m_i32Width = 0;
	m_i32Height = 0;
	m_bStop = true;
	if (m_bPause)
	{
		m_wcPause.wakeAll();
		m_bPause = false;
	}
	//clear buffer
// 	clearBuffer();
	wait();
}

void PlayMgr::run()
{
	QElapsedTimer frameTimer;

	bool bFirstFrame = true;
	uint uiLastPts = 0;
	qint64 i64Spend = 0;
	m_uiCurrentGMT = m_uiStartSec;
	while (!m_bStop)
	{
		//check is pause
		if (m_bPause)
		{
			m_mxPause.lock();
			m_wcPause.wait(&m_mxPause);
			m_mxPause.unlock();
		}
		FrameData *pFrameData = NULL;
		if (m_quFrameBuffer.size() > 1)//keep one frame in queue
		{
			pFrameData = &(m_quFrameBuffer.first());
		}
		else
		{
			msleep(10);//wait for new frames
			continue;
		}
		
		//wait when no frames in this period time
		qint32 timeOffset = pFrameData->uiGentime - m_uiCurrentGMT - m_uiSkipTime;
		if (timeOffset > 1)
		{
			m_mxWait.lock();
			m_wcWait.wait(&m_mxWait, timeOffset*1000);
			m_mxWait.unlock();
		}
		//decode audio frame
		if (FT_Audio == pFrameData->uiType)
		{
			if (m_bIsAudioOpen && m_pAudioPlayer)
			{
				if (m_i32SmapleRate != pFrameData->AudioConfig.uiSamplerate 
					|| m_i32SmapleWidth != pFrameData->AudioConfig.uiSamplebit 
					|| m_i32AudioChl != pFrameData->AudioConfig.uiChannels)
				{
					m_i32SmapleRate = pFrameData->AudioConfig.uiSamplerate;
					m_i32SmapleWidth = pFrameData->AudioConfig.uiSamplebit;
					m_i32AudioChl = pFrameData->AudioConfig.uiChannels;
					m_pAudioPlayer->SetAudioParam(m_i32AudioChl, m_i32SmapleRate,m_i32SmapleWidth);
				}
				m_pAudioPlayer->Play(pFrameData->pBuffer, pFrameData->uiLength);

				delete[] pFrameData->pBuffer;
				m_quFrameBuffer.removeFirst();
				continue;
			}
			else
			{
				delete[] pFrameData->pBuffer;
				m_quFrameBuffer.removeFirst();
				continue;
			}
		}
		//decode first frame directory 
		if (bFirstFrame)
		{
			if (NULL == m_pVedioDecoder)
			{
				qDebug()<<__FUNCTION__<<__LINE__<<"no decoder interface";
				return;
			}
			uiLastPts = pFrameData->uiPts;
			m_uiCurrentGMT = pFrameData->uiGentime;
			m_pVedioDecoder->decode(pFrameData->pBuffer, pFrameData->uiLength);//decode
			delete[] pFrameData->pBuffer;
			m_quFrameBuffer.removeFirst();

			bFirstFrame = false;
			frameTimer.start();
			continue;
		}
		m_uiCurrentGMT = pFrameData->uiGentime;
		if (!m_bStop && m_pcbTimeChg && m_pUser)
		{
			m_pcbTimeChg(QString("playingTime"), pFrameData->uiGentime, m_pUser);
		}
		
		//keep play speed
		qint64 i64WaitSec = (pFrameData->uiPts - uiLastPts)*1000 - frameTimer.nsecsElapsed()/1000 + m_i32SpeedRate*10*1000;
		qint64 i64Before = frameTimer.nsecsElapsed()/1000;
		qint64 i64Sec = 0;
		if (i64WaitSec > 0)
		{
			i64Sec = i64WaitSec - frameTimer.nsecsElapsed()/1000 + i64Before - i64Spend;
			usleep(i64Sec > 0 ? i64Sec : 0);
		}

		uiLastPts = pFrameData->uiPts;
		i64Spend = frameTimer.nsecsElapsed()/1000 - i64Before - i64Sec;

		m_pVedioDecoder->decode(pFrameData->pBuffer, pFrameData->uiLength);
		frameTimer.start();	

		delete[] pFrameData->pBuffer;
		m_quFrameBuffer.removeFirst();
	}

	if (!m_quFrameBuffer.isEmpty())
	{
		clearBuffer();
	}
}

qint32 PlayMgr::prePlay(QVariantMap &item)
{
	if (NULL == m_pVedioRender || m_bStop)
	{
		return 1;
	}

	char* pData=(char*)item.value("data").value<quintptr>();
	char* pYdata=(char*)item.value("Ydata").value<quintptr>();
	char* pUdata=(char*)item.value("Udata").value<quintptr>();
	char* pVdata=(char*)item.value("Vdata").value<quintptr>();
	int iWidth=item.value("width").toInt();
	int iHeight=item.value("height").toInt();
	int iYStride=item.value("YStride").toInt();
	int iUVStride=item.value("UVStride").toInt();
	int iLineStride=item.value("lineStride").toInt();
	QString iPixeFormat=item.value("pixelFormat").toString();
	int iFlags=item.value("flags").toInt();

	if (m_i32Height != iHeight || m_i32Width != iWidth)
	{
		m_pVedioRender->init(iWidth,iHeight);
		m_i32Height = iHeight;
		m_i32Width = iWidth;
	}

	m_pVedioRender->render(pData,pYdata,pUdata,pVdata,iWidth,iHeight,iYStride,iUVStride,iLineStride,iPixeFormat,iFlags);
	return 0;
}

void PlayMgr::clearBuffer()
{
	//clear buffer
	FrameData *pFrameData = NULL;
	while(!m_quFrameBuffer.isEmpty())
	{
		pFrameData = &(m_quFrameBuffer.first());
		delete[] pFrameData->pBuffer;
		m_quFrameBuffer.pop_front();
	}
}

void PlayMgr::openAudio( bool bEnable )
{
	m_bIsAudioOpen = bEnable;
	if (!bEnable)
	{
		m_i32AudioChl = 1;
		m_i32SmapleRate = 0;
		m_i32SmapleWidth = 0;
	}
}

qint32 PlayMgr::setVolume( uint uiPersent )
{
	return m_pAudioPlayer ? m_pAudioPlayer->SetVolume(uiPersent) : 1;
}

void PlayMgr::audioSwitch( bool bOpen )
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
		}
	}
}

PlayMgr * PlayMgr::getPlayMgrPointer( QWidget *pwnd )
{
	return (m_pRenderWnd == pwnd) ? this : NULL;
}

void PlayMgr::onSkipTime( uint seconds )
{
	m_uiSkipTime = seconds;
}

int _cdecl cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser)
{
	if ("DecodedFrame" == evName)
	{
		((PlayMgr*)pUser)->prePlay(evMap);
		return 0;
	}
	else
		return 1;
}