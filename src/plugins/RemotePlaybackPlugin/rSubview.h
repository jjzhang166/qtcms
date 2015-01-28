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
#include <suspensionwnd.h>
#include <QLabel>

typedef void (*pfnCb)(QString, QVariantMap, void*); 

void cbReciveMsg(QVariantMap evMap, void* pUser);

class RSubView :public QWidget
{
	Q_OBJECT

public:
	RSubView(QWidget *parent = 0);
	~RSubView();

	virtual void paintEvent( QPaintEvent * );
	virtual void mouseDoubleClickEvent( QMouseEvent * );
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual void changeEvent(QEvent *);
	QVariantMap ScreenShot();

	enum __enConnectStatus{
		CONNECT_STATUS_CONNECTED,
		CONNECT_STATUS_CONNECTING,
		CONNECT_STATUS_DISCONNECTED,
		CONNECT_STATUS_DISCONNECTING,
	}ConnectStatus;
public:
	void SetLpClient(IDeviceGroupRemotePlayback *m_GroupPlayback);
	int AudioEnabled(bool bEnabled);
	void SetCurConnectState(__enConnectStatus parm);
	void CacheState(QVariantMap evMap);
	void saveCacheImage();
	void SetFoucs(bool flags);
	void setCbpfn(pfnCb cbPro, void* pUser);
	void recMsg(QVariantMap msg);

	static void showSusWnd(bool enabled);
	void destroySusWnd();
signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);

	void connecttingUpdateSig();
	void CacheStateSig(QVariantMap evMap);
	
	void sigValidateFail(QVariantMap vmap);

public:    
	Ui::titleview * ui;
	IDeviceClient *m_LpClient;

	__enConnectStatus _curState;
	int _countConnecting;
	QTimer m_checkTime;
	QTimer m_cacheTime;
public slots:
	void connecttingUpdateSlot();
	void connecttingUpdate();
	void CacheStateSlot(QVariantMap evMap);
	void setProgress(int progress);
	void slCloseSusWnd();
private:
	static bool m_bGlobalAudioStatus;
	static SuspensionWnd *ms_susWnd;
	static QMap<quintptr, QRect> ms_rectMap;
	static bool m_bSuspensionVisable;

	IDeviceGroupRemotePlayback* m_pRemotePlayBack;

	int _curCache;
	QPixmap _cacheBackImage;
	bool _bSaveCacheImage;
	QLabel *_cacheLable;
	QPixmap _ScreenShotImage;
	bool _bIsFocus;

	QPoint m_pressPoint;
	pfnCb m_pcbfn;
	void* m_pUser;
	bool m_bPressed;
private:
	void paintEventNoVideo( QPaintEvent * );
	void paintEventConnecting( QPaintEvent * );
	void paintEventCache(QPaintEvent *);
	void _cacheLableShow();
	bool verify(quint64 mainCode, quint64 subCode);
	void clearOriginRect();
};


#endif // RSUBVIEW_H
