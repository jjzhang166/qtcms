#include <libpcom.h>
#include "div1.h"

IPcomBase * CreateInstance()
{
	div1 * pInstance = new div1;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}