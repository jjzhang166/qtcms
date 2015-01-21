#include "LocalBackupWindows.h"
#include "guid.h"


LocalBackupWindows::LocalBackupWindows(QWidget *parent)
: QWidget(parent),
QWebPluginFWBase(this)
{
	connect(&m_backupThread, SIGNAL(sendMsg(QString, QVariantMap)), this, SLOT(sendToUi(QString, QVariantMap)));
}

LocalBackupWindows::~LocalBackupWindows(void)
{
	
}

void LocalBackupWindows::sendToUi( QString evName, QVariantMap item )
{
	EventProcCall(evName, item);
}

int LocalBackupWindows::startLocalFileBackUp( int nTypes,const QString sChannel,const QString &startTime,const QString &endTime )
{
	QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
	QDateTime end = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");

// 	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
// 		".",
// 		QFileDialog::ShowDirsOnly
// 		| QFileDialog::DontResolveSymlinks);

	QString dir = "F:\\Project\\Qtcms\\output\\Debug\\playback";

	dir = dir.replace("\\", "/");
	m_backupThread.setBackupPath(dir);
	return m_backupThread.startLocalFileBackUp(nTypes, sChannel, start, end);
}

void LocalBackupWindows::stopLocalFileBackUp()
{
	m_backupThread.stopLocalFileBackUp();
}
