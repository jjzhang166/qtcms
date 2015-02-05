#ifndef __IDEVICESEARCH_HEAD_FILE_HVPAPdsfdsfG87WDHFVC8SA__
#define __IDEVICESEARCH_HEAD_FILE_HVPAPdsfdsfG87WDHFVC8SA__
#include <libpcom.h>
#include <IEventRegister.h>

interface IOnvifRemoteInfo : public IPComBase
{
	virtual void setOnvifDeviceInfo(const QString &sIp, const QString &sPort, const QString &sUserName, const QString &sPassword) = 0;
	
	virtual bool getOnvifDeviceNetworkInfo(QString &sMac,QString &sGateway,QString &sMask,QString &sDns)=0;
	
	virtual bool setOnvifDeviceNetWorkInfo(QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns)=0;
	
	//返回值格式为 xml 格式的 字符串
	/*
	<OnvifStreamEncoderInfo itemNum=''>
		<streamItem index='' width='' height='' enc_fps='' enc_bps='' codeFormat='' enc_interval='' enc_profile=''/>
	<OnvifStreamEncoderInfo>
	index:码流索引（0：主码流），width：码流宽，height：码流高，enc_fps：帧率，enc_bps：码率，codeFormat：编码格式，enc_interval：I帧间隔
	*/
	virtual QString getOnvifDeviceEncoderInfo() = 0;

	virtual bool setOnvifDeviceEncoderInfo(int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile)=0;
};

#endif