#include <libpcom.h>
#include "LocalPlayerEx.h"

IPcomBase * CreateInstance()
{
	LocalPlayerEx * pInstance = new LocalPlayerEx;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}