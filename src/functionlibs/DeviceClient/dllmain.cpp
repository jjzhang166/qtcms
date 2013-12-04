#include <libpcom.h>
#include "deviceclient.h"
#include <IWebPluginBase.h>

IPcomBase * CreateInstance()
{
	DeviceClient *pInstance=new DeviceClient;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}