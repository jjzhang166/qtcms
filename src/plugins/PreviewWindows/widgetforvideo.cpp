#include "widgetforvideo.h"
#include <QtGui/QApplication>
#include <QDebug>
#include <QPainter>
widgetforvideo::widgetforvideo(QWidget *parent):QWidget(parent)
{
	QWidget *parentWidget=(QWidget*)this->parent();
	resize(parentWidget->size());

	//QPalette pal = palette();
	//pal.setColor( QPalette::Background, QColor( 0x00,0xff,0x00,0x00 ) );

	//QWidget::setPalette( pal );
	//QWidget::setAttribute( Qt::WA_TranslucentBackground, true );
	//QWidget::setWindowOpacity( 0.1);

	//QString curDir=QApplication::applicationDirPath();
	//curDir.append("/Time-For-Lunch-2.png");
	//QPixmap bgpix(curDir);
	//this->setAutoFillBackground(true);
	//QPalette palette;
	//palette.setBrush(QPalette::Background, QBrush(QPixmap(curDir)));
	//this->setPalette(palette);

}


widgetforvideo::~widgetforvideo(void)
{
}

void widgetforvideo::paintEvent( QPaintEvent *aEvent )
{

}


