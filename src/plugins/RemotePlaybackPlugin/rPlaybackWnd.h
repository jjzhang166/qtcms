#ifndef __RPLAYBACKWND_H__
#define __RPLAYBACKWND_H__

#include <QWidget>
#include <QMutex>
#include <QtNetwork/QHostAddress>
#include "RemotePlaybackObject.h"
#include "qwfw.h"
#include "IWindowDivMode.h"
#include "IDeviceRemotePlayback.h"
#include "IDeviceClient.h"
#include "rSubview.h"
#include <IChannelManager.h>
#include <IDeviceManager.h>
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

class RPlaybackWnd : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	RPlaybackWnd(QWidget *parent = 0);
	~RPlaybackWnd();

	virtual void resizeEvent( QResizeEvent * );

public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};

    // 设置设备连接信息
    int setDeviceHostInfo(const QString & sAddress,unsigned int uiPort,const QString &eseeID);
    // 设置设备厂商信息
    int setDeviceVendor(const QString & vendor);
    // 设置回放同步组
    //int AddChannelIntoPlayGroup(uint uiWndId,unsigned int uiChannel);
	int AddChannelIntoPlayGroup(uint uiWndId,int uiChannelId);
	int GetWndInfo(int uiWndId );
    void setUserVerifyInfo(const QString & sUsername,const QString & sPassword);

	int startSearchRecFile(int nChannel,int nTypes,const QString & startTime,const QString & endTime);
    QString GetNowPlayedTime();

    int   GroupPlay(int nTypes,const QString & start,const QString & end);
    int   GroupPause();
    int   GroupContinue();
    int   GroupStop();
    int  AudioEnabled(bool bEnable);
	int   SetVolume(const unsigned int &uiPersent);
    int   GroupSpeedFast() ;
    int   GroupSpeedSlow();
    int   GroupSpeedNormal();
public:
    int   GetCurrentWnd();
    void  CurrentStateChangePlugin(int stateValue);
    void  SocketErrorPlugin(int stateValue);
    int   GetRecFileNum(uint uiNum);
	
	QVariantMap ScreenShot();

	void  FoundFile(QVariantMap evMap);
	void  RecFileSearchFinished(QVariantMap evMap);
	void  SocketError(QVariantMap evMap);
	void  StateChange(QVariantMap evMap);
	void  CacheState(QVariantMap evMap);
	virtual void hideEvent(QHideEvent *);
	virtual void showEvent(QShowEvent *);
	typedef enum __enConnectStatus{
		STATUS_CONNECTED,
		STATUS_CONNECTING,
		STATUS_DISCONNECTED,
		STATUS_DISCONNECTING,
	}ConnectStatus;
	typedef enum __enConnectType{
		TYPE_NULL,
		TYPE_SEARCH,
		TYPE_STREAM,
	}ConnectType;
public:
	ConnectStatus _curConnectState;
	ConnectType _curConnectType;
	QList<int> _widList;
	QList<int >_widInfo;
	
private slots:
    void  OnSubWindowDblClick(QWidget *,QMouseEvent *);
    void  SetCurrentWind(QWidget *);

private:
	RSubView         m_PlaybackWnd[4];
	IWindowDivMode * m_DivMode;
    IDeviceGroupRemotePlayback * m_GroupPlayback;
//     IDeviceClient *  m_DeviceClient;
	QList<QWidget *> m_PlaybackWndList;
    QList<QVariantMap>m_SelectedRecList;
	RemotePlaybackObject m_RemotePlaybackObject;

    QHostAddress  m_HostAddress;
	QString		  m_sHostAddress;
    QString       m_sEseeId;
    QString       m_sVendor;
    QString       m_sUserName;
    QString       m_sUserPwd;
    uint          m_uiPort;
    int           m_nCurrentWnd;
    uint          m_uiRecFileSearched;
	unsigned int  m_uiPersent;

    bool bIsInitFlags;
    bool bIsCaseInitFlags;
	bool bIsOpenAudio;
   
	QMutex _mutexWidList;
	DevCliSetInfo m_DevCliSetInfo;
	private:
	int  cbInit();
	int GetDeviceInfo(int chlId);
};


#endif // __RPLAYBACKWND_H__
