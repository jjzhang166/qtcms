#ifndef _RECORDPLAYERVIEW_HEAD_FILE_H_
#define _RECORDPLAYERVIEW_HEAD_FILE_H_

#include <QWidget>


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

signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
	void SetCurrentWindSignl(QWidget *);
};


#endif

