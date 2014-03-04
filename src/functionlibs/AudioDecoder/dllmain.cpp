#include <libpcom.h>
#include "AudioDecoder.h"

IPcomBase * CreateInstance()
{
	AudioDecoder * pInstance = new AudioDecoder;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}