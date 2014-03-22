#pragma once
#include <QtGui/QApplication>
#include <QWidget>
#include <QPoint>
class Qqwidget:public QWidget
{
public:
<<<<<<< HEAD
	Qqwidget(QWidget *parent,QString pixdir);
=======
	Qqwidget(QWidget *parent);
>>>>>>> 补充录像提示的文件
	~Qqwidget();
	void paintEvent( QPaintEvent* aEvent );
	virtual void resizeEvent(QResizeEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
<<<<<<< HEAD
	void setNewPos(qreal w,qreal h);
=======
>>>>>>> 补充录像提示的文件
private:
	QPixmap bgpix;
	QPoint dragPosition;
	QWidget *_parent;
<<<<<<< HEAD
	QString _pixdir;
	qreal _width;
	qreal _height;
	
=======
>>>>>>> 补充录像提示的文件
};

