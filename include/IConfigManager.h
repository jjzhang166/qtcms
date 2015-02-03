#ifndef __INTERFACE_ICOMMUNICATE_HEAD_FILE_FR8HD3ASFD__
#define __INTERFACE_ICOMMUNICATE_HEAD_FILE_FR8HD3ASFD__
#include <libpcom.h>
#include <QtCore/QVariantMap>

interface IConfigManager : public IPComBase
{
	//设置导入配置文件的路径
	//sFilePath：配置文件的路径
	//0：调用成功
	//1：调用失败
	virtual int Import(const QString &sFilePath) = 0;
	
	//设置导出配置文件的路径
	//sFilePath：配置文件的路径
	//0：调用成功
	//1：调用失败
	virtual int Export(const QString &sFilePath) = 0;	
};

#endif