#ifndef IACTIVITIES_H
#define IACTIVITIES_H

#include "libpcom.h"

interface IActivities : public IPcomBase
{
    virtual void Active() = 0;
};

#endif // IACTIVITIES_H
