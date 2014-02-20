#ifndef QSUBVIEW_H
#define QSUBVIEW_H

#include <QWidget>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QVariantMap>
#include <IDeviceClient.h>
#include <IEventRegister.h>
#include <IVideoDecoder.h>
#include "PreviewWindowsGlobalSetting.h"
#include "QSubviewThread.h"
#include <IVideoRender.h>
#include <IRecorder.h>
#include <ISetRecordTime.h>
#include <QTime>
#include <QTimer>
#include <QMenu>
#include <ISwitchStream.h>
#include "QSubViewObject.h"
#include <QLineEdit>
#include "ui_TitleView.h"


int cbLiveStream(QString evName,QVariantMap evMap,void*pUser);
int cbForRecord(QString evName,QVariantMap evMap,void*pUser);
int cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser);
int cbConnectError(QString evName,QVariantMap evMap,void*pUser);
int cbStateChange(QString evName,QVariantMap evMap,void*pUser);

class QSubView :public QWidget
{
	Q_OBJECT
public:
	QSubView(QWidget *parent = 0);
	~QSubView();

	virtual void paintEvent( QPaintEvent * );

	virtual void mouseDoubleClickEvent( QMouseEvent * );

	virtual void mousePressEvent(QMouseEvent *);


	int GetCurrentWnd();
	int OpenCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId
		,unsigned int uiChannelId,unsigned int uiStreamId
		,const QString & sUsername,const QString & sPassword
		,const QString & sCameraname,const QString & sVendor);
	int SetCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId
		,unsigned int uiChannelId,unsigned int uiStreamId
		,const QString & sUsername,const QString & sPassword
		,const QString & sCameraname,const QString & sVendor);
	int CloseWndCamera();
	int GetWindowConnectionStatus();

	//手动录像
	int StartRecord();
	int StopRecord();
	int SetDevInfo(const QString&devname,int nChannelNum);

	typedef enum __enQSubViewConnectStatus{
		STATUS_CONNECTED,
		STATUS_CONNECTING,
		STATUS_DISCONNECTED,
		STATUS_DISCONNECTING,
	}QSubViewConnectStatus;
signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void mousePressEvent(QWidget *,QMouseEvent *);
	void mouseLeftClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);
	void CurrentStateChangeSignl(int statevalue,QWidget *);
	void FreshWindow();
	void Connectting();
	void DisConnecting();
	void DisConnected();
	void RMousePressMenu();


private:
	DevCliSetInfo m_DevCliSetInfo;
	IVideoRender *m_IVideoRender;
	IVideoDecoder *m_IVideoDecoder;
	IDeviceClient *m_IDeviceClientDecideByVendor;
	QSubViewObject m_QSubViewObject;
	IRecorder *m_pRecorder;
	ISetRecordTime *m_pRecordTime;

	QSubViewConnectStatus m_CurrentState;
	QSubViewConnectStatus m_HistoryState;
	int iInitWidth;
	int iInitHeight;
	bool bRendering;
	bool m_bIsRecording;
	bool bCloseFromUi;//true:from ui;false:from net;
	QTime dieTime;
	QTimer m_checkTime;

	Ui::titleview * ui;

	QMutex m_MutexdoubleClick;
	QMenu m_RMousePressMenu;

	QAction *m_QActionCloseView;

	int CountDisConnecting;
	int CountConnecting;
private:
	int cbInit();

public:
	//回调
	int PrevPlay(QVariantMap evMap);
	int ForRecord(QVariantMap evMap);
	int PrevRender(QVariantMap evMap);
	int CurrentStateChange(QVariantMap evMap);
	//////////////
	int SetDeviceByVendor(const QString & sVendor);
public slots:
	void OnFreshWindow();
	//void emitOnFreshWindow();
	void OnRMousePressMenu();
	void OnCloseFromMouseEv();
	void OnConnectting();
	void OnDisConnecting();
	void OnDisConnected();
	virtual void timerEvent( QTimerEvent * );

	void OnCheckTime();
private:
	void AutoConnect();
};


#endif // QSUBVIEW_H
