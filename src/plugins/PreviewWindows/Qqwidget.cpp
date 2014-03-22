#include "Qqwidget.h"
#include <QBitmap>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QResizeEvent>
Qqwidget::Qqwidget(QWidget *parent,QString pixdir):QWidget(parent),_pixdir(pixdir)

{
	setWindowFlags(Qt::FramelessWindowHint);
	QPalette p=palette();
	p.setColor(QPalette::WindowText,Qt::green);
	setPalette(p);
	

	bgpix.load(pixdir,0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither); 
	resize(parent->size()/15);

	setMask(QBitmap(bgpix.mask())); 
	_parent=parent;
}


Qqwidget::~Qqwidget(void)
{
}

void Qqwidget::paintEvent( QPaintEvent* aEvent )
{
	QPainter painter(this);  
	QPixmap newbgpix=bgpix.scaled(this->width(),this->height());
	painter.drawPixmap(0,0,newbgpix);  

}

void Qqwidget::resizeEvent( QResizeEvent *event )
{
	QPixmap newbgpix=bgpix.scaled(this->width(),this->height());
	setMask(QBitmap(newbgpix.mask())); 
	move(_parent->width()-this->width()*_width,this->height());

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

void Qqwidget::setNewPos( qreal w,qreal h )
{
	_width=w;
	_height=h;
}
