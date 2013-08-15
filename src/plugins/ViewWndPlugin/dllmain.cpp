#include <libpcom.h>
#include <IWebPluginBase.h>
#include "viewwndplugin.h"

IPcomBase * CreateInstance()
{
	ViewWndPlugin * pInstance = new ViewWndPlugin;
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}