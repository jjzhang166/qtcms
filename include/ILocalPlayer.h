#ifndef __ILOCALPLAYER_HEAD_FILE_SADVN98ASD7YASGV__
#define __ILOCALPLAYER_HEAD_FILE_SADVN98ASD7YASGV__
#include <libpcom.h>
#include <QtGui/QWidget>
#include <QtCore/QDateTime>
#include <QStringList>
interface ILocalPlayer:IPComBase
{
	//添加播放文件到同步组中，并指定文件播放的窗口ID
	//输入参数：
	//filelist:文件名列表，列表中的文件必须是同一个通道号下的文件，包括完整的文章路径，如：F:\project\date1\devname\chl2\filename.avi，F:\project\date2\devname\chl2\filename.avi
	//wnd:播放窗口的id
	//start:开始播放时间(时间格式："yyyy-MM-dd hh:mm:ss")
	//end：结束播放时间(时间格式："yyyy-MM-dd hh:mm:ss")
	//返回值：
	//0：添加成功
	//1：添加失败，通道组已经满
	//2：添加失败，窗口已经被占用
	//3：添加失败，（失败原因未定义）
	virtual int AddFileIntoPlayGroup(QStringList const filelist,QWidget *wnd,const QDateTime &start,const QDateTime &end)=0;

	//设置同步组数量
	//输入参数：
	//num:设置同步组的数量值
	//返回值：
	//0：设置成功
	//1：设置失败
	virtual int SetSynGroupNum(int num)=0;
	//组播放
	//返回值：
	//0：调用成功
	//1：调用失败
	virtual int GroupPlay()=0;
	//暂停播放
	//返回值：
	//0：调用成功
	//1：调用失败
	virtual int GroupPause()=0;
	//继续播放
	//返回值：
	//0：调用成功
	//调用失败
	virtual int GroupContinue()=0;
	//停止播放
	//返回值：
	//0：调用成功
	//1：调用失败
	virtual int GroupStop()=0;
	//快进
	//输入参数：
	//speed：正常播放速度的倍数（可选值：2，4，8）
	//返回值：
	//0：调用成功
	//1：调用失败
	virtual int GroupSpeedFast(int speed)=0;
	//慢放
	//输入参数：
	//speed：正常播放速度的1/N倍（可选值：2，4，8）
	//返回值：
	//0：调用成功
	//1：调用失败
	virtual int GroupSpeedSlow(int speed)=0;
	//恢复正常的播放速度
	//返回值：
	//0：调用成功
	//1：调用失败
	virtual int GroupSpeedNormal() = 0;
};
#endif