#include <libpcom.h>
#include "RemoteBackupPlugin.h"

IPcomBase * CreateInstance()
{
	RemoteBackupPlugin * pInstance = new RemoteBackupPlugin;
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}