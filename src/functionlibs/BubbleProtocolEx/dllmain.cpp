#include <libpcom.h>
#include <bubbleprotocolex.h>

IPcomBase * CreateInstance()
{
	BubbleProtocolEx *pInstance=new BubbleProtocolEx;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}