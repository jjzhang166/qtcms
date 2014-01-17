#include <libpcom.h>
#include <IWebPluginBase.h>
#include "qtqwebviewplugin.h"

IPcomBase * CreateInstance()
{
	QtQWebViewPlugin * pInstance = new QtQWebViewPlugin();
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}