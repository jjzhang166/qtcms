#include "DigitalZoomView.h"
#include <QDebug>

DigitalZoomView::DigitalZoomView(QWidget *parent):QWidget(parent)
	,m_bIsDrawRect(false)
{
}


DigitalZoomView::~DigitalZoomView(void)
{
}

void DigitalZoomView::mousePressEvent( QMouseEvent * event )
{
	qDebug()<<__FUNCTION__<<__LINE__<<"mousePressEvent";
	m_bIsDrawRect=true;
	m_tRectStartPoint.setX(event->pos().x());
	m_tRectStartPoint.setY(event->pos().y());
}

void DigitalZoomView::mouseReleaseEvent( QMouseEvent * event )
{
	qDebug()<<__FUNCTION__<<__LINE__<<"mouseReleaseEvent";
	m_bIsDrawRect=false;
}

void DigitalZoomView::mouseMoveEvent( QMouseEvent * event )
{
	if (m_bIsDrawRect)
	{
		qDebug()<<__FUNCTION__<<__LINE__<<"mouseMoveEvent";
		m_tRectCurrentPoint.setX(event->pos().x());
		m_tRectCurrentPoint.setY(event->pos().y());
		//qDebug()<<__FUNCTION__<<__LINE__<<m_tRectStartPoint<<m_tRectCurrentPoint;
		emit sgDrawRect(m_tRectStartPoint,m_tRectCurrentPoint);
	}
}

void DigitalZoomView::hideEvent( QHideEvent *event )
{
	emit sgHideEvnet();
}
