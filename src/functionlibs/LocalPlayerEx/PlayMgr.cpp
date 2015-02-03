#include "PlayMgr.h"
#include <guid.h>
#include <QElapsedTimer>
#include "IEventRegister.h"
#include "IVideoRenderDigitalZoom.h"

//#include "vld.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>

#define qDebug() qDebug()<<(int)this<<__FUNCTION__<<__LINE__
#define STANDARD_FRAME_INTERVAL 40*1000

int _cdecl cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser);

bool PlayMgr::m_bIsSkiped = false;
bool PlayMgr::m_bPause = false;
QMutex PlayMgr::m_mxPause;
QWaitCondition PlayMgr::m_wcPause;
IAudioPlayer* PlayMgr::m_pAudioPlayer = NULL;

static bool gs_timerSwitch = false;
static QElapsedTimer gs_globalTimer;

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
	m_i64FrameInterval(0),
	m_nScreenShotChl(0),
	m_nScreenShotType(1),
	m_bStop(false),
	m_bIsAudioOpen(false),
	m_bIsChangeSpeed(false),
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
	m_eventNameList<<"screenShot";
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
	qDebug()<<"-------------------------speed_rate: "<<m_i32SpeedRate;
// 	m_i64FrameInterval = 0;
	m_bIsChangeSpeed = true;
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
	m_i64FrameInterval = 0;
	m_i32SpeedRate = 0;
	m_bIsSkiped = false;
	m_bIsChangeSpeed = false;

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
	if (gs_timerSwitch)
	{
		gs_timerSwitch = false;
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
	if (!gs_timerSwitch)
	{
		gs_globalTimer.start();
		gs_timerSwitch = true;
	}

// 	QElapsedTimer frameTimer;
	qint64 timeStartCount = 0;
	bool bFirstFrame = true;
	bool bSkip = false;
	uint uiLastGMT = 0;
	uint uiLastPts = 0;
	uint start = 0;
// 	qint64 m_i64FrameInterval = 0;
	m_uiCurrentGMT = m_uiStartSec;
	PeriodTime per = m_filePeriod.value(m_i32FileStartPos);

	m_bScreenShot=false;
	while (!m_bStop)
	{
		//check is pause
		if (m_bPause)
		{
			m_mxPause.lock();
			m_wcPause.wait(&m_mxPause);
			m_mxPause.unlock();

// 			frameTimer.restart();
			timeStartCount = gs_globalTimer.elapsed();
		}
		FrameData *pFrameData = NULL;
		if (m_quFrameBuffer.size() > 1)//keep one frame in queue
		{
			pFrameData = &(m_quFrameBuffer.first());
// 			qDebug()<<"wndId: "<<pFrameData->uiChannel<<" buff_size: "<<m_quFrameBuffer.size();
		}
		else
		{
			//qDebug()<<"no frame, sleep";

			msleep(10);//wait for new frames
			continue;
		}
		//clear audio frames when fast or slow play
		if (m_i32SpeedRate){
			if (FT_Audio == pFrameData->uiType){
				delete[] pFrameData->pBuffer;
				pFrameData->pBuffer = NULL;
				m_quFrameBuffer.removeFirst();
				continue;
			}
		}
//		qDebug()<<"GMT: "<<pFrameData->uiGentime<<" type: "<<pFrameData->uiType<<" length: "<<pFrameData->uiLength<<" pts: "<<pFrameData->uiPts<<" buff size:"<<m_quFrameBuffer.size();

		if (m_i32SpeedRate < 0){
			//speed rate = *8, only decode I frame
			if (-8 == m_i32SpeedRate && FT_IFrame != pFrameData->uiType){
				delete[] pFrameData->pBuffer;
				pFrameData->pBuffer = NULL;
				m_quFrameBuffer.removeFirst();
				continue;
			}
		}

		//wait when no frames in this period time
		if (pFrameData->uiGentime > per.end)
		{
			per = m_filePeriod.value(++m_i32FileStartPos);
			uiLastPts = pFrameData->uiPts;
			uiLastGMT = pFrameData->uiGentime;
		}
		uint status = 0;
		start = per.start;
		adjustTimeLine(start, status);
		//if wait seconds when playing, clear timer
		if (status)
		{
// 			frameTimer.restart();
			timeStartCount = gs_globalTimer.elapsed();
		}

		if (FT_Audio == pFrameData->uiType)
		{
			//audio is open && play in normal speed
			if (m_bIsAudioOpen && m_pAudioPlayer && !m_i32SpeedRate)
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
			uiLastGMT = pFrameData->uiGentime;
			m_uiCurrentGMT = pFrameData->uiGentime;
			m_pVedioDecoder->decode(pFrameData->pBuffer, pFrameData->uiLength);//decode
			delete[] pFrameData->pBuffer;
			m_quFrameBuffer.removeFirst();

			bFirstFrame = false;
// 			frameTimer.start();
			timeStartCount = gs_globalTimer.elapsed();
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
		//clear interval time when speed changed
		if (m_bIsChangeSpeed)
		{
			m_i64FrameInterval = 0;
			m_bIsChangeSpeed = false;
		}
		//keep play speed
		qint64 ptsDiff = (qint64)pFrameData->uiPts - (qint64)uiLastPts;
		qint64 i64before = gs_globalTimer.elapsed();
		i64before -= m_i64FrameInterval;

		qint64 curTick = gs_globalTimer.elapsed();
		if (m_i32SpeedRate < 0){
			ptsDiff /= qAbs(m_i32SpeedRate);
		}
		if (m_i32SpeedRate > 0){
			ptsDiff *= m_i32SpeedRate;
		}
		ptsDiff -= gs_globalTimer.elapsed() - timeStartCount;

		if (ptsDiff > 1000){
			ptsDiff = 40;
		}

//   		qDebug()<<"wait sec: "<<ptsDiff<<" i64before: "<<i64before<<" m_i64FrameInterval: "<<m_i64FrameInterval<<" cur_pts: "<<pFrameData->uiPts<<" lst_pts: "<<uiLastPts<<" diff: "<<pFrameData->uiPts - uiLastPts;

		
		while (curTick - i64before < ptsDiff)
		{
			msleep(1);
			curTick = gs_globalTimer.elapsed();
		}
		m_i64FrameInterval = curTick - i64before - ptsDiff;

// 		qint64 i64spend = gs_globalTimer.nsecsElapsed()/1000 - timeStartCount;
// 		qint64 i64WaitSec = ptsDiff - i64spend;
// 		qint64 i64Before = i64spend;
// 		qint64 i64Sec = i64WaitSec - m_i64FrameInterval;
// 		
// 		//qDebug()<<"wait sec: "<<i64Sec<<" i64WaitSec: "<<i64WaitSec<<" m_i64FrameInterval: "<<m_i64FrameInterval<<" cur_pts: "<<pFrameData->uiPts<<" lst_pts: "<<uiLastPts<<" diff: "<<pFrameData->uiPts - uiLastPts;
// 
// 		//if frame interval is greater than 1 second, then set it 40 milliseconds
// 		if (i64Sec > 1000*1000)
// 		{
// 			i64Sec = STANDARD_FRAME_INTERVAL;
// 		}
// // 		if (i64WaitSec > 0)
// 		if (i64Sec > 0)
// 		{
// // 			i64Sec = i64WaitSec - frameTimer.nsecsElapsed()/1000 + i64Before - m_i64FrameInterval;
// 
// // 			qDebug()<<"wait sec: "<<i64Sec<<" m_bStop: "<<m_bStop<<" cur_pts: "<<pFrameData->uiPts<<" lst_pts: "<<uiLastPts<<" diff: "<<pFrameData->uiPts - uiLastPts;
// 
// // 			usleep(i64Sec > 0 ? i64Sec : 0);
// 			usleep(i64Sec);
// 		}
// 		else
// 		{
// 			//if fast play, release cpu
// 			if (m_i32SpeedRate < 0)
// 			{
// 				usleep(1000);
// 			}
// 		}

		uiLastPts = pFrameData->uiPts;
// 		m_i64FrameInterval = gs_globalTimer.nsecsElapsed()/1000 - timeStartCount - i64Before - i64Sec;
		timeStartCount = gs_globalTimer.elapsed();

 		m_pVedioDecoder->decode(pFrameData->pBuffer, pFrameData->uiLength);
// 		frameTimer.start();	

		delete[] pFrameData->pBuffer;
		m_quFrameBuffer.removeFirst();
	}

	if (!m_quFrameBuffer.isEmpty())
	{
		qDebug()<<"clear buffer";

		clearBuffer();
// 		m_quFrameBuffer.clear();
	}
	if (bSkip)
	{
		m_bIsSkiped = false;
	}

	qDebug()<<"-----------stop run----------\t";
}
static void YUV420ToRGB888(unsigned char *py, unsigned char *pu, unsigned char *pv, int width, int height, unsigned char *dst)
{
	int line, col, linewidth;
	int y, u, v, yy, vr, ug, vg, ub;
	int r, g, b;
	unsigned char *pRGB = NULL;

	linewidth = width >> 1;

	y = *py++;
	yy = y << 8;
	u = *pu - 128;
	ug = 88 * u;
	ub = 454 * u;
	v = *pv - 128;
	vg = 183 * v;
	vr = 359 * v;

	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col++) {
			r = (yy + vr) >> 8;
			g = (yy - ug - vg) >> 8;
			b = (yy + ub ) >> 8;

			if (r < 0) r = 0;
			if (r > 255) r = 255;
			if (g < 0) g = 0;
			if (g > 255) g = 255;
			if (b < 0) b = 0;
			if (b > 255) b = 255;

			pRGB = dst + line*width*3 + col*3;
			*pRGB = r;
			*(pRGB + 1) = g;
			*(pRGB + 2) = b;

			y = *py++;
			yy = y << 8;
			if (col & 1) {
				pu++;
				pv++;

				u = *pu - 128;
				ug = 88 * u;
				ub = 454 * u;
				v = *pv - 128;
				vg = 183 * v;
				vr = 359 * v;
			}
		} 
		if ((line & 1) == 0) { 
			pu -= linewidth;
			pv -= linewidth;
		}
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
	//½ØÆÁ
	if (m_bScreenShot)
	{
		QString sFileName;
		QString sFileDir;
		quint64 uiTime;
		int nType;
		int nChl;
		m_bScreenShot=false;
		if (getScreenShotInfo(sFileName,sFileDir,uiTime,nChl,nType))
		{
			unsigned char *rgbBuff=new unsigned char[iWidth*iHeight*3];
			memset(rgbBuff,0,iWidth*iHeight*3);
			YUV420ToRGB888((unsigned char*)pYdata,(unsigned char*)pUdata,(unsigned char*)pVdata,iWidth,iHeight,rgbBuff);
			QImage img(rgbBuff, iWidth,iHeight, QImage::Format_RGB888);
			QString sFilePath=sFileDir+"/"+sFileName;
			img.save(sFilePath, "JPG");
			delete [] rgbBuff;
			if (saveScreenShotInfoToDatabase(sFileName,sFileDir,uiTime,nChl,nType))
			{
				QVariantMap tScreenShotInfo;
				tScreenShotInfo.insert("fileName",sFileName);
				tScreenShotInfo.insert("fileDir",sFileDir);
				tScreenShotInfo.insert("chl",nChl);
				tScreenShotInfo.insert("type",nType);
				tScreenShotInfo.insert("user",m_sScreenUser);
				eventCallBack("screenShot",tScreenShotInfo);
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"save screenShot info to database fail as saveScreenShotInfoToDatabase";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"screenShot fail as get getScreenShotInfo fail";
			//do nothing
		}
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

qint32 PlayMgr::adjustTimeLine( uint uiStart, uint &status )
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
					status = 1;
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
				status = 1;
			}
		}
	} while (!m_bStop);
	return 0;
}

