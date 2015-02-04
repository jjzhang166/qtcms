#include <libpcom.h>
#include "ScreenShotSearchPlugin.h"

IPcomBase * CreateInstance()
{
	ScreenShotSearchPlugin * pInstance = new ScreenShotSearchPlugin();
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}