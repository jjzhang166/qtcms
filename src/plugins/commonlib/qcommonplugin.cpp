#include "qcommonplugin.h"
#include <IUserManager.h>
#include <IGroupManager.h>
#include <IAreaManager.h>
#include <IChannelManager.h>
#include <IDeviceManager.h>
#include <QtGui/QMessageBox>

QMutex QCommonPlugin::Group_lock;
QMutex QCommonPlugin::Area_lock;
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

/*igroupmanager Module*/
int QCommonPlugin::AddGroup(QString sName)
{
	//fix me
	Group_lock.lock();
	int group_id=-1;
	QSqlQuery _query(m_db);
	_query.prepare("insert into dev_group(name) values(:name)");
	_query.bindValue(":name",sName);
	_query.exec();

	if(_query.isActive()){
		QString command=QString("select max(id) as id from dev_group");
		QSqlQuery query2(m_db);
		bool bRet = query2.exec(command);
		int id_index=query2.record().indexOf("id");
		if(query2.isActive()){
			if(query2.first()){
				group_id=query2.value(id_index).toInt();
				Group_lock.unlock();
				return group_id;
			}
		}
	}
	Group_lock.unlock();
	return -1;
}

int QCommonPlugin::RemoveGroup(int group_ip)
{
	//fix me
	Group_lock.lock();
	if(IsGroupExists(group_ip)){
		QSqlQuery _query(m_db);
		QString command = QString("delete from dev_group where id=%1").arg(group_ip);
		_query.exec(command);
		if(_query.isActive()){
			Group_lock.unlock();
			return 0;
		}
	}
	else
		Group_lock.unlock();
		return IGroupManager::E_GROUP_NOT_FOUND;
} 

int QCommonPlugin::ModifyGroupName(int group_id,QString sName)
{
	//fix me
	Group_lock.lock();
	if(IsGroupExists(group_id)){
		QSqlQuery _query(m_db);
		QString command = QString("update dev_group set name='%1' where id=%2").arg(sName).arg(group_id);
		_query.exec(command);
		if(_query.isActive()){
			Group_lock.unlock();
			return 0;
		}
		return IGroupManager::E_SYSTEM_FAILED;
	}
		Group_lock.unlock();
		return IGroupManager::E_GROUP_NOT_FOUND;
}

int QCommonPlugin::GetGroupCount()
{
	//fix me
	Group_lock.lock();
	int nCount=0;
	QSqlQuery _query(m_db);
	QString command=QString("select * from dev_group");
	_query.exec(command);
	while(_query.next()){
		nCount++;
	}
	Group_lock.unlock();
	return nCount;
}

QStringList QCommonPlugin::GetGroupList()
{
	//fix me
	Group_lock.lock();
	QStringList CGrouplist;
	CGrouplist.clear();
	int index=0;
	QSqlQuery _query(m_db);
	QString command=QString("select * from dev_group");
	_query.exec(command);
	int id_num=_query.record().indexOf("id");
	while(_query.next()){
		CGrouplist.insert(index,_query.value(id_num).toString());
		index++;
	}
	Group_lock.unlock();
	return CGrouplist;
}

int QCommonPlugin::GetGroupName(int group_id,QString &sName)
{
	//fix me
	Group_lock.lock();
	if(IsGroupExists(group_id)){
		QSqlQuery _query(m_db);
		QString command=QString("select * from dev_group where id=%1").arg(group_id);
		_query.exec(command);
		bool isActive=_query.isActive();
		if(isActive){
			while(_query.first()){
				int name_index=_query.record().indexOf("name");
				sName.append(_query.value(name_index).toString());
				Group_lock.unlock();
				return 0;
			}
		}	
			Group_lock.unlock();
			return IGroupManager::E_SYSTEM_FAILED;
	}
		Group_lock.unlock();
		return IGroupManager::E_GROUP_NOT_FOUND;
}

QString QCommonPlugin::GetGroupName(int group_id)
{
	//fix me
	Group_lock.lock();
	QString CGroup;
	CGroup.clear();
	if(IsGroupExists(group_id)){
		QSqlQuery _query(m_db);
		QString command=QString("select * from dev_group where id=%1").arg(group_id);
		_query.exec(command);
		if(_query.isActive()){
			while(_query.first()){
				int name_index=_query.record().indexOf("name");
				CGroup.append(_query.value(name_index).toString());
				Group_lock.unlock();
				return CGroup;
			}
		}
	}
		Group_lock.unlock();
		return CGroup;
}

bool QCommonPlugin::IsGroupExists(int group_id)
{
	//fix me
	QSqlQuery _query(m_db);
	QString command = QString("select * from dev_group where id=%1").arg(group_id);
	_query.exec(command);
	if(_query.isActive()){
		if(_query.first()){
			return true;
		}
	}
	return false;
}

