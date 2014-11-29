/******************************************************************************

  Copyright (C), 2014-2020, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : minirtsp.c
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtsplib.h"
#include "minirtsp.h"
#include "sock.h"
#include "gnu_win.h"
#include "portmanage.h"
#include "rtspsession.h"

#if defined(_WIN32) || defined(_WIN64)

#else
#include <signal.h>
#endif

struct minirtsp_ctx
{
	int trigger;
	ThreadId_t pid;
	EventHandle_t eventHandle;

	char userName[32];
	char userPwd[32];

	char streamUrl[256];

	int autoReconnect;

	int enableRTCP;

	int enableAudio;

	int transport;

	int rtspRole; // 0, client, 1, server

	int useRtpTimestamp;

	Rtsp_t *rtsp;

	fMINIRTSP_DATA_HOOK dataHook;
	fMINIRTSP_EVENT_HOOK eventHook;
	fMINIRTSP_GET_AVC_HOOK getAvcHook;
	void *dataCustomCtx;
	void *eventCustomCtx;
	int dataCustomCtxSize;
	int eventCustomCtxSize;

	int logLevel;
	int chn, streamId;
};

static volatile int g_TotalClientDaemon = 0;
static volatile int g_TotalServerDaemon = 0;

static void fMINIRTSP_DEFAULT_DATA_HOOK(void *pdata, unsigned int dataSize, 
	unsigned int timestamp,
	int dataType, void *customCtx)
{
	printf("MINIRTSP Got %d data(size:%u timestamp:%u)\n", dataType, dataSize, timestamp);
}

static void fMINIRTSP_DEFAULT_EVENT_HOOK(int eventType, int lParam, void *rParam,
	void *customCtx)
{

	switch (eventType)
	{
		case MINIRTSP_EVENT_CONNECTED:
		case MINIRTSP_EVENT_DISCONNECTED:
		case MINIRTSP_EVENT_DESTROYED:
		case MINIRTSP_EVENT_PLAY:
		case MINIRTSP_EVENT_RECORD:
		case MINIRTSP_EVENT_PAUSE:
		case MINIRTSP_EVENT_DATA_RECEIVED:
		case MINIRTSP_EVENT_AUTH_REQUIRE:
		case MINIRTSP_EVENT_AUTH_FAILED:
		case MINIRTSP_EVENT_RTCP_SENDER_REPORT:
		case MINIRTSP_EVENT_RTCP_RECEIVER_REPORT:
		case MINIRTSP_EVENT_CHECK_ALIVE_FAILED:
			printf("MINIRTSP Got EVENT[%d] param(%p)\n", eventType, customCtx);
			break;
		default:
			printf("MINIRTSP Got INVALID EVENT[%d] param(%p)\n", eventType, customCtx);
			break;
	}
}

static lpMINIRTSP minirtsp_new()
{
	lpMINIRTSP thiz = NULL;

	thiz = (lpMINIRTSP)calloc(1, sizeof(stMINIRTSP));
	if (thiz == NULL) {
		return NULL;
	}
	
	thiz->enableRTCP = FALSE;
	thiz->enableAudio = FALSE;
	thiz->trigger = FALSE;
	thiz->pid = 0;
	thiz->userName[0] = 0;
	thiz->userPwd[0] = 0;

	thiz->streamUrl[0] = 0;

	thiz->dataHook = fMINIRTSP_DEFAULT_DATA_HOOK;
	thiz->eventHook= fMINIRTSP_DEFAULT_EVENT_HOOK;
	thiz->dataCustomCtx = NULL;
	thiz->eventCustomCtx = NULL;

	thiz->logLevel = 2;
	
	return thiz;
}

static void *rtsp_client_proc(void *param)
{
	int error_occur = FALSE;
	int ret;
	int connect_status;
	SOCKLEN_t connect_len;
	Rtsp_t *r=NULL;
	int max_sock=0;
	fd_set read_set;
	struct timeval timeout;
	int timeout_cnt=0;
	char url[256];
	int TIMEOUTCNT = 1000;
	unsigned int timestamp;
	time_t curtime;

	lpMINIRTSP thiz=(lpMINIRTSP)param;
	r=(Rtsp_t *)thiz->rtsp;

#if defined(_WIN32) || defined(_WIN64)
#else
    struct sigaction sa; 
    sa.sa_handler = SIG_IGN; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &sa, 0);
	
	pthread_detach(pthread_self());	
#endif	
	if (thiz->eventHook) {
		thiz->eventHook(MINIRTSP_EVENT_CONNECTED, 0, NULL, thiz->eventCustomCtx);
	}
	VLOG(VLOG_CRIT,"rtsp client proc start %d_%d....",thiz->chn, thiz->streamId);

	if(!(r->state == RTSP_STATE_READY||r->state==RTSP_STATE_PLAYING)){
		error_occur = TRUE;
	}
	while((r->toggle == RTSPC_RUNNING) && (thiz->trigger)){		
		if((thiz->autoReconnect)&& (error_occur == TRUE)){
			// sent event
			if (thiz->eventHook && (r->state == RTSP_STATE_PLAYING))
				thiz->eventHook(MINIRTSP_EVENT_DISCONNECTED,
					0, NULL, thiz->eventCustomCtx);
			//
			RTSP_cleanup(r);
			MSLEEP(500);
			timeout_cnt = 0;
			sprintf(url,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
			ret = RTSP_connect_server2(r,url,r->user,r->pwd,(enRTP_TRANSPORT)r->b_interleavedMode,r->buffer_time);
			if(ret==RTSP_RET_FAIL)
			{
				VLOG(VLOG_WARNING,"RTSP reconnect to %s failed,retry later",url);
				continue;
			}else{
				error_occur = FALSE;
			}
			
			if(RTSP_request_play(r)==RTSP_RET_FAIL) {
				continue;
			}
			time(&r->m_sync);
			if(r->b_interleavedMode){
				SOCK_set_buf_size(r->sock, 0, 400*1024);
			}else{
				if(r->rtp_video){
					SOCK_set_buf_size(r->rtp_video->sock,0 , 400*1024);
				}
			}

			
			if (thiz->eventHook && (r->state == RTSP_STATE_PLAYING))
				thiz->eventHook(MINIRTSP_EVENT_CONNECTED,
					0, NULL, thiz->eventCustomCtx);
		}else if ((thiz->autoReconnect== FALSE) && (error_occur == TRUE)){
			break;
		}

		if(r->state != RTSP_STATE_PLAYING){
			MSLEEP(500);
			continue;
		}
		timeout.tv_sec=0;
		timeout.tv_usec=20*1000;
		FD_ZERO(&read_set);
		max_sock=0;
		if(r->rtp_video){
			FD_SET(r->rtp_video->sock,&read_set);
			if(r->rtp_video->sock > max_sock) max_sock=r->rtp_video->sock;
		}
		if(r->rtp_audio){
			FD_SET(r->rtp_audio->sock,&read_set);
			if(r->rtp_audio->sock > max_sock) max_sock=r->rtp_audio->sock;
		}
		if(r->rtcp_video){
			FD_SET(r->rtcp_video->sock,&read_set);
			if(r->rtcp_video->sock > max_sock) max_sock=r->rtcp_video->sock;
		}
		if(r->rtcp_audio){
			FD_SET(r->rtcp_audio->sock,&read_set);
			if(r->rtcp_audio->sock > max_sock) max_sock=r->rtcp_audio->sock;
		}
		ret = select(max_sock +1,&read_set,NULL,NULL,&timeout);
		if(ret < 0){
			VLOG(VLOG_ERROR,"select failed");
			error_occur = TRUE;
			continue;
		}else if(ret == 0){
			connect_len = sizeof(connect_status);
			if(getsockopt(r->sock,SOL_SOCKET,SO_ERROR,(char *)&connect_status,&connect_len) < 0){
				VLOG(VLOG_ERROR,"rtspc: socket disconnect,errno:%d",errno);
				error_occur = TRUE;
				continue;
			}else{
				if(connect_status != 0){
					VLOG(VLOG_ERROR,"rtspc: socket disconnect,errno:%d",errno);
					error_occur = TRUE;
					continue;
				}
			}
			timeout_cnt ++;
			if(r->data_available == FALSE){
				TIMEOUTCNT = 1000;
			}else{
				TIMEOUTCNT = 200;
			}
			if(timeout_cnt > TIMEOUTCNT){
				VLOG(VLOG_ERROR,"rtspc: select timeout");
				error_occur = TRUE;
				continue;
			}
		}else{
			timeout_cnt = 0;
			//
			if(r->b_interleavedMode == TRUE){
				ret=RTSP_read_message2(r, RTP_HANDLE, RTCP_HANDLE2);
				if(ret == RTSP_RET_FAIL){
					error_occur = TRUE;
					continue;
				}
			}else{
				if(FD_ISSET(r->sock,&read_set)){
					ret=RTSP_read_message2(r, NULL, NULL);
					if(ret == RTSP_RET_FAIL){
						error_occur = TRUE;
						continue;
					}
				}
				if(r->rtp_video){
					if(FD_ISSET(r->rtp_video->sock,&read_set)){
						if((ret=RTP_handle_packet_nortpbuf(r->rtp_video,NULL,0))==RTSP_RET_FAIL){
							error_occur = TRUE;
							continue;
						}
					}
						
				}
				if(r->rtp_audio){
					if(FD_ISSET(r->rtp_audio->sock,&read_set)){
						if((ret=RTP_handle_packet_nortpbuf(r->rtp_audio,NULL,0))==RTSP_RET_FAIL){
							error_occur = TRUE;
							continue;
						}
						
					}
				}
#ifdef RTCP_RECEIVER
				if(r->rtcp_audio){
					if(FD_ISSET(r->rtcp_audio->sock,&read_set)){
						RTCP_handle_packet2(r->rtcp_audio,NULL,0);
					}
				}
				if(r->rtcp_video){
					if(FD_ISSET(r->rtcp_video->sock,&read_set)){
						RTCP_handle_packet2(r->rtcp_video,NULL,0);
					}
				}
#endif
			}

			if(r->rtp_audio){
				if(r->rtp_audio->packet.iFrameCnt){
					//VLOG(VLOG_DEBUG,"Channel(%d,%d) audio", dec_chn, dec_stream);
					timestamp = r->rtp_audio->timestamp / ( SDP_MEDIA_G711_FREQ / 1000);
					if (thiz->dataHook)
						thiz->dataHook(r->rtp_audio->packet.buffer,
							r->rtp_audio->packet.buf_size[0],timestamp,r->rtp_audio->mediaType, 
							thiz->dataCustomCtx);
					r->rtp_audio->packet.buf_size[0] = 0;
					r->rtp_audio->packet.iFrameCnt = 0;
					//VLOG(VLOG_ERROR,"Channel(%d,%d) audio timestamp:%lld(%lld)", dec_chn, dec_stream, timestamp, src_timestamp);
				}
			}
			if(r->rtp_video){
				if(r->rtp_video->packet.iFrameCnt){
					//VLOG(VLOG_DEBUG,"Channel(%d,%d) video", dec_chn, dec_stream);
					timestamp = r->rtp_video->timestamp / ( SDP_MEDIA_H264_FREQ / 1000);
										
					if (thiz->dataHook)
						thiz->dataHook(r->rtp_video->packet.buffer,
							r->rtp_video->packet.buf_size[0],timestamp,r->rtp_video->mediaType, 
							thiz->dataCustomCtx);
					if (thiz->eventHook && (r->data_available == 0))
						thiz->eventHook(MINIRTSP_EVENT_DATA_RECEIVED,
							r->rtp_video->mediaType, "video", thiz->eventCustomCtx);
					r->data_available = TRUE;
					r->rtp_video->packet.buf_size[0] = 0;
					r->rtp_video->packet.iFrameCnt = 0;					
					//VLOG(VLOG_ERROR,"Channel(%d,%d) video timestamp:%lld(%lld)", dec_chn, dec_stream, timestamp, src_timestamp);
					

				}
			}
		}

		// keep liveness of rtsp session
		time(&curtime);
		if((curtime - r->m_sync) > 28){
			if(RTSP_keep_liveness(r) < 0)
			{
				//usleep(5000);
				MSLEEP(5);
				if(RTSP_keep_liveness(r) < 0)
				{
					error_occur = true;
					VLOG(VLOG_ERROR,"Channel(%d,%d) keep liveness fail!", thiz->chn, thiz->streamId);
				}
			}
			time(&r->m_sync);
		}


#ifdef RTCP_RECEIVER
		if(r->rtcp_audio){
			RTCP_process(r->rtcp_audio);
		}
		if(r->rtcp_video){
			RTCP_process(r->rtcp_video);
		}
#endif
	}

	
	RTSP_cleanup(r);
	//RTSP_destroy(r);

	thiz->trigger = FALSE;
	VLOG(VLOG_CRIT,"(%d:%d) rtsp client proc exit", thiz->chn, thiz->streamId);
	if (thiz->eventHook) {
		thiz->eventHook(MINIRTSP_EVENT_DISCONNECTED, 0, NULL, thiz->eventCustomCtx);
	}
	thiz->eventHook(MINIRTSP_EVENT_DESTROYED,
		0, NULL, thiz->eventCustomCtx);
	
	EVT_post(thiz->eventHandle);

	THREAD_exit();	
	return NULL;
}

static void *rtsp_server_proc(void *param)
{
	return NULL;
}

static int rtsp_client_proc_start(lpMINIRTSP thiz)
{
	if (thiz->pid == 0) {
		thiz->trigger = TRUE;
		THREAD_create(thiz->pid,rtsp_client_proc,thiz);
		return 0;
	} else {
		printf("warning : rtsp_client_proc has started!\n");
		return -1;
	}
}

static int rtsp_client_proc_stop(lpMINIRTSP thiz)
{
	thiz->trigger = FALSE;
	if (thiz->pid) {
		int ret;
		EVT_wait(thiz->eventHandle,10000000L,ret);
		thiz->pid = 0;
		if (ret != 0) {
			return -1;
		}
	}
	return 0;
}

static int rtsp_server_proc_start(lpMINIRTSP thiz)
{
	if (thiz->pid == 0) {
		THREAD_create(thiz->pid,rtsp_server_proc,thiz);
		return 0;
	} else {
		printf("warning : rtsp_server_proc has started!\n");
		return -1;
	}
	return 0;
}

static int rtsp_server_proc_stop(lpMINIRTSP thiz)
{
	return -1;
}


lpMINIRTSP
MINIRTSP_client_new(const char *streamUrl, int transport, 
	const char *userName, const char *userPwd, 
	int enableAudio, int autoReconnect)
{
	lpMINIRTSP thiz = NULL;

	/*
	check params
	*/
	if (!((transport == MINIRTSP_TRANSPORT_AUTO) || (transport == MINIRTSP_TRANSPORT_UDP)
		|| (transport == MINIRTSP_TRANSPORT_OVER_RTSP))) {
		return NULL;
	}
	
	thiz = minirtsp_new();
	if (thiz == NULL) {
		return NULL;
	}

	strncpy(thiz->streamUrl, streamUrl, sizeof(thiz->streamUrl));
	strncpy(thiz->userName, userName, sizeof(thiz->userName));
	strncpy(thiz->userPwd, userPwd, sizeof(thiz->userPwd));

	thiz->transport = transport;
	thiz->autoReconnect = autoReconnect;
	thiz->enableRTCP = TRUE;

	
	thiz->rtsp = RTSP_CLIENT_init(streamUrl,(char *)userName,(char *)userPwd,transport,1000, enableAudio);
	if (thiz->rtsp == NULL) {
		free(thiz);
		return NULL;
	}

	thiz->rtspRole = RTSP_CLIENT;

	g_TotalClientDaemon++;

	EVT_init(thiz->eventHandle, 0);
	
	//RTSP_session_init();

	// init port manage
	PORT_MANAGE_init(60000,65000);

	return thiz;
}

