/*
	NetSDK / Net Video Property  Definitions
	NOTE / WARNNING:
	1. 1st Argument contain comon arguments, ipaddress / port / username / password
	2. 2nd Argument 如果存在第一个参数无法表达的参数，由第二参数表示。
		在GET 时可能第二个参数中包含部分输入参数部分(由IN /INOUT 前缀特别注明)。
		在调用前需要初使化这部分变量。
		另外在这个接口实现时也要小心使用memset，避免错误地修改了其中输入参数的部分。
	3. 接口的实例化时不能为空指针，当协议内没有相关的接口定义时，定义一个空函数，
		函数内不做具体工作，直接返回-1；
	4. index : start from 0 ; 0, 1, 2...
	5. 接口的初使化不能是null , 如果是协议不支持或暂未计划实现可以在函数
		实现时直接返回-1.
	6. 对于一些协议如果不支持NVP 相关接口，例如RTSP，可以直接使用gNVPNULL实例。
	7. 以上5，6点的规范都是避免了方法调用前的是否为空的检查。
*/
#ifndef __NVP_DEFINE_H_
#define __NVP_DEFINE_H_

/*#include "stdinc.h"*/

#include "env_common.h"

#define NVP_RET_OK						(0)
#define NVP_RET_FAULT					(-1)
#define NVP_RET_SERVER_NOT_SUPPORT		(-2)
#define NVP_RET_NO_ROUTE_TO_HOST		(-3)
#define NVP_RET_HOST_UNREACHABLE		(-4)
#define NVP_RET_AUTH_FAILED				(-5)
#define NVP_RET_INVALID_ARG				(-6)
#define NVP_RET_CONNECT_REFUSED		(-7)

/* common input args */
typedef struct NVP_ARGS
{
	void *thiz; // reference to stNVP_INTERFACE
	char ip[20];
	unsigned short port;
	char username[24];
	char password[24];
	int chn;
}stNVP_ARGS, *lpNVP_ARGS;

#define NVP_INIT_ARGS(ARG, IP, PORT, USR, PWD)	\
	do{\
		strcpy(ARG.ip, IP);\
		ARG.thiz = NULL;\
		ARG.chn = 0; \
		ARG.port = PORT;\
		if(USR) strcpy(ARG.username, USR);\
		else strcpy(ARG.username, "");\
		if(PWD) strcpy(ARG.password, PWD);\
		else strcpy(ARG.password, "");\
	}while(0);
#define NVP_INIT_ARGS2(ARG, THIZ, IP, PORT, USR, PWD, CHN)	\
		do{\
			strcpy(ARG.ip, IP);\
			ARG.thiz = THIZ; \
			ARG.port = PORT;\
			ARG.chn = CHN;\
			if(USR) strcpy(ARG.username, USR);\
			else strcpy(ARG.username, "");\
			if(PWD) strcpy(ARG.password, PWD);\
			else strcpy(ARG.password, "");\
		}while(0);

typedef enum
{
	NVP_SELF_EVENT_ALL = -1,
	NVP_AUTH_FAILED = 0,
	NVP_EVENT_RENEW_FAILED,
	
	NVP_HOOK_EVENT_NR
}NVP_HOOK_EVENT;
	
