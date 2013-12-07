#include <libpcom.h>
#include "RemotePreview.h"
#include <IWebPluginBase.h>

IPcomBase * CreateInstance()
{
	qDebug("CreateInstance:RemotePreview%x");
	RemotePreview * pInstance = new RemotePreview;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}