bool QCommonPlugin::IsChannelExists(int chl_id)
{
	//fix me
	QSqlQuery _query(m_db);
	QString command = QString("select * from chl where id=%1").arg(chl_id);
	_query.exec(command);
	if(_query.first()){
		return true;
	}
	else{
		return false;
	}
}
bool QCommonPlugin::IsR_Channel_GroupExist(int rgc_id)
{
	//fixme
	QSqlQuery _query(m_db);
	QString command = QString("select * from r_chl_group where id=%1").arg(rgc_id);
	_query.exec(command);
	if(_query.first()){
		return true;
	}
	else{
		return false;
	}
}
int QCommonPlugin::AddChannelInGroup(int group_id,int chl_id,QString sName)
{
	//fix me
	Group_lock.lock();
	if(IsGroupExists(group_id)){
		if(IsChannelExists(chl_id)){
			QSqlQuery _query(m_db);
			_query.prepare("insert into r_chl_group(chl_id,group_id,name) values(:chl_id,:group_id,:name)");
			_query.bindValue(":name",sName);
			_query.bindValue(":chl_id",chl_id);
			_query.bindValue(":group_id",group_id);
			_query.exec();
			int rgc_id=-1;
			QString command=QString("select max(id) as id from r_chl_group");
			_query.exec(command);
			if(_query.isActive()){
				int rgc_index=_query.record().indexOf("id");
				if(_query.first()){
					rgc_id=_query.value(rgc_index).toInt();
					Group_lock.unlock();
					return rgc_id;
				}
			}
			Group_lock.unlock();
			return rgc_id;
		}else{
			Group_lock.unlock();
			return -1;
		}
	}
	else{
		Group_lock.unlock();
		return -1;
	}
}

int QCommonPlugin::RemoveChannelFromGroup(int rgc_id)
{
	//fix me
	Group_lock.lock();
	if(IsR_Channel_GroupExist(rgc_id)){
		QSqlQuery _query(m_db);
		QString command = QString("delete from r_chl_group where id=%1").arg(rgc_id);
		_query.exec(command);
		if(_query.isActive()){
			Group_lock.unlock();
			return 0;
		}
		Group_lock.unlock();
		return IGroupManager::E_SYSTEM_FAILED;
	}
	Group_lock.unlock();
	return IGroupManager::E_CHANNEL_NOT_IN_GROUP;
}

int QCommonPlugin:: ModifyGroupChannelName(int rgc_id,QString sName)
{
	//fix me
	Group_lock.lock();
	if(IsR_Channel_GroupExist(rgc_id)){
		QSqlQuery _query(m_db);
		QString command = QString("update r_chl_group set name='%1' where id=%2").arg(sName).arg(rgc_id);
		_query.exec(command);
		if(_query.isActive()){
			Group_lock.unlock();
			return 0;
		}
		Group_lock.unlock();
		return IGroupManager::E_SYSTEM_FAILED;
	}
	Group_lock.unlock();
	return IGroupManager::E_CHANNEL_NOT_IN_GROUP;
}

int QCommonPlugin::MoveChannelToGroup(int rgc_id,int group_id)
{
	//fix me
	Group_lock.lock();
	if(false==IsR_Channel_GroupExist(rgc_id)){
		Group_lock.unlock();
		return IGroupManager::E_CHANNEL_NOT_IN_GROUP;
	}
	if(false==IsGroupExists(group_id)){
		Group_lock.unlock();
		return IGroupManager::E_GROUP_NOT_FOUND;
	}
	QSqlQuery _query(m_db);
	QString command = QString("update r_chl_group set group_id=%1 where id=%2").arg(group_id).arg(rgc_id);
	_query.exec(command);
	if(_query.isActive()){
		Group_lock.unlock();
		return 0;
	}
	Group_lock.unlock();
	return IGroupManager::E_SYSTEM_FAILED;
}

int QCommonPlugin::GetGroupChannelCount(int group_id)
{
	//fix me
	QSqlQuery _query(m_db);
	QString Command=QString("select * from r_chl_group where group_id=%1").arg(group_id);
	_query.exec(Command);
	int Count=0;
	if(_query.isActive()){
		if(_query.next()){
			Count++;
		}
		return Count;
	}
	return Count;
}

QStringList QCommonPlugin::GetGroupChannelList(int group_id)
{
	//fix me
	QStringList i_result;
	i_result.clear();
	if(IsGroupExists(group_id)){
		QSqlQuery _query(m_db);
		QString Command=QString("select * from r_chl_group where group_id=%1").arg(group_id);
		_query.exec(Command);
		if(_query.isActive()){
			int Id_Index=_query.record().indexOf("id");
			if(_query.next()){
				i_result.append(_query.value(Id_Index).toString());
			}
			return i_result;
		}
	}
	return i_result;
}

