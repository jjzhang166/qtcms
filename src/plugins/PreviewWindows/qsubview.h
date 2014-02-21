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
public:
	//回调
	int PrevPlay(QVariantMap evMap);
	int ForRecord(QVariantMap evMap);
	int PrevRender(QVariantMap evMap);
	int CurrentStateChange(QVariantMap evMap);
	//////////////
	int SetDeviceByVendor(const QString & sVendor);
private:
	int cbInit();
	
	void In_CloseAutoConnect();
public slots:
		void OnRMousePressMenu();
		void OnCloseFromMouseEv();
		void OnConnectting();
		void OnDisConnecting();
		void OnDisConnected();
		virtual void timerEvent( QTimerEvent * );
		void OnRenderHistoryPix();
		void OnCheckTime();
		void OnCreateAutoConnectTime();
		void In_OpenAutoConnect();
signals:
		void mouseDoubleClick(QWidget *,QMouseEvent *);
		void mousePressEvent(QWidget *,QMouseEvent *);
		void mouseLeftClick(QWidget *,QMouseEvent *);
		void SetCurrentWindSignl(QWidget *);
		void CurrentStateChangeSignl(int statevalue,QWidget *);
		void Connectting();
		void DisConnecting();
		void DisConnected();
		void RMousePressMenu();
		void RenderHistoryPix();
		void AutoConnectSignals();
		void CreateAutoConnectTimeSignals();
private:
	DevCliSetInfo m_DevCliSetInfo;//设备信息
	RenderInfo m_HistoryRenderInfo;//上一帧图像的信息
	IVideoRender *m_IVideoRender;
	IVideoDecoder *m_IVideoDecoder;
	IDeviceClient *m_IDeviceClientDecideByVendor;
	QSubViewObject m_QSubViewObject;
	IRecorder *m_pRecorder;
	ISetRecordTime *m_pRecordTime;
	
	QSubViewConnectStatus m_CurrentState;//设备当前的连接状态
	QSubViewConnectStatus m_HistoryState;//设备上一次的连接状态
	int iInitWidth;
	int iInitHeight;
	//标志位
	bool bRendering;
	bool m_bIsRecording;
	bool m_bIsRenderHistory;
	bool m_bIsAutoConnect;
	bool m_bStateAutoConnect;
	bool m_bIsAutoConnecting;

	QTime dieTime;
	QTimer m_checkTime;

	Ui::titleview * ui;

	QMutex m_MutexdoubleClick;
	QMutex m_csRender;
	QMenu m_RMousePressMenu;

	QAction *m_QActionCloseView;
	//正在连接和正在断开，刷新图片的计数
	int CountDisConnecting;
	int CountConnecting;
	//时钟id
	int m_DisConnectingTimeId;
	int m_DisConnectedTimeId;
	int m_RenderTimeId;
	int m_AutoConnectTimeId;
};


#endif // QSUBVIEW_H
