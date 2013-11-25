#include <libpcom.h>
#include "SDLRender.h"

IPcomBase * CreateInstance()
{
	SDLRender * pInstance = new SDLRender;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}