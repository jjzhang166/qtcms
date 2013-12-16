#include <libpcom.h>
#include "ddrawrender.h"

IPcomBase * CreateInstance()
{
	DdrawRender * pInstance = new DdrawRender();
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}