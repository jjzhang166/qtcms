#ifndef PREVIEWACTIVITY_H
#define PREVIEWACTIVITY_H

#include "previewactivity_global.h"
#include <IActivities.h>

class previewactivity : public IActivities
{
public:

    virtual long __stdcall QueryInterface(const IID & iid,void **ppv);
    virtual unsigned long __stdcall AddRef();
    virtual unsigned long __stdcall Release();

	virtual void Active();
private:

};

#endif // PREVIEWACTIVITY_H
