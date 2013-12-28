#ifndef __IDISKSETTING_HEAD_FILE_ASDNVG8Y9ASDF__
#define __IDISKSETTING_HEAD_FILE_ASDNVG8Y9ASDF__
#include <libpcom.h>
#include <QtCore/QString>

//接口定义参见doc\Interface\IDisksSetting.htm

interface IDisksSetting : public IPComBase
{

	virtual int setUseDisks(const QString & sDisks) = 0;

	virtual int getUseDisks(QString & sDisks) = 0;

	virtual int getEnableDisks(QString & sDisks) = 0;

	virtual int setFilePackageSize(const int filesize) = 0;

	virtual int getFilePackageSize(int& filesize) = 0;

	virtual int setLoopRecording(bool bcover) = 0;

	virtual bool getLoopRecording() = 0;

	virtual int setDiskSpaceReservedSize(const int spacereservedsize) = 0;

	virtual int getDiskSpaceReservedSize(int& spacereservedsize) = 0;
	
	enum _emError{
		S_OK = 0,          //成功	
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};

};

/*
	数据库存储：system.db
	表general_setting
	字段：                    
	id     name                       value
		  "storage_usedisks"          "D:"
	      "storage_cover"             "true"
		  "storage_filesize"          "128"
		  "storage_reservedsize"      "1024"
	（初始value如上）
	事件：无
*/


#endif //__IDISKSETTING_HEAD_FILE_ASDNVG8Y9ASDF__