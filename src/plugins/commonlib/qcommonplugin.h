#ifndef QCOMMONPLUGIN_H
#define QCOMMONPLUGIN_H

#include <QObject>
#include <qwfw.h>
#include <QtGui/QWidget>
#include <QtSql>
#include <QMutex>
class QCommonPlugin : public QWidget,
	public QWebPluginFWBase
{
	Q_OBJECT

public:
	QCommonPlugin(QWidget *parent = 0);
	~QCommonPlugin();

private:
	QSqlDatabase * m_db;
	static QMutex Group_lock;
	QString m_sDbConnectionName;
	static QMutex Area_lock;

public:
	static int m_randSeed;
	static QMutex m_csRandSeed;

private:
	bool CheckTimeFormat(QString sTime);


public:
	int GetUserLevel( const QString & sUsername,int & nLevel );

	int GetUserAuthorityMask( const QString & sUsername,int & nAuthorityMask1, int & nAuthorityMask2 );

public slots:
	// Event proc
	void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);}

	// User Management
	int AddUser( const QString & sUsername,const QString & sPassword,int nLevel,int nAuthorityMask1,int nAuthorityMask2 );

	int RemoveUser( const QString & sUsername );

	int ModifyUserPassword( const QString & sUsername,const QString & sNewPassword );

	int ModifyUserLevel( const QString & sUsername,int nLevel );

	int ModifyUserAuthorityMask( const QString & sUsername,int nAuthorityMask1,int nAuthorityMask2 );

	bool IsUserExists( const QString & sUsername );

	bool CheckUser( const QString & sUsername,const QString & sPassword );

	int GetUserLevel( const QString & sUsername );

	QStringList GetUserAuthorityMask( const QString & sUsername );

	int GetUserCount();

	QStringList GetUserList();

	/*igroupmanager module*/
	int AddGroup(QString sName);
	int RemoveGroup(int group_ip);
	int ModifyGroupName(int group_id,QString sName);
	int GetGroupCount();
	QStringList GetGroupList();
	int GetGroupName(int group_id,QString &sName);
	QString GetGroupName(int group_id);
	bool IsGroupExists(int group_id);

	bool IsChannelExists(int chl_id);
	bool IsR_Channel_GroupExist(int rgc_id);
	int AddChannelInGroup(int group_id,int chl_id,QString sName);
	int RemoveChannelFromGroup(int rgc_id);
	int  ModifyGroupChannelName(int rgc_id,QString sName);
	int MoveChannelToGroup(int rgc_id,int group_id);
	int GetGroupChannelCount(int group_id);
	QStringList GetGroupChannelList(int group_id);
	int GetGroupChannelName(int rgc_id,QString & sName);
	QString GetGroupChannelName(int rgc_id);
	int GetChannelIdFromGroup(int rgc_id,int &chl_id);
	int GetChannelIdFromGroup(int rgc_id);
	int GetChannelInfoFromGroup(int rgc_id,int & chl_id,int & group_id, QString & sName);
	QVariantMap GetChannelInfoFromGroup(int rgc_id);

	/*IAreaManager module*/
	int AddArea(int nPid,QString sName);
	int RemoveAreaById(int nId);
	int RemoveAreaByName(QString sName);
	int SetAreaName(int nId,QString sName);
	bool IsAreaNameExist(QString sName);
	bool IsAreaIdExist(int nid);
	int GetAreaCount();
	QStringList GetAreaList();
	QStringList GetSubArea(int nId);
	int GetAreaPid(int id);
	QString GetAreaName(int id);
	int GetAreaInfo(int nId,int &nPid,QString &sName);
	QVariantMap GetAreaInfo(int nId);

	/*IDeviceManager module*/
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

	/*ChannelMangeger module*/
	int ModifyChannelName(int chl_id,QString sName);
	int ModifyChannelStream(int chl_id,int nStream);
	int GetChannelCount(int dev_id);
	QStringList GetChannelList(int dev_id);
	int GetChannelName(int chl_id,QString & sName);
	int GetChannelStream(int chl_id,int & nStream);
	int GetChannelNumber(int chl_id,int & nChannelNum);
	int GetChannelInfo(int chl_id,QString &sName,int &nStream,int &nChannelNum);
	QVariantMap GetChannelInfo(int chl_id);

	/*IDisksSetting module*/
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
	virtual int ModifyRecordTime( int recordtime_id,QString starttime,QString endtime,bool enable );
	virtual QStringList GetRecordTimeBydevId( int chl_id );
	virtual QVariantMap GetRecordTimeInfo( int recordtime_id );

	/*ILocalSetting module*/
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

};

#endif // QCOMMONPLUGIN_H
