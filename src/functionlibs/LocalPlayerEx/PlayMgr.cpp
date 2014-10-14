#include "PlayMgr.h"
#include <guid.h>
#include <QElapsedTimer>
#include "IEventRegister.h"

#include "vld.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>

#define qDebug() qDebug()<<(int)this<<__FUNCTION__<<__LINE__

int _cdecl cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser);

bool PlayMgr::m_bIsSkiped = false;
bool PlayMgr::m_bPause = false;
QMutex PlayMgr::m_mxPause;
QWaitCondition PlayMgr::m_wcPause;
IAudioPlayer* PlayMgr::m_pAudioPlayer = NULL;

PlayMgr::PlayMgr( void )
	:m_uiStartSec(0),
	m_uiEndSec(0),
	m_uiCurrentGMT(0),
	m_i32SpeedRate(0),
	m_i32Width(0),
	m_i32Height(0),
	m_i32AudioChl(1),
	m_i32SmapleRate(0),
	m_i32SmapleWidth(0),
	m_i32FileStartPos(0),
	m_i32SkipStartPos(0),
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
	if (m_pVedioRender)
	{
		m_pVedioRender->deinit();
		m_pVedioRender->Release();
		m_pVedioRender = NULL;
	}
	if (m_pVedioDecoder)
	{
		m_pVedioDecoder->deinit();
		m_pVedioDecoder->Release();
		m_pVedioDecoder = NULL;
	}
	//clear buffer
// 	clearBuffer();
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
	qDebug()<<"pWnd: "<<(int)pWnd;

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

void PlayMgr::printVector(uint types, const QVector<PeriodTime> &vec)
{
	for (qint32 i = 0; i < vec.size(); ++i)
	{
		QString str = QDateTime::fromTime_t(vec[i].start).toString("hh:mm:ss");
		QString typ = "------";
		if (types)
		{
			typ = "<------------>";
		}
	 	qDebug()<<str<<typ<<QDateTime::fromTime_t(vec[i].end).toString("hh:mm:ss");
	}
}

void PlayMgr::setSkipTime( const QVector<PeriodTime> &skipTime )
{
	if (!skipTime.isEmpty())
	{
		m_skipTime = skipTime;
		m_i32SkipStartPos = findStartPos(skipTime);
	}
}

void PlayMgr::setFilePeriod( const QVector<PeriodTime> &filePeriod )
{
	if (!filePeriod.isEmpty())
	{
		m_filePeriod = filePeriod;
		m_i32FileStartPos = findStartPos(filePeriod);
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
	qDebug()<<"call stop start";
// 	m_bStop = true;
	m_wcWait.wakeAll();//wake all if thread is sleep
	if (m_bPause)
	{
		m_bPause = false;
		m_wcPause.wakeAll();
	}
	if (isRunning())
	{
		m_bStop = true;
		wait();
	}

	m_uiCurrentGMT = 0;
	m_i32Width = 0;
	m_i32Height = 0;
	m_bIsSkiped = false;

	m_skipTime.clear();
	m_filePeriod.clear();

	if (m_pVedioDecoder)
	{
		m_pVedioDecoder->deinit();
	}
	if (m_pVedioRender)
	{
		m_pVedioRender->deinit();
	}
	qDebug()<<"call stop end";
// 	wait();
}

void PlayMgr::run()
{
	qDebug()<<"----------start run-----------\t";
// 	qDebug()<<"fileStartPos: "<<m_i32FileStartPos;
// 	qDebug()<<"skipStartPos: "<<m_i32SkipStartPos;
// 	printVector(0, m_filePeriod);


	QElapsedTimer frameTimer;

	bool bFirstFrame = true;
	bool bSkip = false;
	uint uiLastGMT = 0;
	uint uiLastPts = 0;
	uint start = 0;
	qint64 i64Spend = 0;
	m_uiCurrentGMT = m_uiStartSec;
	PeriodTime per = m_filePeriod.value(m_i32FileStartPos);

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
			qDebug()<<"no frame, sleep";

			msleep(10);//wait for new frames
			continue;
		}

		//wait when no frames in this period time
		if (pFrameData->uiGentime > per.end)
		{
			per = m_filePeriod.value(++m_i32FileStartPos);
			uiLastPts = pFrameData->uiPts;
			uiLastGMT = pFrameData->uiGentime;
		}
		start = per.start;
		adjustTimeLine(start);

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

// 					m_pAudioPlayer->SetAudioParam(m_i32AudioChl, m_i32SmapleRate,m_i32SmapleWidth);
					m_pAudioPlayer->SetAudioParam(1, m_i32SmapleRate,m_i32SmapleWidth);
				}
				m_pAudioPlayer->Play(pFrameData->pBuffer, pFrameData->uiLength);

