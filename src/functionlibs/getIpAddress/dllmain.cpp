#include <libpcom.h>

#include "getipaddress.h"
IPcomBase * CreateInstance()
{
	getIpAddress * pInstance = new getIpAddress;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}