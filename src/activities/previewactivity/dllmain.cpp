#include <libpcom.h>
#include <IActivities.h>
#include "previewactivity.h"

IPcomBase * CreateInstance()
{
	previewactivity * pInstance = new previewactivity;
	IPcomBase * pBase = static_cast<IActivities *>(pInstance);
	pBase->AddRef();
	return pInstance;
}