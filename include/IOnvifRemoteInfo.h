#ifndef __IDEVICESEARCH_HEAD_FILE_HVPAPdsfdsfG87WDHFVC8SA__
#define __IDEVICESEARCH_HEAD_FILE_HVPAPdsfdsfG87WDHFVC8SA__
#include <libpcom.h>
#include <IEventRegister.h>

interface IOnvifRemoteInfo : public IPComBase
{
	virtual void setOnvifDeviceInfo(const QString &sIp, const QString &sPort, const QString &sUserName, const QString &sPassword) = 0;
	
	//返回值格式为 xml 的字符串
	/* 
	<OnvifDeviceInfo manufacturer="" devname="" model="" firmware="" sn="" hwid="" sw_builddate="" sw_version="" hw_version="" />
	manufacturer：生产厂家，devname：设备名称，model：型号，firmware：固件，hwid：硬件ID，sw_builddate：软件生成时间，sw_version：软件版本，hw_version：硬件版本
	*/
	virtual QString getOnvifDeviceInfo() = 0;

	virtual bool getOnvifDeviceNetworkInfo(QString &sMac,QString &sGateway,QString &sMask,QString &sDns)=0;
	
	virtual bool setOnvifDeviceNetWorkInfo(QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns)=0;
	
	//返回值格式为 xml 格式的 字符串
	/*
	<OnvifStreamEncoderInfo itemNum="">
		<StreamItem index="" width="" height="" enc_fps="" enc_bps="" codeFormat="" enc_interval="" enc_profile="">
			<EncodeOption>
				<enc_quality min="" max=""/>
				<enc_fps min="" max=""/>
				<enc_bps min="" max=""/>
				<enc_gov min="" max=""/>
				<enc_interval min="" max=""/>
				<resolution itemNum="">
					<item width="" height=""/>
					<item width="" height=""/>
				</resolution>
				<enc_profile itemNum="">
					<item profile=""/>
					<item profile=""/>
				</enc_profile>
			</EncodeOption>
		</StreamItem>
	<OnvifStreamEncoderInfo>
	index:码流索引（0：主码流），width：码流宽，height：码流高，enc_fps：帧率，enc_bps：码率，codeFormat：编码格式，enc_interval：I帧间隔
	*/
	virtual QString getOnvifDeviceEncoderInfo() = 0;

	virtual bool setOnvifDeviceEncoderInfo(int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile)=0;
};

#endif