void PlayMgr::enableVideoStretch( bool bEnable )
{
	if (m_pVedioRender)
	{
		m_pVedioRender->enableStretch(bEnable);
	}
}

bool PlayMgr::getVideoStretchStatus()
{
	if (m_pVedioRender)
	{
		return m_pVedioRender->isStretchEnable();
	}
	return false;
}

void PlayMgr::getZoomInterface( void** ppv )
{
	if (m_pVedioRender){
		m_pVedioRender->QueryInterface(IID_IVideoRenderDigitalZoom, ppv);
	}
}

void PlayMgr::addWnd( QWidget* pWnd, QString sName )
{
	IVideoRenderDigitalZoom *pZoomInterface = NULL;
	getZoomInterface((void**)&pZoomInterface);
	if (pZoomInterface){
		pZoomInterface->addExtendWnd(pWnd, sName);
		pZoomInterface->setRenderRectPen(5, 255, 255, 0);
		pZoomInterface->Release();
	}
}

void PlayMgr::removeWnd( QString sName )
{
	IVideoRenderDigitalZoom *pZoomInterface = NULL;
	getZoomInterface((void**)&pZoomInterface);
	if (pZoomInterface){
		pZoomInterface->removeExtendWnd(sName);
		pZoomInterface->Release();
	}
}

