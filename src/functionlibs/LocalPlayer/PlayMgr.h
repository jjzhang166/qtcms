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

int cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser);

extern QMutex g_mtxPause;
extern QWaitCondition g_waitConPause;

class PlayMgr :
	public QThread
{
	Q_OBJECT
public:
	PlayMgr(void);
	~PlayMgr(void);
	void setParamter(QStringList &fileList, QWidget* wnd, QDateTime &start, QDateTime &end, int &startPos);
	void setPlaySpeed(int speedRate);
	void pause(bool isPause);
	void stop();
	int prePlay(QVariantMap item);
	int getPlayTime();


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
	QStringList m_lstfileList;
	int m_nInitHeight;
	int m_nInitWidth;
	int m_nSpeedRate;
	int m_nStartPos;
	QDateTime m_startTime;
	QDateTime m_endTime;
	QTime m_playTime;

	QMutex m_mutex;
	QWaitCondition m_waitForPlay;
};


#endif

