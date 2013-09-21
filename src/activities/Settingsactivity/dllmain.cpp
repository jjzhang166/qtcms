#include <libpcom.h>
#include <IActivities.h>
#include "settingsactivity.h"

IPcomBase * CreateInstance()
{
	settingsActivity * pInstance = new settingsActivity;
	IPcomBase * pBase = static_cast<IActivities *>(pInstance);
	pBase->AddRef();
	return pInstance;
}