int QCommonPlugin::GetGroupChannelName(int rgc_id,QString & sName)
{
	//fix me
	sName.clear();
	if(false==IsR_Channel_GroupExist(rgc_id)){
		return IGroupManager::E_CHANNEL_NOT_IN_GROUP;
	}
	QSqlQuery _query(m_db);
	QString Command=QString("select * from r_chl_group where id=%1").arg(rgc_id);
	_query.exec(Command);
	if(_query.isActive()){
		int sName_Index=_query.record().indexOf("name");
		if(_query.first()){
			sName.append(_query.value(sName_Index).toString());
			return 0;
		}
	}
	return IGroupManager::E_SYSTEM_FAILED;
}

QString QCommonPlugin::GetGroupChannelName(int rgc_id)
{
	QString sName;
	sName.clear();
	if(false==IsR_Channel_GroupExist(rgc_id)){
		return sName;
	}
	QSqlQuery _query(m_db);
	QString Command=QString("select * from r_chl_group where id=%1").arg(rgc_id);
	_query.exec(Command);
	if(_query.isActive()){
		int sName_Index=_query.record().indexOf("name");
		if(_query.first()){
			sName.append(_query.value(sName_Index).toString());
			return sName;
		}
	}
	return sName;
}

int QCommonPlugin::GetChannelIdFromGroup(int rgc_id)
{
	//fix me
	if(false==IsR_Channel_GroupExist(rgc_id)){
		return IGroupManager::E_CHANNEL_NOT_IN_GROUP;
	}
	QSqlQuery _query(m_db);
	QString Command=QString("select * from r_chl_group where id=%1").arg(rgc_id);
	_query.exec(Command);
	if(_query.isActive()){
		int Chl_Index=_query.record().indexOf("chl_id");
		if(_query.first()){
			int Chl_Id;
			Chl_Id=_query.value(Chl_Index).toInt();
			return Chl_Id;
		}
	}
	return IGroupManager::E_SYSTEM_FAILED;
}

int QCommonPlugin::GetChannelIdFromGroup(int rgc_id,int &chl_id)
{
	//fix me
	if(false==IsR_Channel_GroupExist(rgc_id)){
		return IGroupManager::E_CHANNEL_NOT_IN_GROUP;
	}
	QSqlQuery _query(m_db);
	QString Command=QString("select * from r_chl_group where id=%1").arg(rgc_id);
	_query.exec(Command);
	if(_query.isActive()){
		int Chl_Index=_query.record().indexOf("chl_id");
		if(_query.first()){
			chl_id=_query.value(Chl_Index).toInt();
			return 0;
		}
	}
	return IGroupManager::E_SYSTEM_FAILED;
}

int QCommonPlugin::GetChannelInfoFromGroup(int rgc_id,int & chl_id,int & group_id, QString & sName)
{
	//fix me
	if(false==IsR_Channel_GroupExist(rgc_id)){
		return IGroupManager::E_CHANNEL_NOT_IN_GROUP;
	}
	QSqlQuery _query(m_db);
	QString Command=QString("select * from r_chl_group where id=%1").arg(rgc_id);
	_query.exec(Command);
	if(_query.isActive()){
		int Chl_Index=_query.record().indexOf("chl_id");
		int Group_Index=_query.record().indexOf("group_id");
		int sName_Index=_query.record().indexOf("name");
		if(_query.first()){
			chl_id=_query.value(Chl_Index).toInt();
			group_id=_query.value(Group_Index).toInt();
			sName=_query.value(sName_Index).toString();
			return 0;
		}
	}
	return IGroupManager::E_SYSTEM_FAILED;
}

QVariantMap QCommonPlugin::GetChannelInfoFromGroup(int rgc_id)
{
	QVariantMap i_result;
	i_result.clear();
	//fix me
	if(false==IsR_Channel_GroupExist(rgc_id)){
		return i_result;
	}
	QSqlQuery _query(m_db);
	QString Command=QString("select * from r_chl_group where id=%1").arg(rgc_id);
	_query.exec(Command);
	if(_query.isActive()){
		int Chl_Index=_query.record().indexOf("chl_id");
		int Group_Index=_query.record().indexOf("group_id");
		int sName_Index=_query.record().indexOf("name");
		if(_query.first()){
			i_result.insert("chl_id",_query.value(Chl_Index).toString());
			i_result.insert("group_id",_query.value(Group_Index).toString());
			i_result.insert("name",_query.value(sName_Index).toString());
			return i_result;
		}
	}
	return i_result;
}

