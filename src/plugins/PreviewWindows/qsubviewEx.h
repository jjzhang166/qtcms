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
int cbStateChangeEx(QString evName,QVariantMap evMap,void*pUser);
int cbRecordStateEx(QString evName,QVariantMap evMap,void*pUser);

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

	//手动录像
	int startRecord();
	int stopRecord();
	//音频 播放
	int setPlayWnd(int nwnd);//unnecessary
	int setVolume(unsigned int uiPersent);
	int audioEnabled(bool bEnable);
	//截屏
	QVariantMap screenShot();
	//云台控制
	int openPTZ(int ncmd,int nspeed);
	int closePTZ(int ncmd);
	//加载语言
	void loadLanguage(QString tags);
	//设置当前焦点窗口
	void setCurrentFocus(bool flags);
	void setDataBaseFlush();
public:
	//回调函数
	int cbCStateChange(QVariantMap evMap);
	int cbCRecordState(QVariantMap evMap);

public slots:
	void slbackToMainThread(QVariantMap evMap);
	void slmouseMenu();
	void slswitchStreamEx();
	void slclosePreview();
	void slMenRecorder();
signals:
	void sgbackToMainThread(QVariantMap evMap);
	void sgmouseDoubleClick(QWidget *,QMouseEvent*);
	void sgmousePressEvent(QWidget*,QMouseEvent*);
	void sgmouseLeftClick(QWidget*,QMouseEvent *);
	void sgrecordState(bool);
	void sgmouseMenu();
	void sgconnectStatus(QVariantMap,QWidget *);
private:
	void paintEventConnected(QPaintEvent *ev);
	void paintEventDisconnected(QPaintEvent *ev);
	void paintEventConnecting(QPaintEvent *ev);
	void paintEventDisconnecting(QPaintEvent *ev);
	tagDeviceInfo getDeviceInfo();
	QString getLanguageInfo(QString tags);
	void translateLanguage();
	bool getAutoRecordStatus();
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
	tagDeviceInfo m_tDeviceInfo;
	QTranslator *m_pTtanslator;
	int m_nConnectingCount;
	QTimer m_tConnectingTimer;
};

