#ifndef __IEVENTREGISTER_HEAD_FILE_9VH988YASDGVG7AV__
#define __IEVENTREGISTER_HEAD_FILE_9VH988YASDGVG7AV__
#include <libpcom.h>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

interface IEventRegister : public IPComBase
{
	virtual QStringList eventList() = 0;
	virtual int queryEvent(QString eventName) = 0;
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser) = 0;

	enum _enErrorCode{
		OK,
		E_EVENT_NOT_SUPPORT,
		E_INVALID_PARAM
	};
};

#endif