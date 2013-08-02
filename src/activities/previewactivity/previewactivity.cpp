#include "previewactivity.h"
#include <guid.h>

long __stdcall previewactivity::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IActivities == iid)
	{
		*ppv = static_cast<IActivities *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	reinterpret_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall previewactivity::AddRef()
{

	return 0;
}

unsigned long __stdcall previewactivity::Release()
{

	return 0;
}

void previewactivity::Active( QWebFrame * )
{
	
}