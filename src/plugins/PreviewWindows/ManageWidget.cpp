#include "ManageWidget.h"
#include <QtGui/QApplication>
#include <QGraphicsItem>
#include <QDebug>

ManageWidget::ManageWidget(QWidget *parent):QWidget(parent),
	_recordItem(NULL),
<<<<<<< HEAD
	_audioItem(NULL),
=======
>>>>>>> 补充录像提示的文件
	_widgetForVideo(NULL)
{
	QWidget *parentWidget=(QWidget*)this->parent();
	resize(parentWidget->size());
	QString curDir=QApplication::applicationDirPath();
	curDir.append("/Time-For-Lunch-2.png");
	QPixmap bgpix(curDir);
	QPalette palette;
	palette.setBrush(QPalette::Background, QBrush(QPixmap(curDir)));
	//this->setAutoFillBackground(true);
	//this->setPalette(palette);

	__createWidgetForvideo();
	__createRecordItem();
<<<<<<< HEAD
	__createAudioItem();
	
=======



>>>>>>> 补充录像提示的文件
}


ManageWidget::~ManageWidget(void)
{
	if (NULL!=_widgetForVideo)
	{
		delete _widgetForVideo;
		_widgetForVideo=NULL;
	}
	if (NULL!=_recordItem)
	{
		delete _recordItem;
		_recordItem=NULL;
	}
<<<<<<< HEAD
	if (NULL!=_audioItem)
	{
		delete _audioItem;
		_audioItem=NULL;
	}
=======

>>>>>>> 补充录像提示的文件
}

void ManageWidget::resizeEvent( QResizeEvent *event )
{
	if (NULL!=_widgetForVideo)
	{
		_widgetForVideo->resize(this->size());
	}
	if (NULL!=_recordItem)
	{
<<<<<<< HEAD
		_recordItem->setNewPos(1.5,1);
		_recordItem->resize(this->size()/20);
	}
	if (NULL!=_audioItem)
	{
		_audioItem->setNewPos(this->width()-_recordItem->width()*4,_recordItem->height());
		_audioItem->resize(this->size()/10);
=======
		_recordItem->resize(this->size()/10);
>>>>>>> 补充录像提示的文件
	}
}


QWidget * ManageWidget::GetWidgetForVideo()
{
	return _widgetForVideo;
}

void ManageWidget::timerEvent( QTimerEvent * )
{

}

void ManageWidget::__createWidgetForvideo()
{
	_widgetForVideo=new widgetforvideo(this);
}

void ManageWidget::__createRecordItem()
{
<<<<<<< HEAD
	QString dir=QApplication::applicationDirPath();
	dir.append("/qq.png");
	_recordItem=new Qqwidget(this,dir);
=======
	_recordItem=new Qqwidget(this);
>>>>>>> 补充录像提示的文件
	_recordItem->raise();
	_recordItem->hide();
}

QWidget * ManageWidget::GetRecordItem()
{
	return _recordItem;
}

void ManageWidget::RecordState(bool flag)
{
<<<<<<< HEAD
	if (NULL!=_recordItem&&NULL!=_audioItem)
=======
	if (NULL!=_recordItem)
>>>>>>> 补充录像提示的文件
	{
		if (false==flag)
		{
			_recordItem->hide();
		}else{
			_recordItem->show();
		}
	}
}
<<<<<<< HEAD

void ManageWidget::__createAudioItem()
{
	QString dir=QApplication::applicationDirPath();
	dir.append("/audio.png");
	_audioItem=new Qqwidget(this,dir);
	_audioItem->raise();
	_audioItem->hide();
}
=======
>>>>>>> 补充录像提示的文件
