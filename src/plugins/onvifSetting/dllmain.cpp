#include <libpcom.h>
#include <IWebPluginBase.h>
#include "onvifsetting.h"
EXTERN_C IPcomBase * CreateInstance()
{
	onvifSetting * pInstance = new onvifSetting();
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}