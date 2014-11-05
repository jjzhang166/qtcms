#include <libpcom.h>
#include "onvifdevice.h"
IPcomBase * CreateInstance()
{
	onvifDevice *pInstance=new onvifDevice;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}