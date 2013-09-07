#ifndef QCOMMONPLUGIN_H
#define QCOMMONPLUGIN_H

#include <QObject>
#include <qwfw.h>
#include <QtGui/QWidget>

class QCommonPlugin : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	QCommonPlugin(QWidget *parent = 0);
	~QCommonPlugin();

public:
	int GetUserLevel( const QString & sUsername,int & nLevel );

	int GetUserAuthorityMask( const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2 );

public slots:
	// Event proc
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}

	// User Management
	int AddUser( const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2 );

	int RemoveUser( const QString & sUsername );

	int ModifyUserPassword( const QString & sUsername,const QString & sNewPassword );

	int ModifyUserLevel( const QString & sUsername,int nLevel );

	int ModifyUserAuthorityMask( const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2 );

	bool IsUserExists( const QString & sUsername );

	bool CheckUser( const QString & sUsername,const QString & sPassword );

	int GetUserLevel( const QString & sUsername );

	QStringList GetUserAuthorityMask( const QString & sUsername );

	int GetUserCount();

	QStringList GetUserList();

};

#endif // QCOMMONPLUGIN_H
