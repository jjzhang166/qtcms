#include <libpcom.h>
#include "ccommonlib.h"
#include <IWebPluginBase.h>

IPcomBase * CreateInstance()
{
	Ccommonlib * pInstance = new Ccommonlib;
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}