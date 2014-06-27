#include "qcommonpluginEx.h"
#include <QtGui/QMessageBox>

qcommonpluginEx::qcommonpluginEx(QWidget *parent)
	: QWidget(parent),
	QWebPluginFWBase(this),
	m_pDeviceManager(NULL)
{
	pcomCreateInstance(CLSID_CommonlibEx,NULL,IID_IDeviceManager,(void**)&m_pDeviceManager);
}

qcommonpluginEx::~qcommonpluginEx()
{
	if (NULL!=m_pDeviceManager)
	{
		m_pDeviceManager->Release();
		m_pDeviceManager=NULL;
	}
}

//添加用户
int qcommonpluginEx::AddUser( const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2 )
{
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		int nret=pUserManager->AddUser(sUsername,sPassword,nLevel,nAuthorityMask1,nAuthorityMask2);
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return IUserManager::E_SYSTEM_FAILED;
	}
}

//删除用户
int qcommonpluginEx::RemoveUser( const QString & sUsername )
{
	// check if user exists
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		int nret=pUserManager->RemoveUser(sUsername);
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return IUserManager::E_SYSTEM_FAILED;
	}
}

//修改密码
int qcommonpluginEx::ModifyUserPassword( const QString & sUsername,const QString & sNewPassword )
{
	// check if user exists
	// check if user exists
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		int nret=pUserManager->ModifyUserPassword(sUsername,sNewPassword);
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return IUserManager::E_SYSTEM_FAILED;
	}
}

// 修改权限
int qcommonpluginEx::ModifyUserLevel( const QString & sUsername,int nLevel )
{	
	// check if user exists
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		int nret=pUserManager->ModifyUserLevel(sUsername,nLevel);
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return IUserManager::E_SYSTEM_FAILED;
	}
}

//修改详细权限
int qcommonpluginEx::ModifyUserAuthorityMask( const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2 )
{
	// check if user exists
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		int nret=pUserManager->ModifyUserAuthorityMask(sUsername,nAuthorityMask1,nAuthorityMask2);
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return IUserManager::E_SYSTEM_FAILED;
	}
}

//判断用户是否存在,存在返回true 
bool qcommonpluginEx::IsUserExists( const QString & sUsername )
{
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		bool nret=pUserManager->IsUserExists(sUsername);
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return false;
	}
}

//检查用户名和密码是否正确
bool qcommonpluginEx::CheckUser( const QString & sUsername,const QString & sPassword )
{
	// check if user exists
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		int nret=pUserManager->CheckUser(sUsername,sPassword);
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return false;
	}
}

//获取用户权限值
int qcommonpluginEx::GetUserLevel( const QString & sUsername )
{
	int nRet = 0;
	int nR = GetUserLevel(sUsername,nRet);
	if (IUserManager::OK != nR)
	{
		return -1;
	}
	return nRet;
}

int qcommonpluginEx::GetUserLevel( const QString & sUsername,int & nLevel )
{
	// check if user exists
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		int nret=pUserManager->GetUserLevel(sUsername,nLevel);
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return IUserManager::E_SYSTEM_FAILED;
	}
}

//获取用户详细权限
QStringList qcommonpluginEx::GetUserAuthorityMask( const QString & sUsername )
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

int qcommonpluginEx::GetUserAuthorityMask( const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2 )
{
	// check if user exists
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		int nret=pUserManager->GetUserAuthorityMask(sUsername,nAuthorityMask1,nAuthorityMask2);
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return IUserManager::E_SYSTEM_FAILED;
	}
}

//获取系统中的用户数
int qcommonpluginEx::GetUserCount()
{
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	if (NULL!=pUserManager)
	{
		int nret=pUserManager->GetUserCount();
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return IUserManager::E_SYSTEM_FAILED;
	}
}

//获取用户列表
QStringList qcommonpluginEx::GetUserList()
{
	IUserManager *pUserManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IUserManager,(void**)&pUserManager);
	QStringList nret;
	if (NULL!=pUserManager)
	{
		nret=pUserManager->GetUserList();
		pUserManager->Release();
		pUserManager=NULL;
		return nret;
	}else{
		return nret;
	}
}

