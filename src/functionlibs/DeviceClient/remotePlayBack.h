#pragma once
#include <QThread>
#include <QDebug>
#include <QDateTime>
#include <QQueue>
#include <QVariantMap>
#include <QMultiMap>
#include <QStringList>
#include <QEventLoop>
#include <QTimer>
#include <guid.h>
#include <IRemotePlayback.h>
#include <IEventRegister.h>
#include <IDeviceConnection.h>
#include <QMutex>
#include <QWidget>
#include <QMap>
#include <IVideoDecoder.h>
#include <IVideoRender.h>
#include <QElapsedTimer>
#include <IAudioPlayer.h>

typedef int (__cdecl *remoteRecCb)(QString eventName,QVariantMap info,void *pUser);
typedef struct __tagRecStreamFrame{
	uint uiLength;
	char cFrameType;
	char cChannel;
	union{
		uint uiAudioSampleRate;
		uint uiWidth;
	};
	union{
		char cAudioFormat[8];
		uint uiHeight;
	};
	union{
		uint uiAudioDataWidth;
		uint uiFrameRate;
	};
	quint64 ui64TSP;
	uint uiGenTime;
	char* pData;
}tagRecStreamFrame;
typedef struct __tagGroupPlayInfo{
	int nChannel;
	quint64 ui64Tsp;
	volatile bool bStartPlayStream;
	bool bFirstFrame;
	quint64 ui64Spend;
	QElapsedTimer tElapsedTimer;
	QWidget *wPlayWidget;
	IVideoDecoder *pVideoDecoder;
	IVideoRender *pVideoRender;
	QQueue<tagRecStreamFrame> tRecStreamFrame;
}tagGroupPlayInfo;
typedef enum __tagSpeedType{
	RECSPEEDFAST,
	RECSPEEDSLOW,
	RECSPEEDNORMAL,
}tagSpeedType;
typedef struct __tagRecProcInfo{
	remoteRecCb proc;
	void *pUser;
}RecProcInfo;
typedef enum __tagControlStepCode{
	SEARCHRECFILE,
	GROUPPLAY,
	GROUPPAUSE,
	GROUPCONTINUE,
	GROUPSTOP,
	GROUPENABLEAUDIO,
	GROUPSETVOLUME,
	GROUPSPEEDFAST,
	GROUPSPEEDSLOW,
	GROUPSPEEDNORMAL,
	DEALWITHSTREAM,
	DEFAULT,
	RECEND,
}ControlStepCode;
typedef enum __tagPlayStepCode{
	RECPALYINIT,
	FINDMAXSTREAMCHANNEL,
	RECPLAYSTREAM,
	RECPLAYEND,
}PlayStepCode;
typedef enum __tagPlayBackProtocol{
	BUBBLE,
	TURN,
	HOLE,
}tagPlayBackProtocol;
typedef enum __tagRemovePlayBackConnectStatus{
	REC_STATUS_CONNECTED,
	REC_STATUS_CONNECTING,
	REC_STATUS_DISCONNECTED,
	REC_STATUS_DISCONNECTING,
}tagRemovePlayBackConnectStatus;
typedef struct __tagRecDeviceInfo{
	QString sAddr;
	QString sEsee;
	unsigned int uiPorts;
	QString sUserName;
	QString sPassword;
	int nSearchChannel;
	int nPlayBackChannel;
	int nSearchTypes;
	QDateTime tSearchStartTime;
	QDateTime tSearchEndTime;
	int nPlayTypes;
	QDateTime tPlayStartTime;
	QDateTime tPlayEndTime;
	bool bAudio;
	unsigned int uiAudioVolume;
}RecDeviceInfo;

class remotePlayBack:public QThread
{
	Q_OBJECT
public:
	remotePlayBack(void);
	~remotePlayBack(void);

public:
	//×¢²á»Øµ÷º¯Êý
	void registerEvent(QString eventName,int (__cdecl *proc)(QString ,QVariantMap ,void *),void *pUser);
	//setting
	int setDeviceHost(const QString &sAddr);
	int setDevicePorts(unsigned int ports);
	int setDeviceEsee(const QString &Esee);
	int checkUser(const QString &sUserName,const QString& sPassword);
	int addChannelIntoPlayGroup(int nChannel,QWidget * wnd);

	//search file
	int startSearchRecFile(int nChannel,int nTypes,const QDateTime & startTime,const QDateTime & endTime);
	
	//video play
	int groupPlay(int nTypes,const QDateTime & start,const QDateTime & end);
	int groupPause();
	QDateTime getGroupPlayTime();
	int groupContinue();
	int groupStop();
	int groupSpeedFast();
	int groupSpeedSlow();
	int groupSpeedNormal();
	
	//audio play
	bool groupEnableAudio(bool bEnable);
	int groupSetVolume(unsigned int uiPersent, QWidget* pWnd);
public:
	//call back
	int cbConnectStatusChange(QString sEventName,QVariantMap evMap,void *pUser);
	int cbRecFileFound(QString sEventName,QVariantMap evMap,void *pUser);
	int cbRecSearchFail(QString sEventName,QVariantMap evMap,void *pUser);
	int cbRecSearchFinished(QString sEventName,QVariantMap evMap,void *pUser);
	int cbRecSocketError(QString sEventName,QVariantMap evMap,void *pUser);
	int cbRecStream(QString sEventName,QVariantMap evMap,void *pUser);
protected:
	void run();
private:
	void eventCallBack(QString sEventName,QVariantMap evMap);
	void sleepEx(int time);
	void remoteRePeatWnd(QWidget *wWin);
private slots:
	void slCheckBlock();
	void slBackToMainThread(QString sEventName,QVariantMap evMap);
signals:
	void sgBackToMainThread(QString sEventName,QVariantMap evMap);
private:
	QQueue<int> m_stepCode;
	RecDeviceInfo m_tRecDeviceInfo;
	QMultiMap<QString,RecProcInfo> m_tEventMap;
	QMap<int ,tagGroupPlayInfo>m_tPlayGroupMap;
	QStringList m_sEventNameList;
	volatile bool m_bStop;
	int m_nSleepSwitch;
	QQueue<tagRecStreamFrame> m_tStreamBuffer;
	IRemotePlayback *m_pRemotePlayback;
	IAudioPlayer *m_pAudioPlayer;
	QMutex m_tStepCodeLock;
	QMutex m_tStreamLock;
	QTimer m_tCheckBlockTimer;
	bool m_bBlock;
	bool m_bStreamPuase;
	bool m_bPuase;
	bool m_bEnableAudio;
	unsigned int m_uiVolumePersent;
	int m_nPosition;
	QWidget *m_pCurrentWin;
	unsigned int m_uiCurrentFrameTime;
	tagPlayBackProtocol m_tPlaybackProtocol;
	tagRemovePlayBackConnectStatus m_tRecConnectStatus;
	tagRemovePlayBackConnectStatus m_tRecHisConnectStatus;
	tagSpeedType m_tSpeedType;
	int m_nSpeedRate;
	int m_nSampleRate;
	int m_nSampleWidth;
};

