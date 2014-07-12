#pragma once
#include <QThread>
#include <QString>
#include <QtNetwork/QHostAddress>
#include <QStringList>
#include <QMap>
#include <QQueue>
#include <QWidget>
#include <QMultiMap>
int cbXRecRunFoundFile(QString evName,QVariantMap evMap,void*pUser);
int cbXRecRunFileSearchFinished(QString evName,QVariantMap evMap,void*pUser);
int cbXRecRunFileSearchFail(QString evName,QVariantMap evMap,void*pUser);
int cbXRecRunSocketError(QString evName,QVariantMap evMap,void*pUser);
int cbXRecRunStateChange(QString evName,QVariantMap evMap,void*pUser);
int cbXRecRunCacheState(QString evName,QVariantMap evMap,void*pUser);
typedef int (__cdecl *rPlayBackRunExCb)(QString sEventName,QVariantMap evMap,void *pUser);
typedef struct __tagRecPlayBackProcInfo{
	rPlayBackRunExCb proc;
	void *pUser;
}tagRecPlayBackProcInfo;
typedef enum __tagRecPlayBackStep{
	RECSEARCHFILE,
	RECGROUPPLAY,
	RECGROUPPAUSE,
	RECGROUPCONTINUE,
	RECGROUPSTOP,
	RECGROUPSPEEDFAST,
	RECGROUPSEEEDSLOW,
	RECGROUPNORMAL,
	RECSETVOLUME,
	RECAUDIOENABLE,
	RECDEFAULT,
	RECEND,
}tagRecPlayBackStep;
typedef struct __tagRecDeviceInfo{
	QString sAddress;
	QString sEsee;
	unsigned int uiPort;
	QString sVendor;
	QString sUserName;
	QString sPassword;
	int nSearchChannel;
	int nSearchTypes;
	QString sSearchStartTime;
	QString sSearchEndTime;
	int nPlayTypes;
	QString sPlayStartTime;
	QString sPlayEndTime;
	QHostAddress tAddress;
	unsigned int uiPersent;
	QWidget *pWin;
	bool bAudioEnable;
}tagRecDeviceInfo;
typedef struct __tagWinChannelInfo{
	unsigned int uiWin;
	int nChannel;
}tagWinChannelInfo;
class rPlayBackRunEx:public QThread
{
	Q_OBJECT
public:
	rPlayBackRunEx(void);
	~rPlayBackRunEx(void);
public:
	int setDeviceHostInfo(const QString &sAddress,unsigned int uiPort,const QString &sEsee);
	int setDevcieVendor(const QString& sVendor);
	int setUserVerifyInfo(const QString &sUserName,const QString &sPassword);
	int addChannelIntoPlayGroup(unsigned int uiWin,int nChannel);

	int startSearchRecFile(int nChannel,int nTypes,const QString &sStartTime,const QString &sEndTime);

	int groupPlay(int nTypes,const QString &sStartTime,const QString &sEndTime);
	int groupPause();
	int groupContinue();
	int groupStop();
	int groupSpeedFast();
	int groupSpeedSlow();
	int groupSpeedNormal();

	int setVolume(const unsigned int &uiPersent,QWidget *pWin);
	int audioEnable(bool bEnable);

	QString getNowPlayTime();
	//注册回调函数
	void registerEvent(QString sEventName,int (__cdecl *proc)(QString,QVariantMap,void*),void *pUser);
public:
	//回调函数
	int cbRecRunFoundFile(QString evName,QVariantMap evMap,void*pUser);
	int cbRecRunFileSearchFinished(QString evName,QVariantMap evMap,void*pUser);
	int cbRecRunFileSearchFail(QString evName,QVariantMap evMap,void*pUser);
	int cbRecRunSocketError(QString evName,QVariantMap evMap,void*pUser);
	int cbRecRunStateChange(QString evName,QVariantMap evMap,void*pUser);
	int cbRecRunCacheState(QString evName,QVariantMap evMap,void*pUser);
private:
	void eventCallBack(QString sEventName,QVariantMap evMap);
protected:
	void run();
private:
	tagRecDeviceInfo m_tRecDeviceInfo;
	QStringList m_sVendorList;
	QMap<int,tagWinChannelInfo> m_tWinChannelInfo;
	QQueue<int> m_qStepCode;
	bool m_bStop;
	QMultiMap<QString ,tagRecPlayBackProcInfo> m_tEventMap;
	QStringList m_tEventNameList;
};

