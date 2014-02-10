#include <libpcom.h>
#include "Hih264Decoder.h"

IPcomBase * CreateInstance()
{
	Hih264Decoder * pInstance = new Hih264Decoder;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}