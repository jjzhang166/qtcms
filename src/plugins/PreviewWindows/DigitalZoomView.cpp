#include "DigitalZoomView.h"
#include <QDebug>

DigitalZoomView::DigitalZoomView(QWidget *parent):QWidget(parent)
	,m_bIsDrawRect(false)
	,m_bIsDropRect(false)
	,m_bViewIsClose(true)
{
	setWindowFlags(Qt::WindowStaysOnTopHint);
}


DigitalZoomView::~DigitalZoomView(void)
{
}

void DigitalZoomView::mousePressEvent( QMouseEvent * event )
{
	qDebug()<<__FUNCTION__<<__LINE__<<"mousePressEvent";
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
	qDebug()<<__FUNCTION__<<__LINE__<<"mouseReleaseEvent";
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
		qDebug()<<__FUNCTION__<<__LINE__<<"mouseMoveEvent";
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
