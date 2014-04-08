#ifndef _RECORDPLAYERVIEW_HEAD_FILE_H_
#define _RECORDPLAYERVIEW_HEAD_FILE_H_

#include <QWidget>
#include <QMenu>
#include <QAction>
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

	void setAudioHint(QString&);
	void setLocalPlayer(ILocalPlayer* pPlayer);
signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);
	void RMousePressMenu();
	void ChangeAudioHint(QString, RecordPlayerView*);
public slots:
	void OnOpenAudio();
	void OnRMousePressMenu();
private:
	QMenu m_rMousePressMenu;
	QAction *m_ActionOpenAudio;

	static RecordPlayerView* m_pCurView;
	static bool m_bIsAudioOpen;

	ILocalPlayer* m_pLocalPlayer;
};


#endif

