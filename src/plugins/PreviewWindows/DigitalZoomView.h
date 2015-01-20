#pragma once
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QFrame>
#include <QEvent>
#include <QDesktopWidget>
#include <QApplication>
class DigitalZoomView:public QFrame
{
	Q_OBJECT
public:
	DigitalZoomView(QFrame *parent=0);
	~DigitalZoomView(void);
public:
	virtual void	mousePressEvent ( QMouseEvent * event );
	virtual void	mouseReleaseEvent ( QMouseEvent * event );
	virtual void	mouseDoubleClickEvent(QMouseEvent *event);
	virtual void	mouseMoveEvent ( QMouseEvent * event );
	virtual void	hideEvent(QHideEvent *event);
	virtual void	showEvent(QShowEvent *event);
	virtual void	closeEvent(QCloseEvent *event);
	virtual void	resizeEvent(QResizeEvent *event);
	virtual void	moveEvent(QMoveEvent *);
	virtual bool	event ( QEvent * event ) ;
	bool getCurrentViewIsClose();
	QRect getPosition();
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
	static QRect m_tDigitalViewPosition;
	QRect m_tDoubleClickMinPosition;
	static QRect m_tDoubleClickOldPosition;
	int m_nMinWidth;
	int m_nMinHeight;
};

