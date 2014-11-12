/******************************************************************************

  Copyright (C), 2014-2020, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : minirtsp.h
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2014/11/3
  Last Modified : 2014/11/3
  Description   : rtsp  module  interfaces, reference to rfc2326
 
  History       : 
  1.Date        : 2014/11/3
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/

#ifndef __MINIRTSP_H__
#define __MINIRTSP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
/*#include <stdbool.h>*/


struct minirtsp_ctx;
typedef struct minirtsp_ctx stMINIRTSP, *lpMINIRTSP;

enum {
	MINIRTSP_TRANSPORT_AUTO = 0,
	MINIRTSP_TRANSPORT_UDP,
	MINIRTSP_TRANSPORT_OVER_RTSP,
};

/**********************************************************************************************
* client-side interfaces
**********************************************************************************************/

/*
* function: MINIRTSP_client_new
* description: rtsp client context new
* parammeter:
*		-- streamUrl: stream formated as rtsp://[domain][:port][URI]
*		-- transport : transport, [auto|udp|rtp_over_rtsp]
*		-- userName : user name, null is allowed
*		-- userPwd : user password, null is allowed
*		-- autoReconnect: whether auto reconnect or not when disconnected
*		-- enableRTCP: rtcp enable or not
* return: failed return null, else return context of minirtsp
*/
lpMINIRTSP
MINIRTSP_client_new(const char *streamUrl, int transport, 
	const char *userName, const char *userPwd, 
	bool autoReconnect, bool enableRTCP);

/*
* function: MINIRTSP_delete
* description: rtsp client context delete
* parammeter:
*		-- thiz: minirtsp context
*/
void
MINIRTSP_delete(lpMINIRTSP thiz);

/*
* function: MINIRTSP_connect
* description: rtsp perform connect to server
* parammeter:
*		-- thiz: minirtsp context
* return: success return 0, else return -1
*/
int
MINIRTSP_connect(lpMINIRTSP thiz);

/*
* function: MINIRTSP_play
* description: rtsp perform play (which mean to start receive stream data)
* parammeter:
*		-- thiz: minirtsp context
* return: success return 0, else return -1
*/
int
MINIRTSP_play(lpMINIRTSP thiz);

/*
* function: MINIRTSP_pause
* description: rtsp perform pause
* parammeter:
*		-- thiz: minirtsp context
*/
int
MINIRTSP_pause(lpMINIRTSP thiz);

/*
* function: MINIRTSP_disconnect
* description: rtsp perform disconnect
* parammeter:
*		-- thiz: minirtsp context
* return: success return 0, else return -1
*/
int
MINIRTSP_disconnect(lpMINIRTSP thiz);


/*
* function: MINIRTSP_lookup_stream
* description: rtsp get stream properties given in rtsp describe
* parammeter:
*		-- thiz: minirtsp context
*		-- dataType: data type , such as [h264, alaw, ulaw...]
*		-- dataProperties: structured memery location
* return: success return 0, else return -1
*/
typedef struct minirtsp_data_property
{
	char dataType[16];
	int channel;
	int mediaFormat;
	union
	{
		struct{
			int width, height, fps, bps;
			uint8_t u_sps[256];
			uint8_t u_pps[256];
		}h264;		
		struct{
			int sampleRate, sampleSize, channel;
		}g711;
	};
}stMINIRTSP_DATA_PROPERTY, *lpMINIRTSP_DATA_PROPERTY;

int
MINIRTSP_lookup_data(lpMINIRTSP thiz, const char *dataType, 
	lpMINIRTSP_DATA_PROPERTY dataProperties);

/*
* function: MINIRTSP_set_data_hook
* description: rtsp set a hook to handle data
* parammeter:
*		-- thiz: minirtsp context
*		-- hook: type of fMINIRTSP_DATA_HOOK
*		-- customCtx: user-defined context
* return: none
*/
typedef void (*fMINIRTSP_DATA_HOOK)(void *pdata, uint32_t dataSize, uint32_t timestamp,
	char *dataType, void *customCtx);
void
MINIRTSP_set_data_hook(lpMINIRTSP thiz, fMINIRTSP_DATA_HOOK hook, void *customCtx);

enum {
	MINIRTSP_EVENT_CONNECTD = 0,
	MINIRTSP_EVENT_DISCONNECTED,
	MINIRTSP_EVENT_PLAY,
	MINIRTSP_EVENT_RECORD,
	MINIRTSP_EVENT_PAUSE,
	MINIRTSP_EVENT_AUTH_REQUIRE,
	MINIRTSP_EVENT_AUTH_FAILED,
	MINIRTSP_EVENT_RTCP_REPORT,
	MINIRTSP_EVENT_CHECK_ALIVE_FAILED,
};
/*
* function: MINIRTSP_set_event_hook
* description: rtsp set a hook to handle rtsp inner event if you are interest with it 
* parammeter:
*		-- thiz: minirtsp context
*		-- hook: type of fMINIRTSP_EVENT_HOOK
*		-- customCtx: user-defined context
* return : none
*/

typedef void (*fMINIRTSP_EVENT_HOOK)(int eventType, int lParam, void *rParam, void *customCtx);
void
MINIRTSP_set_event_hook(lpMINIRTSP thiz, fMINIRTSP_EVENT_HOOK hook, void *customCtx);



/**********************************************************************************************
* server-side interfaces
**********************************************************************************************/
lpMINIRTSP
MINIRTSP_server_new(bool enableRTCP);

typedef void (*fMINIRTSP_GET_AVC_HOOK)(void *data, void *stream);
lpMINIRTSP
MINIRTSP_add_h264(lpMINIRTSP thiz, const char *streamName, fMINIRTSP_GET_AVC_HOOK hook);
lpMINIRTSP
MINIRTSP_add_g711(lpMINIRTSP thiz, const char *streamName);


lpMINIRTSP
MINIRTSP_server_new2(const char *sdp, bool enableRTCP);


void
MINIRTSP_server_start(lpMINIRTSP thiz);

void
MINIRTSP_server_stop(lpMINIRTSP thiz);

#ifdef __cplusplus
}
#endif

#endif


