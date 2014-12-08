#ifndef __IDEVICESEARCH_HEAD_FILE_HVPASDFSDFPG87WDHFVC8SA__
#define __IDEVICESEARCH_HEAD_FILE_HVPASDFSDFPG87WDHFVC8SA__
#include <libpcom.h>
#include <IEventRegister.h>

interface IAutoSearchDevice : public IPComBase
{
	virtual int autoSearchStart() = 0;
	virtual int autoSearchStop() = 0;
	virtual IEventRegister * QueryEventRegister() = 0;

};

#endif