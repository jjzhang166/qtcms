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


#include <QMenu>
#include <QAction>


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
	bool AudioEnabled(bool bEnabled);
	void SetCurConnectState(__enConnectStatus parm);
	void CacheState(QVariantMap evMap);


signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);
	void RMousePressMenu();
	void ChangeAudioHint(QString, RSubView*);

	void connecttingUpdateSig();
	void CacheStateSig(QVariantMap evMap);

public:    
	Ui::titleview * ui;
	IDeviceClient *m_LpClient;

	__enConnectStatus _curState;
	__enPaintEventStatus _curPaint;
	int _countConnecting;
	QTimer m_checkTime;
	QTimer m_cacheTime;
public slots:
	void OnOpenAudio();
	void OnRMousePressMenu();


	void connecttingUpdateSlot();
	void connecttingUpdate();
	void CacheStateSlot(QVariantMap evMap);
	void CacheStateSlotUpdate();
private:
	static RSubView* m_pCurView;
	static bool m_bLocalAudioStatus;
	static bool m_bGlobalAudioStatus;

	IDeviceGroupRemotePlayback* m_pRemotePlayBack;
	QMenu m_rMousePressMenu;
	QAction *m_ActionOpenAudio;

	int _curCache;

private:
	void paintEventNoVideo( QPaintEvent * );
	void paintEventConnecting( QPaintEvent * );
	void paintEventCache(QPaintEvent *);
};


#endif // RSUBVIEW_H