/*iGroupManager Module*/
int qcommonpluginEx::AddGroup(QString sName)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->AddGroup(sName);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx::RemoveGroup(int group_ip)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->RemoveGroup(group_ip);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
} 

int qcommonpluginEx::ModifyGroupName(int group_id,QString sName)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->ModifyGroupName(group_id,sName);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx::GetGroupCount()
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->GetGroupCount();
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

QStringList qcommonpluginEx::GetGroupList()
{
	//fix me
	QStringList nret;
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		nret=pGroupManger->GetGroupList();
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return nret;
	}
}

int qcommonpluginEx::GetGroupName(int group_id,QString &sName)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->GetGroupName(group_id,sName);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

QString qcommonpluginEx::GetGroupName(int group_id)
{
	//fix me
	QString nret;
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		nret=pGroupManger->GetGroupName(group_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return nret;
	}
}

bool qcommonpluginEx::IsGroupExists(int group_id)
{
	//fix me
	bool nret=false;
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		nret=pGroupManger->IsGroupExists(group_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return nret;
	}
}

bool qcommonpluginEx::IsChannelExists(int chl_id)
{
	//fix me
	bool nret=false;
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		nret=pGroupManger->IsChannelExists(chl_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return nret;
	}
}
bool qcommonpluginEx::IsR_Channel_GroupExist(int rgc_id)
{
	//fixme
	bool nret=false;
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		nret=pGroupManger->IsR_Channel_GroupExist(rgc_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return nret;
	}
}
int qcommonpluginEx::AddChannelInGroup(int group_id,int chl_id,QString sName)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->AddChannelInGroup(group_id,chl_id,sName);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx::RemoveChannelFromGroup(int rgc_id)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->RemoveChannelFromGroup(rgc_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx:: ModifyGroupChannelName(int rgc_id,QString sName)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->ModifyGroupChannelName(rgc_id,sName);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx::MoveChannelToGroup(int rgc_id,int group_id)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->MoveChannelToGroup(rgc_id,group_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx::GetGroupChannelCount(int group_id)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->GetGroupChannelCount(group_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

QStringList qcommonpluginEx::GetGroupChannelList(int group_id)
{
	//fix me
	QStringList nret;
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		nret=pGroupManger->GetGroupChannelList(group_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return nret;
	}
}

int qcommonpluginEx::GetGroupChannelName(int rgc_id,QString & sName)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->GetGroupChannelName(rgc_id,sName);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

QString qcommonpluginEx::GetGroupChannelName(int rgc_id)
{
	QString nret;
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		nret=pGroupManger->GetGroupChannelName(rgc_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return nret;
	}
}

int qcommonpluginEx::GetChannelIdFromGroup(int rgc_id)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->GetChannelIdFromGroup(rgc_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx::GetChannelIdFromGroup(int rgc_id,int &chl_id)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->GetChannelIdFromGroup(rgc_id,chl_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx::GetChannelInfoFromGroup(int rgc_id,int & chl_id,int & group_id, QString & sName)
{
	//fix me
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	if (NULL!=pGroupManger)
	{
		int nret=pGroupManger->GetChannelInfoFromGroup(rgc_id,chl_id,group_id,sName);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return IGroupManager::E_SYSTEM_FAILED;
	}
}

QVariantMap qcommonpluginEx::GetChannelInfoFromGroup(int rgc_id)
{
	IGroupManager *pGroupManger=NULL;
	m_pDeviceManager->QueryInterface(IID_IGroupManager,(void**)&pGroupManger);
	QVariantMap nret;
	if (NULL!=pGroupManger)
	{
		nret=pGroupManger->GetChannelInfoFromGroup(rgc_id);
		pGroupManger->Release();
		pGroupManger=NULL;
		return nret;
	}else{
		return nret;
	}
}

/*IAreaManager module*/
int qcommonpluginEx::AddArea(int nPid,QString sName)
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		int nret=pAreaManager->AddArea(nPid,sName);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return IAreaManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx::RemoveAreaById(int nId)
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		int nret=pAreaManager->RemoveAreaById(nId);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return IAreaManager::E_SYSTEM_FAILED;
	}
}

bool qcommonpluginEx::IsAreaIdExist(int nid)
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		bool nret=pAreaManager->IsAreaIdExist(nid);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return false;
	}
}
int qcommonpluginEx::RemoveAreaByName(QString sName)
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		int nret=pAreaManager->RemoveAreaByName(sName);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return IAreaManager::E_SYSTEM_FAILED;
	}
}