void
MINIRTSP_delete(lpMINIRTSP thiz)
{
	if (thiz) 
	{
		if (thiz->dataCustomCtxSize > 0) {
			free(thiz->dataCustomCtx);
		}
		thiz->dataCustomCtx = NULL;
		if (thiz->eventCustomCtxSize > 0) {
			free(thiz->eventCustomCtx);
		}
		thiz->eventCustomCtx = NULL;
		
		if (thiz->rtspRole == RTSP_CLIENT) {
			MINIRTSP_disconnect(thiz);
#if 1
			g_TotalClientDaemon--;
			if (g_TotalClientDaemon == 0) {
				PORT_MANAGE_destroy();
			}
#else
					
#endif

		} else {
			//MINIRTSP_server_stop(thiz);

		}
		EVT_destroy(thiz->eventHandle);
		
		RTSP_destroy(thiz->rtsp);
		free(thiz);
		thiz = NULL;

	}
}

int
MINIRTSP_connect(lpMINIRTSP thiz)
{
	if (thiz && thiz->rtsp) {
		int ret = RTSP_connect_server2(thiz->rtsp, thiz->streamUrl,thiz->userName, thiz->userPwd,
			thiz->transport, 0);
		if ((ret < 0) && (thiz->autoReconnect == false))
			return -1;
		if ((ret == 0) && thiz->eventHook) {
			thiz->eventHook(MINIRTSP_EVENT_CONNECTED, 0, NULL, thiz->eventCustomCtx);
		}
		return rtsp_client_proc_start(thiz);
	} else
		return -1;
}

