#ifndef __INTERFACE_IAUTOSYCTIME_HEAD_FILE_GIDPOEKFD__
#define __INTERFACE_IAUTOSYCTIME_HEAD_FILE_GIDPOEKFD__
#include <libpcom.h>

interface IAutoSycTime : public IPComBase
{
	virtual int SetAutoSycTime(bool bEnabled) = 0;
};

#endif