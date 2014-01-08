#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "LocalPlayer_global.h"
#include <QObject>
#include <QtCore/QMutex>
#include "IEventRegister.h"
#include "ILocalRecordSearch.h"
#include "IDisksSetting.h"

class LocalPlayer : public QObject,
	public IEventRegister,
	public ILocalRecordSearch
{
public:
	LocalPlayer();
	~LocalPlayer();
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	//ILocalRecordSearch
	virtual int searchDateByDeviceName(const QString& sdevname);
	virtual int searchVideoFile(const QString& sdevname,
		const QString& sdate,
		const QString& sbegintime,
		const QString& sendtime,
		const QString& schannellist);

	//IEventRegister
	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList &eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

	typedef int (__cdecl *PreviewEventCB)(QString name, QVariantMap info, void* pUser);
	typedef struct _tagProcInfoItem
	{
		PreviewEventCB proc;
		void		*puser;
	}ProcInfoItem;
private:
	void eventProcCall(QString sEvent,QVariantMap param);
	int checkUsedDisk(QString &strDisk);
	bool checkChannel(const QString& schannellist);

private:
	int m_nRef;
	QMutex m_csRef;

	QMultiMap<QString, ProcInfoItem> m_eventMap;
	QStringList   m_eventList;
	IDisksSetting *m_pDiskSetting;
};

#endif // LOCALPLAYER_H
