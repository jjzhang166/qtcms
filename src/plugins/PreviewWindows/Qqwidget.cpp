#include "Qqwidget.h"
#include <QBitmap>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QResizeEvent>
Qqwidget::Qqwidget(QWidget *parent):QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint);
	QPalette p=palette();
	p.setColor(QPalette::WindowText,Qt::green);
	setPalette(p);
	
	QString dir=QApplication::applicationDirPath();
	dir.append("/qq.png");
	bgpix.load(dir,0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither); 
	resize(bgpix.size());  
	setMask(QBitmap(bgpix.mask())); 
	_parent=parent;
}


Qqwidget::~Qqwidget(void)
{
}

void Qqwidget::paintEvent( QPaintEvent* aEvent )
{
	QPainter painter(this);  
	QString dir=QApplication::applicationDirPath();
	dir.append("/qq.png");
	painter.drawPixmap(0,0,QPixmap(dir).scaled(this->size()/2));  
}

void Qqwidget::resizeEvent( QResizeEvent *event )
{
	QPixmap newbgpix=bgpix.scaled(event->size()/2);
	setMask(QBitmap(newbgpix.mask())); 
	QSize parentSize=_parent->size();
	move(parentSize.width()-this->width(),this->height());
}

void Qqwidget::mousePressEvent( QMouseEvent * event)
{
	if (event->button()==Qt::LeftButton)
	{
		dragPosition=event->globalPos()-this->geometry().topLeft();
		event->accept();
	}
	if (event->button()==Qt::RightButton)
	{
		close();
	}
}

void Qqwidget::mouseMoveEvent( QMouseEvent *event )
{
	if (event->buttons()&Qt::LeftButton)
	{
		/*move(event->globalPos()-dragPosition);	*/
		event->accept();
	}
}
