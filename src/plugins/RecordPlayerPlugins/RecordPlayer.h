#ifndef __RECORDPLAYER_H__
#define __RECORDPLAYER_H__

#include <QWidget>
#include <QMutex>
#include "qwfw.h"
#include <QDebug>
#include "IDeviceRemotePlayback.h"
#include "ILocalRecordSearch.h"
#include "ILocalPlayer.h"
#include "IWindowDivMode.h"
#include "RecordPlayerView.h"
#include <IAreaManager.h>
#include <IDeviceManager.h>
#include "searchprocess.h"


int cbGetRecordDate(QString evName,QVariantMap evMap,void*pUser);
int cbGetRecordFile(QString evName,QVariantMap evMap,void*pUser);
int cbSearchStop(QString evName,QVariantMap evMap,void*pUser);
typedef enum __enRecordPlayStatus{
	STATUS_PLAY,
	STATUS_PAUSE,
	STATUS_STOP,
	STATUS_SLOW,
	STATUS_FAST,
	STATUS_NORMAL,
	STATUS_CONTINUE
}RecordPlayStatus;
class RecordPlayer : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT 

public:
	RecordPlayer();
	~RecordPlayer();

	virtual void resizeEvent( QResizeEvent * );
	int GetCurrentWnd();

	void transRecordDate(QVariantMap &evMap);
	void transRecordFiles(QVariantMap &evMap);
	void transRecordFilesEx(QVariantMap &evMap);
	void transSearchStop(QVariantMap &evMap);
	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);
	SearchProcess *getCurProc(int wndId);
public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};
	//ILocalRecordSearch
	int searchDateByDeviceName(const QString& sdevname);
	int searchVideoFile(const QString& sdevname,
		const QString& sdate,
		const QString& sbegintime,
		const QString& sendtime,
		const QString& schannellist);
	//ILocalRecordSearchEx
	int searchVideoFileEx(const QString &sDevName,
		const QString& sDate,
		const int& nTypes);
	int searchVideoFileEx2(const int & nWndId,
		const QString & sDate,
		const QString & sStartTime,
		const QString & sEndTime,
		const int & nTypes);

	//ILocalPlayer
	int AddFileIntoPlayGroup(const QString &filelist,const int &nWndID,const QString &startTime,const QString &endTime);
	int SetSynGroupNum(int num);
	int GroupPlay();
	int GroupPause();
	int GroupContinue();
	int GroupStop();
	int GroupSpeedFast(int speed);
	int GroupSpeedSlow(int speed);
	int GroupSpeedNormal();
	QString GetNowPlayedTime();

	//ILocalPlayerEx
	int AddFileIntoPlayGroupEx(const int & nWndId,const QString& sDate,const QString & sStartTime,const QString & sEndTime,const int & nTypes);

	int SetVolume(const unsigned int &uiPersent);
	int AudioEnabled(bool bEnabled);
	QVariantMap ScreenShot();
	int GetCurrentState();
	void sndToUI(int wnd, QVariantMap evMap);

private slots:
	void  OnSubWindowDblClick(QWidget *,QMouseEvent *);
	void  SetCurrentWind(QWidget *);
private:
	int cbInit();
	QDateTime getDateFromPath(QString &filePath);
	int sortFileList(QStringList &fileList);
	bool DevIsExit(QString devicename);
private:
	ILocalRecordSearch *m_pLocalRecordSearch;
	ILocalPlayer *m_pLocalPlayer;
	IWindowDivMode *m_pWindowDivMode;
	QList<QWidget *> m_lstRecordPlayerWndList;


	RecordPlayerView m_subRecPlayerView[4];
	int m_currentWindID;
	int m_wndNum;
	bool m_bIsOpenAudio;
	bool m_bIsHide;
	unsigned int m_uiPersent;
	QString m_devicename;
	RecordPlayStatus m_CurStatus;
	QVariantMap fileMap;
	QString fileKey;
	QMap<int, SearchProcess*> m_schEvMap;
	int m_wndCount;
};


#endif // __RECORDPLAYER_H__
