#include "Qqwidget.h"
#include <QBitmap>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QResizeEvent>
<<<<<<< HEAD
Qqwidget::Qqwidget(QWidget *parent,QString pixdir):QWidget(parent),_pixdir(pixdir)
=======
Qqwidget::Qqwidget(QWidget *parent):QWidget(parent)
>>>>>>> 补充录像提示的文件
{
	setWindowFlags(Qt::FramelessWindowHint);
	QPalette p=palette();
	p.setColor(QPalette::WindowText,Qt::green);
	setPalette(p);
	
<<<<<<< HEAD

	bgpix.load(pixdir,0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither); 
	resize(parent->size()/15);
=======
	QString dir=QApplication::applicationDirPath();
	dir.append("/qq.png");
	bgpix.load(dir,0,Qt::AvoidDither|Qt::ThresholdDither|Qt::ThresholdAlphaDither); 
	resize(bgpix.size());  
>>>>>>> 补充录像提示的文件
	setMask(QBitmap(bgpix.mask())); 
	_parent=parent;
}


Qqwidget::~Qqwidget(void)
{
}

void Qqwidget::paintEvent( QPaintEvent* aEvent )
{
	QPainter painter(this);  
<<<<<<< HEAD
	QPixmap newbgpix=bgpix.scaled(this->width(),this->height());
	painter.drawPixmap(0,0,newbgpix);  
=======
	QString dir=QApplication::applicationDirPath();
	dir.append("/qq.png");
	painter.drawPixmap(0,0,QPixmap(dir).scaled(this->size()/2));  
>>>>>>> 补充录像提示的文件
}

void Qqwidget::resizeEvent( QResizeEvent *event )
{
<<<<<<< HEAD
	QPixmap newbgpix=bgpix.scaled(this->width(),this->height());
	setMask(QBitmap(newbgpix.mask())); 
	move(_parent->width()-this->width()*_width,this->height());
=======
	QPixmap newbgpix=bgpix.scaled(event->size()/2);
	setMask(QBitmap(newbgpix.mask())); 
	QSize parentSize=_parent->size();
	move(parentSize.width()-this->width(),this->height());
>>>>>>> 补充录像提示的文件
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
<<<<<<< HEAD

void Qqwidget::setNewPos( qreal w,qreal h )
{
	_width=w;
	_height=h;
}
=======
>>>>>>> 补充录像提示的文件
