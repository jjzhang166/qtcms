#pragma once
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
class DigitalZoomView:public QWidget
{
	Q_OBJECT
public:
	DigitalZoomView(QWidget *parent=0);
	~DigitalZoomView(void);
public:
	virtual void	mousePressEvent ( QMouseEvent * event );
	virtual void	mouseReleaseEvent ( QMouseEvent * event );
	virtual void	mouseMoveEvent ( QMouseEvent * event );
	virtual void	hideEvent(QHideEvent *event);
signals:
	void sgDrawRect(QPoint tStartPoint,QPoint tEndPoint);
	void sgHideEvnet();
private:
	bool m_bIsDrawRect;
	QPoint m_tRectStartPoint;
	QPoint m_tRectCurrentPoint;
};

