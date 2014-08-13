#include <libpcom.h>
#include "Recorder.h"
#include <RecorderEx.h>
IPcomBase * CreateInstance()
{
	/*Recorder * pInstance = new Recorder;*/
	RecorderEx *pInstance=new RecorderEx;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}