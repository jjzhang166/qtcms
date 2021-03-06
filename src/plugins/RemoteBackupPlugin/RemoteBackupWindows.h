#ifndef REMOTEBACKUPWINDOWS_H
#define REMOTEBACKUPWINDOWS_H

#include <QtGui/QWidget>
#include <QtCore/QVariantMap>
#include <QtCore/QTime>
#include <QFileDialog>
#include "qwfw.h"
#include "RemoteBackup.h"
//#include "IEventRegister.h"

class RemoteBackupWindows : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	RemoteBackupWindows(QWidget *parent = 0);
	~RemoteBackupWindows(void);

// 	void procCallBack(QVariantMap item);
public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}

	int startBackup(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,
		int nChannel,
		int nTypes,
		const QString &sDeviceName,
		const QString & startTime,
		const QString & endTime,
		const QString & sbkpath);
	int stopBackup();
	float getProgress();

	void ChooseDir();
// 	void sendToHtml(QVariantMap item);
	void sendToUi(QString evName, QVariantMap item);
private:
// 	bool LoadDeviceClient(QString vender);
	RemoteBackup m_remoteBackupObj;
	QString m_dir;
signals:
// 	void sendStatus(QVariantMap item);
};

#endif //REMOTEBACKUPWINDOWS_H