/*IAreaManager module*/
int QCommonPlugin::AddArea(int nPid,QString sName)
{
	Area_lock.lock();
	QSqlQuery _query(m_db);
	int area_id=-1;
	if(false==IsAreaNameExist(sName)){
		if(0==nPid){
			_query.prepare("insert into area(pid,name) values(:nPid,:name)");
			_query.bindValue(":pid",nPid);
			_query.bindValue(":name",sName);
			_query.exec();
			if(_query.isActive()){
				QString command=QString("select max(id) as id from area");
				_query.exec(command);
				int id_index=_query.record().indexOf("id");
				if(_query.isActive()){
					if(_query.first()){
						area_id=_query.value(id_index).toInt();
						QString add_path=QString("0|");
						add_path.append(_query.value(id_index).toString()).append("|");
						QString command_addPath=QString("update area set path='%1' where id=%2").arg(add_path).arg(area_id);
						_query.exec(command_addPath);
						if(_query.isActive()){
							Area_lock.unlock();
							return area_id;
						}
					}
				}
			}
			Area_lock.unlock();
			return IAreaManager::E_SYSTEM_FAILED;
		}else if(IsAreaIdExist(nPid)){
			_query.prepare("insert into area(pid,name) values(:nPid,:name)");
			_query.bindValue(":pid",nPid);
			_query.bindValue(":name",sName);
			_query.exec();
			if(_query.isActive()){
				QString command=QString("select max(id) as id from area");
				_query.exec(command);
				int id_index=_query.record().indexOf("id");
				if(_query.isActive()){
					if(_query.first()){
						area_id=_query.value(id_index).toInt();
						QString Command_FindPath=QString("select * from area where id=%1").arg(nPid);
						_query.exec(Command_FindPath);
						if(_query.isActive()){
							int path_index=_query.record().indexOf("path");
							if(_query.first()){
								QString Path_Pid=_query.value(path_index).toString();
								Path_Pid.append(QString("%1").arg(area_id));
								Path_Pid.append("|");
								QString Command_AddPath=QString("update area set path='%1' where id=%2").arg(Path_Pid).arg(area_id);
								_query.exec(Command_AddPath);
								if(_query.isActive()){
									Area_lock.unlock();
									return area_id;
								}
							}
						}
					}
				}
			}
			Area_lock.unlock();
			return IAreaManager::E_SYSTEM_FAILED;
		}
		Area_lock.unlock();
		return IAreaManager::E_AREA_NOT_FOUND;
	}
	Area_lock.unlock();
	return IAreaManager::E_AREA_NAME_EXISTS;
}

int QCommonPlugin::RemoveAreaById(int nId)
{
	Area_lock.lock();
	if(IsAreaIdExist(nId)){
		QSqlQuery _query(m_db);
		QString command=QString("delete from area where path like ((select path from area where id =%1 ) || '%')").arg(nId);
		_query.exec(command);
		if(_query.isActive()){
			Area_lock.unlock();
			return 0;
		}
		Area_lock.unlock();
		return IAreaManager::E_SYSTEM_FAILED;
	}
	Area_lock.unlock();
	return IAreaManager::E_AREA_NOT_FOUND;
}

bool QCommonPlugin::IsAreaIdExist(int nid)
{
	if(0==nid){
		return true;
	}
	QSqlQuery _query(m_db);
	QString command=QString("select id from area where id=%1").arg(nid);
	_query.exec(command);
	if(_query.isActive()){
		while(_query.first()){
			return true;
		}
	}
		return false;
}
int QCommonPlugin::RemoveAreaByName(QString sName)
{
	Area_lock.lock();
	if(IsAreaNameExist(sName)){
		QSqlQuery _query(m_db);
		QString command=QString("delete from area where path like ((select path from area where name ='%1' ) || '%')").arg(sName);
		_query.exec(command);
		if(_query.isActive()){
			Area_lock.unlock();
			return 0;
		}
		Area_lock.unlock();
		return IAreaManager::E_SYSTEM_FAILED;
	}
	Area_lock.unlock();
	return IAreaManager::E_AREA_NAME_EXISTS;
}

int QCommonPlugin::SetAreaName(int nId,QString sName)
{
	Area_lock.lock();
	if(IsAreaIdExist(nId)){
		QSqlQuery _query(m_db);
		QString command=QString("update area set name='%1' where id=%2").arg(sName).arg(nId);
		_query.exec(command);
		if(_query.isActive()){
			Area_lock.unlock();
			return 0;
		}
		Area_lock.unlock();
		return IAreaManager::E_SYSTEM_FAILED;
	}
	Area_lock.unlock();
	return IAreaManager::E_AREA_NOT_FOUND;
}

bool QCommonPlugin::IsAreaNameExist(QString sName)
{
	QSqlQuery _query(m_db);
	QString command=QString("select name from area where name='%1'").arg(sName);
	_query.exec(command);
	if(_query.isActive()){
		while(_query.first()){
			return true;
		}
	}
		return false;
}

int QCommonPlugin::GetAreaCount()
{
	QSqlQuery _query(m_db);
	int Id_Num=0;
	QString command=QString("select * from area");
	_query.exec(command);
	if(_query.isActive()){
		while(_query.next()){
			Id_Num++;
		}
		return Id_Num;
	}
	return IAreaManager::E_SYSTEM_FAILED;
}

QStringList QCommonPlugin::GetAreaList()
{
	QSqlQuery _query(m_db);
	QStringList S_List;
	S_List.clear();
	QString command=QString("select * from area");
	_query.exec(command);
	if(_query.isActive()){
		int Id_Index=_query.record().indexOf("id");
		while(_query.next()){
			S_List.append(_query.value(Id_Index).toString());
		}
		return S_List;
	}
	return S_List;
}

