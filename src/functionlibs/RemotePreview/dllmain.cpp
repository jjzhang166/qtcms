#include <libpcom.h>
#include "RemotePreview.h"
#include <IWebPluginBase.h>

IPcomBase * CreateInstance()
{
	RemotePreview * pInstance = new RemotePreview;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}