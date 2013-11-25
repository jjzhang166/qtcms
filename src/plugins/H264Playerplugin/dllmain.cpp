#include <libpcom.h>
#include <IWebPluginBase.h>
#include "H264Playerplugin.h"

IPcomBase * CreateInstance()
{
	H264Playerplugin * pInstance = new H264Playerplugin;
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}