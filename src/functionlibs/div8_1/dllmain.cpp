#include <libpcom.h>
#include "div8_1.h"

EXTERN_C IPcomBase * CreateInstance()
{
	div8_1 * pInstance = new div8_1;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
