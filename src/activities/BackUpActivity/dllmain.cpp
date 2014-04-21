#include <libpcom.h>
#include <IActivities.h>
#include "backupactivity.h"
EXTERN_C IPcomBase * CreateInstance()
{
	BackUpActivity * pInstance = new BackUpActivity;
	IPcomBase * pBase = static_cast<IActivities *>(pInstance);
	pBase->AddRef();
	return pInstance;
}