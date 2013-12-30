#include <libpcom.h>
#include <IWebPluginBase.h>
#include "RemotePlaybackPlugin.h"

IPcomBase * CreateInstance()
{
	RemotePlaybackPlug * pInstance = new RemotePlaybackPlug();
	IPcomBase * pBase = static_cast<IWebPluginBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}