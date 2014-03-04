#include <libpcom.h>
#include "AudioPlayer.h"

IPcomBase * CreateInstance()
{
	AudioPlayer * pInstance = new AudioPlayer;
	IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
	pBase->AddRef();
	return pInstance;
}