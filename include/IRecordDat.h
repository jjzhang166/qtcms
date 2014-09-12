#ifndef __IRECORDDAT_HEAD_FILE__
#define __IRECORDDAT_HEAD_FILE__
#include "libpcom.h"
#include <QtCore/QVariantMap>
//详细接口说明参见doc\Interface\IRecordDat.html
interface IRecordDat:public IPComBase{
	//录像初始化
	virtual bool init(int nWnd)=0;
	//退出录像
	virtual bool deinit()=0;
	//输入 录像数据
	virtual int  inputFrame(QVariantMap &tFrameInfo)=0;

	//手动录像
	//开启手动录像
	virtual bool manualRecordStart()=0;
	//关闭手动录像
	virtual bool manualRecordStop()=0;

	//移动录像
	//开启移动录像
	//nTime:移动录像信号的持续时间
	virtual bool motionRecordStart(int nTime)=0;

	//获取录像状态
	//返回值按位计算：移动侦测 手动录像 定时录像，高位优先级大于低位，例如 110表示 移动侦测，010 表示 手动录像
	virtual int getRecordStatus()=0;

	//更新录像日程表
	//nChannelId:通道在日程表中id号
	virtual bool updateRecordSchedule(int nChannelId)=0;
	//更新系统数据库
	virtual bool upDateSystemDatabase()=0;
	enum _emError{
			OK = 0,          //成功
			E_PARAMETER_ERROR, //输入参数不正确
			E_SYSTEM_FAILED,   //系统错误
	};
};
/*
	frameinfo参数：
	"frametype":帧类型，取值'I'0x01,'P'0x02,'B','A'0x00
	"data":数据指针
	"length":数据长度
	"pts":当前帧64位时间戳，精确到微妙
	"gentime":帧的产生事件，单位为秒，为GMT时间
	
	"channel":当前帧的通道号，如果是音频帧，不传递该参数
	"winid":窗口号
	"width":视频帧的宽，单位像素，如果是音频帧，不传递该参数
	"height":视频帧的高，单位像素，如果是音频帧，不传递该参数
	"vcodec":视频帧的编码格式，当前定义值:"H264"，如果是音频帧，不传递该参数
	
	"samplerate":音频采样率，如果是视频帧，不传递该参数
	"samplewidth":音频采样位宽，如果是视频帧，不传递该参数
	"audiochannel":音频的采样通道数，如果是视频帧，不传递该参数
	"acodec":音频编码格式，当前定义值："G711"，如果是视频帧，不传递该参数
*/
#endif