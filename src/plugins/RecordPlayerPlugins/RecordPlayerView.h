#ifndef _RECORDPLAYERVIEW_HEAD_FILE_H_
#define _RECORDPLAYERVIEW_HEAD_FILE_H_

#include <QWidget>
#include "ILocalPlayer.h"

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

	void setLocalPlayer(ILocalPlayer* pPlayer);
	int AudioEnabled(bool bEnabled);
	void ScreenShot();
signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);
private:
	static bool m_bGlobalAudioStatus;

	ILocalPlayer* m_pLocalPlayer;
	QPixmap _ScreenShotImage;
};


#endif

