#include <libpcom.h>
#include "Hole.h"

IPcomBase * CreateInstance()
{
	Hole * pInstance = new Hole;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}