#ifndef __IDEVICESEARCH_HEAD_FILE_HVPAPdsfdsfG87WDHFVC8SA__
#define __IDEVICESEARCH_HEAD_FILE_HVPAPdsfdsfG87WDHFVC8SA__
#include <libpcom.h>
#include <IEventRegister.h>

interface IOnvifRemoteInfo : public IPComBase
{
	virtual bool getOnvifDeviceNetwordInfo(QString sIp,QString sPort,QString sUserName,QString sPassword,QString &sMac,QString &sGateway,QString &sMask,QString &sDns)=0;
	virtual bool setOnvifDeviceNetWordInfo(QString sIp,QString sPort,QString sUserName,QString sPassword,QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns)=0;

	virtual QString getOnvifDeviceEncoderInfo(QString sIp,QString sPort,QString sUserName,QString sPassword);
	//返回值格式为 xml 格式的 字符串
	/*
	<OnvifStreamEncoderInfo itemNum=''>
		<streamItem index='' width='' height='' enc_fps='' enc_bps='' codeFormat='' enc_interval='' enc_profile=''/>
	<OnvifStreamEncoderInfo>

	index:码流索引（0：主码流），width：码流宽，height：码流高，enc_fps：帧率，enc_bps：码率，codeFormat：编码格式，enc_interval：I帧间隔

	*/
	virtual bool setOnvifDeviceEncoderInfo(QString sIp,QString sPort,QString sUserName,QString sPassword,int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile)=0;
};

#endif