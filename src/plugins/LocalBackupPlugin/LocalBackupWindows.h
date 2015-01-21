#ifndef LOCALBACKUPWINDOWS_H
#define LOCALBACKUPWINDOWS_H

#include <QtGui/QWidget>
#include <QtCore/QVariantMap>
// #include <QtCore/QTime>
#include <QFileDialog>
#include "qwfw.h"
#include "localbackthread.h"

class LocalBackupWindows : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT
public:
	LocalBackupWindows(QWidget *parent = 0);
	~LocalBackupWindows(void);

public slots:
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}

	int startLocalFileBackUp(int nTypes,const QString sChannel,const QString &startTime,const QString &endTime);
	void stopLocalFileBackUp();

	void sendToUi(QString evName, QVariantMap item);
private:
	LocalBackThread m_backupThread;
};

#endif //LOCALBACKUPWINDOWS_H