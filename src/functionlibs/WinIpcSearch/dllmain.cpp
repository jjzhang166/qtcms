#include <libpcom.h>
#include "winipcsearch.h"

IPcomBase * CreateInstance()
{
	WinIpcSearch * pInstance = new WinIpcSearch;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}