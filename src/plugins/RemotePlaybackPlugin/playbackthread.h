#ifndef PLAYBACKTHREAD_H
#define PLAYBACKTHREAD_H

#include <QThread>
#include <QDateTime>
#include <QMutex>
#include "RemotePlaybackPlugin_global.h"
#include "rSubview.h"
#include "PlayManager.h"
#include "BufferManager.h"
#include "IDevicePlayback.h"
#include "IEventRegister.h"
#include "IDeviceClient.h"
#include "IChannelManager.h"
#include "IDeviceManager.h"

typedef int (__cdecl *runEventCallBack)(QString,QVariantMap,void *);
typedef struct __tagEventCB{
	runEventCallBack evCBName;
	void*         pUser;
}EventCBInfo, *lpEventCBInfo;

typedef struct _tagDevCliSetInfo{
	QString m_sAddress;
	unsigned int m_uiPort;
	QString m_sEseeId;
	unsigned int m_uiChannelId;
	int m_uiChannelIdInDataBase;
	unsigned int m_uiStreamId;
	QString m_sUsername;
	QString m_sPassword;
	QString m_sCameraname;
	QString m_sVendor;
}DevCliSetInfo;

typedef struct _tagSearchDevInfo{
	int nSearchChls;
	int nSearchTypes;
	QDateTime startTime;
	QDateTime endTime;
}SearchDevInfo;

typedef struct _tagWndPlay{
	BufferManager *bufferManager;
	PlayManager *playManager;
	QWidget *wnd;
}WndPlay;

typedef QMap<int, WndPlay>::iterator PlayIter;

typedef enum _emOperation{
	EM_DEFAULT,
	EM_SEARCH,
	EM_PLAY,
	EM_PAUSE,
	EM_CONTINUE,
	EM_STOP,
	EM_FALT
}Operation;

class PlayBackThread : public QThread
{
	Q_OBJECT

public:
	PlayBackThread();
	~PlayBackThread();

	// 设置设备连接信息
	int setDeviceHostInfo(const QString & sAddress,unsigned int uiPort,const QString &eseeID);
	// 设置设备厂商信息
	int setDeviceVendor(const QString & vendor);
	void setUserVerifyInfo(const QString & sUsername,const QString & sPassword);
	QString GetNowPlayedTime();

	int startSearchRecFile(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime);
	// 设置回放同步组
	int AddChannelIntoPlayGroup(uint uiWndId,int uiChannelId);
	int   GroupPlay(int nTypes,const QDateTime & start,const QDateTime & end);
	int   GroupPause();
	int   GroupContinue();
	int   GroupStop();
	int  AudioEnabled(bool bEnable);
	int   SetVolume(const unsigned int &uiPersent, QWidget* pWnd);
	int   GroupSpeedFast() ;
	int   GroupSpeedSlow();
	int   GroupSpeedNormal();

	void setPlaybackWnd(RSubView* wnd);
	int recordFrame(QVariantMap &evMap);

	int setInfromation(QString evName, QVariantMap info);
public:
	void  FoundFile(QVariantMap evMap);
	void  RecFileSearchFinished(QVariantMap evMap);
	void  RecFileSearchFail(QVariantMap evMap);
	void  StateChange(QVariantMap evMap);
signals:
	void FoundFileToUiS(QVariantMap );
	void RecFileSearchFinishedToUiS(QVariantMap );
	void FileSearchFailToUiS(QVariantMap);
	void SocketErrorToUiS(QVariantMap );
	void StateChangeToUiS(QVariantMap );
protected:
	void run();
private slots:
	void action(QString act, BufferManager* pbuff);
private:
	void getPlaybackInterface(void** playbackInterface);
	bool getPlayInterface(QWidget* pwnd, void** playInterface);
	int getStreamInfo(int chl);
	bool removeRepeatWnd(QWidget *wndID);
	void initCallBackFun();
private:
	DevCliSetInfo m_devInfo;
	SearchDevInfo m_schInfo;
// 	Operation m_curOperate;
	IDeviceRemotePlayback *m_playback;
	QVariantMap m_fileMap;
	QList<RSubView*> m_wndList;
	QList<QWidget*> m_zoomWndList;
	QList<int> m_stepQueue;
	QMap<int, WndPlay> m_playMap;
	QWidget* m_susWnd;
	QDateTime m_playStart;
	QDateTime m_playEnd;
	int m_fileTotal;
	int m_fileKey;
	int m_playTypes;
	int m_channelWithAudio;
	int m_nSpeedRate;
	uint m_nChannels;

	bool m_bStop;
	bool m_bInitFlag;

	QMutex m_mx;
};

#endif // PLAYBACKTHREAD_H
