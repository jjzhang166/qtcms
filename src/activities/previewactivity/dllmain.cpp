#include <libpcom.h>
#include <IActivities.h>
#include "previewactivity.h"

EXTERN_C IPcomBase * CreateInstance()
{
	previewactivity * pInstance = new previewactivity;
	IPcomBase * pBase = static_cast<IActivities *>(pInstance);
	pBase->AddRef();
	return pInstance;
}
