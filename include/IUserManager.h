#ifndef __IUSERMANAGER_HEAD_FILE__
#define __IUSERMANAGER_HEAD_FILE__
#include "libpcom.h"
#include <QtCore/QString>
#include <QtCore/QStringList>

interface IUserManager : public IPComBase
{
	virtual int AddUser(const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2) = 0;
	virtual int RemoveUser(const QString & sUsername) = 0;
	virtual int ModifyUserPassword(const QString & sUsername,const QString & sNewPassword) = 0;
	virtual int ModifyUserLevel(const QString & sUsername,int nLevel) = 0;
	virtual int ModifyUserAuthorityMask(const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2) = 0;
	virtual bool IsUserExists(const QString & sUsername) = 0;
	virtual bool CheckUser(const QString & sUsername,const QString & sPassword) = 0;
	virtual int GetUserLevel(const QString & sUsername,int & nLevel) = 0;
	virtual int GetUserAuthorityMask(const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2) = 0;
	virtual int GetUserCount() = 0;
	virtual QStringList GetUserList() = 0;

	enum _emError{
		OK,
		E_USER_NOT_FOUND,
		E_PASSWORD,
		E_SYSTEM_FAILED,
	};
};

#endif