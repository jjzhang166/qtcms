#ifndef __IWINDOW_DIV_MODE_9SU0A8YGAS70V0SAY__
#define __IWINDOW_DIV_MODE_9SU0A8YGAS70V0SAY__

#include "libpcom.h"
#include <QtGui/QWidget>
#include <QtGui/QResizeEvent>

interface IWindowDivMode : public IPComBase
{
	virtual void setSubWindows(	QList<QWidget *> windows,int count) = 0;
	virtual void setParentWindow(QWidget * parent) = 0;
	virtual void flush() = 0;
	virtual void parentWindowResize(QResizeEvent *ev) = 0;
	virtual void subWindowDblClick(QWidget *subWindow,QMouseEvent * ev) = 0;
	virtual void nextPage() = 0;
	virtual void prePage() = 0;
	virtual int getCurrentPage() = 0;
	virtual int getPages() = 0;
	virtual QString getModeName() = 0;
};

#endif