#ifndef _PLAYMGR_HEAD_FILE_H_
#define _PLAYMGR_HEAD_FILE_H_
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QList>
#include <QWidget>
#include <QVariantMap>
#include "LocalPlayerEx_global.h"
#include <IDisksSetting.h>
#include <IScreenShot.h>
#include <QDir>
#include "IVideoDecoder.h"
#include "IVideoRender.h"
#include "IAudioPlayer.h"
typedef int (__cdecl *playMgrEventCb)(QString eventName,QVariantMap info,void *pUser);
typedef struct _tagProcInfo{
	playMgrEventCb proc;
	void *puser;
}tagProcInfo;
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
	void setZoomRect(QRect rect, int nWidth, int nHeight);
	void setOriginRect(QRect rect);

	static void pause(bool bIsPause);
	static qint32 setVolume(uint uiPersent);
	static void audioSwitch(bool bOpen);
	
	void screenShot(int nChl,int nType,QString sUser);

	void registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void*),void *pUser);
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
	//回调函数

	void eventCallBack(QString eventName,QVariantMap evMap);
	//截屏
	bool getScreenShotInfo(QString &sFileName,QString &sFileDir,quint64 &uiTime,int &nChl,int &nType);
	bool saveScreenShotInfoToDatabase(QString sFileName,QString sFileDir ,quint64 uiTime,int nChl,int nType);
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
 
	//截屏 参数
	bool m_bScreenShot;
	int m_nScreenShotType;
	int m_nScreenShotChl;
	QString m_sScreenUser;

	//回调参数
	QStringList m_eventNameList;
	QMultiMap<QString ,tagProcInfo> m_eventMap;
};


#endif //_PLAYMGR_HEAD_FILE_H_

