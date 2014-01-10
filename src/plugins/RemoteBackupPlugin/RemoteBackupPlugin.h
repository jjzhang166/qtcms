#ifndef REMOTEBACKUPPLUGIN_H
#define REMOTEBACKUPPLUGIN_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QVariantMap>
#include "IRemoteBackup.h"
#include "IWebPluginBase.h"
#include "RemoteBackupWindows.h"

/*++
	远程备份插件
	startBackup 开始备份接口
	stopBackup 结束备份接口
	getProgress 获取备份进度接口

	事件：BackupStatusChange 备份状态 参数types
--*/
class RemoteBackupPlugin : public QObject,
	public IWebPluginBase
{
	Q_OBJECT 
public:
	RemoteBackupPlugin();
	~RemoteBackupPlugin();

	virtual int startBackup(const QString &sAddr,unsigned int uiPort,const QString &sEseeId,
		int nChannel,
		int nTypes,
		const QString & startTime,
		const QString & endTime,
		const QString & sbkpath);
	virtual int stopBackup();
	virtual float getProgress();

	virtual QList<QWebPluginFactory::Plugin> plugins() const ;
	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const ;

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

private:
	int m_nRef;
	QMutex m_csRef;

	//DvrTabWindows m_Tabwindow;
};

#endif // REMOTEBACKUPPLUGIN_H
