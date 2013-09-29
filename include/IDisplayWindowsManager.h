#ifndef __DISPLAY_WINDOWS_MANAGER_JS8DVG81234ENUSDV98A__
#define __DISPLAY_WINDOWS_MANAGER_JS8DVG81234ENUSDV98A__
#include <libpcom.h>

interface IDisplayWindowsManager : public IPComBase
{
	virtual void nextPage() = 0;
	virtual void prePage() = 0;
	virtual int getCurrentPage() = 0;
	virtual int getPages() = 0;
	virtual int setDivMode(QString divModeName) = 0;
};


#endif