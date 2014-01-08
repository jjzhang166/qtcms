#ifndef __RECORDPLAYER_H__
#define __RECORDPLAYER_H__

#include <QWidget>
#include <QMutex>
#include "qwfw.h"
#include "IDeviceRemotePlayback.h"
#include "ILocalRecordSearch.h"


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

public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};
	//ILocalRecordSearch
	int searchDateByDeviceName(const QString& sdevname);
	int searchVideoFile(const QString& sdevname,
		const QString& sdate,
		const QString& sbegintime,
		const QString& sendtime,
		const QString& schannellist);

	void transRecordDate(QVariantMap &evMap);
	void transRecordFiles(QVariantMap &evMap);
	void transSearchStop(QVariantMap &evMap);

private:
	int cbInit();
	bool checkChannel(const QString& schannellist);

private:
	ILocalRecordSearch *m_pLocakRecordSearch;

};


#endif // __RECORDPLAYER_H__
