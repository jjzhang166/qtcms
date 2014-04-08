#ifndef __IPTZCONTROL_HEAD_FILE_D90D6FJ3E1E75DSRE__
#define __IPTZCONTROL_HEAD_FILE_D90D6FJ3E1E75DSRE__

#include "libpcom.h"


interface IPTZControl : public IPComBase
{
	//控制云台向上运动，nChl为通道号， nSpeed为运动速度，取值从0到7
	virtual int ControlPTZUp(const int &nChl, const int &nSpeed) = 0;
	
	//控制云台向下运动，nChl为通道号， nSpeed为运动速度，取值从0到7	
	virtual int ControlPTZDown(const int &nChl, const int &nSpeed) = 0;
	
	//控制云台向左运动，nChl为通道号， nSpeed为运动速度，取值从0到7
	virtual int ControlPTZLeft(const int &nChl, const int &nSpeed) = 0;
	
	//控制云台向右运动，nChl为通道号， nSpeed为运动速度，取值从0到7
	virtual int ControlPTZRight(const int &nChl, const int &nSpeed) = 0;
	
	//控制光圈打开，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int ControlPTZIrisOpen(const int &nChl, const int &nSpeed) = 0;
	
	//控制光圈关闭，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int ControlPTZIrisClose(const int &nChl, const int &nSpeed) = 0;
	
	//控制调整远焦，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int ControlPTZFocusFar(const int &nChl, const int &nSpeed) = 0;
	
	//控制调整近焦，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int ControlPTZFocusNear(const int &nChl, const int &nSpeed) = 0;
	
	//控制调整变倍大，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int ControlPTZZoomIn(const int &nChl, const int &nSpeed) = 0;
	
	//控制调整变倍小，nChl为通道号， nSpeed为镜头速度，取值从0到7
	virtual int ControlPTZZoomOut(const int &nChl, const int &nSpeed) = 0;
	
	//控制自动水平旋转，nChl为通道号，bOpend为开启或停止自动
	virtual int ControlPTZAuto(const int &nChl, bool bOpend) = 0;
	
	//停止云台运动, nChl为通道号, nCmd为控制指令
	virtual int ControlPTZStop(const int &nChl, const int &nCmd) = 0;
	
};

#endif