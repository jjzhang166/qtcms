#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "LocalPlayer_global.h"
#include <QObject>
#include <QtCore/QMutex>
#include "IEventRegister.h"
#include <QDebug>
#include <QtSql>
#include "ILocalRecordSearch.h"
#include "IDisksSetting.h"
#include "ILocalPlayer.h"
#include "ILocalRecordSearchEx.h"
#include "ILocalPlayerEx.h"

void cbTimeChange(QString evName, uint playTime, void* pUser);

class LocalPlayer : public QObject,
	public IEventRegister,
	public ILocalRecordSearch,
	public ILocalRecordSearchEx,
	public ILocalPlayerEx,
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
	//ILocalRecordSearchEx
	virtual int searchVideoFileEx(const QString &sDevName,
		const QString& sDate,
		const int& nTypes);
	virtual int searchVideoFileEx(const int & nWndId,
		const QString & sDate,
		const QString & sStartTime,
		const QString & sEndTime,
		const int & nTypes);

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
	virtual QDateTime GetNowPlayedTime();
	virtual bool GroupEnableAudio(bool bEnable);
	virtual int GroupSetVolume(unsigned int uiPersent, QWidget* pWnd);

	//ILocalPlayerEx
	virtual int AddFileIntoPlayGroupEx(const int & nWndId,
		const QWidget * pWnd,
		const QDate& date,
		const QTime & startTime,
		const QTime & endTime,
		const int & nTypes);

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
	void setPlayTime(uint &playTime);
	void setBaseTime(uint &baseTime);
private:
	void eventProcCall(QString sEvent,QVariantMap param);
	int checkUsedDisk(QString &strDisk);
	bool checkChannel(const QString& schannellist);
	int checkFileExist(QStringList const fileList, const QDateTime& startTime, const QDateTime& endTime, QVector<PeriodTime> &perTimeVec);
	bool checkChannelInFileList(QStringList const filelist);
	int countSkipTime();
	bool checkFileFromLong(QString path, unsigned long &tick, QDateTime & endTime);
	QStringList sortFileList(QStringList const fileList);
	QString getTypeList(int nTypes);
	QStringList getFileList(int wndId, QDate date, QTime star, QTime end, int types, QVector<PeriodTime> &playTime);
private:
	int m_nRef;
	QMutex m_csRef;

	QMultiMap<QString, ProcInfoItem> m_eventMap;
	QStringList   m_eventList;
	IDisksSetting *m_pDiskSetting;

	QMap<QWidget*, PrePlay> m_GroupMap;
	int m_nGroupNum;
	bool m_bIsGroupPlaying;

	uint m_startTime;
	uint m_endTime;
	uint m_playTime;

	QWidget* m_pCurView;

	QMap<QString, PeriodTime> m_filePeriodMap;
// 	uint m_skipTime;
	uint m_lastPlayTime;
// 	int m_callTimes;

	QString m_connectId;
	QSqlDatabase *m_db;
};

#endif // LOCALPLAYER_H
