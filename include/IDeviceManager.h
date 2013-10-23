#ifndef __IDEVICEMANAGER_HEAD_FILE_0U0VJQAQ3RX123RN
#define __IDEVICEMANAGER_HEAD_FILE_0U0VJQAQ3RX123RN

#include "libpcom.h"
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

interface IDeviceManager : public IPComBase
{
	virtual bool IsDeviceExist(int dev_id)=0;
	virtual int IsDevExistsInArea(int area_id, QString sDeviceName)=0;
	virtual int AddDevice(int area_id,QString sDeviceName,QString sAddress,int port,int http,QString sEseeid,QString sUsername,QString sPassword,int chlCount,int connectMethod,QString sVendor) = 0;
	virtual int RemoveDevice(int dev_id) = 0;
	virtual int ModifyDeviceName(int dev_id,QString sDeviceName) = 0;
	virtual int ModifyDeviceHost(int dev_id,QString sAddress, int port, int http) = 0;
	virtual int ModifyDeviceEseeId(int dev_id,QString sEseeId) = 0;
	virtual int ModifyDeviceAuthority(int dev_id,QString sUsername,QString sPassword) = 0;
	virtual int ModifyDeviceChannelCount(int dev_id,int chlCount) = 0;
	virtual int ModifyDeviceConnectMethod(int dev_id,int connectMethod) = 0;
	virtual int ModifyDeviceVendor(int dev_id,QString sVendor) = 0;
	virtual int GetDeviceCount(int area_id) = 0;
	virtual QStringList GetDeviceList(int area_id) = 0;
	virtual int GetDeviceName(int dev_id,QString & sName) = 0;
	virtual int GetDeviceHost(int dev_id,QString & sAddress,int & nPort,int &http) = 0;
	virtual int GetDeviceEseeId(int dev_id,QString & sEseeid) = 0;
	virtual int GetDeviceLoginInfo(int dev_id,QString &sUsername,QString & sPassword) = 0;
	virtual int GetDeviceConnectMethod(int dev_id,int & connectMethod) = 0;
	virtual int GetDevicdVendor(int dev_id,QString & sVendor)=0;
	virtual int GetDeviceInfo(int dev_id,QString & sDeviceName, QString & sAddress, int & port, int & http, QString & sEseeid, QString & sUsername,QString &sPassword, int & connectMethod, QString & sVendor) = 0;
	virtual QVariantMap GetDeviceInfo(int dev_id) = 0;

	enum _emError{
		OK,
		E_DEVICE_NOT_FOUND,
		E_AREA_NOT_FOUND,
		E_SYSTEM_FAILED,
	};
};

#endif