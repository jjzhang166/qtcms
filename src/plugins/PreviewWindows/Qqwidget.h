#pragma once
#include <QtGui/QApplication>
#include <QWidget>
#include <QPoint>
class Qqwidget:public QWidget
{
public:
	Qqwidget(QWidget *parent,QString pixdir);
	~Qqwidget();
	void paintEvent( QPaintEvent* aEvent );
	virtual void resizeEvent(QResizeEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	void setNewPos(qreal w,qreal h);
private:
	QPixmap bgpix;
	QPoint dragPosition;
	QWidget *_parent;
	QString _pixdir;
	qreal _width;
	qreal _height;
	
};

