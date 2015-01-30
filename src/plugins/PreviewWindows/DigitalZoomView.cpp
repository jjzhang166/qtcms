#include "DigitalZoomView.h"
#include <QDebug>
#include <QIcon>
QRect DigitalZoomView::m_tDigitalViewPosition(400,400,500,300);
QRect DigitalZoomView::m_tDoubleClickOldPosition(400,400,500,300);
DigitalZoomView::DigitalZoomView(QFrame *parent):QFrame(parent)
	,m_bIsDrawRect(false)
	,m_bIsDropRect(false)
	,m_bViewIsClose(true)
	,m_nViewSizeHisWidth(0)
	,m_nViewSizeHisHeight(0)
{
	setWindowFlags(Qt::WindowStaysOnTopHint);
	
	setWindowFlags(this->windowFlags() &~ (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint));
	QDesktopWidget* desktopWidget = QApplication::desktop();
	m_tDoubleClickMinPosition = desktopWidget->availableGeometry();
	m_tDigitalViewPosition.setX(m_tDoubleClickMinPosition.width()/2-m_tDoubleClickMinPosition.width()/6);
	m_tDigitalViewPosition.setY(m_tDoubleClickMinPosition.height()/2-m_tDoubleClickMinPosition.height()/6);
	m_tDigitalViewPosition.setWidth(m_tDoubleClickMinPosition.width()/3);
	m_tDigitalViewPosition.setHeight(m_tDoubleClickMinPosition.height()/3);
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
	if (m_nViewSizeHisHeight!=this->height()||m_nViewSizeHisWidth!=this->width())
	{
		if (m_nViewSizeHisWidth!=0&&m_nViewSizeHisHeight!=0)
		{
			m_tRectStartPoint.setX(this->width()*m_tRectStartPoint.x()/m_nViewSizeHisWidth);
			m_tRectStartPoint.setY(this->height()*m_tRectStartPoint.y()/m_nViewSizeHisHeight);
			m_tRectCurrentPoint.setX(this->width()*m_tRectCurrentPoint.x()/m_nViewSizeHisWidth);
			m_tRectCurrentPoint.setY(this->height()*m_tRectCurrentPoint.y()/m_nViewSizeHisHeight);
		}
	}
	m_nViewSizeHisWidth=this->width();
	m_nViewSizeHisHeight=this->height();

	if (event->button() == Qt::LeftButton)
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
		int nSetX;
		int nSetY;
		QRect tRect=geometry();
		if (event->pos().x()<0)
		{
			nSetX=0;
		}else if (event->pos().x()>tRect.width())
		{
			nSetX=tRect.width();
		}else{
			nSetX=event->pos().x();
		}
		if (event->pos().y()<0)
		{
			nSetY=0;
		}else if (event->y()>tRect.height())
		{
			nSetY=tRect.height();
		}else{
			nSetY=event->pos().y();
		}
		if (bYInRect&&bXInRect)
		{
			m_tRectDropStartPoint.setX(nSetX);
			m_tRectDropStartPoint.setY(nSetY);
			m_bIsDropRect=true;
		}else{
			m_tRectStartPoint.setX(nSetX);
			m_tRectStartPoint.setY(nSetY);
		}
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
			QRect tRect=geometry();
			int nSetStartX;
			int nSetStartY;
			int nSetEndX;
			int nSetEndY;
			if (m_tRectStartPoint.x()+nX<0)
			{
				nSetStartX=0;
			}else if (m_tRectStartPoint.x()+nX>tRect.width())
			{
				nSetStartX=tRect.width();
			}else{
				nSetStartX=m_tRectStartPoint.x()+nX;
			}
			if (m_tRectStartPoint.y()+nY<0)
			{
				nSetStartY=0;
			}else if (m_tRectStartPoint.y()+nY>tRect.height())
			{
				nSetStartY=tRect.height();
			}else{
				nSetStartY=m_tRectStartPoint.y()+nY;
			}
			if (m_tRectCurrentPoint.x()+nX<0)
			{
				nSetEndX=0;
			}else if (m_tRectCurrentPoint.x()+nX>tRect.width())
			{
				nSetEndX=tRect.width();
			}else{
				nSetEndX=m_tRectCurrentPoint.x()+nX;
			}
			if (m_tRectCurrentPoint.y()+nY<0)
			{
				nSetEndY=0;
			}else if (m_tRectCurrentPoint.y()+nY>tRect.height())
			{
				nSetEndY=tRect.height();
			}else{
				nSetEndY=m_tRectCurrentPoint.y()+nY;
			}
			tStartPoint.setX(nSetStartX);
			tStartPoint.setY(nSetStartY);
			tEndPoint.setX(nSetEndX);
			tEndPoint.setY(nSetEndY);
		}else{
			QRect tRect=geometry();
			int nSetX;
			int nSetY;
			if (event->pos().x()<0)
			{
				nSetX=0;
			}else if (event->pos().x()>tRect.width())
			{
				nSetX=tRect.width();
			}else{
				nSetX=event->pos().x();
			}
			if (event->pos().y()<0)
			{
				nSetY=0;
			}else if (event->pos().y()>tRect.height())
			{
				nSetY=tRect.height();
			}else{
				nSetY=event->pos().y();
			}
			m_tRectCurrentPoint.setX(nSetX);
			m_tRectCurrentPoint.setY(nSetY);
			tStartPoint=m_tRectStartPoint;
			tEndPoint=m_tRectCurrentPoint;
		}
		emit sgDrawRect(tStartPoint,tEndPoint);
	}
}

