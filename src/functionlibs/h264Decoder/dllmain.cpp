#include <libpcom.h>
#include "h264Decoder.h"

IPcomBase * CreateInstance()
{
	h264Decoder * pInstance = new h264Decoder;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}