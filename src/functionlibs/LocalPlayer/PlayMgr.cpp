#include "PlayMgr.h"
#include <guid.h>
#include "IEventRegister.h"
#include "avilib.h"
#include <QElapsedTimer>
#include <QDebug>

QMutex g_mtxPause;
QWaitCondition g_waitConPause;


PlayMgr::PlayMgr(void):
	m_pRenderWnd(NULL),
	m_pVedioDecoder(NULL),
	m_pVedioRender(NULL),
	m_nInitHeight(0),
	m_nInitWidth(0),
	m_nSpeedRate(0),
	m_nStartPos(0),
	m_bStop(false),
	m_bPause(false)
{
	//ÉêÇë½âÂëÆ÷½Ó¿Ú
	pcomCreateInstance(CLSID_h264Decoder,NULL,IID_IVideoDecoder,(void**)&m_pVedioDecoder);
	//ÉêÇëäÖÈ¾Æ÷½Ó¿Ú
	pcomCreateInstance(CLSID_DDrawRender,NULL,IID_IVideoRender,(void**)&m_pVedioRender);

}


PlayMgr::~PlayMgr(void)
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

void PlayMgr::setParamter(QStringList &fileList, QWidget* wnd, QDateTime &start, QDateTime &end, int &startPos)
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

	initCb();
}

void PlayMgr::run()
{
	QString filePath;
	QString fileName;
	QString fileDate;
	QRegExp rx("([0-9]{4}-[0-9]{2}-[0-9]{2})");
	QDateTime fileStartTime;
	QDateTime currentPlayTime;
	QDate date;
	QTime time;
	bool isPlayInMid = false;
	bool isFirstKeyFrame = false;

	for (int i = m_nStartPos; i < m_lstfileList.size() && !m_bStop && currentPlayTime < m_endTime; i++)
	{
		//open file
		filePath = m_lstfileList[i];		
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

		avi_t *file = AVI_open_input_file(filePath.toLatin1().data(),1);
		if (NULL != file)
		{
			int totalFrames = AVI_video_frames(file);//get total frames
			int frameRate = AVI_frame_rate(file);

			int isKeyFrame = 0;
			long length = 0;
			QElapsedTimer frameTimer;

			char vedioBuff[1280*720];
			memset(vedioBuff, 0 , sizeof(vedioBuff));

			//find start point
			if (AVI_seek_start(file))
			{
				AVI_close(file);
				continue;
			}
			//start frame
			int startframe = frameRate*(m_startTime.toTime_t() - fileStartTime.toTime_t());
			if (startframe < 0)
			{
				startframe = 0;
			}
			else
			{
				isPlayInMid = false;
			}
			for (int frame = startframe; frame < totalFrames && !m_bStop && currentPlayTime < m_endTime; frame++)
			{
				//pause
				if (m_bPause)
				{
					g_mtxPause.lock();
					g_waitConPause.wait(&g_mtxPause);
					g_mtxPause.unlock();
				}

				AVI_set_video_position(file, frame);
				length = AVI_read_frame(file, vedioBuff, &isKeyFrame);

				if (startframe > 0 && 0 == isKeyFrame && !isPlayInMid)
				{
					continue;
				}

				if (startframe > 0 && 1 == isKeyFrame && !isPlayInMid)
				{
					isPlayInMid = true;
					isFirstKeyFrame = true;
				}

				//if first frame, play and start Timing
				if (startframe == frame || isFirstKeyFrame)
				{
					m_pVedioDecoder->decode(vedioBuff, length);
					frameTimer.start();
					isFirstKeyFrame = false;
					continue;
				}

				int waitmilliSeconds = 0;
				waitmilliSeconds = 1000000/frameRate - frameTimer.nsecsElapsed()/1000 + m_nSpeedRate*10*1000;
				if (waitmilliSeconds > 0)
				{
					usleep(waitmilliSeconds);
				}
				
				m_pVedioDecoder->decode(vedioBuff, length);
				frameTimer.start();
				//count current play time;
				currentPlayTime = fileStartTime.addSecs(frame/frameRate);
			}
			AVI_close(file);
		}
		else
		{
			continue;
		}
	}
	m_nSpeedRate = 0;
	m_bStop = false;
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
	m_bStop = true;

	m_nInitWidth = 0;
	m_nInitHeight = 0;
	m_nStartPos = 0;
	m_nSpeedRate = 0;
	m_pRenderWnd = NULL;
	m_bPause = false;
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
