#ifndef __IPROTOCOLPTZ_HEAD_FILE_D9WPC23AJ8sdfsdgsdgKW54EG0SYE__
#define __IPROTOCOLPTZ_HEAD_FILE_D9WPC23AJ8sdfsdgsdgKW54EG0SYE__

#include "libpcom.h"


interface IScreenShotDevice : public IPComBase
{
	//user:当前用户，nType:截屏类型（0：预览，1：本地，2：远程）,nChl:截屏窗口号
	virtual void screenShot(QString sUser,int nType,int nChl)=0;

};

#endif