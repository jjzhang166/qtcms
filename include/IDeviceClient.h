#ifndef __DEVICE_CLIENT_HEAD_FILE_V898UY19ASYVB__
#define __DEVICE_CLIENT_HEAD_FILE_V898UY19ASYVB__
#include <libpcom.h>

interface IDeviceClient : public IPComBase
{
	// 连接到设备
	// sAddress:设备ip地址
	// uiPort:设备端口
	// sEseeId:设备id
	// 返回值:
	//	0:连接成功
	//  1:连接失败
	virtual int connectToDevice(const QString & sAddr,unsigned int uiPort,const QString & sEseeId) = 0;

	// 校验用户名密码
	// sUsername:用户名
	// sPassword:密码
	// 返回值:
	//	0:校验成功
	//	1:校验失败
	virtual int checkUser(const QString & sUsername,const QString & sPassword) = 0;

	// 设置当前所连接通道的名称
	// sChannelName:通道名称
	// 返回值:
	//	0:设置成功
	//	1:设置失败
	virtual int setChannelName(const QString & sChannelName) = 0;

	// 请求实时码流
	// nChannel:通道号
	// nStream:码流编号
	// bOpen:为true时打开通道预览，为false时，关闭通道预览
	// 返回值:
	//	0:请求成功
	//	1:请求失败
	virtual int liveStreamRequire(int nChannel,int nStream,bool bOpen) = 0;

	// 关闭连接，并清理资源
	// 返回值:
	//	0:关闭成功
	//	1:关闭失败
	virtual int closeAll() = 0;

	// 获取当前组件的Vendor名称，当前定义值为:"JUAN DVR"，"JUAN IPC","ONVIF"三种
	virtual QString getVendor() = 0;

	// 获取当前的连接状态
	// 返回值:
	//	0:未连接
	//	1:已连接
	//	2:正在连接
	//	3:正在断开
	virtual int getConnectStatus() = 0;
};

// Event
// @1 name:"PreviewStream"
// parameters：
// 	"channel":当前帧的通道号
// 	"pts":当前帧时间戳，单位为微秒
// 	"length":数据长度
// 	"data":数据指针
// 	"frametype":帧类型，取值'I','P','B','A'
// 	"width":视频帧的宽，单位像素，如果是音频帧，不传递该参数
// 	"height":视频帧的高，单位像素，如果是音频帧，不传递该参数
// 	"vcodec":视频帧的编码格式，当前定义值:"H264"，如果是音频帧，不传递该参数
// 	"samplerate":音频采样率，如果是视频帧，不传递该参数
// 	"samplewidth":音频采样位宽，如果是视频帧，不传递该参数
// 	"audiochannel":音频的采样通道数，如果是视频帧，不传递该参数
// 	"acodec":音频编码格式，当前定义值："G711"，如果是视频帧，不传递该参数
// @2 name:"RecordStream"
// parameters:
// "channel":当前帧的通道号
// 	"pts":当前帧时间戳，单位为微秒
// 	"length":数据长度
// 	"data":数据指针
// 	"frametype":帧类型，取值'I','P','B','A'
// 	"width":视频帧的宽，单位像素，如果是音频帧，不传递该参数
// 	"height":视频帧的高，单位像素，如果是音频帧，不传递该参数
// 	"vcodec":视频帧的编码格式，当前定义值:"H264"，如果是音频帧，不传递该参数
// 	"samplerate":音频采样率，如果是视频帧，不传递该参数
// 	"samplewidth":音频采样位宽，如果是视频帧，不传递该参数
// 	"audiochannel":音频的采样通道数，如果是视频帧，不传递该参数
// 	"acodec":音频编码格式，当前定义值："G711"，如果是视频帧，不传递该参数

#endif