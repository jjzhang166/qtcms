#ifndef _RECORDPLAYERVIEW_HEAD_FILE_H_
#define _RECORDPLAYERVIEW_HEAD_FILE_H_

#include <QWidget>
#include "ILocalPlayerEx.h"
#include <QVariantMap>
#include <QtGui/QMenu>
#include <QtCore/QTranslator>
#include <QPoint>
#include <QMap>
#include <ILocalSetting.h>
#include "suspensionwnd.h"

void cbReciveMsg(QVariantMap evMap, void* pUser);

class RecordPlayerView :
	public QWidget
{
	Q_OBJECT

public:
	RecordPlayerView(QWidget *parent = 0);
	~RecordPlayerView();

	virtual void paintEvent( QPaintEvent * );
	virtual void mouseDoubleClickEvent( QMouseEvent * );
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);

	void setLocalPlayer(ILocalPlayerEx* pPlayer);
	int AudioEnabled(bool bEnabled);
	QVariantMap ScreenShot();
	void SetFocus(bool flags);
	void recMsg(QVariantMap msg);
	void setPlayingFlag(bool bPlaying);

	static void setPlayStatus(int status);
	static void showSusWnd(bool enabled);
	static void destroySusWnd();
signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);

public slots:
	void slSuitForWindow(bool checked);
	void slCloseSusWnd();
protected:
	virtual void changeEvent( QEvent * );

	
private:
	static bool m_bGlobalAudioStatus;
	static SuspensionWnd *ms_susWnd;
	static int ms_playStatus;
	static QMap<quintptr, QRect> ms_rectMap;

	ILocalPlayerEx* m_pLocalPlayer;
	QPixmap _ScreenShotImage;
	bool _bIsFocus;
	QAction * m_pWindowsStretchAction;
	QMenu m_WindowMenu;
	
	QPoint m_pressPoint;
// 	QRect m_drawRect;
	bool m_bPlaying;
	bool m_bPressed;
// 	bool m_bHasZoom;
};


#endif