int qcommonpluginEx::SetAreaName(int nId,QString sName)
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		int nret=pAreaManager->SetAreaName(nId,sName);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return IAreaManager::E_SYSTEM_FAILED;
	}
}

bool qcommonpluginEx::IsAreaNameExist(QString sName)
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		bool nret=pAreaManager->IsAreaNameExist(sName);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return false;
	}
}

int qcommonpluginEx::GetAreaCount()
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		int nret=pAreaManager->GetAreaCount();
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return IAreaManager::E_SYSTEM_FAILED;
	}
}

QStringList qcommonpluginEx::GetAreaList()
{
	QStringList nret;
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		nret=pAreaManager->GetAreaList();
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return nret;
	}
}

QStringList qcommonpluginEx::GetSubArea(int nId)
{
	QStringList nret;
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		nret=pAreaManager->GetSubArea(nId);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return nret;
	}
}

int qcommonpluginEx::GetAreaPid(int id)
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		int nret=pAreaManager->GetAreaPid(id);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return IAreaManager::E_SYSTEM_FAILED;
	}
}

QString qcommonpluginEx::GetAreaName(int id)
{
	QString nret;
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		nret=pAreaManager->GetAreaName(id);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return nret;
	}
}

int qcommonpluginEx::GetAreaInfo(int nId,int &nPid,QString &sName)
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	if (NULL!=pAreaManager)
	{
		int nret=pAreaManager->GetAreaInfo(nId,nPid,sName);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return IAreaManager::E_SYSTEM_FAILED;
	}
}

QVariantMap qcommonpluginEx::GetAreaInfo(int nId)
{
	IAreaManager *pAreaManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IAreaManager,(void**)&pAreaManager);
	QVariantMap nret;
	if (NULL!=pAreaManager)
	{
		nret=pAreaManager->GetAreaInfo(nId);
		pAreaManager->Release();
		pAreaManager=NULL;
		return nret;
	}else{
		return nret;
	}
}

/*IDeviceManager Module*/
bool qcommonpluginEx::IsDeviceExist(int dev_id)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		bool nret=pDeviceManager->IsDeviceExist(dev_id);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return false;
	}
}


//判断指定区域内的设备是否存在
int qcommonpluginEx::IsDevExistsInArea(int area_id, QString sDeviceName)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->IsDevExistsInArea(area_id,sDeviceName);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//添加设备信息
int qcommonpluginEx::AddDevice(int area_id,
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
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->AddDevice(area_id,sDeviceName,sAddress,port,http,sEseeid,sUsername,sPassword,chlCount,connectMethod,sVendor);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//删除设备dev_id的所有信息
int qcommonpluginEx::RemoveDevice(int dev_id)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->RemoveDevice(dev_id);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//修改设备dev_id的名称
int qcommonpluginEx::ModifyDeviceName(int dev_id,QString sDeviceName)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->ModifyDeviceName(dev_id,sDeviceName);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//修改设备dev_id的IP信息
int qcommonpluginEx::ModifyDeviceHost(int dev_id,QString sAddress, int port, int http)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->ModifyDeviceHost(dev_id,sAddress,port,http);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//修改设备dev_id的易视网信息
int qcommonpluginEx::ModifyDeviceEseeId(int dev_id,QString sEseeId)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->ModifyDeviceEseeId(dev_id,sEseeId);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//修改设备dev_id的登录用户信息
int qcommonpluginEx::ModifyDeviceAuthority(int dev_id,QString sUsername,QString sPassword)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->ModifyDeviceAuthority(dev_id,sUsername,sPassword);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//将设备dev_id的最大通道数更新为chlCount
int qcommonpluginEx::ModifyDeviceChannelCount(int dev_id,int chlCount)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->ModifyDeviceChannelCount(dev_id,chlCount);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//将设备dev_id的连接方式修改为connectMethod
int qcommonpluginEx::ModifyDeviceConnectMethod(int dev_id,int connectMethod)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->ModifyDeviceConnectMethod(dev_id,connectMethod);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的厂商信息
int qcommonpluginEx::ModifyDeviceVendor(int dev_id,QString sVendor)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->ModifyDeviceVendor(dev_id,sVendor);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取区域area_id下的设备总数
int qcommonpluginEx::GetDeviceCount(int area_id)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->GetDeviceCount(area_id);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取区域area_id下的设备列表
QStringList qcommonpluginEx::GetDeviceList(int area_id)
{
	IDeviceManager *pDeviceManager=NULL;
	QStringList nret;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		nret=pDeviceManager->GetDeviceList(area_id);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return nret;
	}
}