int
MINIRTSP_play(lpMINIRTSP thiz)
{
	if(thiz && thiz->rtsp)
		return RTSP_request_play(thiz->rtsp);
	else
		return -1;
}

int
MINIRTSP_pause(lpMINIRTSP thiz)
{
	if(thiz && thiz->rtsp)
		return RTSP_request_pause(thiz->rtsp);
	else
		return -1;
}

int
MINIRTSP_disconnect(lpMINIRTSP thiz)
{
	if (thiz && thiz->rtsp) {
		time_t _time;
		int ret;
		
		time(&_time);
		MINIRTSP_pause(thiz);
		ret = rtsp_client_proc_stop(thiz);
		printf("MINIRTSP_disconnect elaspse time:%lu S\n", time(NULL) - _time);
		return ret;
	} else
		return -1;
}

static char *_get_stream_name(int dataType)
{
	switch(dataType)
	{
		case MD_TYPE_H264:
			return "H264";
		case MD_TYPE_H265:
			return "H265";
		case MD_TYPE_AAC:
			return "AAC";
		case MD_TYPE_ALAW:
			return "PCMA";
		case MD_TYPE_ULAW:
			return "PCMU";
		default:
			return NULL;			
	}
}

void MINIRTSP_dump_data_property(lpMINIRTSP_DATA_PROPERTY dataProperties)
{
	return;
	if (dataProperties) {
		printf("MdediaName: %s type:%d chn/port:%d\n", dataProperties->mediaName,
			dataProperties->mediaType, dataProperties->chn_port);
		if (strcmp(dataProperties->mediaName, "H264") == 0) {
			printf("\tfps:%d WxH :%dx%d\n", dataProperties->h264.fps, dataProperties->h264.width,
				dataProperties->h264.height);
			printf("\t sps_size: %d\n", dataProperties->h264.spsSize);
			VLOG_Hex(VLOG_CRIT, dataProperties->h264.u_sps,dataProperties->h264.spsSize);
			printf("\t pps_size: %d\n", dataProperties->h264.ppsSize);
			VLOG_Hex(VLOG_CRIT, dataProperties->h264.u_pps,dataProperties->h264.ppsSize);
		}
		else if (strcmp(dataProperties->mediaName, "ALAW") == 0 ||
			(strcmp(dataProperties->mediaName, "ULAW") == 0)) {
			printf("\t sample rate:%d sample size: %d channel:%d\n", dataProperties->g711.sampleRate,
				dataProperties->g711.sampleSize, dataProperties->g711.channel);
		}
	}
}

