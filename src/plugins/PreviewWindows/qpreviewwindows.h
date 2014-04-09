#ifndef QPREVIEWWINDOWS_H
#define QPREVIEWWINDOWS_H

#include <QWidget>
#include <QMutex>
#include "qsubview.h"
#include <QVariantMap>
#include "qwfw.h"
#include "PreviewWindowsGlobalSetting.h"
#include "IWindowDivMode.h"
#include <IChannelManager.h>
#include <ILocalSetting.h>



class QPreviewWindows : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	QPreviewWindows(QWidget *parent = 0);
	~QPreviewWindows();

	virtual void resizeEvent( QResizeEvent * );

public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};

	virtual void nextPage();

	virtual void prePage();

	virtual int getCurrentPage();

	virtual int getPages();

	virtual int setDivMode( QString divModeName );

	virtual QString getCureentDivMode();

	void OnSubWindowDblClick(QWidget *,QMouseEvent *);

	void OnSubWindowRmousePress(QWidget *,QMouseEvent *);

	void SetCurrentWind(QWidget *);

	int GetCurrentWnd();

	int OpenCameraInWnd(unsigned int uiWndIndex
		,const QString sAddress,unsigned int uiPort,const QString & sEseeId
		,unsigned int uiChannelId,unsigned int uiStreamId
		,const QString & sUsername,const QString & sPassword
		,const QString & sCameraname
		,const QString & sVendor);
	int OpenCameraInWnd(unsigned int uiWndIndex,int chlId);
	int SwithStream(unsigned int uiWndIndex,int chlId);
	int SetDevChannelInfo(unsigned int uiWndIndex,int ChannelId);

	int CloseWndCamera(unsigned int uiWndIndex);

	int GetWindowConnectionStatus(unsigned int uiWndIndex);
	QVariantMap GetWindowInfo(unsigned int uiWndIndex);

	void CurrentStateChangePlugin(QVariantMap evMap,QWidget *WID);

	//
	int StartRecord(int nWndID);
	int StopRecord(int nWndID);
	int SetDevInfo(const QString&devname,int nChannelNum,int nWndID);

	int SetVolume(unsigned int uiPersent);
	int AudioEnabled(bool bEnabled);
	QVariantMap ScreenShot();
	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

	//ptz control
	int OpenPTZ(int nCmd, int nSpeed);
	int ClosePTZ(int nCmd);

	//login
	QVariantMap CheckLoginInof(const QString &sUsername, const QString &sPassword, const QString &sLanguageLabel, bool bAutoLogin);
	int ModifyPassword(const QString &sUsername, const QString &sOldPassword, const QString &sNewPassword);

private:
	bool ChlIsExit(int chlId);
	QString GetLanguageLabel();
private:
	QSubView m_PreviewWnd[64];
	IWindowDivMode * m_DivMode;
	QList<QWidget *> m_PreviewWndList;
	QMap<int, int> m_channelWndMap;

	int m_uiWndIndex;
	volatile int m_CurrentWnd;
	bool m_bIsOpenAudio;
	
	QMutex m_mutex;
	QString hisLanguageLabel;


};

#endif // QPREVIEWWINDOWS_H
