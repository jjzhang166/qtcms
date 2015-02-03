#include <libpcom.h>
#include <configmgr.h>

IPcomBase * CreateInstance()
{
	ConfigMgr *pInstance=new ConfigMgr;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}