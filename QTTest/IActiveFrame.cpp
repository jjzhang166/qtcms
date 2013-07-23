#include "IActiveFrame.h"
#include "qpreviewactivity.h"

int CreatInstance( ActiveIns active,void ** ppv )
{
	*ppv = NULL;
	switch(active)
	{
	case AI_PREVIEW: *ppv = (void **)(new QPreviewActivity);break;
	}
	return 0;
}

int ReleaseInstance( void * ppv )
{
	delete ppv;
	return 0;
}