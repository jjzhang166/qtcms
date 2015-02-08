#include <libpcom.h>
#include "userinfoplugin.h"

IPcomBase * CreateInstance()
{
	userInfoPlugin * pInstance = new userInfoPlugin;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}