#ifndef ONVIFNETWORK_H
#define ONVIFNETWORK_H

#include <QObject>
#include <QMutex>
#include "IAutoSycTime.h"
#include "IOnvifRemoteInfo.h"
extern "C" {
#include "nvp_define.h"
};

#pragma comment(lib, "libonvifc.lib")

typedef struct ETHER_CONFIG
{
	char name[NVP_MAX_NT_SIZE];
	IN char token[NVP_MAX_NT_SIZE]; 
	//
	int dhcp;
	unsigned char ip[4];
	unsigned char netmask[4];
	unsigned char gateway[4];
	unsigned char dns1[4];
	unsigned char dns2[4];
	unsigned char mac[6];
	//
	int http_port;
	int rtsp_port;
}stETHER_CONFIG;

class  OnvifNetwork:public QObject,
	public IAutoSycTime,
	public IOnvifRemoteInfo
{
	Q_OBJECT
public:
	OnvifNetwork();
	~OnvifNetwork();

public:
	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	//interface for time Synchronous
	virtual int setAutoSycTime(bool bEnabled);

	//interface for onvif remote info
	virtual void setOnvifDeviceInfo(const QString &sIp, const QString &sPort, const QString &sUserName, const QString &sPassword);
	
	virtual QString getOnvifDeviceInfo();

	virtual bool getOnvifDeviceNetworkInfo(QString &sMac,QString &sGateway,QString &sMask,QString &sDns);
	
	virtual bool setOnvifDeviceNetWorkInfo(QString sSetIp,QString sSetMac,QString sSetGateway,QString sSetMask,QString sSetDns);
	
	virtual QString getOnvifDeviceEncoderInfo();

	virtual bool setOnvifDeviceEncoderInfo(int nIndex,int nWidth,int nHeight,QString sEnc_fps,QString sEnc_bps,QString sCodeFormat,QString sEncInterval,QString sEncProfile);

private:
	bool checkResolution(int index, int width, int height);
	bool checkProfile(int index, int profile);
	bool checkFourParam(QString para);
	bool checkMac(QString mac);
private:
	int m_nRef;
	QMutex m_csRef;

	stNVP_ARGS m_nvpArguments;
	stNVP_VENC_CONFIGS m_stVencConfigs;
	stNVP_VENC_OPTIONS m_stVencOptions[5];
	lpNVP_INTERFACE m_pNvpContext;
	int m_nConfigNum;
};

#endif // ONVIFNETWORK_H