//获取设备dev_id的设备名称
int qcommonpluginEx::GetDeviceName(int dev_id,QString & sName)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->GetDeviceName(dev_id,sName);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的IP信息
int qcommonpluginEx::GetDeviceHost(int dev_id,QString & sAddress,int & nPort,int &http)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->GetDeviceHost(dev_id,sAddress,nPort,http);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的易视网信息
int qcommonpluginEx::GetDeviceEseeId(int dev_id,QString & sEseeid)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->GetDeviceEseeId(dev_id,sEseeid);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的登录信息
int qcommonpluginEx::GetDeviceLoginInfo(int dev_id,QString &sUsername,QString & sPassword)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->GetDeviceLoginInfo(dev_id,sUsername,sPassword);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}

	if (!IsDeviceExist(dev_id))
	{
		return IDeviceManager::E_DEVICE_NOT_FOUND;
	}
}

//获取设备dev_id的连接方式
int qcommonpluginEx::GetDeviceConnectMethod(int dev_id,int & connectMethod)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->GetDeviceConnectMethod(dev_id,connectMethod);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的厂商信息
int qcommonpluginEx::GetDevicdVendor(int dev_id,QString & sVendor)
{
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->GetDevicdVendor(dev_id,sVendor);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的完整信息
int qcommonpluginEx::GetDeviceInfo(int dev_id,
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
	IDeviceManager *pDeviceManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		int nret=pDeviceManager->GetDeviceInfo(dev_id,sDeviceName,sAddress,port,http,sEseeid,sUsername,sPassword,connectMethod,sVendor);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return IDeviceManager::E_SYSTEM_FAILED;
	}
}

//获取设备dev_id的完整信息
QVariantMap qcommonpluginEx::GetDeviceInfo(int dev_id)
{
	IDeviceManager *pDeviceManager=NULL;
	QVariantMap nret;
	m_pDeviceManager->QueryInterface(IID_IDeviceManager,(void**)&pDeviceManager);
	if (NULL!=pDeviceManager)
	{
		nret=pDeviceManager->GetDeviceInfo(dev_id);
		pDeviceManager->Release();
		pDeviceManager=NULL;
		return nret;
	}else{
		return nret;
	}
}

/*ChannelMangeger module*/

// 修改通道chl_id的名称为sName。
int qcommonpluginEx::ModifyChannelName(int chl_id, QString sName) 
{
	// check if channel exists
	IChannelManager *pChannelManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		int nret=pChannelManager->ModifyChannelName(chl_id,sName);
		pChannelManager->Release();
		pChannelManager=NULL;
		return nret;
	}else{
		return IChannelManager::E_SYSTEM_FAILED;
	}
}

// 修改通道chl_id的码流为nStream。
int qcommonpluginEx::ModifyChannelStream(int chl_id,int nStream) 
{
	// check if channel exists
	IChannelManager *pChannelManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		int nret=pChannelManager->ModifyChannelStream(chl_id,nStream);
		pChannelManager->Release();
		pChannelManager=NULL;
		return nret;
	}else{
		return IChannelManager::E_SYSTEM_FAILED;
	}
}

// 获取设备dev_id下的通道数。
int qcommonpluginEx::GetChannelCount(int dev_id) 
{
	IChannelManager *pChannelManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		int nret=pChannelManager->GetChannelCount(dev_id);
		pChannelManager->Release();
		pChannelManager=NULL;
		return nret;
	}else{
		return IChannelManager::E_SYSTEM_FAILED;
	}
}

