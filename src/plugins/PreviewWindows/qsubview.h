#ifndef QSUBVIEW_H
#define QSUBVIEW_H

#include <QWidget>

class QSubView : public QWidget
{
	Q_OBJECT

public:
	QSubView(QWidget *parent = 0);
	~QSubView();

	virtual void paintEvent( QPaintEvent * );

	virtual void mouseDoubleClickEvent( QMouseEvent * );
private:
signals:
	void mouseDoubleClick(QWidget *,QMouseEvent *);
};

#endif // QSUBVIEW_H
