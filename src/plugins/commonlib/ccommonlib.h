#ifndef CCOMMONLIB_H
#define CCOMMONLIB_H

#include "commonlib_global.h"
#include <QObject>
#include <IWebPluginBase.h>
#include <QMutex>
#include <IUserManager.h>
#include <IGroupManager.h>
#include <IAreaManager.h>
#include <IDeviceManager.h>
#include <IChannelManager.h>
#include <IDisksSetting.h>
#include <ISetRecordTime.h>
#include <ILocalSetting.h>
#include "qcommonplugin.h"
#include "qcommonpluginEx.h"

class Ccommonlib : public QObject
	,public IWebPluginBase
	,public IUserManager
	,public IGroupManager
	,public IAreaManager
	,public IDeviceManager
	,public IDisksSetting
	,public IChannelManager
	,public ISetRecordTime
	,public ILocalSetting
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

	/* IGroupManager module*/
	virtual int AddGroup(QString sName);
	virtual int RemoveGroup(int group_ip);
	virtual int ModifyGroupName(int group_id,QString sName);
	virtual int GetGroupCount();
	virtual QStringList GetGroupList();
	virtual int GetGroupName(int group_id,QString &sName);
	virtual QString GetGroupName(int group_id);
	virtual bool IsGroupExists(int group_id);

	virtual bool IsChannelExists(int chl_id);
	virtual bool IsR_Channel_GroupExist(int rgc_id);
	virtual int AddChannelInGroup(int group_id,int chl_id,QString sName);
	virtual int RemoveChannelFromGroup(int rgc_id);
	virtual int  ModifyGroupChannelName(int rgc_id,QString sName);
	virtual int MoveChannelToGroup(int rgc_id,int group_id);
	virtual int GetGroupChannelCount(int group_id);
	virtual QStringList GetGroupChannelList(int group_id);
	virtual int GetGroupChannelName(int rgc_id,QString & sName);
	virtual QString GetGroupChannelName(int rgc_id);
	virtual int GetChannelIdFromGroup(int rgc_id,int &chl_id);
	virtual int GetChannelIdFromGroup(int rgc_id);
	virtual int GetChannelInfoFromGroup(int rgc_id,int & chl_id,int & group_id, QString & sName);
	virtual QVariantMap GetChannelInfoFromGroup(int rgc_id);
	/*IAreaManager module*/
	virtual int AddArea(int nPid,QString sName);
	virtual int RemoveAreaById(int nId);
	virtual int RemoveAreaByName(QString sName);
	virtual int SetAreaName(int nId,QString sName);
	virtual bool IsAreaNameExist(QString sName);
	virtual bool IsAreaIdExist(int nid);
	virtual int GetAreaCount();
	virtual QStringList GetAreaList();
	virtual QStringList GetSubArea(int nId);
	virtual int GetAreaPid(int id);
	virtual QString GetAreaName(int id);
	virtual int GetAreaInfo(int nId,int &nPid,QString &sName);
	virtual QVariantMap GetAreaInfo(int nId);
	/*IDeviceManager Module*/
	virtual bool IsDeviceExist(int dev_id);
	virtual int IsDevExistsInArea(int area_id, QString sDeviceName);
	virtual int AddDevice(int area_id,QString sDeviceName,QString sAddress,int port,int http,QString sEseeid,QString sUsername,QString sPassword,int chlCount,int connectMethod,QString sVendor);
	virtual int RemoveDevice(int dev_id);
	virtual int ModifyDeviceName(int dev_id,QString sDeviceName);
	virtual int ModifyDeviceHost(int dev_id,QString sAddress, int port, int http);
	virtual int ModifyDeviceEseeId(int dev_id,QString sEseeId);
	virtual int ModifyDeviceAuthority(int dev_id,QString sUsername,QString sPassword);
	virtual int ModifyDeviceChannelCount(int dev_id,int chlCount);
	virtual int ModifyDeviceConnectMethod(int dev_id,int connectMethod);
	virtual int ModifyDeviceVendor(int dev_id,QString sVendor);
	virtual int GetDeviceCount(int area_id);
	virtual QStringList GetDeviceList(int area_id);
	virtual int GetDeviceName(int dev_id,QString & sName);
	virtual int GetDeviceHost(int dev_id,QString & sAddress,int & nPort,int &http);
	virtual int GetDeviceEseeId(int dev_id,QString & sEseeid);
	virtual int GetDeviceLoginInfo(int dev_id,QString &sUsername,QString & sPassword);
	virtual int GetDeviceConnectMethod(int dev_id,int & connectMethod);
	virtual int GetDevicdVendor(int dev_id,QString & sVendor);
	virtual int GetDeviceInfo(int dev_id,QString & sDeviceName, QString & sAddress, int & port, int & http, QString & sEseeid, QString & sUsername,QString &sPassword, int & connectMethod, QString & sVendor);
	virtual QVariantMap GetDeviceInfo(int dev_id);
	/*ChannelMangeger module*/
	virtual int ModifyChannelName(int chl_id,QString sName);
	virtual int ModifyChannelStream(int chl_id,int nStream);
	virtual int GetChannelCount(int dev_id);
	virtual QStringList GetChannelList(int dev_id);
	virtual int GetChannelName(int chl_id,QString & sName);
	virtual int GetChannelStream(int chl_id,int & nStream);
	virtual int GetChannelNumber(int chl_id,int & nChannelNum);
	virtual int GetChannelInfo(int chl_id,QString &sName,int &nStream,int &nChannelNum);
	virtual QVariantMap GetChannelInfo(int chl_id);

	/*IDisksSetting module*/
	virtual int setUseDisks(const QString & sDisks);
	virtual int getUseDisks(QString & sDisks);
	virtual QString getUseDisks();
	virtual int getEnableDisks(QString & sDisks);
	virtual QString getEnableDisks();
	virtual int setFilePackageSize(const int filesize);
	virtual int getFilePackageSize(int& filesize);
	virtual int getFilePackageSize ();
	virtual int setLoopRecording(bool bcover);
	virtual bool getLoopRecording();
	virtual int setDiskSpaceReservedSize(const int spacereservedsize);
	virtual int getDiskSpaceReservedSize(int& spacereservedsize);
	virtual int getDiskSpaceReservedSize();

	// ISetRecordTime
	virtual int ModifyRecordTime( int recordtime_id,QString starttime,QString endtime,int enable );

	virtual QStringList GetRecordTimeBydevId( int chl_id );

	virtual QVariantMap GetRecordTimeInfo( int recordtime_id );

	/*ILocalSetting module*/
	virtual int setLanguage(const QString & sLanguage);
	virtual QString getLanguage();
	virtual int setAutoPollingTime(int aptime);
	virtual int getAutoPollingTime();
	virtual int setSplitScreenMode(const QString & smode);
	virtual QString getSplitScreenMode();
	virtual int setAutoLogin(bool alogin);
	virtual bool getAutoLogin();
	virtual int setAutoSyncTime(bool synctime);
	virtual bool getAutoSyncTime();
	virtual int setAutoConnect(bool aconnect);
	virtual bool getAutoConnect();
	virtual int setAutoFullscreen(bool afullscreen);
	virtual bool getAutoFullscreen();
	virtual int setBootFromStart(bool bootstart);
	virtual bool getBootFromStart();

private:
	int m_nRef;
	QMutex m_csRef;
	/*QCommonPlugin m_pluginObj;*/
	qcommonpluginEx m_pluginObj;
};

#endif // CCOMMONLIB_H
