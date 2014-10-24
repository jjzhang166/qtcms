#ifndef _RECORDPLAYERVIEW_HEAD_FILE_H_
#define _RECORDPLAYERVIEW_HEAD_FILE_H_

#include <QWidget>
#include "ILocalPlayerEx.h"
#include <QVariantMap>
#include <QtGui/QMenu>
#include <QtCore/QTranslator>
#include <ILocalSetting.h>
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

	void setLocalPlayer(ILocalPlayerEx* pPlayer);
	int AudioEnabled(bool bEnabled);
	QVariantMap ScreenShot();
	void SetFocus(bool flags);
signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);

public slots:
	void slSuitForWindow(bool checked);

protected:
	virtual void changeEvent( QEvent * );

	
private:
	static bool m_bGlobalAudioStatus;

	ILocalPlayerEx* m_pLocalPlayer;
	QPixmap _ScreenShotImage;
	bool _bIsFocus;
	QAction * m_pWindowsStretchAction;
	QMenu m_WindowMenu;
};


#endif

