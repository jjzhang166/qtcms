#include "ccommonlib.h"
#include <guid.h>
#include <QtPlugin>
#include "qcommonplugin.h"

Ccommonlib::Ccommonlib() :
m_nRef(0),
m_pluginObj(NULL)
{

}

Ccommonlib::~Ccommonlib()
{

}

QList<QWebPluginFactory::Plugin> Ccommonlib::plugins() const
{
	QWebPluginFactory::MimeType mimeType;
	mimeType.name = QString("application/cms-common-library");
	mimeType.description=QString("cms common functions");

	QList<QWebPluginFactory::MimeType> mimeTypes;
	mimeTypes.append(mimeType);

	QWebPluginFactory::Plugin plugin;
	plugin.name = QString("cms common functions");
	plugin.description = QString("cms common functions");
	plugin.mimeTypes=mimeTypes;

	QList<QWebPluginFactory::Plugin> plugins;
	plugins.append(plugin);
	return plugins;
}

QObject * Ccommonlib::create( const QString& mimeType, const QUrl&, const QStringList& argumentNames, const QStringList& argumentValues ) const
{
	QCommonPlugin * obj = new QCommonPlugin(NULL);
	return obj;
}

long __stdcall Ccommonlib::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IWebPluginBase == iid)
	{
		*ppv = static_cast<IWebPluginBase *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else if (IID_IUserManager == iid)
	{
		*ppv = static_cast<IUserManager *>(this);
	}
	else if (IID_IGroupManager == iid)
	{
		*ppv = static_cast<IGroupManager *>(this);
	}
	else if(IID_IAreaManager==iid)
	{
		*ppv = static_cast<IAreaManager *>(this);
	}
	else if(IID_IChannelManager==iid)
	{
		*ppv = static_cast<IChannelManager *>(this);
	}
	else if(IID_IDeviceManager==iid)
	{
		*ppv = static_cast<IDeviceManager *>(this);
	}
	else if(IID_IDiskSetting==iid)
	{
		*ppv = static_cast<IDisksSetting *>(this);
	}
	else if (IID_ISetRecordTime == iid)
	{
		*ppv = static_cast<ISetRecordTime *>(this);
	}
	else if(IID_ILocalSetting==iid)
	{
		*ppv = static_cast<ILocalSetting *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall Ccommonlib::AddRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall Ccommonlib::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}

int Ccommonlib::AddUser( const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2 )
{
	return m_pluginObj.AddUser(sUsername,sPassword,nLevel,nAuthorityMask1,nAuthorityMask2);
}

int Ccommonlib::RemoveUser( const QString & sUsername )
{
	return m_pluginObj.RemoveUser(sUsername);
}

int Ccommonlib::ModifyUserPassword( const QString & sUsername,const QString & sNewPassword )
{
	return m_pluginObj.ModifyUserPassword(sUsername,sNewPassword);
}

int Ccommonlib::ModifyUserLevel( const QString & sUsername,int nLevel )
{
	return m_pluginObj.ModifyUserLevel(sUsername,nLevel);
}

int Ccommonlib::ModifyUserAuthorityMask( const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2 )
{
	return m_pluginObj.ModifyUserAuthorityMask(sUsername,nAuthorityMask1,nAuthorityMask2);
}

bool Ccommonlib::IsUserExists( const QString & sUsername )
{
	return m_pluginObj.IsUserExists(sUsername);
}

bool Ccommonlib::CheckUser( const QString & sUsername,const QString & sPassword )
{
	return m_pluginObj.CheckUser(sUsername,sPassword);
}

int Ccommonlib::GetUserLevel( const QString & sUsername,int & nLevel )
{
	return m_pluginObj.GetUserLevel(sUsername,nLevel);
}

int Ccommonlib::GetUserAuthorityMask( const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2 )
{
	return m_pluginObj.GetUserAuthorityMask(sUsername,nAuthorityMask1,nAuthorityMask2);
}

int Ccommonlib::GetUserCount()
{
	return m_pluginObj.GetUserCount();
}

QStringList Ccommonlib::GetUserList()
{
	return m_pluginObj.GetUserList();
}

/*igroupmaneger module*/
int Ccommonlib::AddGroup(QString sName)
{
	return m_pluginObj.AddGroup(sName);
}

int Ccommonlib::RemoveGroup(int group_ip)
{
	return m_pluginObj.RemoveGroup(group_ip);
}

int Ccommonlib::ModifyGroupName(int group_id,QString sName)
{
	return m_pluginObj.ModifyGroupName(group_id,sName);
}

int Ccommonlib::GetGroupCount()
{
	return m_pluginObj.GetGroupCount();
}

QStringList Ccommonlib::GetGroupList()
{
	return m_pluginObj.GetGroupList();
}

int Ccommonlib::GetGroupName(int group_id,QString &sName)
{
	return m_pluginObj.GetGroupName(group_id,sName);
}

QString Ccommonlib::GetGroupName(int group_id)
{
	return m_pluginObj.GetGroupName(group_id);
}

bool Ccommonlib::IsGroupExists(int group_id)
{
	return m_pluginObj.IsGroupExists(group_id);
}

bool Ccommonlib::IsChannelExists(int chl_id)
{
	return m_pluginObj.IsChannelExists(chl_id);
}

bool Ccommonlib::IsR_Channel_GroupExist(int rgc_id)
{
	return m_pluginObj.IsR_Channel_GroupExist(rgc_id);
}
int Ccommonlib::AddChannelInGroup(int group_id,int chl_id,QString sName)
{
	return m_pluginObj.AddChannelInGroup(group_id,chl_id,sName);
}

int Ccommonlib::RemoveChannelFromGroup(int rgc_id)
{
	return m_pluginObj.RemoveChannelFromGroup(rgc_id);
}

int Ccommonlib:: ModifyGroupChannelName(int rgc_id,QString sName)
{
	return m_pluginObj.ModifyGroupChannelName(rgc_id,sName);
}

int Ccommonlib::MoveChannelToGroup(int rgc_id,int group_id)
{
	return m_pluginObj.MoveChannelToGroup(rgc_id,group_id);
}

int Ccommonlib::GetGroupChannelCount(int group_id)
{
	return m_pluginObj.GetGroupChannelCount(group_id);
}

QStringList Ccommonlib::GetGroupChannelList(int group_id)
{
	return m_pluginObj.GetGroupChannelList(group_id);
}

int Ccommonlib::GetGroupChannelName(int rgc_id,QString & sName)
{
	return m_pluginObj.GetGroupChannelName(rgc_id,sName);
}

QString Ccommonlib::GetGroupChannelName(int rgc_id)
{
	return m_pluginObj.GetGroupChannelName(rgc_id);
}

int Ccommonlib::GetChannelIdFromGroup(int rgc_id,int &chl_id)
{
	return m_pluginObj.GetChannelIdFromGroup(rgc_id,chl_id);
}

int Ccommonlib::GetChannelIdFromGroup(int rgc_id)
{
	return m_pluginObj.GetChannelIdFromGroup(rgc_id);
}

int Ccommonlib::GetChannelInfoFromGroup(int rgc_id,int & chl_id,int & group_id, QString & sName)
{
	return m_pluginObj.GetChannelInfoFromGroup(rgc_id,chl_id,group_id,sName);
}

QVariantMap Ccommonlib::GetChannelInfoFromGroup(int rgc_id)
{
	return m_pluginObj.GetChannelInfoFromGroup(rgc_id);
}

/*IAreaManager module*/
int Ccommonlib::AddArea(int nPid,QString sName)
{
	return m_pluginObj.AddArea(nPid,sName);
}

int Ccommonlib::RemoveAreaById(int nId)
{
	return m_pluginObj.RemoveAreaById(nId);
}

int Ccommonlib::RemoveAreaByName(QString sName)
{
	return m_pluginObj.RemoveAreaByName(sName);
}

int Ccommonlib::SetAreaName(int nId,QString sName)
{
	return m_pluginObj.SetAreaName(nId,sName);
}

bool Ccommonlib::IsAreaNameExist(QString sName)
{
	return m_pluginObj.IsAreaNameExist(sName);
}

bool Ccommonlib::IsAreaIdExist(int nid)
{
	return m_pluginObj.IsAreaIdExist(nid);
}
int Ccommonlib::GetAreaCount()
{
	return m_pluginObj.GetAreaCount();
}

QStringList Ccommonlib::GetAreaList()
{
	return m_pluginObj.GetAreaList();
}

QStringList Ccommonlib::GetSubArea(int nId)
{
	return m_pluginObj.GetSubArea(nId);
}

int Ccommonlib::GetAreaPid(int id)
{
	return m_pluginObj.GetAreaPid(id);
}

QString Ccommonlib::GetAreaName(int id)
{
	return m_pluginObj.GetAreaName(id);
}

int Ccommonlib::GetAreaInfo(int nId,int &nPid,QString &sName)
{
	return m_pluginObj.GetAreaInfo(nId,nPid,sName);
}

QVariantMap Ccommonlib::GetAreaInfo(int nId)
{
	return m_pluginObj.GetAreaInfo(nId);
}
/*IDeviceManager module*/
bool Ccommonlib::IsDeviceExist(int dev_id)
{
	return m_pluginObj.IsDeviceExist(dev_id);
}

int Ccommonlib::IsDevExistsInArea(int area_id, QString sDeviceName)
{
	return m_pluginObj.IsDevExistsInArea(area_id,sDeviceName);
}

int Ccommonlib::AddDevice(int area_id,QString sDeviceName,QString sAddress,int port,int http,QString sEseeid,QString sUsername,QString sPassword,int chlCount,int connectMethod,QString sVendor)
{
	return m_pluginObj.AddDevice(area_id,sDeviceName,sAddress,port,http,sEseeid,sUsername,sPassword,chlCount,connectMethod,sVendor);
}

int Ccommonlib::RemoveDevice(int dev_id)
{
	return m_pluginObj.RemoveDevice(dev_id);
}

int Ccommonlib::ModifyDeviceName(int dev_id,QString sDeviceName)
{
	return m_pluginObj.ModifyDeviceName(dev_id,sDeviceName);
}

int Ccommonlib::ModifyDeviceHost(int dev_id,QString sAddress, int port, int http)
{
	return m_pluginObj.ModifyDeviceHost(dev_id,sAddress,port,http);
}

int Ccommonlib::ModifyDeviceEseeId(int dev_id,QString sEseeId)
{
	return m_pluginObj.ModifyDeviceEseeId(dev_id,sEseeId);
}

int Ccommonlib::ModifyDeviceAuthority(int dev_id,QString sUsername,QString sPassword)
{
	return m_pluginObj.ModifyDeviceAuthority(dev_id,sUsername,sPassword);
}

int Ccommonlib::ModifyDeviceChannelCount(int dev_id,int chlCount)
{
	return m_pluginObj.ModifyDeviceChannelCount(dev_id,chlCount);
}

int Ccommonlib::ModifyDeviceConnectMethod(int dev_id,int connectMethod)
{
	return m_pluginObj.ModifyDeviceConnectMethod(dev_id,connectMethod);
}

int Ccommonlib::ModifyDeviceVendor(int dev_id,QString sVendor)
{
	return m_pluginObj.ModifyDeviceVendor(dev_id,sVendor);
}

int Ccommonlib::GetDeviceCount(int area_id)
{
	return m_pluginObj.GetDeviceCount(area_id);
}

QStringList Ccommonlib::GetDeviceList(int area_id)
{
	return m_pluginObj.GetDeviceList(area_id);
}

int Ccommonlib::GetDeviceName(int dev_id,QString & sName)
{
	return m_pluginObj.GetDeviceName(dev_id,sName);
}

int Ccommonlib::GetDeviceHost(int dev_id,QString & sAddress,int & nPort,int &http)
{
	return m_pluginObj.GetDeviceHost(dev_id,sAddress,nPort,http);
}

int Ccommonlib::GetDeviceEseeId(int dev_id,QString & sEseeid)
{
	return m_pluginObj.GetDeviceEseeId(dev_id,sEseeid);
}

int Ccommonlib::GetDeviceLoginInfo(int dev_id,QString &sUsername,QString & sPassword)
{
	return m_pluginObj.GetDeviceLoginInfo(dev_id,sUsername,sPassword);
}

int Ccommonlib::GetDeviceConnectMethod(int dev_id,int & connectMethod)
{
	return m_pluginObj.GetDeviceConnectMethod(dev_id,connectMethod);
}

int Ccommonlib::GetDevicdVendor(int dev_id,QString & sVendor)
{
	return m_pluginObj.GetDevicdVendor(dev_id,sVendor);
}

int Ccommonlib::GetDeviceInfo(int dev_id,QString & sDeviceName, QString & sAddress, int & port, int & http, QString & sEseeid, QString & sUsername,QString &sPassword, int & connectMethod, QString & sVendor)
{
	return m_pluginObj.GetDeviceInfo(dev_id,sDeviceName,sAddress,port,http,sEseeid,sUsername,sPassword,connectMethod,sVendor);
}

QVariantMap Ccommonlib::GetDeviceInfo(int dev_id)
{
	return m_pluginObj.GetDeviceInfo(dev_id);
}

int Ccommonlib::ModifyChannelName(int chl_id,QString sName)
{
	return m_pluginObj.ModifyChannelName(chl_id,sName);
}

int Ccommonlib::ModifyChannelStream(int chl_id,int nStream)
{
	return m_pluginObj.ModifyChannelStream(chl_id,nStream);
}

int Ccommonlib::GetChannelCount(int dev_id)
{
	return m_pluginObj.GetChannelCount(dev_id);
}

QStringList Ccommonlib::GetChannelList(int dev_id)
{
	return m_pluginObj.GetChannelList(dev_id);
}

int Ccommonlib::GetChannelName(int chl_id,QString & sName)
{
	return m_pluginObj.GetChannelName(chl_id,sName);
}

int Ccommonlib::GetChannelStream(int chl_id,int & nStream)
{
	return m_pluginObj.GetChannelStream(chl_id,nStream);
}

int Ccommonlib::GetChannelNumber(int chl_id,int & nChannelNum)
{
	return m_pluginObj.GetChannelNumber(chl_id,nChannelNum);
}

int Ccommonlib::GetChannelInfo(int chl_id,QString &sName,int &nStream,int &nChannelNum)
{
	return m_pluginObj.GetChannelInfo(chl_id,sName,nStream,nChannelNum);
}

QVariantMap Ccommonlib::GetChannelInfo(int chl_id)
{
	return m_pluginObj.GetChannelInfo(chl_id);
}

int Ccommonlib::setUseDisks(const QString & sDisks)
{
	return m_pluginObj.setUseDisks(sDisks);
}

int Ccommonlib::getUseDisks(QString & sDisks)
{
	return m_pluginObj.getUseDisks(sDisks);
}

QString Ccommonlib::getUseDisks()
{
	return m_pluginObj.getUseDisks();
}

int Ccommonlib::getEnableDisks(QString & sDisks)
{
	return m_pluginObj.getEnableDisks(sDisks);
}

QString Ccommonlib::getEnableDisks()
{
	return m_pluginObj.getEnableDisks();
}

int Ccommonlib::setFilePackageSize(const int filesize)
{
	return m_pluginObj.setFilePackageSize(filesize);
}

int Ccommonlib::getFilePackageSize(int& filesize)
{
	return m_pluginObj.getFilePackageSize(filesize);
}

int Ccommonlib::getFilePackageSize()
{
	return m_pluginObj.getFilePackageSize();
}

int Ccommonlib::setLoopRecording(bool loop)
{
	return m_pluginObj.setLoopRecording(loop);
}

bool Ccommonlib::getLoopRecording()
{
	return m_pluginObj.getLoopRecording();
}

int Ccommonlib::setDiskSpaceReservedSize(const int spacereservedsize)
{
	return m_pluginObj.setDiskSpaceReservedSize(spacereservedsize);
}

int Ccommonlib::getDiskSpaceReservedSize(int& spacereservedsize)
{
	return m_pluginObj.getDiskSpaceReservedSize(spacereservedsize);
}

int Ccommonlib::getDiskSpaceReservedSize()
{
	return m_pluginObj.getDiskSpaceReservedSize();
}

int Ccommonlib::ModifyRecordTime( int recordtime_id,QString starttime,QString endtime,bool enable )
{
	return m_pluginObj.ModifyRecordTime(recordtime_id,starttime,endtime,enable);
}

QStringList Ccommonlib::GetRecordTimeBydevId( int chl_id )
{
	return m_pluginObj.GetRecordTimeBydevId( chl_id);
}

QVariantMap Ccommonlib::GetRecordTimeInfo( int recordtime_id )
{
	return m_pluginObj.GetRecordTimeInfo(recordtime_id);
}

int Ccommonlib::setLanguage(const QString & sLanguage)
{
	return m_pluginObj.setLanguage(sLanguage);
}

QString Ccommonlib::getLanguage()
{
	return m_pluginObj.getLanguage();
}

int Ccommonlib::setAutoPollingTime(int aptime)
{
	return m_pluginObj.setAutoPollingTime(aptime);
}
int Ccommonlib::getAutoPollingTime()
{
	return m_pluginObj.getAutoPollingTime();
}

int Ccommonlib::setSplitScreenMode(const QString & smode)
{
	return m_pluginObj.setSplitScreenMode(smode);
}

QString Ccommonlib::getSplitScreenMode()
{
	return m_pluginObj.getSplitScreenMode();
}

int Ccommonlib::setAutoLogin(bool alogin)
{
	return m_pluginObj.setAutoLogin(alogin);
}

bool Ccommonlib::getAutoLogin()
{
	return m_pluginObj.getAutoLogin();
}

int Ccommonlib::setAutoSyncTime(bool synctime)
{
	return m_pluginObj.setAutoSyncTime(synctime);
}

bool Ccommonlib::getAutoSyncTime()
{
	return m_pluginObj.getAutoSyncTime();
}

int Ccommonlib::setAutoConnect(bool aconnect)
{
	return m_pluginObj.setAutoConnect(aconnect);
}

bool Ccommonlib::getAutoConnect()
{
	return m_pluginObj.getAutoConnect();
}

int Ccommonlib::setAutoFullscreen(bool afullscreen)
{
	return m_pluginObj.setAutoFullscreen(afullscreen);
}

bool Ccommonlib::getAutoFullscreen()
{
	return m_pluginObj.getAutoFullscreen();
}

int Ccommonlib::setBootFromStart(bool bootstart)
{
	return m_pluginObj.setBootFromStart(bootstart);
}

bool Ccommonlib::getBootFromStart()
{
	return m_pluginObj.getBootFromStart();
}


Q_EXPORT_PLUGIN2("commonlib.dll",Ccommonlib)