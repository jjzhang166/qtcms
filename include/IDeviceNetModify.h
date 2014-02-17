#ifndef __IDEVICENETMODIFY_HEAD_FILE_HVG4S5HFGH832A__
#define __IDEVICENETMODIFY_HEAD_FILE_HVG4S5HFGH832A__
#include <libpcom.h>
#include <IEventRegister.h>

interface IDeviceNetModify : public IPComBase
{
	//设置网络参数
	//sDeviceID:设备ID
	//sAddress：IP地址
	//sMask：子网掩码
	//sGateway：网关
	//sMac：MAC地址
	//uiPort:端口号
	//sUsername：用户名
	//sPassword:密码
	virtual int SetNetworkInfo(const QString &sDeviceID,
		const QString &sAddress,
		const QString &sMask,
		const QString &sGateway,
		const QString &sMac,
		const QString &sPort,
		const QString &sUsername,
		const QString &sPassword
		) = 0;

	//查询事件注册句柄
	virtual IEventRegister * QueryEventRegister() = 0;

	enum _enErrorCode{
		OK,
		E_INVALID_PARAM,
		E_SYSTEM_FAILED,
	};
};

/*
Event:
@1	name: "SettingStatus"
	parameters:
		"Status":设置的状态,可选“set info success”,“set port success”，“Unauthorized”
*/

#endif