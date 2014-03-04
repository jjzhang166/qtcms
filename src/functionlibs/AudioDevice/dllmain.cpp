#include <libpcom.h>
#include "AudioDevice.h"

IPcomBase * CreateInstance()
{
	AudioDevice * pInstance = new AudioDevice;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}