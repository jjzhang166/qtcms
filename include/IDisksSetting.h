#ifndef __IDISKSETTING_HEAD_FILE_ASDNVG8Y9ASDF__
#define __IDISKSETTING_HEAD_FILE_ASDNVG8Y9ASDF__
#include <libpcom.h>
#include <QtCore/QString>

interface IDisksSetting : public IPComBase
{
	/*++
	设置录像使用的磁盘(格式为"X:\X:\X:\..."  X为D E F....)
	返回值_emError代码
	S_OK：设置成功
	E_SYSTEM_FAILED：设置数据失败
	--*/
	virtual int setUseDisks(const QString & sDisks) = 0;
	/*++
	获取录像使用的磁盘(格式为"X:\X:\X:\..."  X为D E F....)
	返回值_emError代码
	S_OK：获取成功
	E_SYSTEM_FAILED：读取数据失败
	--*/
	virtual int getUseDisks(QString & sDisks) = 0;
	/*++
	获取系统可用的磁盘分区(格式为"X:\X:\X:\..."  X为D E F....)
	返回值_emError代码
	S_OK：获取成功
	E_SYSTEM_FAILED：设置数据失败
	--*/
	virtual int getEnableDisks(QString & sDisks) = 0;

	/*++
	设置录像文件包大小(单位m)
	返回值_emError代码
	S_OK：设置成功
	E_PARAMETER_ERROR 参数不正确
	--*/
	virtual int setFilePackageSize(const unsigned int filesize) = 0;
	/*++
	读取录像文件包大小(单位m)
	返回值_emError代码
	S_OK：获取成功
	E_SYSTEM_FAILED：读取数据失败
	--*/
	virtual int getFilePackageSize(unsigned int& filesize) = 0;
	/*++
	设置是否循环录像
	返回值_emError代码
	S_OK：设置成功
	E_SYSTEM_FAILED：设置数据失败
	--*/
	virtual int setLoopRecording(bool loop) = 0;
	/*++
	获取是否循环录像
	返回值bool  
	--*/
	virtual bool getLoopRecording() = 0;
	/*++
	设置磁盘预留空间(单位m)
	返回值_emError代码
	S_OK：设置成功
	E_PARAMETER_ERROR 参数不正确
	--*/
	virtual int setDiskSpaceReservedSize(const unsigned int spacereservedsize) = 0;
	/*++
	读取磁盘剩余空间(单位m)
	返回值_emError代码
	S_OK：获取成功
	E_SYSTEM_FAILED：读取数据失败
	--*/
	virtual int getDiskSpaceReservedSize(unsigned int& spacereservedsize) = 0;
	
	enum _emError{
		S_OK = 0,          //成功
		//E_DISK_NOT_FOUND,  //磁盘未找到	
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};

};

/*
	事件：无
*/


#endif //__IDISKSETTING_HEAD_FILE_ASDNVG8Y9ASDF__