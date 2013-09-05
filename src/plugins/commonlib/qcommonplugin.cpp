#include "qcommonplugin.h"

QCommonPlugin::QCommonPlugin(QWidget *parent)
	: QWidget(parent),
	QWebPluginFWBase(this)
{

}

QCommonPlugin::~QCommonPlugin()
{

}

int QCommonPlugin::AddUser( const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2 )
{

	return 0;
}

int QCommonPlugin::RemoveUser( const QString & sUsername )
{

	return 0;
}

int QCommonPlugin::ModifyUserPassword( const QString & sUsername,const QString & sNewPassword )
{

	return 0;
}

int QCommonPlugin::ModifyUserLevel( const QString & sUsername,int nLevel )
{

	return 0;
}

int QCommonPlugin::ModifyUserAuthorityMask( const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2 )
{

	return 0;
}

bool QCommonPlugin::IsUserExists( const QString & sUsername )
{

	return true;
}

bool QCommonPlugin::CheckUser( const QString & sUsername,const QString & sPassword )
{

	return true;
}

int QCommonPlugin::GetUserLevel( const QString & sUsername,int & nLevel )
{

	return 0;
}

int QCommonPlugin::GetUserAuthorityMask( const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2 )
{

	return 0;
}