// 获取设备dev_id下的通道列表
QStringList qcommonpluginEx::GetChannelList(int dev_id) 
{
	IChannelManager *pChannelManager=NULL;
	QStringList nret;
	m_pDeviceManager->QueryInterface(IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		nret=pChannelManager->GetChannelList(dev_id);
		pChannelManager->Release();
		pChannelManager=NULL;
		return nret;
	}else{
		return nret;
	}
}

// 获取通道chl_id的名称，在sName中返回。
int qcommonpluginEx::GetChannelName(int chl_id,QString & sName)
{
	// check if channel exists
	IChannelManager *pChannelManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		int nret=pChannelManager->GetChannelName(chl_id,sName);
		pChannelManager->Release();
		pChannelManager=NULL;
		return nret;
	}else{
		return IChannelManager::E_SYSTEM_FAILED;
	}
}

// 获取通道chl_id当前的码流信息，在nStream中返回。
int qcommonpluginEx::GetChannelStream(int chl_id,int & nStream) 
{
	// check if channel exists
	IChannelManager *pChannelManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		int nret=pChannelManager->GetChannelStream(chl_id,nStream);
		pChannelManager->Release();
		pChannelManager=NULL;
		return nret;
	}else{
		return IChannelManager::E_SYSTEM_FAILED;
	}
}

// 获取通道chl_id的通道号，在nChannelNum中返回
int qcommonpluginEx::GetChannelNumber(int chl_id,int & nChannelNum) 
{
	// check if channel exists
	
	IChannelManager *pChannelManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		int nret=pChannelManager->GetChannelNumber(chl_id,nChannelNum);
		pChannelManager->Release();
		pChannelManager=NULL;
		return nret;
	}else{
		return IChannelManager::E_SYSTEM_FAILED;
	}
}

// 获取通道chl_id的相关信息，通道名称在sName中返回，通道码流在nStream中返回，通道号在nChannelNum中返回。
int qcommonpluginEx::GetChannelInfo(int chl_id,QString &sName,int &nStream,int &nChannelNum)
{
	// check if channel exists
	IChannelManager *pChannelManager=NULL;
	m_pDeviceManager->QueryInterface(IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		int nret=pChannelManager->GetChannelInfo(chl_id,sName,nStream,nChannelNum);
		pChannelManager->Release();
		pChannelManager=NULL;
		return nret;
	}else{
		return IChannelManager::E_SYSTEM_FAILED;
	}
}

// 获取通道chl_id的相关信息，通道名称通过键值"name"访问，通道码流通过键值"stream"访问，通道号通过键值"number"访问。
QVariantMap qcommonpluginEx::GetChannelInfo(int chl_id)
{
	IChannelManager *pChannelManager=NULL;
	QVariantMap nret;
	m_pDeviceManager->QueryInterface(IID_IChannelManager,(void**)&pChannelManager);
	if (NULL!=pChannelManager)
	{
		nret=pChannelManager->GetChannelInfo(chl_id);
		pChannelManager->Release();
		pChannelManager=NULL;
		return nret;
	}else{
		return nret;
	}
}
//设置录像使用的磁盘(格式为"X:X:X:..."  X为D E F....)
int qcommonpluginEx::setUseDisks(const QString & sDisks)
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret=pDisksSetting->setUseDisks(sDisks);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}

//获取录像使用的磁盘(格式为"X:X:X:..."  X为D E F....)
int qcommonpluginEx::getUseDisks(QString & sDisks)
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret=pDisksSetting->getUseDisks(sDisks);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}

//获取录像使用的磁盘(格式为"X:X:X:..."  X为D E F....),
QString qcommonpluginEx::getUseDisks()
{
	IDisksSetting *pDisksSetting=NULL;
	QString nret;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		pDisksSetting->getUseDisks(nret);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return nret;
	}
}

//获取系统可用的磁盘分区(格式为"X:X:X:..."  X为D E F....)
int qcommonpluginEx::getEnableDisks(QString & sDisks)
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret=pDisksSetting->getEnableDisks(sDisks);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}