typedef struct nvp_interface
{
	// private context data, external caller don't care it , it only use for internal developer of detailed protocal
	void *private_ctx; 
	/////////////////////////////////
	/*
	本接口与协议无关，例如当NVP 相关接口调用时出现用户验证失败时，
	能及时通知上层调用对象。
	*/
	int (*SetNVPEventHook)			(lpNVP_ARGS args, NVP_HOOK_EVENT event, fNVPEventHook hook, void *custom);
	////////////////////////////////
	//interfaces relate to specific protocal
	int (*GetSystemVersion)			(lpNVP_ARGS args, char *version);
	int (*GetProtocalVersion)			(lpNVP_ARGS args, char *version);
	
	int (*Search)();
	
	int (*GetDeviceInfo)				(lpNVP_ARGS args, lpNVP_DEV_INFO info);
	int (*GetRtspUri)					(lpNVP_ARGS args, lpNVP_RTSP_STREAM rtsps);
	int (*GetSystemDateTime)			(lpNVP_ARGS args, lpNVP_SYS_TIME dt);
	int (*SetSystemDateTime)			(lpNVP_ARGS args, lpNVP_SYS_TIME dt /*if dt == NULL, use system local time */);
	
	int (*GetVideoEncoderConfigs)		(lpNVP_ARGS args, lpNVP_VENC_CONFIGS venc); // get all stream configurations
	int (*GetVideoEncoderConfig)		(lpNVP_ARGS args, lpNVP_VENC_CONFIG venc);  // get one stream configuration
	int (*SetVideoEncoderConfig)		(lpNVP_ARGS args, lpNVP_VENC_CONFIG venc);
	int (*GetVideoEncoderConfigOption)(lpNVP_ARGS args, lpNVP_VENC_OPTIONS option);
	
	int (*GetColorConfig)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	int (*SetColorConfig)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	int (*SetHue)						(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	int (*SetSharpness)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	int (*SetContrast)					(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	int (*SetBrightness)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	int (*SetSaturation)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);

	int (*SetNetworkInterface)		(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether);
	int (*GetNetworkInterface)		(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether);

	int (*GetTitleOverlay)				(lpNVP_ARGS args, lpNVP_TITLE_OVERLAY overlay);
	int (*GetDateTimeOverlay)			(lpNVP_ARGS args, lpNVP_TIME_OVERLAY overlay);
	int (*SetTitleOverlay)				(lpNVP_ARGS args, lpNVP_TITLE_OVERLAY overlay);
	int (*SetDateTimeOverlay)			(lpNVP_ARGS args, lpNVP_TIME_OVERLAY overlay);

	int (*GetDigitalInputConfig)		(lpNVP_ARGS args, lpNVP_INPUT_CONFIG input);
	int (*SetDigitalInputConfig)		(lpNVP_ARGS args, lpNVP_INPUT_CONFIG input);

	int (*SubscribeEvent)				(lpNVP_ARGS args, lpNVP_SUBSCRIBE subs);
	int (*CancelEvent)					(lpNVP_ARGS args, lpNVP_SUBSCRIBE subs);
	int (*GetEventStatus)				(lpNVP_ARGS args, lpNVP_EVENT event);

	int (*GetMDConfig)					(lpNVP_ARGS args, lpNVP_MD_CONFIG md);
	int (*SetMDConfig)					(lpNVP_ARGS args, lpNVP_MD_CONFIG md);

	int (*ControlPTZ)					(lpNVP_ARGS args, lpNVP_PTZ_CONTROL ptz);	
	int (*Reboot)						(lpNVP_ARGS args, void *data);

	int (*GetJpegImage)				(lpNVP_ARGS args, void *data);
}stNVP_INTERFACE, *lpNVP_INTERFACE;

extern stNVP_INTERFACE gNVPNULL;

#define BASE_NOTIFICATION_PORT		(25300) // WS-BASE NOTIFICATION LOCAL LISTEN PORT
#define BASE_NOTIFICATION_TIMEOUT	(59) //TIMEOUT TO RENEW SUBSCRIBE, UNIT: SECOND

inline void NVP_dump_device_info(lpNVP_DEV_INFO info);
inline void NVP_dump_venc_options(lpNVP_VENC_OPTIONS options);
inline void NVP_dump_venc_profile(lpNVP_VENC_CONFIG profile);
inline void NVP_dump_ether_info(lpNVP_ETHER_CONFIG ether);

extern stNVP_INTERFACE gHichipV110NVPInterface; //only use hichip cgi interfaces or gw2.cgi
extern stNVP_INTERFACE gHichipV120NVPInterface; // mainly use new-well-designed sdk interfaces called NetSDK with json data structure
											// only for JA IPCs' version newer than 1.2.0
extern stNVP_INTERFACE gOnvifInterface;
extern stNVP_INTERFACE gZavioInterface;

extern lpNVP_INTERFACE NVP_HICHIPV110_new(); //OLD cgi interfaces
extern lpNVP_INTERFACE NVP_HICHIPV120_new(); // new netsdk with json interfaces and well designed
extern void NVP_HICHIP_delete(lpNVP_INTERFACE iface);
extern lpNVP_INTERFACE NVP_ONVIF_new();
extern void NVP_ONVIF_delete(lpNVP_INTERFACE iface);
extern lpNVP_INTERFACE NVP_ZAVIO_new();
extern void NVP_ZAVIO_delete(lpNVP_INTERFACE iface);


#endif //__NVP_DEFINE_H_

