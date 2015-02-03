#ifndef __IWINDOW_SCREENSHOT_HEAD_FILE__
#define __IWINDOW_SCREENSHOT_HEAD_FILE__

#include "libpcom.h"

interface IScreenShot : public IPComBase
{
	virtual bool addScreenShotItem(QString sFileName,QString sFileDir ,QString sUserName,int nChl,int nType,quint64 uiSceenTime)=0;
	virtual bool deleteScreenShotItem(QList<int> tIdList)=0;
	virtual QString getScreenItem(QList<int> tChlList,QList<int> tTypeList,quint64 uiStartSceenTime,quint64 uiEndSceenTime)=0;
	// 返回值格式:
	/*
	<screenShot itemNum='n'>
		<item id='' fileName='' fileDir='' chl='' type='' time=''>
	</screenShot>
	*/
};


#endif