//获取系统可用的磁盘分区(格式为"X:X:X:..."  X为D E F....)
QString qcommonpluginEx::getEnableDisks()
{
	IDisksSetting *pDisksSetting=NULL;
	QString nret;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		pDisksSetting->getEnableDisks(nret);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return nret;
	}
}

//设置录像文件包大小(单位m)
int qcommonpluginEx::setFilePackageSize(const int filesize)
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret=pDisksSetting->setFilePackageSize(filesize);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}

//读取录像文件包大小(单位m)
int qcommonpluginEx::getFilePackageSize(int& filesize)
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret=pDisksSetting->getFilePackageSize(filesize);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}

//读取录像文件包大小(单位m)
int qcommonpluginEx::getFilePackageSize()
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret;
		pDisksSetting->getFilePackageSize(nret);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}

//设置是否循环录像
int qcommonpluginEx::setLoopRecording(bool loop)
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret=pDisksSetting->setLoopRecording(loop);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}

//获取是否循环录像
bool qcommonpluginEx::getLoopRecording()
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		bool nret=pDisksSetting->getLoopRecording();
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return false;
	}
}

//设置磁盘预留空间(单位m)
int qcommonpluginEx::setDiskSpaceReservedSize(const int spacereservedsize)
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret=pDisksSetting->setDiskSpaceReservedSize(spacereservedsize);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}

//读取磁盘剩余空间(单位m)
int qcommonpluginEx::getDiskSpaceReservedSize(int& spacereservedsize)
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret=pDisksSetting->getDiskSpaceReservedSize(spacereservedsize);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}

//读取磁盘剩余空间(单位m)
int qcommonpluginEx::getDiskSpaceReservedSize()
{
	IDisksSetting *pDisksSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_IDiskSetting,(void**)&pDisksSetting);
	if (NULL!=pDisksSetting)
	{
		int nret;
		pDisksSetting->getDiskSpaceReservedSize(nret);
		pDisksSetting->Release();
		pDisksSetting=NULL;
		return nret;
	}else{
		return IDisksSetting::E_SYSTEM_FAILED;
	}
}
// ISetRecordTime
int qcommonpluginEx::ModifyRecordTime( int recordtime_id,QString starttime,QString endtime,bool enable )
{
	// check time format
	ISetRecordTime *pSetRecordTime=NULL;
	m_pDeviceManager->QueryInterface(IID_ISetRecordTime,(void**)&pSetRecordTime);
	if (NULL!=pSetRecordTime)
	{
		int nret=pSetRecordTime->ModifyRecordTime(recordtime_id,starttime,endtime,enable);
		pSetRecordTime->Release();
		pSetRecordTime=NULL;
		return nret;
	}else{
		return ISetRecordTime::E_SYSTEM_FAILED;
	}
}

QStringList qcommonpluginEx::GetRecordTimeBydevId( int chl_id )
{
	ISetRecordTime *pSetRecordTime=NULL;
	m_pDeviceManager->QueryInterface(IID_ISetRecordTime,(void**)&pSetRecordTime);
	QStringList nret;
	if (NULL!=pSetRecordTime)
	{
		nret=pSetRecordTime->GetRecordTimeBydevId(chl_id);
		pSetRecordTime->Release();
		pSetRecordTime=NULL;
		return nret;
	}else{
		return nret;
	}
}

QVariantMap qcommonpluginEx::GetRecordTimeInfo( int recordtime_id )
{
	ISetRecordTime *pSetRecordTime=NULL;
	m_pDeviceManager->QueryInterface(IID_ISetRecordTime,(void**)&pSetRecordTime);
	QVariantMap nret;
	if (NULL!=pSetRecordTime)
	{
		nret=pSetRecordTime->GetRecordTimeInfo(recordtime_id);
		pSetRecordTime->Release();
		pSetRecordTime=NULL;
		return nret;
	}else{
		return nret;
	}
}
//ILocalSetting
bool qcommonpluginEx::CheckTimeFormat( QString sTime )
{
	// check the time format if it is correct
	// ex "YYYY-MM-DD hh:mm:ss"
	QString sTemp = sTime;
	QRegExp regTimeFormat("^[0-9]{4}\\-[0-9]{2}\\-[0-9]{2}\\ [0-9]{2}\\:[0-9]{2}\\:[0-9]{2}$");
	return sTemp.contains(regTimeFormat);
}