int
MINIRTSP_lookup_data(lpMINIRTSP thiz, int dataType, 
	lpMINIRTSP_DATA_PROPERTY dataProperties)
{
	int i, ret = -1;
	Attribute_t attr;
	Rtsp_t *r = NULL;
	int gotIt = 0;
	const char *mediaName = _get_stream_name(dataType);

	if (!(thiz && mediaName && dataProperties && thiz->rtsp && thiz->rtsp->sdp)) {
		printf("MINIRTSP_lookup_data invalid param!\n"); 
		return -1;
	}
	r = thiz->rtsp;

	memset(dataProperties, 0, sizeof(stMINIRTSP_DATA_PROPERTY));
	strncpy(dataProperties->mediaName, mediaName, sizeof(dataProperties->mediaName));

	for(i=0;i<r->sdp->media_num;i++){
		if (((MD_TYPE_ALAW == dataType) || (MD_TYPE_ALAW == dataType))
			&& (dataType == r->sdp->media[i].media_n.format)){
				dataProperties->g711.sampleRate = 8000;
				dataProperties->g711.sampleSize = 16;
				dataProperties->g711.channel = 1;		
			//ret = SDP_get_g711_info(r->sdp, &dataProperties->mediaType,
			//	dataProperties->szip, &dataProperties->chn_port);
			if(0 == 0){
				gotIt = TRUE;
			}
			break;
		}
		if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
				SDP_ATTR_RTP_MAP,(void *)&attr) == RTSP_RET_OK){
			if(strncmp(attr.rtpmap.codec_type,mediaName,strlen(mediaName)) == 0) {
				/*
				if(strcmp(r->sdp->media[i].media_n.type, "audio") == 0){
					;
				} else if(strcmp(r->sdp->media[i].media_n.type, "video") == 0){
					;
				} else {
					printf("unknown media type, not video and audio!\n"); 
					return -1;
				}
				*/
				if((strncmp(mediaName,"PCMU",strlen("PCMU")) == 0) ||
					(strncmp(mediaName,"PCMA",strlen("PCMA")) == 0)) {
					dataProperties->g711.sampleRate = 8000;
					dataProperties->g711.sampleSize = 16;
					dataProperties->g711.channel = 1;
					ret = SDP_get_g711_info(r->sdp, &dataProperties->mediaType,
						dataProperties->szip, &dataProperties->chn_port);
					if(ret!=0){
						printf("MINIRTSP_lookup_data(%s) failed!,num:%d\n", mediaName,r->sdp->media_num); 
					}
				}
				else if(strncmp(mediaName,"H264",strlen("H264")) == 0) {
					ret = SDP_get_h264_info(r->sdp, &dataProperties->mediaType,
						dataProperties->szip, &dataProperties->chn_port, 
						dataProperties->h264.u_sps, &dataProperties->h264.spsSize,
						dataProperties->h264.u_pps, &dataProperties->h264.ppsSize,
						&dataProperties->h264.fps, 
						&dataProperties->h264.width, &dataProperties->h264.height);
				} else {
					printf("unknown media type, not g711 or h264!\n"); 
					return -1;
				}

				//
				if (ret == 0) {
					gotIt = TRUE;
				}
				break;
			}
		}
	}

	if (gotIt == TRUE) {
		//MINIRTSP_dump_data_property(dataProperties);
		return 0;
	}else {
		printf("MINIRTSP_lookup_data(%s) failed!,num:%d\n", mediaName,r->sdp->media_num); 
		return -1;
	}
}


