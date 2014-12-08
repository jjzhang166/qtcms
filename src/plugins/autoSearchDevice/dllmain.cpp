#include <libpcom.h>
#include <IWebPluginBase.h>
#include "autoSearchDevicePlugin.h"

IPcomBase * CreateInstance()
{
	autoSearchDevicePlugin * pInstance = new autoSearchDevicePlugin();
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}