QStringList QCommonPlugin::GetSubArea(int nId)
{
	QStringList S_List;
	S_List.clear();
	if(IsAreaIdExist(nId)){
		QSqlQuery _qurey(m_db);
		QString command=QString("select * from area where path like ((select path from area where id =%1 ) || '%')").arg(nId);
		_qurey.exec(command);
		if(_qurey.isActive()){
			int Id_Index=_qurey.record().indexOf("id");
			while(_qurey.next()){
				if(nId!=_qurey.value(Id_Index).toInt()){
					S_List.append(_qurey.value(Id_Index).toString());
				}
			}
			return S_List;
		}
	}
	return S_List;
}

int QCommonPlugin::GetAreaPid(int id)
{
	int Pid=-1;
	if(IsAreaIdExist(id)){
		QSqlQuery _query(m_db);
		QString command=QString("select * from area where id=%1").arg(id);
		_query.exec(command);
		if(_query.isActive()){
			int Pid_Index=_query.record().indexOf("pid");
			if(_query.first()){
				Pid=_query.value(Pid_Index).toInt();
				return Pid;
			}
			return Pid;
		}
	}
	return IAreaManager::E_AREA_NOT_FOUND;
}

QString QCommonPlugin::GetAreaName(int id)
{
	QString sName;
	sName.clear();
	if(IsAreaIdExist(id)){
		QSqlQuery _query(m_db);
		QString command=QString("select * from area where id=%1").arg(id);
		_query.exec(command);
		if(_query.isActive()){
			if(_query.first()){
				int sName_index=_query.record().indexOf("name");
				sName=_query.value(sName_index).toString();
				return sName;
			}
		}
	}
	return sName;
}

int QCommonPlugin::GetAreaInfo(int nId,int &nPid,QString &sName)
{
	if(IsAreaIdExist(nId)){
		nPid=GetAreaPid(nId);
		sName=GetAreaName(nId);
		return 0;
	}
	return IAreaManager::E_AREA_NOT_FOUND;
}

QVariantMap QCommonPlugin::GetAreaInfo(int nId)
{
	QVariantMap nRet_Map;
	nRet_Map.clear();
	if(IsAreaIdExist(nId)){
		QString nPid=QString("%1").arg(GetAreaPid(nId));
		QString sName=GetAreaName(nId);
		nRet_Map.insert(nPid,sName);
		return nRet_Map;
	}
	return nRet_Map;
}

/*IDeviceManager Module*/
bool QCommonPlugin::IsDeviceExist(int dev_id)
{
	QSqlQuery _query(m_db);
	QString command = QString("select * from dev where id='%1'").arg(dev_id);
	_query.exec(command);

	if (!_query.next())
	{
		return false;
	}
	else
	{
		return true;
	}
}


//判断指定区域内的设备是否存在
int QCommonPlugin::IsDevExistsInArea(int area_id, QString sDeviceName)
{
	QSqlQuery _query(m_db);
	QString command = QString("select id from (select * from dev where area_id='%1')where name='%2'").arg(area_id).arg(sDeviceName);
	_query.exec(command);

	if (!_query.next())
	{
		return 0;
	}
	else
	{
		int dev_id = _query.value(0).toInt();

		return dev_id;
	}
}

//添加设备信息
int QCommonPlugin::AddDevice(int area_id,
	QString sDeviceName,
	QString sAddress,
	int port,
	int http,
	QString sEseeid,
	QString sUsername,
	QString sPassword,
	int chlCount,
	int connectMethod,
	QString sVendor)
{
	if (IsDevExistsInArea(area_id,sDeviceName))
	{
		return -1;
	}

	QSqlQuery _query(m_db);
	int dev_id = 0;
	QString command;

	_query.prepare("insert into dev(area_id,address,port,http,eseeid,username,password,name,channel_count,connect_method,vendor) values(:area_id,:address,:port,:http,:eseeid,:username,:password,:name,:channel_count,:connect_method,:vendor)");
	_query.bindValue(":area_id",area_id);
	_query.bindValue(":address",sAddress);
	_query.bindValue(":port",port);
	_query.bindValue(":http",http);
	_query.bindValue(":eseeid",sEseeid);
	_query.bindValue(":username",sUsername);
	_query.bindValue(":password",sPassword);
	_query.bindValue(":name",sDeviceName);
	_query.bindValue(":channel_count",chlCount);
	_query.bindValue(":connect_method",connectMethod);
	_query.bindValue(":vendor",sVendor);
	_query.exec();
	dev_id = this->IsDevExistsInArea(area_id,sDeviceName);
	int nChl=0;
	for (;nChl<chlCount;nChl++)
	{
		_query.prepare("insert into chl(dev_id,channel_number,name,stream_id) values(:dev_id,:channel_number,:name,:stream_id)");
		_query.bindValue(":dev_id",dev_id);
		_query.bindValue(":channel_number",nChl);
		_query.bindValue(":name","chl");
		_query.bindValue(":stream_id",0);
		_query.exec();
	}
	return dev_id;
}

