#ifndef IACTIVITIES_H
#define IACTIVITIES_H

#include "libpcom.h"
#include <QtWebKit/QWebFrame>

interface IActivities : public IPcomBase
{
    virtual void Active(QWebFrame *) = 0;
};

#endif // IACTIVITIES_H
