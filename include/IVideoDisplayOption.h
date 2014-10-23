#ifndef __IVIDEODISPLAYOPTION_HEAD_FILE__
#define __IVIDEODISPLAYOPTION_HEAD_FILE__

#include <libpcom.h>
#include <QtGui/QWidget>

interface IVideoDisplayOption : public IPComBase
{
	virtual void enableWindowStretch(QWidget * window,bool bEnable) = 0;

	virtual bool getWindowStretchStatus(QWidget * window) = 0;
};

#endif