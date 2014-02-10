#ifndef __IEVENTREGISTER_HEAD_FILE_9VH988YASDGVG7AV__
#define __IEVENTREGISTER_HEAD_FILE_9VH988YASDGVG7AV__
#include <libpcom.h>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

interface IEventRegister : public IPComBase
{
	//返回事件列表
	virtual QStringList eventList() = 0;
	//查询事件参数
	//eventName：请求返回事件参数的名称
	//eventParams：返回事件参数的列表
	//返回值：
	//OK：成功
	//E_EVENT_NOT_SUPPORT：无此事件
	//E_INVALID_PARAM：系统错误
	virtual int queryEvent(QString eventName,QStringList &eventParams) = 0;
	//注册事件
	//eventName：注册的事件名称
	//proc：注册事件的回调函数
	//pUser：注册事件的父指针
	//返回值：
	//OK：成功
	//E_EVENT_NOT_SUPPORT：无此事件
	//E_INVALID_PARAM：系统错误
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser) = 0;

	enum _enErrorCode{
		OK,
		E_EVENT_NOT_SUPPORT,
		E_INVALID_PARAM
	};
};

#endif