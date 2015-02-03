#pragma once
#include <QWidget>
#include "qwfw.h"
#include "PreviewWindowsGlobalSetting.h"
#include <IWindowDivMode.h>
#include <QtCore/QtCore>
#include <QtXml/QtXml>
#include <libpcom.h>
#include "IUserManager.h"
#include "IChannelManager.h"
#include <guid.h>
#include <qwfw_tools.h>
#include <qsubviewEx.h>
#include "ILocalSetting.h"
#include <QList>
#include <IDisplayWindowsManager.h>
#include <QVariantMap>
#include <QShowEvent>
#include <QHideEvent>
#include <QTimer>
class qpreviewwindowsex:public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	qpreviewwindowsex(QWidget *parent = 0);
	virtual ~qpreviewwindowsex();

public:
	virtual void resizeEvent(QResizeEvent *);
	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);
public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};
	//窗口信息
	virtual void nextPage();
	virtual void prePage();
	virtual int getCurrentPage();
	virtual int getPages();
	virtual int setDivMode( QString divModeName );
	virtual QString getCureentDivMode();
	virtual int GetCurrentWnd();
	int GetWindowConnectionStatus(unsigned int uiWndIndex);
	QVariantMap GetWindowInfo(unsigned int uiWndIndex);
	//打卡与关闭预览
	int OpenCameraInWnd(unsigned int uiWndIndex
		,const QString sAddress,unsigned int uiPort,const QString & sEseeId
		,unsigned int uiChannelId,unsigned int uiStreamId
		,const QString & sUsername,const QString & sPassword
		,const QString & sCameraname
		,const QString & sVendor);
	
	int CloseWndCamera(unsigned int uiWndIndex);
	int CloseAll();
	//切换码流
	int SwithStream(unsigned int uiWndIndex,int chlId);
	//设置窗口的设备信息 	
	int SetDevChannelInfo(unsigned int uiWndIndex,int ChannelId);//may be unnecessary
	//录像
	int StartRecord(int nWndID);
	int StopRecord(int nWndID);
	int SetDevInfo(const QString&devname,int nChannelNum,int nWndID);//may be unnecessary
	//音频
	int SetVolume(unsigned int uiPersent);
	int AudioEnabled(bool bEnabled);
	//截屏
	void screenShot(QString sUser,int nType);
	//全屏
	void SetFullScreenFlag();
	void OnBackToMainWnd();
	//云台
	int OpenPTZ(int nCmd, int nSpeed);
	int ClosePTZ(int nCmd);
	//子窗口信号函数
	void subWindowDblClick(QWidget*,QMouseEvent *);
	void subWindowMousePress(QWidget*,QMouseEvent *);
	void subWindowMouseRelease(QWidget*,QMouseEvent *);
	void subWindowConnectStatus(QVariantMap,QWidget *);
	void subWindowConnectRefuse(QVariantMap,QWidget *);
	void subWindowAuthority(QVariantMap,QWidget *);
	void subWindowScreenShot(QVariantMap,QWidget *);
	void subWindowVerify(QVariantMap vmap);
	// 图像拉伸
	void AllWindowStretch(bool bEnable);
	//自动轮巡
	void StartAutoPolling();
	void StopAutoPolling();
	void slPolling();
	//电子放大
	void shutDownDigtalZoom();
	void enableDigtalZoom();
	void ViewNewPosition(QRect tRect,int nWidth,int nHeight);
private:
	bool chlIsExist(int chlId);
	QString getLanguageLable();
	int OpenCameraInWnd(unsigned int uiWndIndex,int chlId);
	int getPollInterval();
	void hideDigitalView();
	void restoreDigitalView();
private:
	IWindowDivMode * m_divMode;
	qsubviewEx m_sPreviewWnd[MAX_WINDOWS_NUM];
	QList<QWidget*>m_pPreviewWndList;
	int m_nCurrentWnd;
	bool m_bAudioEnabled;
	QString m_sLastLanguageLabel;
	QTimer *m_pAutoPollingTimer;
	bool m_bIsEnableDigitalZoom;
	int m_nRestoreViewNum;
	bool m_bIsRestroreView;
};

