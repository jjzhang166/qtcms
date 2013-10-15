#include <libpcom.h>
#include "div6_1.h"

IPcomBase * CreateInstance()
{
	div6_1 * pInstance = new div6_1;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}