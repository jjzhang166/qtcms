#include <libpcom.h>
#include "div7_7.h"

EXTERN_C IPcomBase * CreateInstance()
{
	div7_7 * pInstance = new div7_7;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
