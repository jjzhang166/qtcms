#include <libpcom.h>
#include "LocalBackupPlugin.h"

IPcomBase * CreateInstance()
{
	LocalBackupPlugin * pInstance = new LocalBackupPlugin();
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}