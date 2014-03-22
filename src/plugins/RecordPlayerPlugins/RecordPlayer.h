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

int cbGetRecordDate(QString evName,QVariantMap evMap,void*pUser);
int cbGetRecordFile(QString evName,QVariantMap evMap,void*pUser);
int cbSearchStop(QString evName,QVariantMap evMap,void*pUser);

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
	void transSearchStop(QVariantMap &evMap);

public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};
	//ILocalRecordSearch
	int searchDateByDeviceName(const QString& sdevname);
	int searchVideoFile(const QString& sdevname,
		const QString& sdate,
		const QString& sbegintime,
		const QString& sendtime,
		const QString& schannellist);
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

	void ChangeAudioHint(QString, RecordPlayerView*);

private slots:
	void  OnSubWindowDblClick(QWidget *,QMouseEvent *);
	void  SetCurrentWind(QWidget *);


private:
	int cbInit();
	QDateTime getDateFromPath(QString &filePath);
	int sortFileList(QStringList &fileList);

private:
	ILocalRecordSearch *m_pLocalRecordSearch;
	ILocalPlayer *m_pLocalPlayer;
	IWindowDivMode *m_pWindowDivMode;
	QList<QWidget *> m_lstRecordPlayerWndList;


	RecordPlayerView m_subRecPlayerView[4];
	int m_currentWindID;

};


#endif // __RECORDPLAYER_H__
