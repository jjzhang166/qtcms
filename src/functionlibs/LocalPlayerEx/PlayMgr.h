#ifndef _PLAYMGR_HEAD_FILE_H_
#define _PLAYMGR_HEAD_FILE_H_
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QList>
#include <QWidget>
#include <QVariantMap>
#include "LocalPlayerEx_global.h"
#include "IVideoDecoder.h"
#include "IVideoRender.h"
#include "IAudioPlayer.h"


class PlayMgr :
	public QThread
{
	Q_OBJECT
public:
	PlayMgr(void);
	~PlayMgr(void);
	void setParamter(QWidget* pWnd, uint uiStartSec, uint uiEndSec);
	void setSkipTime(const QVector<PeriodTime> &skipTime);
	void setFilePeriod(const QVector<PeriodTime> &filePeriod);
	QList<FrameData>* getBufferPointer();
	void startPlay();
	void stop();
	void setSpeedRate(qint32 i32Speed);
	qint32 prePlay(QVariantMap &item);
	void setCbTimeChange(pcbTimeChange pro, void* pUser);
	PlayMgr *getPlayMgrPointer(QWidget *pwnd);
	void openAudio(bool bEnable);

	void enableVideoStretch(bool bEnable);
	bool getVideoStretchStatus();

	void addWnd(QWidget* pWnd, QString sName);
	void removeWnd(QString sName);
	void setZoomRect(QRect rect);

	static void pause(bool bIsPause);
	static qint32 setVolume(uint uiPersent);
	static void audioSwitch(bool bOpen);
protected:
	void run();
private:
	qint32 initCbFuction();
	void clearBuffer();
	qint32 findStartPos(const QVector<PeriodTime> &vecPeriod);
	qint32 adjustTimeLine(uint uiStart, uint &status);
	void getZoomInterface(void** ppv);

	//for test
	void printVector(uint types, const QVector<PeriodTime> &vec);
private:
	QList<FrameData> m_quFrameBuffer;
	volatile bool m_bStop;
	bool m_bIsAudioOpen;
	bool m_bIsChangeSpeed;
	QWidget *m_pRenderWnd;
	uint m_uiStartSec;
	uint m_uiEndSec;
	uint m_uiCurrentGMT;
	qint32 m_i32SpeedRate;
	qint32 m_i32Width;
	qint32 m_i32Height;
	qint32 m_i32AudioChl;
	qint32 m_i32SmapleRate;
	qint32 m_i32SmapleWidth;
	qint32 m_i32FileStartPos;
	qint32 m_i32SkipStartPos;
	qint64 m_i64FrameInterval;


	QMutex m_mxWait;
	QWaitCondition m_wcWait;
	IVideoDecoder *m_pVedioDecoder;
	IVideoRender *m_pVedioRender;

	pcbTimeChange m_pcbTimeChg;
	void* m_pUser;

	QVector<PeriodTime> m_skipTime;
	QVector<PeriodTime> m_filePeriod;

	static bool m_bIsSkiped;
	static bool m_bPause;
	static QMutex m_mxPause;
	static QWaitCondition m_wcPause;
	static IAudioPlayer* m_pAudioPlayer;
 
};


#endif //_PLAYMGR_HEAD_FILE_H_

