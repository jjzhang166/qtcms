#include <libpcom.h>
#include "div4_4.h"

EXTERN_C IPcomBase * CreateInstance()
{
	div4_4 * pInstance = new div4_4;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
