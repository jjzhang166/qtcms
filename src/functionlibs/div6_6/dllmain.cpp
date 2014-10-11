#include <libpcom.h>
#include "div6_6.h"

EXTERN_C IPcomBase * CreateInstance()
{
	div6_6 * pInstance =new div6_6;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
