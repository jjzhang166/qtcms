#include "PlayManager.h"
#include <guid.h>
#include "IEventRegister.h"
#include <QElapsedTimer>
#include "IVideoRenderDigitalZoom.h"

#include <QDebug>


IAudioPlayer* PlayManager::m_pAudioPlayer = NULL;
PlayManager* PlayManager::m_pCurView = NULL;
int PlayManager::m_nSampleRate = 0;
int PlayManager::m_nSampleWidth = 0;


PlayManager::PlayManager(void):
    m_bPause(false),
    m_bStop(false),
    m_bFirstFrame(true),
    m_bRendFinished(false),
	m_bScreenShot(false),
    m_nInitHeight(0),
    m_nInitWidth(0),
    m_nSpeedRate(0),
    m_speed(SpeedNomal),
    m_ui64TSP(0),
    m_uiCurrentFrameTime(0),
	m_nScreenShotType(2),
	m_nScreenShotChl(0),
    m_pRenderWnd(NULL)
{
	//申请解码器接口
	pcomCreateInstance(CLSID_HiH264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pVedioDecoder);
//	pcomCreateInstance(CLSID_h264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pVedioDecoder);
	//申请渲染器接口
	pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_pVedioRender);
	m_eventNameList<<"screenShot";
}


PlayManager::~PlayManager(void)
{
	m_bStop = true;
	//if(this->isRunning())
	//{
	//	wait(1000);
	//}
	int nCount=0;
	while(this->isRunning()&&nCount<50){
		wait(100);
		nCount++;
	}
	if (nCount>49)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"it may be cuase crash,as the qthread is still running";
	}else{

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
    pEventRegister->registerEvent(QString("DecodedFrame"), cbDecodedFrame, this);

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
	qDebug()<<(int)this<<__FUNCTION__<<__LINE__<<"-----------run start------------";

	QElapsedTimer frameTimer;
	unsigned int nLength = 0;
	char * lpdata = NULL;
	qint64 spend = 0;
	m_bScreenShot=false;
	while(!m_bStop)
	{
		if (m_bPause)
		{
// 			g_mutex.lock();
// 			g_pause.wait(&g_mutex);
// 			g_mutex.unlock();
			//usleep(40);
			msleep(10);
			continue;
		}

		RecordStreamFrame recStream;
		memset(&recStream, 0, sizeof(RecordStreamFrame));

		if (1 == m_pBufferManager->readStream(recStream))
		{
			m_pBufferManager->removeItem(&recStream);
			msleep(10);
			continue;
		}

		if (0 == recStream.cFrameType && NULL != m_pAudioPlayer && m_pCurView == this)
		{
			//when play in fast or slow, close audio
			if (SpeedNomal != m_speed)
			{
				m_pBufferManager->removeItem(&recStream);
				continue;
			}
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

	qDebug()<<(int)this<<__FUNCTION__<<__LINE__<<"---------run end----------";

	m_bStop = false;
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
int PlayManager::prePlay(QVariantMap item)
{
	if (NULL == m_pVedioRender || m_bStop)
	{
		return 1;
	}

	m_bRendFinished = false;
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

	if (m_nInitHeight != iHeight || m_nInitWidth != iWidth)
	{
		m_pVedioRender->init(iWidth,iHeight);
		m_nInitHeight = iHeight;
		m_nInitWidth = iWidth;
	}

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
			unsigned char *rgbBuff = new unsigned char[iWidth*iHeight*3];
			memset(rgbBuff, 0, iWidth*iHeight*3);
			YUV420ToRGB888((unsigned char*)pYdata, (unsigned char*)pUdata, (unsigned char*)pVdata,iWidth, iHeight, rgbBuff);
			QImage img(rgbBuff, iWidth, iHeight, QImage::Format_RGB888);
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
				//do nothing
			}else{
				qDebug()<<__FUNCTION__<<__LINE__<<"save screenShot info to database fail as saveScreenShotInfoToDatabase";
			}
		}else{
			qDebug()<<__FUNCTION__<<__LINE__<<"screenShot fail as get getScreenShotInfo fail";
			//do nothing
		}
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
    if (NULL == m_pAudioPlayer || (int)uiPersent < 0)
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

void PlayManager::addWnd( QWidget* pWnd, QString sName )
{
	IVideoRenderDigitalZoom *pZoomInterface = NULL;
	m_pVedioRender->QueryInterface(IID_IVideoRenderDigitalZoom, (void**)&pZoomInterface);
	if (pZoomInterface){
		pZoomInterface->addExtendWnd(pWnd, sName);
		pZoomInterface->setRenderRectPen(5, 255, 255, 0);
		pZoomInterface->Release();
	}
}

void PlayManager::removeWnd( QString sName )
{
	IVideoRenderDigitalZoom *pZoomInterface = NULL;
	m_pVedioRender->QueryInterface(IID_IVideoRenderDigitalZoom, (void**)&pZoomInterface);
	if (pZoomInterface){
		pZoomInterface->removeExtendWnd(sName);
		pZoomInterface->Release();
	}
}

void PlayManager::setZoomRect( QRect rect, int nWidth, int nHeight )
{
	IVideoRenderDigitalZoom *pZoomInterface = NULL;
	m_pVedioRender->QueryInterface(IID_IVideoRenderDigitalZoom, (void**)&pZoomInterface);
	if (pZoomInterface){
		pZoomInterface->setRenderRect(rect.left(), rect.top(), rect.right(), rect.bottom(), nWidth, nHeight);
		pZoomInterface->Release();
	}
}

void PlayManager::setOriginRect( QRect rect )
{
	IVideoRenderDigitalZoom *pZoomInterface = NULL;
	m_pVedioRender->QueryInterface(IID_IVideoRenderDigitalZoom, (void**)&pZoomInterface);
	if (pZoomInterface){
		pZoomInterface->drawRectToOriginalWnd(rect.left(), rect.top(), rect.right(), rect.bottom());
		pZoomInterface->Release();
	}
}

bool PlayManager::getScreenShotInfo( QString &sFileName,QString &sFileDir,quint64 &uiTime,int &nChl,int &nType )
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

bool PlayManager::saveScreenShotInfoToDatabase( QString sFileName,QString sFileDir ,quint64 uiTime,int nChl,int nType )
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

void PlayManager::eventCallBack( QString eventName,QVariantMap evMap )
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

void PlayManager::registerEvent( QString eventName,int (__cdecl *proc)(QString,QVariantMap,void*),void *pUser )
{
	if (!m_eventNameList.contains(eventName))
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"register event :"<<eventName<<"fail";
		return;
	}else{
		tagProcInfo proInfo;
		proInfo.proc=proc;
		proInfo.puser=pUser;
		m_eventMap.insert(eventName,proInfo);
		return;
	}
}

void PlayManager::screenShot( QString sUser,int nType,int nChl )
{
	if (QThread::isRunning())
	{
		m_nScreenShotChl=nChl;
		m_nScreenShotType=nType;
		m_sScreenUser=sUser;
		m_bScreenShot=true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"screenShot fail as the thread is not running";
	}
}
