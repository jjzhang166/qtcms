#include <libpcom.h>
#include "Recorder.h"

IPcomBase * CreateInstance()
{
	Recorder * pInstance = new Recorder;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}