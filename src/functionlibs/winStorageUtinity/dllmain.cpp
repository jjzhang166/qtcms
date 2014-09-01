#include <libpcom.h>
#include "winstorageutinity.h"

IPcomBase * CreateInstance()
{
	winStorageUtinity * pInstance = new winStorageUtinity;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}