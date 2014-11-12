#include <libpcom.h>
#include <OnvifProtocol.h>

IPcomBase * CreateInstance()
{
	OnvifProtocol *pInstance=new OnvifProtocol;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}