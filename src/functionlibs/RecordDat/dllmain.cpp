#include <libpcom.h>
#include "recorddat.h"
IPcomBase * CreateInstance()
{
	/*Recorder * pInstance = new Recorder;*/
	RecordDat *pInstance=new RecordDat;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}