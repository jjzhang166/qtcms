#include <libpcom.h>
#include <IWebPluginBase.h>
#include "previewwindows.h"

EXTERN_C IPcomBase * CreateInstance()
{
	PreviewWindows * pInstance = new PreviewWindows();
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
