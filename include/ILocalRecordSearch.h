#ifndef __ILOCALPLAYSEARCH_HEAD_FILE_NS89VY9ASUD8__
#define __ILOCALPLAYSEARCH_HEAD_FILE_NS89VY9ASUD8__
#include <libpcom.h>
#include <QtCore/QString>

interface ILocalRecordSearch : public IPComBase
{
	/*++
	搜索sdevname设备录像的日期，检索结果以事件的形式抛出。
	返回值_emError代码
	OK：进行检索。E_PARAMETER_ERROR：输入参数不正确。具体错误号参考“错误号定义”。

	--*/
	virtual int searchDateByDeviceName(const QString& sdevname) = 0;

	/*++
	搜索符合条件的视频文件。检索结果以事件的形式抛出。
	输入参数sdevname：为设备名
	输入参数sdate：为要搜索的日期 (格式"yyyy-MM-dd")
	输入参数sbegintime：为开始时间 (格式为"hh:mm:ss")
	输入参数sendtime：为结束时间 (格式为"hh:mm:ss")
	输入参数schannellist：为检索的通道组合 (格式为"x;xx;x" ，例如"1;13;3"，取1、13、3 通道的的组合)
	--*/
	virtual int searchVideoFile(const QString& sdevname,
								const QString& sdate,
								const QString& sbegintime,
								const QString& sendtime,
								const QString& schannellist) = 0;

	enum _emError{
		OK = 0,          //成功
		E_PARAMETER_ERROR, //输入参数不正确
		E_SYSTEM_FAILED,   //系统错误
	};
};

/*
备注：以上两个接口为非阻塞实现
	  获取收索结果以注册事件来得到
Event:
@1	name: "GetRecordDate" 用接口searchDateByDeviceName来开始
	parameters:
		"devname":设备名(QString)
		"date":录像日期(QDateTime)
	
@2	name: "GetRecordFile" 用接口searchVideoFile来开始
	parameters:
		"filename":文件名 (QString)
		"filepath":文件路径 (QString)(注意文件路径以'/'来分隔)
		"filesize":文件大小 (int) (单位(MB))
		"channelnum":通道号
		"startTime":开始时间(QDateTime)
		"stopTime":结束时间(QDateTime)


@3	name: "SearchStop"
	parameters:
		"stopevent":对应事件检索结束(QString) （可用参数 GetRecordDate 、GetRecordFile)
*/


#endif