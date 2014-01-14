#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "LocalPlayer_global.h"
#include <QObject>
#include <QtCore/QMutex>
#include "IEventRegister.h"
#include "ILocalRecordSearch.h"
#include "IDisksSetting.h"
#include "ILocalPlayer.h"

class LocalPlayer : public QObject,
	public IEventRegister,
	public ILocalRecordSearch,
	public ILocalPlayer
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

	//ILocalPlayer
	virtual int AddFileIntoPlayGroup(QStringList const filelist,QWidget *wnd,const QDateTime &start,const QDateTime &end);
	virtual int SetSynGroupNum(int num);
	virtual int GroupPlay();
	virtual int GroupPause();
	virtual int GroupContinue();
	virtual int GroupStop();
	virtual int GroupSpeedFast(int speed);
	virtual int GroupSpeedSlow(int speed);
	virtual int GroupSpeedNormal();

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
	int checkFileExist(QStringList const fileList, const QDateTime& startTime, const QDateTime& endTime);
	bool checkChannelInFileList(QStringList const filelist);

private:
	int m_nRef;
	QMutex m_csRef;

	QMultiMap<QString, ProcInfoItem> m_eventMap;
	QStringList   m_eventList;
	IDisksSetting *m_pDiskSetting;

	QMap<QWidget*, PrePlay> m_GroupMap;
	int m_nGroupNum;
	bool m_bIsGroupPlaying;
};

#endif // LOCALPLAYER_H
