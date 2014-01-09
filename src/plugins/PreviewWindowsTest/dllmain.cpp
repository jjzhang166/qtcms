#include <libpcom.h>
#include <IWebPluginBase.h>
#include "previewwindowstest.h"

IPcomBase * CreateInstance()
{
	PreviewWindowTest * pInstance = new PreviewWindowTest();
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}