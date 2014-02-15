#include <libpcom.h>

#include "ipcdeviceclient.h"

IPcomBase * CreateInstance()
{
	IpcDeviceClient * pInstance = new IpcDeviceClient;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}