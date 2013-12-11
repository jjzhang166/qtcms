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
#include "PreviewPlay.h"
#include <IVideoRender.h>
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



	typedef struct _tagDevCliSetInfo{
		QString m_sAddress;
		unsigned int m_uiPort;
		QString m_sEseeId;
		unsigned int m_uiChannelId;
		unsigned int m_uiStreamId;
		QString m_sUsername;
		QString m_sPassword;
		QString m_sCameraname;
		QString m_sVendor;
	}DevCliSetInfo;
signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void mouseLeftClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);
	void SignalPreviewPlay();
	void CurrentStateChangeSignl(int statevalue,QWidget *);
private:
	DevCliSetInfo m_DevCliSetInfo;
	IVideoRender *m_IVideoRender;
	IVideoDecoder *m_IVideoDecoder;
	IDeviceClient *m_IDeviceClient;

	int iInitWidth;
	int iInitHeight;
	bool bIsInitFlags;

	Ui::titleview * ui;

	QMutex m_MutexdoubleClick;
private:
	int cbInit();

public:
	int PrevPlay(QVariantMap evMap);
	int PrevRender(QVariantMap evMap);
	int CurrentStateChange(QVariantMap evMap);

public slots:
	int ConnectOn();
	int ConnectOff();
};


#endif // QSUBVIEW_H
