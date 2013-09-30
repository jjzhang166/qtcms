#include "qcommonplugin.h"
#include <IUserManager.h>
#include <QtGui/QMessageBox>

int QCommonPlugin::m_randSeed = 0;
QMutex QCommonPlugin::m_csRandSeed;

QCommonPlugin::QCommonPlugin(QWidget *parent)
	: QWidget(parent),
	QWebPluginFWBase(this)
{
	m_csRandSeed.lock();
	int nConnectionSerialId = m_randSeed ++;
	m_csRandSeed.unlock();

	m_sDbConnectionName = QString("jacms") + QString::number(nConnectionSerialId);
	m_db = QSqlDatabase::addDatabase("QSQLITE",m_sDbConnectionName);
	QString sAppPath = QCoreApplication::applicationDirPath();
	QString sDatabasePath = sAppPath + "/system.db";
	m_db.setDatabaseName(sDatabasePath);
	if (!m_db.open())
	{
		qDebug("InitDatabase failed!!!");	
	}
}

QCommonPlugin::~QCommonPlugin()
{
	m_db.close();
	QSqlDatabase::removeDatabase(m_sDbConnectionName);
}

//添加用户
int QCommonPlugin::AddUser( const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2 )
{
	if (IsUserExists(sUsername))
	{
		return IUserManager::E_USER_EXISTS;
	}

	QSqlQuery _query(m_db);
	_query.prepare("insert into user_infomation(username,password,level,mask1,mask2) values(:username,:password,:level,:mask1,:mask2)");
	_query.bindValue(":username",sUsername);
	_query.bindValue(":password",QString(QCryptographicHash::hash(sPassword.toLatin1(),QCryptographicHash::Md5).toHex()));
	_query.bindValue(":level",nLevel);
	_query.bindValue(":mask1",nAuthorityMask1);
	_query.bindValue(":maks2",nAuthorityMask2);
    _query.exec();

	return IUserManager::OK;
}

//删除用户
int QCommonPlugin::RemoveUser( const QString & sUsername )
{
	// check if user exists
	if (!IsUserExists(sUsername))
	{
		return IUserManager::E_USER_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("delete from user_infomation where username='%1'").arg(sUsername);
	if (!QString::compare(sUsername,QString("admin")))
	{
		return IUserManager::E_SYSTEM_FAILED;
	}
	_query.exec(command);

	return IUserManager::OK;
}

//修改密码
int QCommonPlugin::ModifyUserPassword( const QString & sUsername,const QString & sNewPassword )
{
	// check if user exists
	if (!IsUserExists(sUsername))
	{
		return IUserManager::E_USER_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update user_infomation set password='%1' where username='%2'").arg(QCryptographicHash::hash(sNewPassword.toLatin1(),QCryptographicHash::Md5).toHex().data()).arg(sUsername);
	_query.exec(command);

	return IUserManager::OK;
}

// 修改权限
int QCommonPlugin::ModifyUserLevel( const QString & sUsername,int nLevel )
{	
	// check if user exists
	if (!IsUserExists(sUsername))
	{
		return IUserManager::E_USER_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update user_infomation set level='%1' where username='%2'").arg(nLevel).arg(sUsername);
	if (nLevel < 0 || nLevel > 3)
	{
		return IUserManager::E_SYSTEM_FAILED;
	}
	_query.exec(command);

	return IUserManager::OK;
}

//修改详细权限
int QCommonPlugin::ModifyUserAuthorityMask( const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2 )
{
	// check if user exists
	if (!IsUserExists(sUsername))
	{
		return IUserManager::E_USER_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update user_infomation set mask1='%1',mask2='%2' where username='%3'").arg(nAuthorityMask1).arg(nAuthorityMask2).arg(sUsername);
	_query.exec(command);
	
	return IUserManager::OK;
}

//判断用户是否存在,存在返回true 
bool QCommonPlugin::IsUserExists( const QString & sUsername )
{
	QSqlQuery _query(m_db);
	QString command = QString("select * from user_infomation where username='%1'").arg(sUsername);
	_query.exec(command);

	return _query.next();
}

//检查用户名和密码是否正确
bool QCommonPlugin::CheckUser( const QString & sUsername,const QString & sPassword )
{
	// check if user exists
	if (!IsUserExists(sUsername))
	{
		return IUserManager::E_USER_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select password from user_infomation where username='%1'").arg(sUsername);
	_query.exec(command);
	while(_query.next())
	{
		QString sPasswd = _query.value(0).toString();
		if (!QString::compare(sPasswd,QCryptographicHash::hash(sPassword.toLatin1(),QCryptographicHash::Md5).toHex().data()))
			return true;
		else
			return false;
	}
	return false;
}

//获取用户权限值
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
	// check if user exists
	if (!IsUserExists(sUsername))
	{
		return IUserManager::E_USER_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select level from user_infomation where username='%1'").arg(sUsername);
	_query.exec(command);
	while(_query.next())
	{
		nLevel = _query.value(0).toInt();
	}

	return IUserManager::OK;
}

//获取用户详细权限
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
	// check if user exists
	if (!IsUserExists(sUsername))
	{
		return IUserManager::E_USER_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select mask1,mask2 from user_infomation where username='%1'").arg(sUsername);
	_query.exec(command);
	while(_query.next())
	{
		nAuthorityMask1 = _query.value(0).toInt();
		nAuthorityMask2 = _query.value(1).toInt();
	}
	return IUserManager::OK;
}

//获取系统中的用户数
int QCommonPlugin::GetUserCount()
{
	int nCount = 0;
	QSqlQuery _query(m_db);
	QString command = QString("select * from user_infomation");
	_query.exec(command);
	while(_query.next())
	{
		nCount++;
	}
	return nCount;
}

//获取用户列表
QStringList QCommonPlugin::GetUserList()
{
	QStringList listRet;
	QSqlQuery _query(m_db);
	QString command = QString("select username from user_infomation");
	_query.exec(command);

	QSqlRecord rec = _query.record();
	int nCol = rec.indexOf("username");

	int index = 0;
	while (_query.next())
	{
		listRet.insert(index ++,_query.value(nCol).toString());
	}

	qDebug("%d",listRet.count());

	return listRet;
}