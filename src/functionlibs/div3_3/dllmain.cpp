#include <libpcom.h>
#include "div3_3.h"

EXTERN_C IPcomBase * CreateInstance()
{
	div3_3 * pInstance = new div3_3;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
