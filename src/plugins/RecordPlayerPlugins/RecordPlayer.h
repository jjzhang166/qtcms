#ifndef __RECORDPLAYER_H__
#define __RECORDPLAYER_H__

#include <QWidget>
#include <QMutex>
#include "qwfw.h"
#include <QDebug>
#include "IDeviceRemotePlayback.h"
#include "ILocalRecordSearchEx.h"
#include "ILocalPlayerEx.h"
#include "IWindowDivMode.h"
#include "RecordPlayerView.h"
#include <IAreaManager.h>
#include <IScreenShotDevice.h>
#include <IDeviceManager.h>
#include <QtCore/QTranslator>
#include "searchprocess.h"

#define MAX_WINDOWS_NUM 49

int cbGetRecordDate(QString evName,QVariantMap evMap,void*pUser);
int cbGetRecordFile(QString evName,QVariantMap evMap,void*pUser);
int cbSearchStop(QString evName,QVariantMap evMap,void*pUser);
int cbThrowException(QString evName, QVariantMap evMap, void* pUser);
int cbScreenShot(QString evName, QVariantMap evMap, void* pUser);
typedef enum __enRecordPlayStatus{
	STATUS_NORMAL_PLAY,
	STATUS_FAST_PLAY,
	STATUS_SLOW_PLAY,
	STATUS_PAUSE,
	STATUS_STOP
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
	void throwException(QVariantMap &evMap);
	void transScreenShot(QVariantMap &evMap);
	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);
	SearchProcess *getCurProc(int wndId);
public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};
	//ILocalRecordSearch
// 	int searchDateByDeviceName(const QString& sdevname);
// 	int searchVideoFile(const QString& sdevname,
// 		const QString& sdate,
// 		const QString& sbegintime,
// 		const QString& sendtime,
// 		const QString& schannellist);
// 	//ILocalRecordSearchEx
// 	int searchVideoFileEx(const QString &sDevName,
// 		const QString& sDate,
// 		const int& nTypes);
	int searchVideoFileEx2(const int & nWndId,
		const QString & sDate,
		const QString & sStartTime,
		const QString & sEndTime,
		const int & nTypes);

	//ILocalPlayer
// 	int AddFileIntoPlayGroup(const QString &filelist,const int &nWndID,const QString &startTime,const QString &endTime);
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
	//½ØÆÁ
	void screenShot(QString sUser,int nType);

	int GetCurrentState();
	void sndToUI(int wnd, QVariantMap evMap);
	void slValidateFail(QVariantMap vmap);
private slots:
	void  OnSubWindowDblClick(QWidget *,QMouseEvent *);
	void  SetCurrentWind(QWidget *);
private:
	int cbInit();
	QDateTime getDateFromPath(QString &filePath);
	int sortFileList(QStringList &fileList);
	bool DevIsExit(QString devicename);
	void loadlanguage();
private:
	ILocalRecordSearchEx *m_pLocalRecordSearch;
	ILocalPlayerEx *m_pLocalPlayer;
	IScreenShotDevice *m_pScreenShotDevice;
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
	RecordPlayStatus m_lastStatus;
	QVariantMap fileMap;
	QString fileKey;
	QMap<int, SearchProcess*> m_schEvMap;
	int m_wndCount;
	QTranslator m_translator;
	QList<int> m_wndList;
};


#endif // __RECORDPLAYER_H__
