#ifndef _PLAYMANAGER_HEARD_FILE_H_
#define _PLAYMANAGER_HEARD_FILE_H_

#include <QThread>
#include "IVideoDecoder.h"
#include "IVideoRender.h"
#include "BufferManager.h"
#include "IAudioPlayer.h"
#include <IDisksSetting.h>
#include <QDir>
#include <QDateTime>
#include <IScreenShot.h>

typedef int (__cdecl *playManagerEventCb)(QString eventName,QVariantMap info,void *pUser);
typedef struct _tagProcInfo{
	playManagerEventCb proc;
	void *puser;
}tagProcInfo;

int cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser);


class PlayManager :
	public QThread
{
	Q_OBJECT
public:
	PlayManager(void);
	~PlayManager(void);
	void setParamter(BufferManager *pBufferManager, QWidget* wnd);
	void setPlaySpeed(int types, int speedRate);
	void pause(bool isPause);
	void stop();
	int prePlay(QVariantMap item);
	int getPlayTime();
	void AudioSwitch(bool);
	int setVolume(unsigned int &uiPersent);
	void setCurAudioWnd(PlayManager* curWnd);

	void addWnd(QWidget* pWnd, QString sName);
	void removeWnd(QString sName);
	void setZoomRect(QRect rect, int nWidth, int nHeight);
	void setOriginRect(QRect rect);

	//注册回调函数
	void registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void*),void *pUser);

	//截屏
	void screenShot( QString sUser,int nType,int nChl );
	enum SpeedType{
		SpeedNomal,
		SpeedFast,
		SpeedSlow
	};
protected:
	void run();
signals:
	void action(QString, BufferManager*);
private:
	bool getScreenShotInfo(QString &sFileName,QString &sFileDir,quint64 &uiTime,int &nChl,int &nType);
	bool saveScreenShotInfoToDatabase(QString sFileName,QString sFileDir ,quint64 uiTime,int nChl,int nType);
	void eventCallBack(QString eventName,QVariantMap evMap);
private:
	bool m_bPause;
	bool m_bStop;
	bool m_bFirstFrame;
	bool m_bRendFinished;
	int m_nInitHeight;
	int m_nInitWidth;
	int m_nSpeedRate;
	static int m_nSampleRate;
	static int m_nSampleWidth;

	SpeedType m_speed;
	quint64 m_ui64TSP;
	uint m_uiCurrentFrameTime;
	IVideoDecoder *m_pVedioDecoder;
	IVideoRender *m_pVedioRender;
	BufferManager *m_pBufferManager;
	QWidget* m_pRenderWnd;

	static IAudioPlayer* m_pAudioPlayer;
	static PlayManager* m_pCurView;

	//截屏参数
	int m_nScreenShotType;
	int m_nScreenShotChl;
	QString m_sScreenUser;
	bool m_bScreenShot;

	//回调参数
	QMultiMap<QString ,tagProcInfo> m_eventMap;
	QStringList m_eventNameList;
private:
	int initCb();

};


#endif

