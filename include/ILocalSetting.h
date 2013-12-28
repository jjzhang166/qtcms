#ifndef __ILOCALSETTING_HEAD_FILE_ASDNVG8Y9ASDF__
#define __ILOCALSETTING_HEAD_FILE_ASDNVG8Y9ASDF__
#include <libpcom.h>
#include <QtCore/QString>

//详细接口定义参见doc\Interface\ILocalSetting.html

interface ILocalSetting : public IPComBase
{
	virtual int setLanguage(const QString & sLanguage) = 0;

	virtual QString getLanguage() = 0;
	
	virtual int setAutoPollingTime(int aptime) = 0;

	virtual int getAutoPollingTime() = 0;

	virtual int setSplitScreenMode(const QString & smode) = 0;

	virtual QString getSplitScreenMode() = 0;
	
	virtual int setAutoLogin(bool alogin);

	virtual bool getAutoLogin();

	virtual int setAutoSyncTime(bool synctime);

	virtual bool getAutoSyncTime();

	virtual int setAutoConnect(bool aconnect);

	virtual bool getAutoConnect();

	virtual int setAutoFullscreen(bool afullscreen);

	virtual bool getAutoFullscreen();

	virtual int setBootFromStart(bool bootstart);

	virtual bool getBootFromStart();

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
	id     name                value
		"misc_slanguage"       "en_GB"
		"misc_aptime"          "120"
		"misc_smode"           "div4_4"
		"misc_alogin"          "true"
		"misc_synctime"		   "true"
		"misc_aconnent"		   "false"
		"misc_bootstart"	   "false"
	（初始value如上）
	
	事件：无
*/


#endif //__ILOCALSETTING_HEAD_FILE_ASDNVG8Y9ASDF__