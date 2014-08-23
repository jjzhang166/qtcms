#pragma once
#include <QWidget>
#include "qwfw.h"
#include "rSubview.h"
#include "rPlayBackRunEx.h"
#include <QtNetwork/QHostAddress>
#include <IChannelManager.h>
#include <IDeviceManager.h>
typedef struct __tagRemotePlayBackDeviceInfo{
	QString sAddress;
	QHostAddress tHostAddress;
	QString sEseeId;
	QString sVendor;
	QString sUserName;
	QString sPassword;
	QString sDeviceName;
	unsigned int uiPort;
	unsigned int uiChannelId;
	int nChannelIdInDatabase;
	unsigned int uiStream;
}tagRemotePlayBackDeviceInfo;
class rPlaybackWndEx:public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	rPlaybackWndEx(QWidget *parent=0);
	~rPlaybackWndEx(void);
	virtual void resizeEvent(QResizeEvent *);
public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};

	// 设置设备连接信息
	int setDeviceHostInfo(const QString & sAddress,unsigned int uiPort,const QString &sEseeID);
	// 设置设备厂商信息
	int setDeviceVendor(const QString & sVendor);
	// 设置回放同步组
	int AddChannelIntoPlayGroup(uint uiWndId,int uiChannelId);
	int GetWndInfo(int uiWndId );
	void setUserVerifyInfo(const QString & sUsername,const QString & sPassword);

	int startSearchRecFile(int nChannel,int nTypes,const QString & sStartTime,const QString & sEndTime);
	QString GetNowPlayedTime();

	int   GroupPlay(int nTypes,const QString & sStart,const QString & sEnd);
	int   GroupPause();
	int   GroupContinue();
	int   GroupStop();
	int  AudioEnabled(bool bEnable);
	int   SetVolume(const unsigned int &uiPersent);
	int   GroupSpeedFast() ;
	int   GroupSpeedSlow();
	int   GroupSpeedNormal();
	int	GetCurrentState();
private:
	bool getChannelInfo(int nChlId);
private:
	tagRemotePlayBackDeviceInfo m_tDeviceInfo;
	rPlayBackRunEx m_tPlaybackRunEx;
	RSubView m_tPlaybackWnd[4];
};

