#ifndef __IWINDOW_DIV_MODE_9SU0A8YGAS70V0SAY__
#define __IWINDOW_DIV_MODE_9SU0A8YGAS70V0SAY__

#include "libpcom.h"
#include <QtGui/QWidget>

interface IWindowDivMode : public IPComBase
{
	virtual setSubWindows(QWidget * windows,int count) = 0;
	virtual setParentWindow(QWidget * parent) = 0;
};

#endif