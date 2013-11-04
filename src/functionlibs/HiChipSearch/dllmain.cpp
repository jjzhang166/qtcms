#include <libpcom.h>
#include "HiChipSearch.h"
#include <IWebPluginBase.h>

IPcomBase * CreateInstance()
{
	HiChipSearch * pInstance = new HiChipSearch;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}