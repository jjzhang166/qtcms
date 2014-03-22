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

class CPaintEventStatus{
public:
	enum __enPaintEventStatus{
		STATUS_NOVIDEO,
		STATUS_CONNECTING,
		STATUS_CACHE,
	};
};
class CConnectStatus{
public:
	enum __enConnectStatus{
		STATUS_CONNECTED,
		STATUS_CONNECTING,
		STATUS_DISCONNECTED,
		STATUS_DISCONNECTING,
	};
};

class RSubView :public QWidget
{
	Q_OBJECT

public:
	RSubView(QWidget *parent = 0);
	~RSubView();

	virtual void paintEvent( QPaintEvent * );
	virtual void mouseDoubleClickEvent( QMouseEvent * );
	virtual void mousePressEvent(QMouseEvent *);
public:
	void SetLpClient(IDeviceGroupRemotePlayback *m_GroupPlayback);
	void setAudioHint(QString&);
	void SetCurConnectState(CConnectStatus::__enConnectStatus parm);
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

	CConnectStatus::__enConnectStatus _curState;
	CPaintEventStatus::__enPaintEventStatus _curPaint;
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
	static bool m_bIsAudioOpen;

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
