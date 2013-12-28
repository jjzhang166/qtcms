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
#include <QTime>
#include <QMenu>
#include "QSubViewObject.h"
#include <QLineEdit>
#include "ui_TitleView.h"


int cbLiveStream(QString evName,QVariantMap evMap,void*pUser);
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
	void RMousePressMenu();


private:
	DevCliSetInfo m_DevCliSetInfo;
	IVideoRender *m_IVideoRender;
	IVideoDecoder *m_IVideoDecoder;
	IDeviceClient *m_IDeviceClient;
	QSubViewObject m_QSubViewObject;

	QSubViewConnectStatus m_CurrentState;
	int iInitWidth;
	int iInitHeight;
	bool bIsInitFlags;
	bool bRendering;
	QTime dieTime;

	Ui::titleview * ui;

	QMutex m_MutexdoubleClick;
	QMenu m_RMousePressMenu;

	QAction *m_QActionCloseView;


private:
	int cbInit();

public:
	int PrevPlay(QVariantMap evMap);
	int PrevRender(QVariantMap evMap);
	int CurrentStateChange(QVariantMap evMap);

public slots:
	void OnFreshWindow();
	void emitOnFreshWindow();
	void OnRMousePressMenu();
	void OnCloseFromMouseEv();

	virtual void timerEvent( QTimerEvent * );


};


#endif // QSUBVIEW_H
