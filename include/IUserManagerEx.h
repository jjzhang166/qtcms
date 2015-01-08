#ifndef __IUSERMANAGEREX_HEAD_FILE__
#define __IUSERMANAGEREX_HEAD_FILE__
#include "libpcom.h"
#include <QtCore/QString>
#include <QtCore/QStringList>

interface IUserManagerEx : public IPComBase
{
	virtual int addUser(const QString &sUserName,const QString &sPassword,quint64 uiLimit,quint64 uiLogOutInterval,QVariantMap tSubCode)=0;//0:添加用户成功；1：添加用户失败
	virtual int deleteUser(const QString &sUserName)=0;//0:删除用户成功;1:删除失败
	virtual int checkUserLimit(quint64 uiMainCode,quint64 uiSubCode)=0;//0：用户具有权限；1：用户未登录；2：用户登录但是没有权限
	virtual int login(const QString &sUserName,const QString &sPassword ,int nCode)=0;//nCode:0 表示用户请求登录，nCode:1 表示用户请求注销;
	virtual int loginEx()=0;
	virtual int setLoginOutInterval(int nTime)=0;//0:设置成功；1：设置失败（参数错误）；nTime 单位为S，最大值限定为1一个小时（3600）
	virtual int getUserList(QStringList &sUserList)=0;//0:获取成功 ；1：获取失败
	virtual int getUserLimit(QString sUserName,quint64 &uiLimit,QVariantMap &tSubCode)=0;//0:获取成功；1：获取失败
	virtual int getUserDatabaseId(QString sUserName,int &nId)=0;//0:获取成功；1：获取失败
	virtual int getLoginOutInterval(QString sUserName)=0;//返回值为时间间隔
	virtual int modifyUserInfo(const QString &sOldUserName,const QString &sNewUserName,const QString &sNewPassword,quint64 uiLimit, quint64 uiLogOutInterval, QVariantMap tSubCode)=0;//0:设置成功，1：设置失败
	virtual QString getCurrentUser()=0;
	virtual void setIsKeepCurrentUserPassWord(bool bFlags)=0;
	virtual bool getIsKeepCurrentUserPassWord(QString &sUserName,QString &sUserPassword)=0;
	virtual bool setCurrentUserInfo(QString sUserName,QString sUserPassword)=0;
	virtual int modifyCurrentUserInfo(const QString &sOldUserName,const QString &sNewUserName,const QString &sOldPassword,const QString &sNewPassword,int iLogOutInterval)=0;
};

#endif