//删除设备dev_id的所有信息
int QCommonPlugin::RemoveDevice(int dev_id)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("delete from dev where id='%1'").arg(dev_id);

	_query.exec(command);
	QString command_chl=QString("delete from chl where id='%1'").arg(dev_id);
	_query.exec(command_chl);
	return IDeviceManager::OK;
}

//修改设备dev_id的名称
int QCommonPlugin::ModifyDeviceName(int dev_id,QString sDeviceName)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update dev set name='%1' where id='%2'").arg(sDeviceName).arg(dev_id);
	_query.exec(command);

	return IDeviceManager::OK;
}

//修改设备dev_id的IP信息
int QCommonPlugin::ModifyDeviceHost(int dev_id,QString sAddress, int port, int http)
{
	if ( port < 0 && http < 0 )
	{
		return IDeviceManager::E_SYSTEM_FAILED;
	}

	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update dev set address='%1',port='%2',http='%3' where id='%4'").arg(sAddress).arg(port).arg(http).arg(dev_id);
	_query.exec(command);

	return IDeviceManager::OK;

}

//修改设备dev_id的易视网信息
int QCommonPlugin::ModifyDeviceEseeId(int dev_id,QString sEseeId)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update dev set eseeid='%1' where id='%2'").arg(sEseeId).arg(dev_id);
	_query.exec(command);

	return IDeviceManager::OK;
}

//修改设备dev_id的登录用户信息
int QCommonPlugin::ModifyDeviceAuthority(int dev_id,QString sUsername,QString sPassword)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update dev set username='%1',password='%2' where id='%3'").arg(sUsername).arg(sPassword).arg(dev_id);
	_query.exec(command);

	return IDeviceManager::OK;
}

//将设备dev_id的最大通道数更新为chlCount
int QCommonPlugin::ModifyDeviceChannelCount(int dev_id,int chlCount)
{
	if ( chlCount < 0 )
	{
		return IDeviceManager::E_SYSTEM_FAILED;
	}

	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update dev set channel_count='%1' where id='%2'").arg(chlCount).arg(dev_id);
	_query.exec(command);

	return IDeviceManager::OK;

}

//将设备dev_id的连接方式修改为connectMethod
int QCommonPlugin::ModifyDeviceConnectMethod(int dev_id,int connectMethod)
{
	if ( connectMethod < 0 )
	{
		return IDeviceManager::E_SYSTEM_FAILED;
	}

	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update dev set connect_method='%1' where id='%2'").arg(connectMethod).arg(dev_id);
	_query.exec(command);

	return IDeviceManager::OK;
}

//获取设备dev_id的厂商信息
int QCommonPlugin::ModifyDeviceVendor(int dev_id,QString sVendor)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("update dev set vendor='%1' where id='%2'").arg(sVendor).arg(dev_id);
	_query.exec(command);

	return IDeviceManager::OK;
}

//获取区域area_id下的设备总数
int QCommonPlugin::GetDeviceCount(int area_id)
{
	if ( !IsAreaIdExist(area_id) )
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select count(*) from dev where area_id='%1'").arg(area_id);
	_query.exec(command);

	if (_query.next())
	{
		int dev_num = _query.value(0).toInt();

		return dev_num;
	}
	else
	{
		return 0;
	}
}

