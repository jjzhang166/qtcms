#include "qcommonplugin.h"
#include <IUserManager.h>

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
	return IUserManager::OK;
}

int QCommonPlugin::RemoveUser( const QString & sUsername )
{
	return IUserManager::OK;
}

int QCommonPlugin::ModifyUserPassword( const QString & sUsername,const QString & sNewPassword )
{
	return IUserManager::OK;
}

int QCommonPlugin::ModifyUserLevel( const QString & sUsername,int nLevel )
{
	return IUserManager::OK;
}

int QCommonPlugin::ModifyUserAuthorityMask( const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2 )
{
	return IUserManager::OK;
}

bool QCommonPlugin::IsUserExists( const QString & sUsername )
{
	return IUserManager::OK;
}

bool QCommonPlugin::CheckUser( const QString & sUsername,const QString & sPassword )
{
	return IUserManager::OK;
}

int QCommonPlugin::GetUserLevel( const QString & sUsername )
{
	int nRet = 0;
	int nR = GetUserLevel(sUsername,nRet);
	if (IUserManager::OK != nR)
	{
		return -1;
	}
	return nRet;
}

int QCommonPlugin::GetUserLevel( const QString & sUsername,int & nLevel )
{
	return IUserManager::OK;
}

QStringList QCommonPlugin::GetUserAuthorityMask( const QString & sUsername )
{
	QStringList listRet;
	int nMask1,nMask2;
	int nR = GetUserAuthorityMask(sUsername,nMask1,nMask2);
	if (IUserManager::OK != nR)
	{
		listRet.clear();
		return listRet;
	}

	listRet.insert(0,QString::number(nMask1));
	listRet.insert(1,QString::number(nMask2));
	return listRet;
}

int QCommonPlugin::GetUserAuthorityMask( const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2 )
{
	return IUserManager::OK;
}

int QCommonPlugin::GetUserCount()
{
	return 0;
}

QStringList QCommonPlugin::GetUserList()
{
	QStringList listRet;
	return listRet;
}