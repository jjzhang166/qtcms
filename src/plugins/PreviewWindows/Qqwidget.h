#pragma once
#include <QtGui/QApplication>
#include <QWidget>
#include <QPoint>
class Qqwidget:public QWidget
{
public:
	Qqwidget(QWidget *parent);
	~Qqwidget();
	void paintEvent( QPaintEvent* aEvent );
	virtual void resizeEvent(QResizeEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
private:
	QPixmap bgpix;
	QPoint dragPosition;
	QWidget *_parent;
};

