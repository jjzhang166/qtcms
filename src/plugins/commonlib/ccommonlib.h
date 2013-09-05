#ifndef CCOMMONLIB_H
#define CCOMMONLIB_H

#include "commonlib_global.h"
#include <QObject>
#include <IWebPluginBase.h>
#include <QMutex>
#include <IUserManager.h>
#include "qcommonplugin.h"

class Ccommonlib : public QObject,
	public IWebPluginBase,
	public IUserManager
{
public:
	Ccommonlib();
	~Ccommonlib();

	virtual QList<QWebPluginFactory::Plugin> plugins() const;

	virtual QObject * create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const;

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();

	virtual int AddUser( const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2 );

	virtual int RemoveUser( const QString & sUsername );

	virtual int ModifyUserPassword( const QString & sUsername,const QString & sNewPassword );

	virtual int ModifyUserLevel( const QString & sUsername,int nLevel );

	virtual int ModifyUserAuthorityMask( const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2 );

	virtual bool IsUserExists( const QString & sUsername );

	virtual bool CheckUser( const QString & sUsername,const QString & sPassword );

	virtual int GetUserLevel( const QString & sUsername,int & nLevel );

	virtual int GetUserAuthorityMask( const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2 );

	virtual int GetUserCount();

	virtual QStringList GetUserList();
private:
	int m_nRef;
	QMutex m_csRef;
	QCommonPlugin m_pluginObj;
};

#endif // CCOMMONLIB_H
