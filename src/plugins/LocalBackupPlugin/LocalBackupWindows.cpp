#include "LocalBackupWindows.h"
#include "guid.h"

#include <QDebug>

LocalBackupWindows::LocalBackupWindows(QWidget *parent)
: QWidget(parent),
QWebPluginFWBase(this)
{
	m_lastPath = ".";
	connect(&m_backupThread, SIGNAL(sendMsg(QString, QVariantMap)), this, SLOT(sendToUi(QString, QVariantMap)));
}

LocalBackupWindows::~LocalBackupWindows(void)
{
	
}

void LocalBackupWindows::sendToUi( QString evName, QVariantMap item )
{
	qDebug()<<__FUNCTION__<<"nWnd:"<<item["nChannel"].toInt()<<"progress:"<<item["Progress"].toInt();
	EventProcCall(evName, item);
}

int LocalBackupWindows::startLocalFileBackUp( int nTypes,const QString sChannel,const QString &startTime,const QString &endTime )
{
	QDateTime start = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
	QDateTime end = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");

	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		m_lastPath,
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty()){
		return 1;
	}
	m_lastPath = dir;
	dir = dir.replace("\\", "/");
	m_backupThread.setBackupPath(dir);
	return m_backupThread.startLocalFileBackUp(nTypes, sChannel, start, end);
}

void LocalBackupWindows::stopLocalFileBackUp()
{
	m_backupThread.stopLocalFileBackUp();
}
