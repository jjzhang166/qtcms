#ifndef RSUBVIEW_H
#define RSUBVIEW_H

#include <QWidget>
#include <QString>
#include <QVariantMap>
#include <IDeviceClient.h>
#include <IEventRegister.h>
#include <IVideoDecoder.h>
#include <QLineEdit>
#include "ui_TitleView.h"
#include <IDeviceClient.h>
#include <IDeviceConnection.h>
#include <IDeviceRemotePlayback.h>
#include <QTimer>
#include <QPixmap>

#include <QMenu>
#include <QAction>

#include <QLabel>

class RSubView :public QWidget
{
	Q_OBJECT

public:
	RSubView(QWidget *parent = 0);
	~RSubView();

	virtual void paintEvent( QPaintEvent * );
	virtual void mouseDoubleClickEvent( QMouseEvent * );
	virtual void mousePressEvent(QMouseEvent *);

	enum __enPaintEventStatus{
		PAINTEVENT_STATUS_NOVIDEO,
		PAINTEVENT_STATUS_CONNECTING,
		PAINTEVENT_STATUS_CACHE,
	}PaintEventStatus;
	enum __enConnectStatus{
		CONNECT_STATUS_CONNECTED,
		CONNECT_STATUS_CONNECTING,
		CONNECT_STATUS_DISCONNECTED,
		CONNECT_STATUS_DISCONNECTING,
	}ConnectStatus;
public:
	void SetLpClient(IDeviceGroupRemotePlayback *m_GroupPlayback);
	void setAudioHint(QString&);

signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);
	void RMousePressMenu();
	void ChangeAudioHint(QString, RSubView*);

public:    
	Ui::titleview * ui;
	IDeviceClient *m_LpClient;
public slots:
	void OnOpenAudio();
	void OnRMousePressMenu();

private:
	static RSubView* m_pCurView;
	static bool m_bIsAudioOpen;

	IDeviceGroupRemotePlayback* m_pRemotePlayBack;
	QMenu m_rMousePressMenu;
	QAction *m_ActionOpenAudio;
};


#endif // RSUBVIEW_H