void
MINIRTSP_set_data_hook(lpMINIRTSP thiz, fMINIRTSP_DATA_HOOK hook, void *customCtx, int CustomCtxSize)
{
	if (thiz) {
		if (CustomCtxSize > 0) {
			thiz->dataCustomCtx = (void *)calloc(1, CustomCtxSize);
			if (thiz->dataCustomCtx == NULL) {
				return;
			}
			thiz->dataCustomCtxSize = CustomCtxSize;
		} else {
			thiz->dataCustomCtx = customCtx;
			thiz->dataCustomCtxSize = 0;
		}
				
		thiz->dataHook = hook;
	}
}




void
MINIRTSP_set_event_hook(lpMINIRTSP thiz, fMINIRTSP_EVENT_HOOK hook, void *customCtx, int CustomCtxSize)
{
	if (thiz) {
		if (CustomCtxSize > 0) {
			thiz->eventCustomCtx = (void *)calloc(1, CustomCtxSize);
			if (thiz->eventCustomCtx == NULL) {
				return;
			}
			thiz->eventCustomCtxSize = CustomCtxSize;
		} else {
			thiz->eventCustomCtx = customCtx;
			thiz->eventCustomCtxSize = 0;
		}
				
		thiz->eventHook = hook;

		// set inner hook
		thiz->rtsp->eventHook = hook;
		thiz->rtsp->eventCustomCtx = thiz->eventCustomCtx;
	}

}



