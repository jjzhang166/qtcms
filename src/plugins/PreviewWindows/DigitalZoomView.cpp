#include "DigitalZoomView.h"
#include <QDebug>
#include <QIcon>
QRect DigitalZoomView::m_tDigitalViewPosition(400,400,500,300);
QRect DigitalZoomView::m_tDoubleClickOldPosition(400,400,500,300);
DigitalZoomView::DigitalZoomView(QFrame *parent):QFrame(parent)
	,m_bIsDrawRect(false)
	,m_bIsDropRect(false)
	,m_bViewIsClose(true)
{
	setWindowFlags(Qt::WindowStaysOnTopHint);
	setWindowFlags(this->windowFlags() &~ (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint));
	QDesktopWidget* desktopWidget = QApplication::desktop();
	m_tDoubleClickMinPosition = desktopWidget->availableGeometry();
	m_nMinWidth=m_tDoubleClickMinPosition.width()/10;
	m_nMinHeight=m_tDoubleClickMinPosition.height()/40;
	m_tDoubleClickMinPosition.setX(m_tDoubleClickMinPosition.width()-m_tDoubleClickMinPosition.width()/10);
	m_tDoubleClickMinPosition.setY(m_tDoubleClickMinPosition.height()-m_tDoubleClickMinPosition.height()/40);
	this->setWindowTitle(tr("Zoom"));

	QString image;
	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);
	image = IniFile.value("background/zoom-icon-image", QVariant("")).toString();
	QString PixPath = sAppPath + image;
	QIcon tWindowIcon(PixPath);
	this->setWindowIcon(tWindowIcon);
}


DigitalZoomView::~DigitalZoomView(void)
{
}

void DigitalZoomView::mousePressEvent( QMouseEvent * event )
{
	m_bIsDrawRect=true;
	bool bXInRect=false;
	bool bYInRect=false;
	if (m_tRectCurrentPoint.x()>m_tRectStartPoint.x())
	{
		if (event->pos().x()>m_tRectStartPoint.x()&&event->pos().x()<m_tRectCurrentPoint.x())
		{
			bXInRect=true;
		}
	}else{
		if (event->pos().x()>m_tRectCurrentPoint.x()&&event->pos().x()<m_tRectStartPoint.x())
		{
			bXInRect=true;
		}
	}
	if (m_tRectCurrentPoint.y()>m_tRectStartPoint.y())
	{
		if (event->pos().y()>m_tRectStartPoint.y()&&event->pos().y()<m_tRectCurrentPoint.y())
		{
			bYInRect=true;
		}
	}else{
		if (event->pos().y()>m_tRectCurrentPoint.y()&&event->pos().y()<m_tRectStartPoint.y())
		{
			bYInRect=true;
		}
	}
	if (bYInRect&&bXInRect)
	{
		m_tRectDropStartPoint.setX(event->pos().x());
		m_tRectDropStartPoint.setY(event->pos().y());
		m_bIsDropRect=true;
	}else{
		m_tRectStartPoint.setX(event->pos().x());
		m_tRectStartPoint.setY(event->pos().y());
	}
}

void DigitalZoomView::mouseReleaseEvent( QMouseEvent * event )
{
	m_bIsDrawRect=false;

	QPoint tStartPoint;
	QPoint tEndPoint;
	if (m_bIsDropRect)
	{
		int nX=event->pos().x()-m_tRectDropStartPoint.x();
		int nY=event->pos().y()-m_tRectDropStartPoint.y();
		m_tRectStartPoint.setX(m_tRectStartPoint.x()+nX);
		m_tRectStartPoint.setY(m_tRectStartPoint.y()+nY);
		m_tRectCurrentPoint.setX(m_tRectCurrentPoint.x()+nX);
		m_tRectCurrentPoint.setY(m_tRectCurrentPoint.y()+nY);
	}else{
		//do nothing
	}
	m_bIsDropRect=false;
}