// 				delete[] pFrameData->pBuffer;
				m_quFrameBuffer.removeFirst();
				continue;
			}
			else
			{
// 				delete[] pFrameData->pBuffer;
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
			uiLastGMT = pFrameData->uiGentime;
			m_uiCurrentGMT = pFrameData->uiGentime;
			m_pVedioDecoder->decode(pFrameData->pBuffer, pFrameData->uiLength);//decode
// 			delete[] pFrameData->pBuffer;
			m_quFrameBuffer.removeFirst();

			bFirstFrame = false;
			frameTimer.start();
			continue;
		}
		m_uiCurrentGMT = pFrameData->uiGentime;
		if (!m_bIsSkiped)
		{
			bSkip = true;
			m_bIsSkiped = true;
		}
		qint32 diff = m_uiCurrentGMT - uiLastGMT;
		if (!m_bStop && diff > 0 && bSkip && m_pcbTimeChg && m_pUser)
		{
			m_pcbTimeChg(QString("playingTime"), m_uiCurrentGMT, m_pUser);
			uiLastGMT = m_uiCurrentGMT;
		}
		
		//keep play speed
		qint64 i64WaitSec = ((qint64)pFrameData->uiPts - (qint64)uiLastPts)*1000 - frameTimer.nsecsElapsed()/1000 + m_i32SpeedRate*100*1000;
		qint64 i64Before = frameTimer.nsecsElapsed()/1000;
		qint64 i64Sec = i64WaitSec - i64Spend;
// 		if (i64WaitSec > 0)
		if (i64Sec > 0)
		{
// 			i64Sec = i64WaitSec - frameTimer.nsecsElapsed()/1000 + i64Before - i64Spend;

// 			qDebug()<<"wait sec: "<<i64Sec<<" m_bStop: "<<m_bStop<<" cur_pts: "<<pFrameData->uiPts<<" lst_pts: "<<uiLastPts<<" diff: "<<pFrameData->uiPts - uiLastPts;

// 			usleep(i64Sec > 0 ? i64Sec : 0);
			usleep(i64Sec);
		}

		uiLastPts = pFrameData->uiPts;
		i64Spend = frameTimer.nsecsElapsed()/1000 - i64Before - i64Sec;

		m_pVedioDecoder->decode(pFrameData->pBuffer, pFrameData->uiLength);
		frameTimer.start();	

// 		delete[] pFrameData->pBuffer;
		m_quFrameBuffer.removeFirst();
	}

	if (!m_quFrameBuffer.isEmpty())
	{
		qDebug()<<"clear buffer";

// 		clearBuffer();
		m_quFrameBuffer.clear();
	}
	if (bSkip)
	{
		m_bIsSkiped = false;
	}

	qDebug()<<"-----------stop run----------\t";
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

// void PlayMgr::clearBuffer()
// {
// 	//clear buffer
// 	FrameData *pFrameData = NULL;
// 	while(!m_quFrameBuffer.isEmpty())
// 	{
// 		pFrameData = &(m_quFrameBuffer.first());
// // 		delete[] pFrameData->pBuffer;
// 		m_quFrameBuffer.pop_front();
// 	}
// }

void PlayMgr::openAudio( bool bEnable )
{
	qDebug()<<bEnable;

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

qint32 PlayMgr::findStartPos( const QVector<PeriodTime> &vecPeriod )
{
	qint32 startPos = 0;
	QVector<PeriodTime>::const_iterator it = vecPeriod.constBegin();
	while (it != vecPeriod.constEnd())
	{
		if (it->end > m_uiStartSec)
		{
			startPos = it - vecPeriod.constBegin();
			break;
		}
		++it;
	}
	return startPos;
}

qint32 PlayMgr::adjustTimeLine( uint uiStart )
{
	/* comments means:
	** |_____| skip time
	** |XXXXX| wait time
	** |FFFFF| file time
	*/

	if (!m_skipTime.isEmpty() && (m_i32SkipStartPos < 0 || m_i32SkipStartPos >= m_skipTime.size()))
	{
// 		qDebug()<<"skipStartPos: "<<m_i32SkipStartPos<<" is out of range";
		return 1;
	}
	do 
	{
		//skip time table is not empty
		if (!m_skipTime.isEmpty())
		{
			qint32 timeOffset = m_uiCurrentGMT - uiStart;
// 			qDebug()<<"timeoffse: "<<timeOffset;
			if (timeOffset >= 0)// |FFFFF|_____|XXXXX| or |FFFFF|XXXXX|_____|
			{
				break;
			}
			else
			{
				timeOffset = m_uiCurrentGMT - m_skipTime[m_i32SkipStartPos].start;
				if (timeOffset < 0)// |XXXXX|_____|FFFFF| or |XXXXX|FFFFF|_____|
				{
					qint32 waitSec = qMin(m_skipTime[m_i32SkipStartPos].start, uiStart) - m_uiCurrentGMT;
// 					qDebug()<<"waitSec: "<<waitSec;
					m_mxWait.lock();
					m_wcWait.wait(&m_mxWait, waitSec*1000);
					m_mxWait.unlock();
					m_uiCurrentGMT += waitSec;
				}
				else// |_____|XXXXX|FFFFF| or |_____|FFFFF|XXXXX|
				{
					m_uiCurrentGMT += m_skipTime[m_i32SkipStartPos].end - m_uiCurrentGMT;
					m_i32SkipStartPos++;
					if (m_i32SkipStartPos >= m_skipTime.size())
					{
						//skip pos is out of range
						break;
					}
				}
			}
		}
		else//skip time table is empty
		{
			qint32 timeOffset = m_uiCurrentGMT - uiStart;
			if (timeOffset >= 0)//|FFFFF|XXXXX|
			{
				break;
			}
			else //|XXXXX|FFFFF|
			{
				m_mxWait.lock();
				m_wcWait.wait(&m_mxWait, (0 - timeOffset)*1000);
				m_mxWait.unlock();
				m_uiCurrentGMT += 0 - timeOffset;
			}
		}
	} while (!m_bStop);
	return 0;
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