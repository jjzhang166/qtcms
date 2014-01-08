#include <libpcom.h>
#include <IWebPluginBase.h>
#include "RecordPlayerPlugins.h"

IPcomBase * CreateInstance()
{
	RecordPlayerPlug * pInstance = new RecordPlayerPlug();
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}