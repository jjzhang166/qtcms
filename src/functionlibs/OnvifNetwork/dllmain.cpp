#include <libpcom.h>
#include <OnvifNetwork.h>

IPcomBase * CreateInstance()
{
	OnvifNetwork *pInstance=new OnvifNetwork;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}