#ifndef __INTERFACE_ICOMMUNICATE_HEAD_FILE_FR8HD3ASFD__
#define __INTERFACE_ICOMMUNICATE_HEAD_FILE_FR8HD3ASFD__
#include <libpcom.h>
#include <QtCore/QVariantMap>

interface ICommunicate : public IPComBase
{
	//用于插件层向组件层的通信
	//msgName：消息名称，用于区分不同的消息
	//info：消息内容
	//返回值：
	//0：调用成功
	//1：调用失败
	virtual int setInfromation(const QString &msgName, const QVariantMap &info) = 0;
};

#endif