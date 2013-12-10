#include <libpcom.h>
#include "Turn.h"

IPcomBase * CreateInstance()
{
	Turn * pInstance = new Turn;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}