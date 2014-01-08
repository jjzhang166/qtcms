#include <libpcom.h>
#include "LocalPlayer.h"

IPcomBase * CreateInstance()
{
	LocalPlayer * pInstance = new LocalPlayer;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}