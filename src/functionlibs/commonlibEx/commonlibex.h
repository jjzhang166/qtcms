#ifndef COMMONLIBEX_H
#define COMMONLIBEX_H

#include "commonlibex_global.h"
#include <IUserManager.h>
#include <IGroupManager.h>
#include <IAreaManager.h>
#include <QtSql>
#include <IDeviceManager.h>
#include <IChannelManager.h>
#include <IDisksSetting.h>
#include <ISetRecordTime.h>
#include <ILocalSetting.h>
#include <IWindowSettings.h>
#include <QMutex>
#include "DisksInfo.h"
#include <IUserManagerEx.h>
class  commonlibEx:public IUserManager
	,public IUserManagerEx
	,public IGroupManager
	,public IAreaManager
	,public IDeviceManager
	,public IChannelManager
	,public IDisksSetting
	,public ISetRecordTime
	,public ILocalSetting
	,public IWindowSettings
{
public:
	commonlibEx();
	~commonlibEx();

public:
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	//IUserManager
	virtual int AddUser(const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2);
	virtual int RemoveUser(const QString & sUsername);
	virtual int ModifyUserPassword(const QString & sUsername,const QString & sNewPassword);
	virtual int ModifyUserLevel(const QString & sUsername,int nLevel);
	virtual int ModifyUserAuthorityMask(const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2);
	virtual bool IsUserExists(const QString & sUsername);
	virtual bool CheckUser(const QString & sUsername,const QString & sPassword);
	virtual int GetUserLevel(const QString & sUsername,int & nLevel);
	virtual int GetUserLevel( const QString & sUsername );
	virtual int GetUserAuthorityMask(const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2);
	virtual int GetUserCount() ;
	virtual QStringList GetUserList();
	QStringList GetUserAuthorityMask( const QString & sUsername );

	virtual int addUser(const QString &sUserName,const QString &sPassword,quint64 uiLimit,quint64 uiLogOutInterval,QVariantMap tSubCode);//0:添加用户成功；1：添加用户失败
	virtual int deleteUser(const QString &sUserName);//0:删除用户成功;1:删除失败
	virtual int checkUserLimit(quint64 uiMainCode,quint64 uiSubCode);//0：用户具有权限；1：用户未登录；2：用户登录但是没有权限
	virtual int login(const QString &sUserName,const QString &sPassword ,int nCode);//nCode:0 表示用户请求登录，nCode:1 表示用户请求注销;
	virtual int setLoginOutInterval(int nTime);//0:设置成功；1：设置失败（参数错误）；nTime 单位为S，最大值限定为1一个小时（3600）
	virtual int getUserList(QStringList &sUserList);//0:获取成功 ；1：获取失败
	virtual int getUserLimit(QString sUserName,quint64 &uiLimit,QVariantMap &tSubCode);//0:获取成功；1：获取失败
	virtual int modifyUserInfo(const QString &sOldUserName,const QString &sNewUserName,const QString &sNewPassword,quint64 uiLimit,quint64 uiLogOutInterval, QVariantMap tSubCode);//0:设置成功，1：设置失败
	virtual int getUserDatabaseId(QString sUserName,int &nId);
	virtual QString getCurrentUser();
	virtual void setIsKeepCurrentUserPassWord(bool bFlags);
	virtual bool getIsKeepCurrentUserPassWord(QString &sUserName,QString &sUserPassword);
	virtual bool setCurrentUserInfo(QString sUserName,QString sUserPassword);
	virtual int getLoginOutInterval(QString sUserName);
	virtual int modifyCurrentUserInfo(const QString &sOldUserName,const QString &sNewUserName,const QString &sOldPassword,const QString &sNewPassword,int iLogOutInterval);
	//IGroupManager
	virtual int AddGroup(QString sName) ;
	virtual int RemoveGroup(int group_id) ;
	virtual int ModifyGroupName(int group_id,QString sName) ;
	virtual int GetGroupCount();
	virtual QStringList GetGroupList() ;
	virtual int GetGroupName(int group_id,QString & sName);
	virtual QString GetGroupName(int group_id);
	virtual bool IsGroupExists(int group_id) ;

	virtual bool IsChannelExists(int chl_id);
	virtual bool IsR_Channel_GroupExist(int rgc_id);
	virtual int AddChannelInGroup(int group_id,int chl_id,QString sName) ;
	virtual int RemoveChannelFromGroup(int rgc_id) ;
	virtual int ModifyGroupChannelName(int rgc_id,QString sName);
	virtual int MoveChannelToGroup(int rgc_id,int group_id) ;
	virtual int GetGroupChannelCount(int group_id) ;
	virtual QStringList GetGroupChannelList(int group_id);
	virtual int GetGroupChannelName(int rgc_id,QString & sName) ;
	virtual QString GetGroupChannelName(int rgc_id) ;
	virtual int GetChannelIdFromGroup(int rgc_id,int & chl_id) ;
	virtual int GetChannelIdFromGroup(int rgc_id) ;
	virtual int GetChannelInfoFromGroup(int rgc_id,int & chl_id, int & group_id, QString & sName);
	virtual QVariantMap GetChannelInfoFromGroup(int rgc_id);

	//IAreaManager
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

	//IDeviceManager
	bool IsDeviceExist(int dev_id);
	int IsDevExistsInArea(int area_id, QString sDeviceName);
	int AddDevice(int area_id,QString sDeviceName,QString sAddress,int port,int http,QString sEseeid,QString sUsername,QString sPassword,int chlCount,int connectMethod,QString sVendor);
	int RemoveDevice(int dev_id);
	int ModifyDeviceName(int dev_id,QString sDeviceName);
	int ModifyDeviceHost(int dev_id,QString sAddress, int port, int http);
	int ModifyDeviceEseeId(int dev_id,QString sEseeId);
	int ModifyDeviceAuthority(int dev_id,QString sUsername,QString sPassword);
	int ModifyDeviceChannelCount(int dev_id,int chlCount);
	int ModifyDeviceConnectMethod(int dev_id,int connectMethod);
	int ModifyDeviceVendor(int dev_id,QString sVendor);
	int GetDeviceCount(int area_id);
	QStringList GetDeviceList(int area_id);
	int GetDeviceName(int dev_id,QString & sName);
	int GetDeviceHost(int dev_id,QString & sAddress,int & nPort,int &http);
	int GetDeviceEseeId(int dev_id,QString & sEseeid);
	int GetDeviceLoginInfo(int dev_id,QString &sUsername,QString & sPassword);
	int GetDeviceConnectMethod(int dev_id,int & connectMethod);
	int GetDevicdVendor(int dev_id,QString & sVendor);
	int GetDeviceInfo(int dev_id,QString & sDeviceName, QString & sAddress, int & port, int & http, QString & sEseeid, QString & sUsername,QString &sPassword, int & connectMethod, QString & sVendor);
	QVariantMap GetDeviceInfo(int dev_id);

	//ChannelMangeger
	int ModifyChannelName(int chl_id,QString sName);
	int ModifyChannelStream(int chl_id,int nStream);
	int GetChannelCount(int dev_id);
	QStringList GetChannelList(int dev_id);
	int GetChannelName(int chl_id,QString & sName);
	int GetChannelStream(int chl_id,int & nStream);
	int GetChannelNumber(int chl_id,int & nChannelNum);
	int GetChannelInfo(int chl_id,QString &sName,int &nStream,int &nChannelNum);
	QVariantMap GetChannelInfo(int chl_id);

	//IDisksSetting
	int setUseDisks(const QString & sDisks);
	int getUseDisks(QString & sDisks);
	QString getUseDisks();
	int getEnableDisks(QString & sDisks);
	QString getEnableDisks();
	int setFilePackageSize(const int filesize);
	int getFilePackageSize(int& filesize);
	int getFilePackageSize ();
	int setLoopRecording(bool bcover);
	bool getLoopRecording();
	int setDiskSpaceReservedSize(const int spacereservedsize);
	int getDiskSpaceReservedSize(int& spacereservedsize);
	int getDiskSpaceReservedSize();

	// ISetRecordTime
	virtual int ModifyRecordTime( int recordtime_id,QString starttime,QString endtime,int enable );
	virtual QStringList GetRecordTimeBydevId( int chl_id );
	virtual QVariantMap GetRecordTimeInfo( int recordtime_id );

	// IWindowSettings
	virtual void setEnableStretch( int uiWnd,bool bEnable );
	virtual bool getEnableStretch( int uiWnd );
	virtual void setAllWindowStretch( bool bEnable );
	virtual void setChannelInWnd( int uiWnd,int nChl );
	virtual int getChannelInWnd( int nWnd );


	//ILocalSetting
	int setLanguage(const QString & sLanguage);
	QString getLanguage();
	int setAutoPollingTime(int aptime);
	int getAutoPollingTime();
	int setSplitScreenMode(const QString & smode);
	QString getSplitScreenMode();
	int setAutoLogin(bool alogin);
	bool getAutoLogin();
	int setAutoSyncTime(bool synctime);
	bool getAutoSyncTime();
	int setAutoConnect(bool aconnect);
	bool getAutoConnect();
	int setAutoFullscreen(bool afullscreen);
	bool getAutoFullscreen();
	int setBootFromStart(bool bootstart);
	bool getBootFromStart();
	bool setIsPersian(bool bFlags);
	bool getIsPersian();
private:
	bool CheckTimeFormat(QString sTime);
	bool checkDeviceNameIsExist(QString sDevcie);
	QString checkCurrentLoginUser();
	bool checkUserIsExist(QString sUserName);


private:
	QSqlDatabase * m_db;
	static QMutex Group_lock;
	static QMutex Area_lock;
	static QMutex Device_lock;
	static QMutex m_tUserLock;
	QMutex m_csRef;
	int m_nRef;
	QString m_sDbConnectionName;
public:
	static int m_randSeed;
	static QMutex m_csRandSeed;
};

#endif // COMMONLIBEX_H