void PlayMgr::setZoomRect( QRect rect, int nWidth, int nHeight )
{
	IVideoRenderDigitalZoom *pZoomInterface = NULL;
	getZoomInterface((void**)&pZoomInterface);
	if (pZoomInterface){
		pZoomInterface->setRenderRect(rect.left(), rect.top(), rect.right(), rect.bottom(), nWidth, nHeight);
		pZoomInterface->Release();
	}
}

void PlayMgr::setOriginRect( QRect rect )
{
	IVideoRenderDigitalZoom *pZoomInterface = NULL;
	getZoomInterface((void**)&pZoomInterface);
	if (pZoomInterface){
		pZoomInterface->drawRectToOriginalWnd(rect.left(), rect.top(), rect.right(), rect.bottom());
		pZoomInterface->Release();
	}
}

bool PlayMgr::getScreenShotInfo( QString &sFileName,QString &sFileDir,quint64 &uiTime,int &nChl,int &nType )
{
	IDisksSetting *pDisksSetting=NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		QString sDisk;
		if (0==pDisksSetting->getUseDisks(sDisk))
		{
			QStringList tDiskList=sDisk.split(":");
			if (tDiskList.size()!=0)
			{
				foreach(QString sDiskItem,tDiskList){
					QString sDiskEx=sDiskItem+":/screenShotEx";
					QDir tDir;
					if (tDir.exists(sDiskEx))
					{
						sFileDir=sDiskEx;
						break;
					}else{
						//create dir
						if (tDir.mkdir(sDiskEx))
						{
							sFileDir=sDiskEx;
							break;
						}else{
							//keep going
						}
					}
				}
				if (!sFileDir.isEmpty())
				{
					nType=m_nScreenShotType;
					nChl=m_nScreenShotChl;
					uiTime=QDateTime::currentDateTime().toMSecsSinceEpoch();
					QString sDatetime=QDateTime::currentDateTime().toString("yyyy-MM-dd")+"-"+QDateTime::currentDateTime().toString("hh-mm-ss-zzz");
					sFileName=m_sScreenUser+"-"+QString::number(nChl)+"-"+QString::number(nType)+'-'+sDatetime+".jpg";
					pDisksSetting->Release();
					pDisksSetting=NULL;
					return true;
				}else{
					qDebug()<<__FUNCTION__<<__LINE__<<"getScreenShotInfo fail as sFileDir is not been created";
				}
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"getScreenShotInfo fail as there is not disk for store screen";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"getScreenShotInfo fail as getUseDisks fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"getScreenShotInfo fail as pDisksSetting is null";
	}
	if (NULL!=pDisksSetting)
	{
		pDisksSetting->Release();
		pDisksSetting=NULL;
	}
	return false;
}

bool PlayMgr::saveScreenShotInfoToDatabase( QString sFileName,QString sFileDir ,quint64 uiTime,int nChl,int nType )
{
	IScreenShot *pScreenShot=NULL;
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IScreenShot,(void**)&pScreenShot);
	if (NULL!=pScreenShot)
	{
		if (pScreenShot->addScreenShotItem(sFileName,sFileDir,nChl,nType,uiTime))
		{
			pScreenShot->Release();
			pScreenShot=NULL;
			return true;
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"saveScreenShotInfoToDatabase fail as addScreenShotItem fail";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"saveScreenShotInfoToDatabase fail as pScreenShot is null";
	}
	if (NULL!=pScreenShot)
	{
		pScreenShot->Release();
		pScreenShot=NULL;
	}
	return false;
}

void PlayMgr::screenShot( int nChl,int nType,QString sUser )
{
	if (QThread::isRunning())
	{
		m_bScreenShot=true;
		m_nScreenShotChl=nChl;
		m_nScreenShotType=nType;
		m_sScreenUser=sUser;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"screenShot fail as the thread is not running";
	}
}

void PlayMgr::eventCallBack( QString eventName,QVariantMap evMap )
{
	if (m_eventNameList.contains(eventName))
	{
		tagProcInfo proInfo=m_eventMap.value(eventName);
		if (NULL!=proInfo.proc)
		{
			proInfo.proc(eventName,evMap,proInfo.puser);
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<eventName<<" event is not regist";
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"not support :"<<eventName;
	}
}

void PlayMgr::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void*),void *pUser )
{
	if (!m_eventNameList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register event fail as m_eventNameList do not contains:"<<eventName;
		return;
	}else{
		tagProcInfo proInfo;
		proInfo.proc=proc;
		proInfo.puser=pUser;
		m_eventMap.insert(eventName,proInfo);
		return;
	}
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