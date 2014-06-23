#include <libpcom.h>
#include "commonlibex.h"

IPcomBase * CreateInstance()
{
	commonlibEx *pInstance=new commonlibEx;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}