#include <libpcom.h>
#include "BubbleProtocol.h"
#include <IWebPluginBase.h>

IPcomBase * CreateInstance()
{
	BubbleProtocol * pInstance = new BubbleProtocol;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}