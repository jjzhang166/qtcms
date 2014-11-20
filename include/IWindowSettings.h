#ifndef __IWINDOW_SETTINGS_HEAD_FILE__
#define __IWINDOW_SETTINGS_HEAD_FILE__

#include "libpcom.h"

interface IWindowSettings : public IPComBase
{
	virtual void setEnableStretch(int uiWnd,bool bEnable) = 0;

	virtual bool getEnableStretch(int uiWnd) = 0;

	virtual void setAllWindowStretch(bool bEnable) = 0;

	virtual void setChannelInWnd(int uiWnd,int nChl) = 0;

	virtual int getChannelInWnd(int nWnd) = 0;
};


#endif