void DigitalZoomView::mouseMoveEvent( QMouseEvent * event )
{
	if (m_bIsDrawRect)
	{
		QPoint tStartPoint;
		QPoint tEndPoint;
		if (m_bIsDropRect)
		{
			int nX=event->pos().x()-m_tRectDropStartPoint.x();
			int nY=event->pos().y()-m_tRectDropStartPoint.y();
			tStartPoint.setX(m_tRectStartPoint.x()+nX);
			tStartPoint.setY(m_tRectStartPoint.y()+nY);
			tEndPoint.setX(m_tRectCurrentPoint.x()+nX);
			tEndPoint.setY(m_tRectCurrentPoint.y()+nY);
		}else{
			m_tRectCurrentPoint.setX(event->pos().x());
			m_tRectCurrentPoint.setY(event->pos().y());
			tStartPoint=m_tRectStartPoint;
			tEndPoint=m_tRectCurrentPoint;
		}
		emit sgDrawRect(tStartPoint,tEndPoint);
	}
}

void DigitalZoomView::hideEvent( QHideEvent *event )
{
	emit sgHideEvnet();
}

void DigitalZoomView::showEvent( QShowEvent *event )
{
	emit sgShowEvent();
	m_bViewIsClose=false;
}

void DigitalZoomView::closeEvent( QCloseEvent *event )
{
	m_bViewIsClose=true;
	emit sgHideEvnet();
}

void DigitalZoomView::mouseDoubleClickEvent( QMouseEvent *event )
{
	m_tRectCurrentPoint=m_tRectStartPoint;
	emit sgDrawRect(m_tRectStartPoint,m_tRectCurrentPoint);
}

bool DigitalZoomView::getCurrentViewIsClose()
{
	return m_bViewIsClose;
}

QRect DigitalZoomView::getPosition()
{
	return m_tDigitalViewPosition;
}

void DigitalZoomView::resizeEvent( QResizeEvent *event )
{
	if (m_bViewIsClose==false)
	{
		m_tDigitalViewPosition=this->geometry();
	}
}

void DigitalZoomView::moveEvent( QMoveEvent * event)
{
	if (m_bViewIsClose==false)
	{
		m_tDigitalViewPosition=this->geometry();
	}
}
bool DigitalZoomView::event( QEvent * eventt)
{
	if (eventt->type() == QEvent::NonClientAreaMouseButtonDblClick){
		QRect tCurrentPosition=geometry();
		int nCurX=tCurrentPosition.width();
		int nCurY=tCurrentPosition.height();
		if (nCurX<m_nMinWidth+10&&nCurY<m_nMinHeight+10)
		{
			setGeometry(m_tDoubleClickOldPosition);
		}else{
			m_tDoubleClickOldPosition=geometry();
			setGeometry(m_tDoubleClickMinPosition);
		}
	}else{
		//do nothing
	}	
	return QWidget::event(eventt);
}

void DigitalZoomView::clearRectPoint()
{
	QPoint tPoint;
	m_tRectCurrentPoint=tPoint;
	m_tRectStartPoint=tPoint;
	m_tRectDropStartPoint=tPoint;
	m_tRectDropEndPoint=tPoint;
}

void DigitalZoomView::getDigitalViewSize( int &nWidth,int &nHeight )
{
	nWidth=m_tDigitalViewPosition.right()-m_tDigitalViewPosition.left();
	nHeight=m_tDigitalViewPosition.bottom()-m_tDigitalViewPosition.top();
}

void DigitalZoomView::initRectPoint( QPoint tStart,QPoint tEnd )
{
	m_tRectStartPoint=tStart;
	m_tRectCurrentPoint=tEnd;
}

void DigitalZoomView::changeEvent( QEvent *event )
{
	if (event->type()==QEvent::LanguageChange)
	{
		//do something
		translateLanguage();
	}
}

void DigitalZoomView::translateLanguage()
{
	this->setWindowTitle(tr("Zoom"));
}

void DigitalZoomView::paintEvent( QPaintEvent *ev )
{
	Q_UNUSED(ev);
	QPainter p(this);
	QString image;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);

	image = IniFile.value("background/zoom-background-image", QVariant("")).toString();

	QRect rcClient = contentsRect();
	this->geometry().center();
	QPixmap pix;
	QString PixPaht = sAppPath + image;
	pix.load(PixPaht);
	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);
	//±³¾°
	p.drawPixmap(rcClient,pix);
}
