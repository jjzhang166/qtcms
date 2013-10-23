#ifndef __IAREA_MANAGER_HEAD_FILE_9SDVGHAFUAS90DF__
#define __IAREA_MANAGER_HEAD_FILE_9SDVGHAFUAS90DF__

#include "libpcom.h"
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

interface IAreaManager : public IPComBase
{
	virtual int AddArea(int nPid,QString sName) = 0;
	virtual int RemoveAreaById(int nId) = 0;
	virtual int RemoveAreaByName(QString sName) = 0;
	virtual int SetAreaName(int nId,QString sName) = 0;
	virtual bool IsAreaNameExist(QString sName) = 0;
	virtual bool IsAreaIdExist(int nid)=0;
	virtual int GetAreaCount() = 0;
	virtual QStringList GetAreaList() = 0;
	virtual QStringList GetSubArea(int nId) = 0;
	virtual int GetAreaPid(int id) = 0;
	virtual QString GetAreaName(int id) = 0;
	virtual int GetAreaInfo(int nId,int &nPid,QString &sName) = 0;
	virtual QVariantMap GetAreaInfo(int nId) = 0;

	enum _emError{
		OK,
		E_AREA_NOT_FOUND,
		E_AREA_NAME_EXISTS,
		E_SYSTEM_FAILED,
	};
};

#endif