#include <libpcom.h>

#include "dvrsearch.h"

IPcomBase * CreateInstance()
{
	DvrSearch * pInstance = new DvrSearch;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}