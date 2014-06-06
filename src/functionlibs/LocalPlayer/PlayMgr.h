#ifndef _PLAYMGR_HEAD_FILE_H_
#define _PLAYMGR_HEAD_FILE_H_
#include <QThread>
#include "IVideoDecoder.h"
#include "IVideoRender.h"
#include <QVariantMap>
#include <QDateTime>
#include <QTime>
#include <QMutex>
#include <QWaitCondition>
#include "IAudioPlayer.h"

int cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser);

extern QMutex g_mtxPause;
extern QWaitCondition g_waitConPause;

typedef struct _tagPeriodTime{
	uint start;
	uint end;
}PeriodTime;

typedef void (*pcbTimeChange)(QString evName, uint playTime, void* pUser);

class PlayMgr :
	public QThread
{
	Q_OBJECT
public:
	PlayMgr(void);
	~PlayMgr(void);
	void setParamter(QStringList &fileList, QWidget* wnd, QDateTime &start, QDateTime &end, int &startPos, QVector<PeriodTime> &skipTime);
	void setPlaySpeed(int speedRate);
	void setCbTimeChange(pcbTimeChange pro, void* pUser);
	void pause(bool isPause);
	void stop();
	int prePlay(QVariantMap item);
	void OpneAudio(bool bEnabled);
	int setVolume(unsigned int &uiPersent);
	void AudioSwitch(bool bOpen);
private:
	int initCb();

protected:
	void run();
private:
	IVideoDecoder *m_pVedioDecoder;
	IVideoRender *m_pVedioRender;
	QWidget* m_pRenderWnd;
	bool m_bStop;
	bool m_bPause;
	bool m_bPlaying;
	bool m_bIsAudioOpen;
	QStringList m_lstfileList;
	int m_nInitHeight;
	int m_nInitWidth;
	int m_nSpeedRate;
	int m_nStartPos;
	int m_nAudioChl;
	int m_nSampleRate;
	int m_nSampleWidth;
	QDateTime m_startTime;
	QDateTime m_endTime;
	QVector<PeriodTime> m_skipTime;
	static uint m_playingTime;
	pcbTimeChange m_pcbTimeChg;
	void *m_pUser;
	static bool m_bIsPickThread;
	static bool m_bIsChange;
	static void* m_who;
	static IAudioPlayer* m_pAudioPlayer;

	QMutex m_mutex;
	QWaitCondition m_waitForPlay;
};


#endif

