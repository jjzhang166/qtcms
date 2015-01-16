#ifndef LOCALPLAYEREX_H
#define LOCALPLAYEREX_H

#include "LocalPlayerEx_global.h"
#include <QtCore/QMutex>
#include <QtSql>
#include <QVariantMap>
#include "IEventRegister.h"
#include "ILocalPlayerEx.h"
#include "ILocalRecordSearchEx.h"
#include "IVideoDisplayOption.h"
#include "ICommunicate.h"
#include "QFileData.h"
#include "PlayMgr.h"

extern "C"{
#include "sqlite3.h"
};

void cbTimeChange(QString evName, uint playTime, void* pUser);
// void cbThrowException(QString evName, QVariantMap item, void* pUser);

class LocalPlayerEx : public QObject,
	public IEventRegister,
	public ILocalRecordSearchEx,
	public ILocalPlayerEx,
	public IVideoDisplayOption,
	public ICommunicate
{
	Q_OBJECT
public:
	LocalPlayerEx();
	~LocalPlayerEx();
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	//ILocalRecordSearchEx
	virtual int searchVideoFileEx(const int & nWndId,
		const QString & sDate,
		const QString & sStartTime,
		const QString & sEndTime,
		const int & nTypes);

	//ILocalPlayerEx
	virtual int AddFileIntoPlayGroupEx(const int & nWndId,
		const QWidget * pWnd,
		const QDate& date,
		const QTime & startTime,
		const QTime & endTime,
		const int & nTypes);
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

// 	virtual int GroupPlayBack();

	//IEventRegister
	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList &eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

	// IVideoDisplayOption
	virtual void enableWindowStretch(QWidget * window,bool bEnable);
	virtual bool getWindowStretchStatus(QWidget * window);

	//ICommunicate
	virtual int setInfromation(const QString &msgName, const QVariantMap &info);

	typedef int (__cdecl *PreviewEventCB)(QString name, QVariantMap info, void* pUser);
	typedef struct _tagProcInfoItem
	{
		PreviewEventCB proc;
		void		*puser;
	}ProcInfoItem;

	void setPlayTime(uint &playtime);
public slots:
	void onStartPlayMgr(uint wndId);
	void onStopPlayMgr();
private:
	QSqlDatabase * initDataBase(QString sDatabaseName);
	sqlite3* initDataBase(char *dbPath);
	void deInitDataBase();
	QString getTypeList(int nTypes);
	void eventProcCall(QString sEvent,QVariantMap param);
	void getWndIdList(QList<qint32> &wndList);
	QList<QString> getFileList(qint32 &i32Pos, QMap<uint, QVector<PeriodTime> >& filePeriodMap);
	QString intToStr(QList<qint32> &wndList);
	PlayMgr* getPlayMgrPointer(QWidget* pwnd);
	qint32 countSkipTime(const QMap<uint, QVector<PeriodTime> >& filePeriodMap, QVector<PeriodTime> &skipTime);
// 	void appendFile(QList<QString> &fileList, QString fileName, QVector<uint> &vecTime, uint time);
	void appendPeriodTime(QVector<PeriodTime> &vecPeriod, const PeriodTime &per);
	bool exceCommand(QSqlQuery &queue, const QString &cmd);
	int execCommand(sqlite3 *pdb, const char* cmd, char*** pppRet, int* row, int* col, char** pMsg);
private:
	qint32 m_nRef;
	qint32 m_i32GroupNum;
	qint32 m_i32Types;
	uint m_uiStartSec;
	uint m_uiEndSec;
	uint m_uiPlayTime;
	QMutex m_csRef;

	QStringList m_eventList;
	QMultiMap<QString, ProcInfoItem> m_eventMap;

	QMultiMap<QString, sqlite3*> m_sqlMap;
	QMultiMap<QString, QSqlDatabase*> m_dbMap;
	QString m_disklst;
	PlayMgr *m_pCurAudioWnd;

	typedef struct _tagPlayInfo{
// 		union{
// 			PlayMgr *pPlayMgr;
// 			QFileData *pFileData;
// 		};
		PlayMgr *pPlayMgr;
		qint32 i32WndId;
	}PlayInfo;

	typedef struct _tagTimePath{
		uint start;
		QString path;
	}TimePath;


	PlayInfo m_arrPlayInfo[MAX_PLAY_THREAD];
	QFileData *m_pFileData;

	//about digital zoom
	QMap<QWidget*, PlayMgr*> m_wndMap;
	QList<QWidget*> m_wndList;
	QWidget *m_susWnd;
private:
	void appendTimePath(QList<TimePath> &tpList, const uint &start, const QString &path, qint32 &insertPos);

};

#endif // LOCALPLAYEREX_H
