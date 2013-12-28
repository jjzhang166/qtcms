#ifndef __IDISKSETTING_HEAD_FILE_ASDNVG8Y9ASDF__
#define __IDISKSETTING_HEAD_FILE_ASDNVG8Y9ASDF__
#include <libpcom.h>
#include <QtCore/QString>

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
	数据库存储：
	system.db
	字段id     name           初始 value
			  "use_disks"          "D;"
	          "b_cover"            "true"
			  "file_size"          "128"
			  "reserved_size"      "1024"
	事件：无
*/


#endif //__IDISKSETTING_HEAD_FILE_ASDNVG8Y9ASDF__