//获取区域area_id下的设备列表
QStringList QCommonPlugin::GetDeviceList(int area_id)
{
	QStringList dev_list;

	if ( !IsAreaIdExist(area_id) )
	{
		return dev_list;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select id from dev where area_id='%1'").arg(area_id);
	_query.exec(command);

	int index = 0;
	while (_query.next())
	{
		dev_list.insert(index,_query.value(0).toString());
		index++;
	}

	return dev_list;
}

//获取设备dev_id的设备名称
int QCommonPlugin::GetDeviceName(int dev_id,QString & sName)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select name from dev where id='%1'").arg(dev_id);
	_query.exec(command);

	if (_query.next())
	{
		sName = _query.value(0).toString();

		return IDeviceManager::OK;
	}
	else
	{
		sName = "";

		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的IP信息
int QCommonPlugin::GetDeviceHost(int dev_id,QString & sAddress,int & nPort,int &http)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select address,port,http from dev where id='%1'").arg(dev_id);
	_query.exec(command);

	if (_query.next())
	{
		sAddress = _query.value(0).toString();
		nPort = _query.value(1).toInt();
		http = _query.value(2).toInt();

		return IDeviceManager::OK;
	}
	else
	{
		sAddress = "";
		nPort = 0;
		http = 0;

		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的易视网信息
int QCommonPlugin::GetDeviceEseeId(int dev_id,QString & sEseeid)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select eseeid from dev where id='%1'").arg(dev_id);
	_query.exec(command);

	if (_query.next())
	{
		sEseeid = _query.value(0).toString();

		return IDeviceManager::OK;
	}
	else
	{
		sEseeid = "";

		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的登录信息
int QCommonPlugin::GetDeviceLoginInfo(int dev_id,QString &sUsername,QString & sPassword)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select username,password from dev where id='%1'").arg(dev_id);
	_query.exec(command);

	if (_query.next())
	{
		sUsername = _query.value(0).toString();
		sPassword = _query.value(1).toString();

		return IDeviceManager::OK;
	}
	else
	{
		sUsername = "";
		sPassword = "";

		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的连接方式
int QCommonPlugin::GetDeviceConnectMethod(int dev_id,int & connectMethod)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select connect_method from dev where id='%1'").arg(dev_id);
	_query.exec(command);

	if (_query.next())
	{
		connectMethod = _query.value(0).toInt();

		return IDeviceManager::OK;
	}
	else
	{
		connectMethod = 0;

		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的厂商信息
int QCommonPlugin::GetDevicdVendor(int dev_id,QString & sVendor)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select vendor from dev where id='%1'").arg(dev_id);
	_query.exec(command);

	if (_query.next())
	{
		sVendor = _query.value(0).toString();

		return IDeviceManager::OK;
	}
	else
	{
		sVendor = "";

		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的完整信息
int QCommonPlugin::GetDeviceInfo(int dev_id,
	QString & sDeviceName, 
	QString & sAddress, 
	int & port, 
	int & http, 
	QString & sEseeid, 
	QString & sUsername,
	QString &sPassword, 
	int & connectMethod, 
	QString & sVendor)
{
	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select name,address,port,http,eseeid,username,password,connect_method,vendor from dev where id='%1'").arg(dev_id);
	_query.exec(command);

	if (_query.next())
	{
		sDeviceName		= _query.value(0).toString();
		sAddress		= _query.value(1).toString();
		port			= _query.value(2).toInt();
		http			= _query.value(3).toInt();
		sEseeid			= _query.value(4).toString();
		sUsername		= _query.value(5).toString();
		sPassword		= _query.value(6).toString();
		connectMethod	= _query.value(7).toInt();
		sVendor			= _query.value(8).toString();

		return IDeviceManager::OK;
	}
	else
	{
		sDeviceName		= "";
		sAddress		= "";
		port			= 0;
		http			= 0;
		sEseeid			= "";
		sUsername		= "";
		sPassword		= "";
		connectMethod	= 0;
		sVendor			= "";

		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的完整信息
QVariantMap QCommonPlugin::GetDeviceInfo(int dev_id)
{
	QVariantMap dev_info;

	dev_info.insert("name","");
	dev_info.insert("address","");
	dev_info.insert("port",0);
	dev_info.insert("http",0);
	dev_info.insert("eseeid","");
	dev_info.insert("username","");
	dev_info.insert("password","");
	dev_info.insert("method",0);
	dev_info.insert("vendor","");

	if (!IsDeviceExist(dev_id))
	{
		return dev_info;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select name,address,port,http,eseeid,username,password,connect_method,vendor from dev where id='%1'").arg(dev_id);
	_query.exec(command);

	if (_query.next())
	{
		dev_info.insert("name",_query.value(0).toString());
		dev_info.insert("address",_query.value(1).toString());
		dev_info.insert("port",_query.value(2).toInt());
		dev_info.insert("http",_query.value(3).toInt());
		dev_info.insert("eseeid",_query.value(4).toString());
		dev_info.insert("username",_query.value(5).toString());
		dev_info.insert("password",_query.value(6).toString());
		dev_info.insert("method",_query.value(7).toInt());
		dev_info.insert("vendor",_query.value(8).toString());
	}

	return dev_info;
}

/*ChannelMangeger module*/

// 修改通道chl_id的名称为sName。
int QCommonPlugin::ModifyChannelName(int chl_id, QString sName) 
{
	// check if channel exists
	if (!IsChannelExists(chl_id))
	{
		return IChannelManager::E_CHANNEL_NOT_FOUND;
	}
	QSqlQuery _query(m_db);
	QString command = QString("update chl set name='%1' where id='%2'").arg(sName).arg(chl_id);
	_query.exec(command);
	return IChannelManager::OK;
}

// 修改通道chl_id的码流为nStream。
int QCommonPlugin::ModifyChannelStream(int chl_id,int nStream) 
{
	// check if channel exists
	if (!IsChannelExists(chl_id))
	{
		return IChannelManager::E_CHANNEL_NOT_FOUND;
	}
	QSqlQuery _query(m_db);
	QString command = QString("update chl set stream_id='%1' where id='%2'").arg(nStream).arg(chl_id);
	_query.exec(command);
	return IChannelManager::OK;
}

// 获取设备dev_id下的通道数。
int QCommonPlugin::GetChannelCount(int dev_id) 
{
	QSqlQuery _query(m_db);
	QString command = QString("select channel_count from dev where id='%1'").arg(dev_id);
	_query.exec(command);
	int channel_count = -1;
	while(_query.next())
	{
		channel_count = _query.value(0).toInt();
	}
	return channel_count;
}

// 获取设备dev_id下的通道列表
QStringList QCommonPlugin::GetChannelList(int dev_id) 
{
	//QStringList listRet;
	//QSqlQuery _query(m_db);
	//QString command = QString("select id from chl where dev_id = '%1' group by dev_id").arg(dev_id);
	//_query.exec(command);

	//QSqlRecord rec = _query.record();
	//int nCol = rec.indexOf("id");

	//int index = 0;
	//while (_query.next())
	//{
	//	listRet.insert(index ++,_query.value(nCol).toString());
	//}
	//if (listRet.isEmpty())
	//{
	//	return listRet;
	//}

	//qDebug("%d",listRet.count());
	//return listRet;
	QSqlQuery _query(m_db);
	QStringList S_List;
	S_List.clear();
//	QString command= QString("select * from chl where dev_id = '%1' group by dev_id").arg(dev_id);
	QString command= QString("select * from chl where dev_id = '%1'").arg(dev_id);
	_query.exec(command);
	if(_query.isActive()){
		int Id_Index=_query.record().indexOf("id");
		while(_query.next()){
			S_List.append(_query.value(Id_Index).toString());
		}
		return S_List;
	}
	return S_List;
}

// 获取通道chl_id的名称，在sName中返回。
int QCommonPlugin::GetChannelName(int chl_id,QString & sName)
{
	// check if channel exists
	if (!IsChannelExists(chl_id))
	{
		return IChannelManager::E_CHANNEL_NOT_FOUND;
	}
	QSqlQuery _query(m_db);
	QString command = QString("select name from chl where id='%1'").arg(chl_id);
	_query.exec(command);

	while(_query.next())
	{
		sName = _query.value(0).toString();
	}

	return IChannelManager::OK;
}

// 获取通道chl_id当前的码流信息，在nStream中返回。
int QCommonPlugin::GetChannelStream(int chl_id,int & nStream) 
{
	// check if channel exists
	if (!IsChannelExists(chl_id))
	{
		return IChannelManager::E_CHANNEL_NOT_FOUND;
	}
	QSqlQuery _query(m_db);
	QString command = QString("select stream_id from chl where id='%1'").arg(chl_id);
	_query.exec(command);

	while(_query.next())
	{
		nStream = _query.value(0).toInt();
	}
	return IChannelManager::OK;
}

// 获取通道chl_id的通道号，在nChannelNum中返回
int QCommonPlugin::GetChannelNumber(int chl_id,int & nChannelNum) 
{
	// check if channel exists
	if (!IsChannelExists(chl_id))
	{
		return IChannelManager::E_CHANNEL_NOT_FOUND;
	}
	QSqlQuery _query(m_db);
	QString command = QString("select channel_number from chl where id='%1'").arg(chl_id);
	_query.exec(command);

	while(_query.next())
	{
		nChannelNum = _query.value(0).toInt();
	}

	return IChannelManager::OK;
}

// 获取通道chl_id的相关信息，通道名称在sName中返回，通道码流在nStream中返回，通道号在nChannelNum中返回。
int QCommonPlugin::GetChannelInfo(int chl_id,QString &sName,int &nStream,int &nChannelNum)
{
	// check if channel exists
	if (!IsChannelExists(chl_id))
	{
		return IChannelManager::E_CHANNEL_NOT_FOUND;
	}

	QSqlQuery _query(m_db);
	QString command = QString("select name,stream_id,channel_number from chl where id='%1'").arg(chl_id);
	_query.exec(command);
	while(_query.next())
	{
		sName       = _query.value(0).toString();
		nStream     = _query.value(1).toInt();
		nChannelNum = _query.value(2).toInt();
	}
	return IChannelManager::OK;
}

// 获取通道chl_id的相关信息，通道名称通过键值"name"访问，通道码流通过键值"stream"访问，通道号通过键值"number"访问。
QVariantMap QCommonPlugin::GetChannelInfo(int chl_id)
{
	QVariantMap chl_info;
	chl_info.insert("name"  , "");
	chl_info.insert("stream", 0);
	chl_info.insert("number", 0);

	// check if channel exists
	if (!IsChannelExists(chl_id))
	{
		return chl_info;
	}

	QSqlQuery _query(m_db);

	QString command = QString("select name,stream_id,channel_number from chl where id='%1'").arg(chl_id);
	_query.exec(command);

	if (_query.next())
	{
		chl_info.insert("name"    ,_query.value(0).toString());
		chl_info.insert("stream"  ,_query.value(1).toInt());
		chl_info.insert("number"  ,_query.value(2).toInt());
	}

	return chl_info;
}