//设置语言,sLanguage为输入语言字符串
int qcommonpluginEx::setLanguage(const QString & sLanguage)
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		int nret=pLocalSetting->setLanguage(sLanguage);
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return ILocalSetting::E_SYSTEM_FAILED;
	}
}

//获取设置的语言,返回值QString 为获取数据库保存的语言。
QString qcommonpluginEx::getLanguage()
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	QString nret;
	if (NULL!=pLocalSetting)
	{
		nret=pLocalSetting->getLanguage();
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return nret;
	}
}

//设置轮询时间,aptime为输入时间(单位秒)
int qcommonpluginEx::setAutoPollingTime(int aptime)
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		int nret=pLocalSetting->setAutoPollingTime(aptime);
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return ILocalSetting::E_SYSTEM_FAILED;
	}
}

//获取轮询时间,返回值int 为获取数据库保存时间的文本。
int qcommonpluginEx::getAutoPollingTime()
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		int nret=pLocalSetting->getAutoPollingTime();
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return ILocalSetting::E_SYSTEM_FAILED;
	}
}

//设置分屏模式, smode为输入分屏模式字符串
int qcommonpluginEx::setSplitScreenMode(const QString & smode)
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		int nret=pLocalSetting->setSplitScreenMode(smode);
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return ILocalSetting::E_SYSTEM_FAILED;
	}
}

//获取以设置的分屏模式。返回值QString 为获取数据库保存分屏模式的文本。
QString qcommonpluginEx::getSplitScreenMode()
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	QString nret;
	if (NULL!=pLocalSetting)
	{
		nret=pLocalSetting->getSplitScreenMode();
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return nret;
	}
}

//设置是否自动登录, alogin为输入bool 参数
int qcommonpluginEx::setAutoLogin(bool alogin)
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		int nret=pLocalSetting->setAutoLogin(alogin);
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return ILocalSetting::E_SYSTEM_FAILED;
	}
}

//获取是否自动登录，返回值bool 类型。
bool qcommonpluginEx::getAutoLogin()
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		bool nret=pLocalSetting->getAutoLogin();
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return false;
	}
}

int qcommonpluginEx::setAutoSyncTime(bool synctime)
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		int nret=pLocalSetting->setAutoSyncTime(synctime);
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return ILocalSetting::E_SYSTEM_FAILED;
	}
}

bool qcommonpluginEx::getAutoSyncTime()
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		bool nret=pLocalSetting->getAutoSyncTime();
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return false;
	}
}

int qcommonpluginEx::setAutoConnect(bool aconnect)
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		int nret=pLocalSetting->setAutoConnect(aconnect);
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return ILocalSetting::E_SYSTEM_FAILED;
	}
}

bool qcommonpluginEx::getAutoConnect()
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		bool nret=pLocalSetting->getAutoConnect();
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return false;
	}
}

int qcommonpluginEx::setAutoFullscreen(bool afullscreen)
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		int nret=pLocalSetting->setAutoFullscreen(afullscreen);
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return ILocalSetting::E_SYSTEM_FAILED;
	}
}

bool qcommonpluginEx::getAutoFullscreen()
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		bool nret=pLocalSetting->getAutoFullscreen();
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return false;
	}
}

int qcommonpluginEx::setBootFromStart(bool bootstart)
{
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
	if (NULL!=pLocalSetting)
	{
		int nret=pLocalSetting->setBootFromStart(bootstart);
		pLocalSetting->Release();
		pLocalSetting=NULL;
		return nret;
	}else{
		return ILocalSetting::E_SYSTEM_FAILED;
	}
}

bool qcommonpluginEx::getBootFromStart()
{	
	ILocalSetting *pLocalSetting=NULL;
	m_pDeviceManager->QueryInterface(IID_ILocalSetting,(void**)&pLocalSetting);
    if (NULL!=pLocalSetting)
    {
    	bool nret=pLocalSetting->getBootFromStart();
    	pLocalSetting->Release();
    	pLocalSetting=NULL;
    	return nret;
    }else{
    
    	return false;
	}
}

