#pragma once
#include <QWidget>
#include <QVariantMap>
#include "QSubviewRun.h"
#include <ManageWidget.h>
#include <QDebug>
#include <QEventLoop>
#include <QMouseEvent>
#include <QMenu>
#include <QTranslator>
#include <QPixmap>
#include <QPen>
#include <QPainter>
#include <QTimer>
#include "IUserManagerEx.h"

int cbStateChangeEx(QString evName,QVariantMap evMap,void*pUser);
int cbRecordStateEx(QString evName,QVariantMap evMap,void*pUser);
int cbConnectRefuseEx(QString evName,QVariantMap evMap,void*pUser);
int cbAuthorityEx(QString evName,QVariantMap evMap,void*pUser);
class qsubviewEx:public QWidget
{
	Q_OBJECT
public:
	qsubviewEx(QWidget *parent=0);
	~qsubviewEx();
	
public:
	virtual void paintEvent(QPaintEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void changeEvent(QEvent *);

	int openPreview(int chlId);
	int closePreview();

	int switchStream();

	int getCurrentConnectStatus();
	int setDevChannelInfo(int chlId);//unnecessary
	QVariantMap getWindowInfo();

	// 初始化部分,必须在调用完其他的参数设置接口后调用initAfterConstructor
	void setCurWindId(int nWindId);
	void initAfterConstructor();

	//手动录像
	int startRecord();
	int stopRecord();
	int getRecordStatus();
	//音频 播放
	int setPlayWnd(int nwnd);//unnecessary
	int setVolume(unsigned int uiPersent);
	int audioEnabled(bool bEnable);
	//截屏
	QVariantMap screenShot();
	//全屏
	int SetFullScreen(bool bFullScreen);
	//云台控制
	int openPTZ(int ncmd,int nspeed);
	int closePTZ(int ncmd);
	//加载语言
	void loadLanguage(QString tags);
	//设置当前焦点窗口
	void setCurrentFocus(bool flags);
	void setDataBaseFlush();

	bool getDigtalViewIsClose();
	void deInitDigtalView();
	bool isSuitForDigitalZoom();
	void showDigitalView();
	void closeDigitalView();
public:
	//回调函数
	int cbCStateChange(QVariantMap evMap);
	int cbCRecordState(QVariantMap evMap);
	int cbCConnectRefuse(QVariantMap evMap);
	int cbCAuthority(QVariantMap evMap);
public slots:
	void slbackToMainThread(QVariantMap evMap);
	void slmouseMenu();
	void slswitchStreamEx();
	void slclosePreview();
	void slMenRecorder();
	void slbackToManiWnd();
	// 拉伸窗口
	void enableStretch(bool bEnable);
	void enableStretchEx(bool bEnable);
signals:
	void sgbackToMainThread(QVariantMap evMap);
	void sgmouseDoubleClick(QWidget *,QMouseEvent*);
	void sgmousePressEvent(QWidget*,QMouseEvent*);
	void sgmouseLeftClick(QWidget*,QMouseEvent *);
	void sgrecordState(bool);
	void sgmouseMenu();
	void sgconnectStatus(QVariantMap,QWidget *);
	void sgconnectRefuse(QVariantMap,QWidget *);
	void sgAuthority(QVariantMap,QWidget *);
	void sgbackToMainWnd();
	void sgVerify(QVariantMap vmap);
	void sgShutDownDigtalZoom();
private:
	void paintEventConnected(QPaintEvent *ev);
	void paintEventDisconnected(QPaintEvent *ev);
	void paintEventConnecting(QPaintEvent *ev);
	void paintEventDisconnecting(QPaintEvent *ev);
	tagDeviceInfo getDeviceInfo();
	QString getLanguageInfo(QString tags);
	void translateLanguage();
	int verify(qint64 mainCode, qint64 subCode);
private:
	typedef enum __tagConnectStatus{
		STATUS_CONNECTED,
		STATUS_CONNECTING,
		STATUS_DISCONNECTED,
		STATUS_DISCONNECTING,
	}tagConnectStatus;
	tagConnectStatus m_tCurConnectStatus;
	tagConnectStatus m_tHistoryConnectStatus;
	QSubviewRun m_sSubviewRun;
	ManageWidget *m_pManageWidget;
	bool m_bIsFocus;
	bool m_bIsRecording;
	QMenu m_mRightMenu;
	QAction *m_pClosePreviewAction;
	QAction *m_pSwitchStreamAciton;
	QAction *m_pRecorderAction;
	QAction *m_pBackMainViewAction;
	QAction * m_pStreachVideo;
	tagDeviceInfo m_tDeviceInfo;
	QTranslator *m_pTtanslator;
	int m_nConnectingCount;
	QTimer m_tConnectingTimer;
	static bool ms_bIsFullScreen;
	int m_nWindowIndex;
	bool m_bStretch; // 在对象内暂存状态，与配置中的状态一致，避免在调用菜单和其他操作的时候需要频繁读取配置
	int m_chlId;
	
};

