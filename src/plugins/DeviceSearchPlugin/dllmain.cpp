#include <libpcom.h>
#include "DeviceSearchPlugin.h"

IPcomBase * CreateInstance()
{
	DeviceSearchPlugin * pInstance = new DeviceSearchPlugin;
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}