#include <libpcom.h>
#include "ccommonlib.h"


IPcomBase * CreateInstance()
{
	Ccommonlib * pInstance = new Ccommonlib;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}