void DigitalZoomView::hideEvent( QHideEvent *event )
{
	//emit sgHideEvnet();
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
	if (event->button() == Qt::LeftButton)
	{
		m_tRectCurrentPoint=m_tRectStartPoint;
		emit sgDrawRect(m_tRectStartPoint,m_tRectCurrentPoint);
	}
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
		emit sgViewNewPosition(m_tDigitalViewPosition,m_nViewSizeHisWidth,m_nViewSizeHisHeight);
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
			//还原 矩形原始坐标
		}else{
			m_tDoubleClickOldPosition=geometry();
			setGeometry(m_tDoubleClickMinPosition);
			qDebug()<<__FUNCTION__<<__LINE__<<m_tRectStartPoint<<m_tRectCurrentPoint;
			//映射矩形 到窗口的 新坐标
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
	m_nViewSizeHisHeight=this->height();
	m_nViewSizeHisWidth=this->width();
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
	//背景
	p.drawPixmap(rcClient,pix);
}

void DigitalZoomView::setParentWnd( QWidget *wnd )
{
	setParent(wnd);
	setWindowFlags(Qt::Window);
	setWindowFlags(this->windowFlags() &~ (Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint));
}

void DigitalZoomView::ViewNewPosition( QRect tRect,int nWidth,int nHeight )
{
	if (m_tRectStartPoint!=m_tRectCurrentPoint)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<this<<m_tRectStartPoint<<m_tRectCurrentPoint<<nWidth<<nHeight;
		int nViewNewWidth=tRect.width();
		int nViewNewHeight=tRect.height();
		QPoint tNewStartPoint;
		QPoint tNewEndPoint;
		if (nWidth!=0||nHeight!=0)
		{
			tNewEndPoint.setX(nViewNewWidth*m_tRectCurrentPoint.x()/nWidth);
			tNewEndPoint.setY(nViewNewHeight*m_tRectCurrentPoint.y()/nHeight);
			tNewStartPoint.setX(nViewNewWidth*m_tRectStartPoint.x()/nWidth);
			tNewStartPoint.setY(nViewNewHeight*m_tRectStartPoint.y()/nHeight);
			//emit sgDrawRect(tNewStartPoint,tNewEndPoint);
		}else{

		}
	}else{

	}
}



