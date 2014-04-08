#ifndef __IPROTOCOLPTZ_HEAD_FILE_D9WPC23AJ8KW54EG0SYE__
#define __IPROTOCOLPTZ_HEAD_FILE_D9WPC23AJ8KW54EG0SYE__

#include "libpcom.h"


interface IProtocolPTZ : public IPComBase
{
	//控制云台向上运动，nChl为通道号， nSpeed为运动速度，取值从0到7
	virtual int PTZUp(const int &nChl, const int &nSpeed) = 0;
	
	//控制云台向下运动，nChl为通道号， nSpeed为运动速度，取值从0到7	
	virtual int PTZDown(const int &nChl, const int &nSpeed) = 0;
	
	//控制云台向左运动，nChl为通道号， nSpeed为运动速度，取值从0到7
	virtual int PTZLeft(const int &nChl, const int &nSpeed) = 0;
	
	//控制云台向右运动，nChl为通道号， nSpeed为运动速度，取值从0到7
	virtual int PTZRight(const int &nChl, const int &nSpeed) = 0;
	
	//控制光圈打开，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int PTZIrisOpen(const int &nChl, const int &nSpeed) = 0;
	
	//控制光圈关闭，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int PTZIrisClose(const int &nChl, const int &nSpeed) = 0;
	
	//控制调整远焦，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int PTZFocusFar(const int &nChl, const int &nSpeed) = 0;
	
	//控制调整近焦，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int PTZFocusNear(const int &nChl, const int &nSpeed) = 0;
	
	//控制调整变倍大，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int PTZZoomIn(const int &nChl, const int &nSpeed) = 0;
	
	//控制调整变倍小，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int PTZZoomOut(const int &nChl, const int &nSpeed) = 0;
	
	//控制自动水平旋转，nChl为通道号，bOpend为开启或停止自动
	virtual int PTZAuto(const int &nChl, bool bOpend) = 0;
	
	//停止云台运动, nChl为通道号, nCmd为控制指令
	virtual int PTZStop(const int &nChl, const int &nCmd) = 0;
};

#endif