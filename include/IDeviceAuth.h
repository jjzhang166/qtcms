#ifndef __IDEVICEAUTH_HEAD_FILE__
#define __IDEVICEAUTH_HEAD_FILE__

#include "libpcom.h"
#include <QtCore/QString>

interface IDeviceAuth : public IPComBase
{
	virtual void setDeviceAuth(const QString & sUsername, const QString & sPassword) = 0;
};

#endif