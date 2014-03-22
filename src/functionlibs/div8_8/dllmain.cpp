#include <libpcom.h>
#include "div8_8.h"

EXTERN_C IPcomBase * CreateInstance()
{
	div8_8 * pInstance = new div8_8;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
