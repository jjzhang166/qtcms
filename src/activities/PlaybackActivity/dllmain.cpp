#include <libpcom.h>
#include <IActivities.h>
#include "playbackactivity.h"
EXTERN_C IPcomBase * CreateInstance()
{
	PlaybackActivity * pInstance = new PlaybackActivity;
	IPcomBase * pBase = static_cast<IActivities *>(pInstance);
	pBase->AddRef();
	return pInstance;
}