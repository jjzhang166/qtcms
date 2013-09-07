#include <libpcom.h>
#include <IWebPluginBase.h>
#include "ffmpegvideoplugin.h"

IPcomBase * CreateInstance()
{
	ffmpegVideoplugin * pInstance = new ffmpegVideoplugin;
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}