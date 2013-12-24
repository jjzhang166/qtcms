#include <libpcom.h>
#include "BubbleProtocol.h"
#include <IWebPluginBase.h>

IPcomBase * CreateInstance()
{
	qDebug("CreateInstance:RemotePreview%x");
	BubbleProtocol * pInstance = new BubbleProtocol;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}