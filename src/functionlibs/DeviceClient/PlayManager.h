#ifndef _PLAYMANAGER_HEARD_FILE_H_
#define _PLAYMANAGER_HEARD_FILE_H_

#include <QThread>
#include "IVideoDecoder.h"
#include "IVideoRender.h"
#include "BufferManager.h"
#include "IAudioPlayer.h"

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
	bool m_bPause;
	bool m_bStop;
	bool m_bFirstFrame;
	bool m_bRendFinished;
	int m_nInitHeight;
	int m_nInitWidth;
	int m_nSpeedRate;
	int m_nSampleRate;
	int m_nSampleWidth;

	SpeedType m_speed;
	quint64 m_ui64TSP;
	uint m_uiCurrentFrameTime;
	IVideoDecoder *m_pVedioDecoder;
	IVideoRender *m_pVedioRender;
	BufferManager *m_pBufferManager;
	QWidget* m_pRenderWnd;

	static IAudioPlayer* m_pAudioPlayer;
	static PlayManager* m_pCurView;

private:
	int initCb();

};


#endif

