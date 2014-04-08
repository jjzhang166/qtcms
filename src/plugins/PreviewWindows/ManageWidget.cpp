#include "ManageWidget.h"
#include <QtGui/QApplication>
#include <QGraphicsItem>
#include <QDebug>

ManageWidget::ManageWidget(QWidget *parent):QWidget(parent),
	_recordItem(NULL),
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

}

void ManageWidget::resizeEvent( QResizeEvent *event )
{
	if (NULL!=_widgetForVideo)
	{
		_widgetForVideo->resize(this->size());
	}
	if (NULL!=_recordItem)
	{
		_recordItem->resize(this->size()/10);
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
	_recordItem=new Qqwidget(this);
	_recordItem->raise();
	_recordItem->hide();
}

QWidget * ManageWidget::GetRecordItem()
{
	return _recordItem;
}

void ManageWidget::RecordState(bool flag)
{
	if (NULL!=_recordItem)
	{
		if (false==flag)
		{
			_recordItem->hide();
		}else{
			_recordItem->show();
		}
	}
}
