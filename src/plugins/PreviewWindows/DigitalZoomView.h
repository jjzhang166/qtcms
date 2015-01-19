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
	virtual void	mouseDoubleClickEvent(QMouseEvent *event);
	virtual void	mouseMoveEvent ( QMouseEvent * event );
	virtual void	hideEvent(QHideEvent *event);
	virtual void	showEvent(QShowEvent *event);
	virtual void	closeEvent(QCloseEvent *event);
	bool getCurrentViewIsClose();
signals:
	void sgDrawRect(QPoint tStartPoint,QPoint tEndPoint);
	void sgHideEvnet();
	void sgShowEvent();
private:
	bool m_bIsDrawRect;
	bool m_bIsDropRect;
	QPoint m_tRectStartPoint;
	QPoint m_tRectCurrentPoint;
	QPoint m_tRectDropStartPoint;
	QPoint m_tRectDropEndPoint;
	bool m_bViewIsClose;
};

