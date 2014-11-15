/******************************************************************************

  Copyright (C), 2013-2020, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : rtsplib.c
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2013/04/25
  Last Modified : 2013/04/25
  Description   : rtsp  utils , reference to rfc2326
 
  History       : 
  1.Date        : 2013/04/25
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <assert.h>


#include "sock.h"
#include "rtsplib.h"
#include "rtspsession.h"
#include "portmanage.h"

//response status code definitions
typedef struct _RStatusCode
{
	int code;
	const char *info;
}RStatusCode_t;
enum{
	RTSP_RSC_CONTINUE = 0,			//// -0
	RTSP_RSC_OK,						////-1
	RTSP_RSC_CREATED,
	RTSP_RSC_LOW_STORAGE,
	RTSP_RSC_MULTI_CHOICES,
	RTSP_RSC_MOVED_PERMANENTLY,		////-5
	RTSP_RSC_MOVED_TEMPORARILY,
	RTSP_RSC_SEE_OTHER,
	RTSP_RSC_NOT_MODIFIED,
	RTSP_RSC_USE_PROXY,
	RTSP_RSC_BAD_REQUEST,			////-10
	RTSP_RSC_UNAUTHORIZED,
	RTSP_RSC_PAYMENT_REQUIRED,
	RTSP_RSC_FORBIDDEN,
	RTSP_RSC_NOT_FOUND,
	RTSP_RSC_METHOD_NOT_ALLOWED,		////-15
	RTSP_RSC_NOT_ACCEPTABLE,
	RTSP_RSC_PROXY_AUTHEN_REQUIRED,
	RTSP_RSC_REQUEST_TIMEOUT,
	RTSP_RSC_GONE,
	RTSP_RSC_LENGTH_REQUIRED,		////-20
	RTSP_RSC_PRECONDITION_FAILED,
	RTSP_RSC_REQUEST_ENTITY_TOOLARGE,
	RTSP_RSC_REQUEST_URI_TOOLARGE,
	RTSP_RSC_UNSUPPORTED_MEDIA,
	RTSP_RSC_PARAMETER_NOT_UNDERSTOOD,	////-25
	RTSP_RSC_CONFERENCE_NOT_FOUND,
	RTSP_RSC_NOT_ENOUGHT_BANDWIDTH,
	RTSP_RSC_SESSION_NOT_FOUND,
	RTSP_RSC_METHOD_NOT_VAILD,
	RTSP_RSC_HEADER_FIELD_NOT_VAILD,		////-30
	RTSP_RSC_INVALID_RANGE,
	RTSP_RSC_PARAMETER_IS_READONLY,
	RTSP_RSC_AGGREGATE_NOT_ALLOWED,
	RTSP_RSC_ONLY_AGGREGATE_ALLOWED,
	RTSP_RSC_UNSUPPORTED_TRANSPORT,		////-35
	RTSP_RSC_DESITIATION_UNREACHABLE,
	RTSP_RSC_INTERNAL_SERVER_ERROR,
	RTSP_RSC_NOT_IMPLEMENTED,
	RTSP_RSC_GAD_GATEWAY,
	RTSP_RSC_SERVICE_UNAVAILABLE,		////-40
	RTSP_RSC_GATEWAY_TIMEOUT,
	RTSP_RSC_RTSP_VERSION_NOTSUPPORTED,
	RTSP_RSC_OPTION_NOTSUPPORTED,
	RTSP_RSC_END							////-44
};


static RStatusCode_t rtspRStatusCodes[]=
{
	{100,"Continue"},					////-0
	{200,"OK"},
	{201,"Created"},
	{250,"Low on Storage Space"},
	{300,"Multiple Choices"},
	{301,"Moved Permanently"},
	{302,"Moved Temporarily"},
	{303,"See Other"},
	{304,"Not MOdified"},
	{305,"Use Proxy"},
	{400,"Bad Request"},				////-10
	{401,"Unauthorized"},
	{402,"Payment Required"},
	{403,"Forbidden"},
	{404,"Not Found"},
	{405,"Method Not Allowed"},
	{406,"Not Acceptable"},
	{407,"Proxy Authentication Required"},
	{408,"Request Time-out"},
	{410,"Gone"},
	{411,"Length Required"},			////-20
	{412,"Precondition Failed"},
	{413,"Request Entity Too Large"},
	{414,"Request-URI Too Large"},
	{415,"Unsupported Media Type"},
	{451,"Parameter Not Understood"},
	{452,"Conference Not Found"},
	{453,"Not Enough Bandwidth"},
	{454,"Session Not Found"},
	{455,"Method Not Valid in This State"},
	{456,"Header Field Not Vaild for Resource"},	////-30
	{457,"Invalid Range"},
	{458,"Parameter Is Read-Only"},
	{459,"Aggregate operation not allowed"},
	{460,"Only aggregate operation allowed"},
	{461,"Unsupported transport"},
	{462,"Destination unreachable"},
	{500,"Internal Server Error"},
	{501,"Not Implemented"},
	{502,"Bad Gateway"},
	{503,"Service Unavailable"},					////-40
	{504,"Gateway Time-out"},
	{505,"RTSP Version not supported"},
	{551,"Option not supported"},					////-43
};


static const char  *rtspMethods[RTSP_METHOD_CNT]=
{
	"DESCRIBE",
	"ANNOUNCE",
	"GET_PARAMETER",
	"OPTIONS",
	"PAUSE",
	"PLAY",
	"RECORD",
	"REDIRECT",
	"SETUP",
	"SET_PARAMETER",
	"TEARDOWN",
	//""
};

//increase by 4
#define RTSP_MIN_CHN_PORT	1
#define RTSP_MAX_CHN_PORT	200
static RtspStreamTable_t g_RtspStreamTable={0,};


static int RTSP_request_describe(Rtsp_t *r);


//#if defined(_WIN32) || defined(_WIN64)
char *STRCASESTR(char *s1,char *s2)
{
    char *ptr = s1;
	
    if (!s1 || !s2 || !*s2) return s1;
	
    while (*ptr) {
		if (toupper(*ptr) == toupper(*s2)) {
			char * cur1 = ptr + 1;
			char * cur2 = s2 + 1;
			while (*cur1 && *cur2 && toupper(*cur1) == toupper(*cur2)) {
				cur1++;
				cur2++;
			}
			if (!*cur2) {
				return ptr;
			}
		}
		ptr++;
    }
    return NULL;
}
//#else
//#define STRCASESTR strcasestr
//#endif


static inline int http_get_number(char *src,
	char *key,int *ret)
{
	char *tmp=STRCASESTR(src,key);
	if(tmp == NULL){
		*ret=-1;
		return -1;
	}else{
		tmp+=strlen(key);
		if((*tmp) == ' ') tmp++;
		sscanf(tmp,"%d",ret);
		return 0;
	}
}

static inline int http_get_string(char *src,
	char *key,char *ret)
{
	char *tmp=STRCASESTR(src,key);
	if(tmp == NULL){
		*ret=0;
		return -1;
	}else{
		tmp+=strlen(key);
		if(*tmp == ' ') tmp++;
		sscanf(tmp,"%[^\r\n]",ret);
		return 0;
	}
}

/*
	if src contain full-url , and parse ok, return 0;
	else if url is "*"; return -2;
	else return -1
*/
static inline int http_get_url(const char *src,char *ip,int *port,char *stream)
{
	int i_ip[4];
	char tmp[128],*ptr=tmp;
	char *p=NULL;
	if(strncmp(src,"rtsp://",strlen("rtsp://"))!=0){
		if(sscanf(src,"%*s %s %*s",tmp)!=1){
			VLOG(VLOG_ERROR,"request url format wrong");
			return -1;
		}
		if (strcmp(tmp, "*") == 0) {
			return -2;
		} else if(strncmp(tmp,"rtsp://",strlen("rtsp://")) != 0){
			VLOG(VLOG_ERROR,"request url format wrong");
			return -1;
		}
		ptr=tmp;
	}else{
		ptr=(char *)src;
	}
	if((p = strstr(ptr+strlen("rtsp://"),":")) ==NULL){
		*port = RTSP_DEFAULT_PORT;
		sscanf(ptr,"rtsp://%[^/]/%s",ip,stream);
	}else{
		sscanf(ptr,"rtsp://%[^:]:%d/%s",ip,port,stream);
	}
	if(sscanf(ip,"%d.%d.%d.%d",&i_ip[0],&i_ip[1],&i_ip[2],&i_ip[3])!=4){
		SOCK_gethostbyname(ip,tmp);
		strcpy(ip,tmp);
	}
	VLOG(VLOG_DEBUG,"ip:%s stream:%s port:%d",ip,stream,*port);
	return 0;
}

static uint32_t hash_string(char *str)
{
#define HASHWORDBITS	(32)
	uint32_t hval=0xFFFFFFFF,g;
	char *pstr=str;
	while(*str){
		hval <<=4;
		hval += (unsigned char)*str++;
		g = hval & ((unsigned int)0xf << (HASHWORDBITS -4));
		if(g != 0){
			hval ^= g >> (HASHWORDBITS - 8);
			hval ^= g;
		}
	}

	VLOG(VLOG_DEBUG,"string:%s hashval:%x",pstr,hval);
	return hval;
}


static int rtsp_check_setup_url(Rtsp_t *r)
{
	VLOG(VLOG_DEBUG,"setup steam:%s",r->stream);
	return RTSP_RET_OK;
}

static int rtsp_parse_transport(Rtsp_t *r,char *buf)
{
	char *p,*q;
	char transport[128];
	p=transport;
	*buf=0;
	if(http_get_string(r->payload,"Transport:",transport) == RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	}
	if((q=strstr(transport,"RTP/AVP/TCP")) != NULL){
		r->low_transport = RTP_TRANSPORT_TCP;
		strcat(buf,"RTP/AVP/TCP");
	}else{
		strcat(buf,"RTP/AVP/UDP");
		r->low_transport = RTP_TRANSPORT_UDP;
	}
	if((q=strstr(p,"multicast"))!=NULL){
		r->cast_type = RTP_MULTICAST;
		strcat(buf,";multicast");
	}else{
		r->cast_type = RTP_UNICAST;
		strcat(buf,";unicast");
	}
	if((q=strstr(p,"interleaved"))!=NULL){
		r->b_interleavedMode = TRUE;
		q+=strlen("interleaved=");
		sscanf(q,"%d%*s",&r->channel);
		q=buf+strlen(buf);
		sprintf(q,";interleaved=%d-%d",r->channel,r->channel + 1);
		r->client_port = r->channel;
		r->server_port = r->channel;
	}else{
		unsigned int port_tmp;
		r->b_interleavedMode = FALSE;
		if (r->role == RTSP_SERVER) {
#if	defined(RTCP_SENDER) || defined(RTCP_RECEIVER)
			PORT_MANAGE_apply2_port3(&port_tmp);
#else
			PORT_MANAGE_apply1_port3(&port_tmp);
#endif
			r->server_port = port_tmp;
		}
		if((q=strstr(p,"client_port"))!=NULL){
			q+=strlen("client_port=");
			printf("q:%s\n",q);
			sscanf(q,"%d%*s",&r->client_port);
		}
		if((q=strstr(p,"server_port"))!=NULL){
			q+=strlen("server_port=");
			sscanf(q,"%d%*s",&r->server_port);
		}
		q=buf+strlen(buf);
		sprintf(q,";client_port=%d-%d;server_port=%d-%d",
			r->client_port,r->client_port+ 1,r->server_port,r->server_port+1);
	}
	if(r->role == RTSP_SERVER){
		q=buf+strlen(buf);
		sprintf(q,";ssrc=%x",hash_string(r->stream));
	}else{
		if((q=strstr(p,"ssrc="))!=NULL){
			q+=strlen("ssrc=");
			sscanf(q,"%x%*s",&r->ssrc);
		}else{
			r->ssrc = 0;
		}
	}
	if((q=strstr(p,"mode"))!=NULL){
		q+=strlen("mode=\"");
		if(strcmp(q,"PLAY") == 0){
			r->work_mode =RTSP_MODE_PLAY;
			strcat(buf,";mode=\"PLAY\"");
		}else if(strcmp(q,"RECORD") == 0){
			r->work_mode = RTSP_MODE_RECORD;
			strcat(buf,";mode=\"RECORD\"");
		}
	}
	//ssrc
	// ttl
	//....
	VLOG(VLOG_DEBUG,"parse transport:client_port:%d server_port:%d,%s",
	r->client_port,r->server_port,r->cast_type ? "multicast" : "unicast");
	VLOG(VLOG_DEBUG,"Transport: %s",buf);
	return RTSP_RET_OK;
}

inline int rtsp_setup_transport(Rtsp_t *r,char *buf)
{
	char *ptr=buf;
	if(r->low_transport == RTP_TRANSPORT_TCP)
		sprintf(ptr,"RTP/AVP/TCP");
	else
		sprintf(ptr,"RTP/AVP");
	ptr = buf + strlen(buf);
	if(r->cast_type == RTP_MULTICAST)
		sprintf(ptr,";multicast");
	else
		sprintf(ptr,";unicast");
	ptr = buf + strlen(buf);
	if(r->b_interleavedMode == true)
		sprintf(ptr,";interleaved=%d-%d",r->channel,r->channel+1);
	else
		sprintf(ptr,";client_port=%d-%d",r->client_port,r->client_port+1);

	return RTSP_RET_OK;
}

static void rtsp_gmt_time_string(char *ret,int size)
{
	time_t t;
	time(&t);
	strftime(ret, size, "%a, %b %d %Y %H:%M:%S GMT", localtime(&t));
}

/**************************************************************
* nat detect
***************************************************************/
int rtsp_request_nat_detect(Rtp_t *r)
{
	int ret;
	char buf[1024];
	int port;
	const char *format="NatDetect * RTSP/1.0\r\n";
	if(r->interleaved == true){
		VLOG(VLOG_DEBUG,"rtp over rtsp,no need to do nat detect");
		return RTSP_RET_OK;
	}
	ret=SOCK_sendto(r->sock,r->peername,r->peer_chn_port,(char *)format,strlen(format));
	if(ret == RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	}
	ret=SOCK_recvfrom(r->sock,r->peername,&port,buf,sizeof(buf),0);
	if(ret == RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	}
	buf[ret]=0;
	VLOG(VLOG_DEBUG,"nat detect response:%s",buf);
	if(memcmp(buf,format,strlen(format))==0){
		VLOG(VLOG_DEBUG,"Nat detect success");
	}else{
		VLOG(VLOG_ERROR,"Nat detect failed");
		return RTSP_RET_FAIL;
	}

	return RTSP_RET_OK;
}

int rtsp_parse_nat_detect(Rtp_t *r)
{
	int ret;
	char buf[1024];
	fd_set read_set;
	int port;
	struct timeval timeout;
	if(r->interleaved == true){
		VLOG(VLOG_DEBUG,"rtp over rtsp,no need to do nat detect");
		return RTSP_RET_OK;
	}
	timeout.tv_sec=1;
	timeout.tv_usec = 500*1000;
	FD_ZERO(&read_set);
	FD_SET(r->sock,&read_set);
	ret=select(r->sock+1,&read_set,NULL,NULL,&timeout);
	if(ret <= RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	}
	ret=SOCK_recvfrom(r->sock,r->peername,&port,buf,sizeof(buf),0);
	if(ret == RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	}
	buf[ret]=0;
	VLOG(VLOG_DEBUG,"nat detect response:%s",buf);
	// send back
	ret=SOCK_sendto(r->sock,r->peername,r->peer_chn_port,buf,ret);
	if(ret == RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	}
	
	return RTSP_RET_OK;
}

static void fRTSP_DEFAULT_EVENT_HOOK(int eventType, int lParam, void *rParam,
	void *customCtx)
{

	switch (eventType)
	{
		case RTSP_EVENT_CONNECTED:
		case RTSP_EVENT_DISCONNECTED:
		case RTSP_EVENT_PLAY:
		case RTSP_EVENT_RECORD:
		case RTSP_EVENT_PAUSE:
		case RTSP_EVENT_DATA_RECEIVED:
		case RTSP_EVENT_AUTH_REQUIRE:
		case RTSP_EVENT_AUTH_FAILED:
		case RTSP_EVENT_RTCP_SENDER_REPORT:
		case RTSP_EVENT_RTCP_RECEIVER_REPORT:
		case RTSP_EVENT_CHECK_ALIVE_FAILED:
			printf("RTSP Got EVENT[%d] param(%p)\n", eventType, customCtx);
			break;
		default:
			printf("RTSP Got INVALID EVENT[%d] param(%p)\n", eventType, customCtx);
			break;
	}
}


static int rtsp_send_packet(Rtsp_t *r)
{
	fd_set wset;
	int ret;
	struct timeval timeo;
	int remind = r->payload_size;
	char *ptr = r->payload;
	
	while(remind > 0){
		timeo.tv_sec = 3;
		timeo.tv_usec = 0;
		FD_ZERO(&wset);
		FD_SET(r->sock, &wset);
		ret = select(r->sock + 1, NULL, &wset, NULL, &timeo);
		if(ret > 0){
			if(FD_ISSET(r->sock, &wset)){
				ret=send(r->sock, ptr, remind, 0);
				if(ret < 0){
					if(SOCK_ERR == SOCK_EINTR){
						VLOG(VLOG_WARNING, "RTSP SEND errno:%d retry!\n", SOCK_ERR);
						continue;
					}else{
						goto ERR_EXIT;
					}
				}else{
					ptr += ret;
					remind -= ret;
				}
				
			}else{
				goto ERR_EXIT;
			}
		}else{
			goto ERR_EXIT;
		}
	}
	
	VLOG(VLOG_DEBUG,"rtsp send packet(size:%d) ok",r->payload_size);
	return RTSP_RET_OK;

ERR_EXIT:
	VLOG(VLOG_ERROR,"rtsp send packet(size:%d) failed.",r->payload_size);
	return RTSP_RET_FAIL;
}

static int rtsp_handle_notallowed_method(Rtsp_t *r)
{
	int ret;
	const char *format=
		"%s %d %s\r\n"\
		"CSeq: %d\r\n"\
		"Allow: %s\r\n"
		"\r\n";
	sprintf(r->payload,format,RTSP_VERSION,
		rtspRStatusCodes[RTSP_RSC_METHOD_NOT_ALLOWED].code,
		rtspRStatusCodes[RTSP_RSC_METHOD_NOT_ALLOWED].info,
		r->cseq,
		RTSP_ALLOW_METHODS);
	r->payload_size = strlen(r->payload);
	
	ret=rtsp_send_packet(r);
	return ret;
}

static int rtsp_response_error(Rtsp_t *r,int err_no)
{
	int ret;
	const char *format=
		"%s %d %s\r\n"\
		"\r\n";
	sprintf(r->payload,format,RTSP_VERSION,
		rtspRStatusCodes[err_no].code,
		rtspRStatusCodes[err_no].info);
	r->payload_size = strlen(r->payload);
	
	ret=rtsp_send_packet(r);
	return RTSP_RET_FAIL;//make it response failed alaways
}

static int rtsp_response_unauthorized(Rtsp_t *r)
{
	int ret;
	char www_auth[512];
	const char *format1=
		"%s %d %s\r\n"\
		"CSeq: %d\r\n"\
		"WWW-Authenticate: %s\r\n"\
		"Server: %s\r\n"\
		"\r\n";

	// create realm and nonce data
	if(HTTP_AUTH_chanllenge(r->auth,www_auth,sizeof(www_auth))==AUTH_RET_FAIL){
		return RTSP_RET_FAIL;
	}
		
	sprintf(r->payload,format1,RTSP_VERSION,
		rtspRStatusCodes[RTSP_RSC_UNAUTHORIZED].code,
		rtspRStatusCodes[RTSP_RSC_UNAUTHORIZED].info,
		r->cseq,
		www_auth, RTSP_USER_AGENT);
	r->payload_size = strlen(r->payload);
	
	ret=rtsp_send_packet(r);
	return ret;
}


static int rtsp_parse_response(Rtsp_t *r,int *code,char *info)
{
	int cseq;
	char tmp[256],*ptr=NULL;
	if(memcmp(r->payload,RTSP_VERSION,strlen(RTSP_VERSION)) == 0){
		sscanf(r->payload,"%*s %d %s",code,info);
		VLOG(VLOG_DEBUG,"response code:%d info:%s",*code,info);
		if((*code) == rtspRStatusCodes[RTSP_RSC_UNAUTHORIZED].code){
			//AUTH_init(&r->auth,HTTP_AUTH_BASIC);
			r->bLogin = false;
		}else if((*code) != rtspRStatusCodes[RTSP_RSC_OK].code){
			VLOG(VLOG_ERROR,"response code not ok,%d",*code);
			return RTSP_RET_FAIL;
		}
	}else{
		VLOG(VLOG_ERROR,"invalid rtsp response:\n%s",r->payload);
		return RTSP_RET_FAIL;
	}
	// check cseq
	if(http_get_number(r->payload,"CSeq:",&cseq)==RTSP_RET_FAIL){
		VLOG(VLOG_ERROR,"invaild request format,not found cseq");
		return RTSP_RET_FAIL;
	}else{
		if(cseq != r->cseq){
			VLOG(VLOG_ERROR,"cseq number is wrong");
			return RTSP_RET_FAIL;
		}
	}
	if(http_get_string(r->payload,"Public:",tmp)==RTSP_RET_OK){
		strcpy(r->allow_method,tmp);
	}
	if(http_get_string(r->payload, "Server:", tmp) == RTSP_RET_OK){
		strncpy(r->agent, tmp, sizeof(r->agent));
	}
	if(http_get_string(r->payload, "User-Agent:", tmp) == RTSP_RET_OK){
		strncpy(r->agent, tmp, sizeof(r->agent));
	}
	if(http_get_string(r->payload,"Content-type:",tmp)==RTSP_RET_OK){
		if(strcmp(tmp,SDP_MEDIA_TYPE)==0){
			// parse sdp here....
			VLOG(VLOG_DEBUG,"Get SDP, Goto parse it...");
			ptr=strstr(r->payload,"\r\n\r\n");
			if(ptr == NULL){
				VLOG(VLOG_ERROR,"decribe response format err,check it");
				return RTSP_RET_FAIL;
			}
			ptr+=strlen("\r\n\r\n");
			r->sdp = SDP_decode(ptr);
			if(r->sdp==NULL){
				return RTSP_RET_FAIL;
			}
		}
	}
	if(http_get_string(r->payload,"Session:",tmp)==RTSP_RET_OK){
		if(strstr(tmp,";")!=NULL)
			sscanf(tmp,"%[^;];timeout=%d",r->session_id,&r->session_timeout);
		else
			strcpy(r->session_id,tmp);
		VLOG(VLOG_DEBUG,"\tsessin id:%s",r->session_id);
	}
	if(http_get_string(r->payload,"Transport:",tmp)==RTSP_RET_OK){
		rtsp_parse_transport(r,tmp);
	}
	if(http_get_string(r->payload,"RTP-Info:",tmp)==RTSP_RET_OK){
		if((ptr=strstr(tmp,"seq=")) != NULL){
			ptr += strlen("seq=");
			sscanf(ptr, "%u", &r->rtpseq);
		}else
			r->rtpseq = 0;
		if((ptr=strstr(tmp,"rtptime=")) != NULL){
			ptr += strlen("rtptime=");
			sscanf(ptr, "%u", &r->rtptime);
		}else
			r->rtptime= 0;
		
		VLOG(VLOG_DEBUG,"\trtp seq: %u rtptime:%u", r->rtpseq, r->rtptime);
	}
	// auth
	if(http_get_string(r->payload,"WWW-Authenticate:",tmp)==RTSP_RET_OK){
		if(HTTP_AUTH_client_init(&r->auth,tmp)==AUTH_RET_FAIL){
			return RTSP_RET_FAIL;
		}
		//return RTSP_request_describe(r);
	}
		
	VLOG(VLOG_DEBUG,"parse response success");
	return RTSP_RET_OK;
}

static int rtsp_handle_describe(Rtsp_t *r)
{
	int ret;
	char tmp[128];
	const char *format=
		"%s %d %s\r\n"\
		"CSeq: %d\r\n"\
		"Content-Type: %s\r\n"\
		"Content-Length: %d\r\n"\
		"Server: %s\r\n"\
		"\r\n"\
		"%s";
	
#if RTSP_ENABLE_AUTHTICATION
	char szAuth[512];
	char *ptr;
	if(http_get_string(r->payload,"Authorization:",szAuth)==RTSP_RET_FAIL){
		if(rtsp_response_unauthorized(r)==RTSP_RET_FAIL)
			return RTSP_RET_FAIL;
		return RTSP_RET_OK;
	}else{
		if(http_get_string(r->payload,"Authorization:",szAuth)==RTSP_RET_OK){
			r->bLogin = HTTP_AUTH_validate(r->auth,szAuth,"DESCRIBE");
		}
		if(r->bLogin != true){
			VLOG(VLOG_ERROR,"authorized failed");
			rtsp_response_unauthorized(r);
			return RTSP_RET_FAIL;
		}
	}
#endif

	if(http_get_string(r->payload,"Accept:",tmp)==RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	}
	if(strstr(tmp,SDP_MEDIA_TYPE) == NULL){
		VLOG(VLOG_ERROR,"unsupport Accept type:%s",tmp);
		// send error ack here...
		return RTSP_RET_FAIL;
	}
	if(r->sdp == NULL){
		if((r->sdp=SDP_new_default(r->stream,r->ip_me))==NULL){
			return RTSP_RET_FAIL;
		}
		if(r->stream_type & RTSP_STREAM_AUDIO){
			SDP_add_g711(r->sdp,SDP_DEFAULT_AUDIO_CONTROL);
		}
		if(r->stream_type & RTSP_STREAM_VIDEO){
			SDP_add_h264(r->sdp,SDP_DEFAULT_VIDEO_CONTROL);
		}

		if (r->get_avc) {
			H264AVC_t avc;
			if (r->get_avc(r->stream, (void *)&avc) == 0) {
				if (avc.with_startcode)
					SDP_add_sps_pps(r->sdp, avc.sps + 4, avc.sps_size - 4, avc.pps + 4, avc.pps_size -4);
				else
					SDP_add_sps_pps(r->sdp, avc.sps, avc.sps_size, avc.pps, avc.pps_size);
			}
		}
	}
	SDP_encode(r->sdp);
	sprintf(r->payload,format,RTSP_VERSION,
		rtspRStatusCodes[RTSP_RSC_OK].code,
		rtspRStatusCodes[RTSP_RSC_OK].info,
		r->cseq,SDP_MEDIA_TYPE,
		strlen(r->sdp->buffer),
		RTSP_USER_AGENT,
		r->sdp->buffer);
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"ack (size:%d):\r\n%s\r\n",r->payload_size,r->payload);

	VLOG(VLOG_CRIT,"rtsp get request from:%s:%d/%s",r->ip_me,r->port,r->stream);
	
	ret=rtsp_send_packet(r);
	return ret;
}

static int rtsp_handle_announce(Rtsp_t *r)
{
	VLOG(VLOG_DEBUG,"not yet");
	return RTSP_RET_OK;
}

static int rtsp_handle_options(Rtsp_t *r)
{
	int ret;
	const char *format=
		"%s %d %s\r\n"\
		"CSeq: %d\r\n"\
		"Public: %s\r\n"\
		"Server: %s\r\n"\
		"\r\n";
	sprintf(r->payload,format,RTSP_VERSION,
		rtspRStatusCodes[RTSP_RSC_OK].code,
		rtspRStatusCodes[RTSP_RSC_OK].info,
		r->cseq,
		RTSP_ALLOW_METHODS,
		RTSP_USER_AGENT);
	r->payload_size = strlen(r->payload);
	VLOG(VLOG_DEBUG,"ack (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	
	ret=rtsp_send_packet(r);
	return ret;
}

static int rtsp_handle_get_parameter(Rtsp_t *r)
{
	int ret;
	int content_length = 0;
	const char *format=
		"%s %d %s\r\n"\
		"CSeq: %d\r\n"\
		"Server: %s\r\n"\
		"\r\n";

	ret = http_get_number(r->payload, "Content-Type:", &content_length);
	if(content_length > 0){
		rtsp_response_error(r, RTSP_RSC_NOT_IMPLEMENTED);
		return RTSP_RET_OK;
	}
	
	sprintf(r->payload,format,RTSP_VERSION,
		rtspRStatusCodes[RTSP_RSC_OK].code,
		rtspRStatusCodes[RTSP_RSC_OK].info,
		r->cseq, RTSP_USER_AGENT);
	r->payload_size = strlen(r->payload);
	VLOG(VLOG_DEBUG,"ack (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	
	ret=rtsp_send_packet(r);
	return ret;
}

static int rtsp_handle_play(Rtsp_t *r)
{
	int ret;
	char tmp[128];
	const char *format=
		"%s %d %s\r\n"\
		"CSeq: %d\r\n"\
		"Date: %s\r\n"\
		"Server: %s\r\n"\
		"\r\n";
	rtsp_gmt_time_string(tmp,sizeof(tmp));
	sprintf(r->payload,format,RTSP_VERSION,
		rtspRStatusCodes[RTSP_RSC_OK].code,
		rtspRStatusCodes[RTSP_RSC_OK].info,
		r->cseq,
		tmp, RTSP_USER_AGENT);
	r->payload_size = strlen(r->payload);
	VLOG(VLOG_DEBUG,"ack (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	
	ret=rtsp_send_packet(r);
	return ret;
}

static int rtsp_handle_pause(Rtsp_t *r)
{
	VLOG(VLOG_DEBUG,"not yet");
	return RTSP_RET_OK;
}

static int rtsp_handle_record(Rtsp_t *r)
{
	VLOG(VLOG_DEBUG,"not yet");
	return RTSP_RET_OK;
}

static int rtsp_handle_redirect(Rtsp_t *r)
{
	VLOG(VLOG_DEBUG,"not yet");
	return RTSP_RET_OK;
}

static int rtsp_handle_setup(Rtsp_t *r)
{
	int ret;
	int rtp_chn_port=0,rtcp_chn_port=0;
	uint32_t ssrc;
	char sockname[20];
	Rtp_t **rtp=NULL;
	Rtcp_t **rtcp=NULL;
	int rtp_sock;
	char tmp[256];
	const char *format=
		"%s %d %s\r\n"\
		"CSeq: %d\r\n"\
		"Session: %s\r\n"\
		"Transport: %s\r\n"\
		"Server: %s\r\n"\
		"\r\n";
	
	int b_video=strstr(r->payload,SDP_DEFAULT_VIDEO_CONTROL) ? TRUE : FALSE;
	int payload_type = (b_video == TRUE) ? RTP_DEFAULT_VIDEO_TYPE : RTP_DEFAULT_AUDIO_TYPE;
	int protocal;
	if(b_video){
		rtp = (Rtp_t **)&r->rtp_video;
		rtcp = (Rtcp_t **)&r->rtcp_video;
	}else{
		rtp = (Rtp_t **)&r->rtp_audio;
		rtcp = (Rtcp_t **)&r->rtcp_audio;
	}
	
	ssrc = hash_string(r->stream);
	
	if(rtsp_check_setup_url(r) == RTSP_RET_FAIL){
		VLOG(VLOG_ERROR,"session not found");
		// send err ack here
		return RTSP_RET_FAIL;
	}

	if(rtsp_parse_transport(r,tmp)== RTSP_RET_FAIL){
		// send error ack here...
		return RTSP_RET_FAIL;
	}
	protocal = r->b_interleavedMode ? RTP_TRANSPORT_TCP : RTP_TRANSPORT_UDP;
	
	sprintf(r->payload,format,RTSP_VERSION,
		rtspRStatusCodes[RTSP_RSC_OK].code,
		rtspRStatusCodes[RTSP_RSC_OK].info,
		r->cseq,
		r->session_id,
		tmp,
		RTSP_USER_AGENT);
	r->payload_size = strlen(r->payload);
	VLOG(VLOG_DEBUG,"ack (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	
	ret=rtsp_send_packet(r);

	//init rtp 
	VLOG(VLOG_DEBUG,"%s setup: port:%d",(b_video==TRUE) ? "video" : "audio",r->client_port);
	if(r->b_interleavedMode){
		if((r->channel % 2) == 0){
			rtp_chn_port = r->channel;
			rtcp_chn_port = r->channel + 1;
		}else{
			rtcp_chn_port = r->channel;
			rtp_chn_port = r->channel + 1;
		}
		rtp_sock = r->sock;
	}else{
		if((r->client_port % 2) == 0){
			rtp_chn_port = r->client_port;
			rtcp_chn_port = r->client_port + 1;
		}else{
			rtcp_chn_port = r->client_port;
			rtp_chn_port = r->client_port + 1;
		}
		
		rtp_sock = SOCK_udp_init(NULL, r->server_port,RTSP_SOCK_TIMEOUT);
		if(rtp_sock == -1) return RTSP_RET_FAIL;
	}
	*rtp = RTP_server_new(ssrc,payload_type,protocal,r->b_interleavedMode,rtp_sock,r->peername,rtp_chn_port);
	if(*rtp == NULL) return RTSP_RET_FAIL;

	//init rtcp 
	ret=RTCP_init(rtcp,r->role,ssrc,r->low_transport,r->cast_type,
		r->b_interleavedMode,r->sock,r->server_port + 1,rtcp_chn_port,*rtp);
	if(ret == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// do nat detect
	SOCK_getsockname(r->sock,sockname);
	if(SOCK_isreservedip(sockname)==true && SOCK_isreservedip((*rtp)->peername)!=true && r->b_interleavedMode==false)
		rtsp_parse_nat_detect(*rtp);
	//
	return ret;
}

static int rtsp_handle_set_parameter(Rtsp_t *r)
{
	VLOG(VLOG_DEBUG,"not yet");
	return RTSP_RET_OK;
}
static int rtsp_handle_teardown(Rtsp_t *r)
{
	int ret;
	const char *format=
		"%s %d %s\r\n"\
		"CSeq: %d\r\n"\
		"Server: %s\r\n"\
		"\r\n";
	sprintf(r->payload,format,RTSP_VERSION,
		rtspRStatusCodes[RTSP_RSC_OK].code,
		rtspRStatusCodes[RTSP_RSC_OK].info,
		r->cseq, RTSP_USER_AGENT);
	r->payload_size = strlen(r->payload);
	VLOG(VLOG_DEBUG,"ack (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	
	ret=rtsp_send_packet(r);
	VLOG(VLOG_CRIT,"Recv method: TEARDOWN");
	r->toggle = false;
	//return ret;
	return RTSP_RET_OK;// to teardown this session
}


static int RTSP_request_options(Rtsp_t *r)
{
	int status_code;
	char tmp[256];
	const char format[]=
		"OPTIONS %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"\r\n";
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	sprintf(r->payload,format,tmp,RTSP_VERSION,
		++r->cseq,
		RTSP_USER_AGENT);
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// get response
	if(RTSP_read_message(r)==RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// parse response
	if(rtsp_parse_response(r,&status_code,tmp)==RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	
	return RTSP_RET_OK;
}

static int RTSP_request_describe(Rtsp_t *r)
{
	int status_code;
	char tmp[256];
	char url[200];
	char szAuth[512];
	int auth_flag=false;
	const char format1[]=
		"DESCRIBE %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Accept: %s\r\n"\
		"\r\n";
	const char format2[]=
		"DESCRIBE %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Accept: %s\r\n"\
		"Authorization: %s\r\n"\
		"\r\n";
	
	__TRY_AGIN:
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	if(r->bLogin == true){
		sprintf(r->payload,format1,tmp,RTSP_VERSION,
			++r->cseq,
			RTSP_USER_AGENT,
			SDP_MEDIA_TYPE);
	}else{
		sprintf(url,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
		if(HTTP_AUTH_setup(r->auth,r->user,r->pwd,url,"DESCRIBE",szAuth,sizeof(szAuth))==AUTH_RET_FAIL){
			return RTSP_RET_FAIL;
		}
		sprintf(r->payload,format2,tmp,RTSP_VERSION,
			++r->cseq,
			RTSP_USER_AGENT,
			SDP_MEDIA_TYPE,
			szAuth);
	}
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// get response
	if(RTSP_read_message(r)==RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// parse response
	if(rtsp_parse_response(r,&status_code,tmp)==RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	if((status_code == rtspRStatusCodes[RTSP_RSC_UNAUTHORIZED].code) && (auth_flag == false)){
		VLOG(VLOG_CRIT,"REQUIRE AUTH!!!!");
		auth_flag = true;
		if (r->eventHook) {
			char usernamepwd[128] = {0,}; // formated with : "username:userpwd"
			r->eventHook(RTSP_EVENT_AUTH_REQUIRE, sizeof(usernamepwd), (void *)usernamepwd, r->eventCustomCtx);
			if (strlen(usernamepwd) > 0) {
				sscanf(usernamepwd, "%[^:]:%s", r->user, r->pwd);
				VLOG(VLOG_CRIT,"got user info(%s:%s)", r->user, r->pwd);
			}
		}
		goto __TRY_AGIN;
	}else if(status_code == rtspRStatusCodes[RTSP_RSC_UNAUTHORIZED].code){
		VLOG(VLOG_ERROR,"AUTH FAILED!!!!");
		if (r->eventHook) {
			r->eventHook(RTSP_EVENT_AUTH_FAILED, 0, NULL, r->eventCustomCtx);
		}
		return RTSP_RET_FAIL;
	}
	// parse sdp here...
	//
	return RTSP_RET_OK;
}

static int RTSP_request_annouce(Rtsp_t *r)
{
	return RTSP_RET_OK;
}

static int RTSP_request_get_parameter(Rtsp_t *r, char *content)
{
	int status_code;
	char tmp[256];
	const char format[]=
		"GET_PARAMETER %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Content-Type: text/parameters\r\n"\
		"Session: %s\r\n"\
		"Content-Length: %d\r\n"\
		"\r\n"
		"%s";
	const char format2[]=
		"GET_PARAMETER %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Session: %s\r\n"\
		"\r\n";
		
	if(strstr(r->allow_method, "GET_PARAMETER") == NULL){
		VLOG(VLOG_CRIT, "%s not suport GET_PARAMETER!", r->ip_me);
		return RTSP_RET_OK;
	}else{
		VLOG(VLOG_CRIT, "%s suport GET_PARAMETER!", r->ip_me);
	}
	
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	if(content == NULL){
		sprintf(r->payload,format2,tmp,RTSP_VERSION,
			++r->cseq,
			RTSP_USER_AGENT,
			r->session_id);

	}else{
		sprintf(r->payload,format,tmp,RTSP_VERSION,
			++r->cseq,
			RTSP_USER_AGENT,
			r->session_id,
			strlen(content),
			content);
	}
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// get response
	if(RTSP_read_message(r)==RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// parse response
	if(rtsp_parse_response(r,&status_code,tmp)==RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	//
	VLOG(VLOG_CRIT, "%s GET_PARAMETER response %d!", r->ip_me, status_code);
	return RTSP_RET_OK;
}

static int RTSP_request_set_parameter(Rtsp_t *r)
{
	return RTSP_RET_OK;
}

int RTSP_request_play(Rtsp_t *r)
{
	char tmp[256];
	//char stream_url[128];
	const char format[]=
		"PLAY %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Session: %s\r\n"\
		"Range: npt=0.000-\r\n"\
		"\r\n";

	if (r->state != RTSP_STATE_READY){
		VLOG(VLOG_DEBUG,"request play, but not in ready!");
		return -1;
	}
	/*
	strncpy(stream_url, r->stream, sizeof(stream_url) -1);
	if ((ptr = strstr(stream_url, "?")) != NULL) {
		*ptr = '\0';
	}
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,stream_url);
	*/
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	sprintf(r->payload,format,tmp,RTSP_VERSION,
		++r->cseq,
		RTSP_USER_AGENT,
		r->session_id);
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// get response
	//if(RTSP_read_message(r)==RTSP_RET_FAIL)
	//	return RTSP_RET_FAIL;
	// parse response
	//if(rtsp_parse_response(r,&status_code,tmp)==RTSP_RET_FAIL)
	//	return RTSP_RET_FAIL;
	//
	r->state = RTSP_STATE_PLAYING;
	//
	return RTSP_RET_OK;
}

int RTSP_request_pause(Rtsp_t *r)
{
	char tmp[256];
	const char format[]=
		"PAUSE %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Session: %s\r\n"\
		"\r\n";

	if (!((r->state == RTSP_STATE_PLAYING) || (r->state == RTSP_STATE_RECORDING))){
		VLOG(VLOG_DEBUG,"request pause, but not in playing/recording!");
		return -1;
	}

	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	sprintf(r->payload,format,tmp,RTSP_VERSION,
		++r->cseq,
		RTSP_USER_AGENT,
		r->session_id);
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// get response
	//if(RTSP_read_message(r)==RTSP_RET_FAIL)
	//	return RTSP_RET_FAIL;
	// parse response
	//if(rtsp_parse_response(r,&status_code,tmp)==RTSP_RET_FAIL)
	//	return RTSP_RET_FAIL;
	//
	r->state = RTSP_STATE_READY;

	return RTSP_RET_OK;
}

static int RTSP_request_record(Rtsp_t *r)
{
	return RTSP_RET_OK;
}
static int RTSP_request_redirect(Rtsp_t *r)
{
	return RTSP_RET_OK;
}

/*
	media_type: "video" , "audio"
	type : rtp payload type
	real type : detail media type, if type not dynamic type , real type is equal to type
*/
static int RTSP_request_setup(Rtsp_t *r,char *control, char *media_type,int type, int real_type)
{
	int ret;
	int status_code;
	int b_video;
	unsigned int chn_port_tmp = 0;
	char sockname[20];
	Rtp_t **rtp=NULL;
	Rtcp_t **rtcp = NULL;
	int rtp_chn_port=0,rtcp_chn_port = 0;
	char tmp[256],tmp2[256];
	char stream_url[128];
	char *ptr = NULL;
	int rtp_sock;
	const char format[]=
		"SETUP %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Transport: %s\r\n"\
		"\r\n";
	const char format2[]=
		"SETUP %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Transport: %s\r\n"\
		"Session: %s\r\n"\
		"\r\n";
	
	if(r->transport == RTP_AUTO){
#if RTSP_RTP_DEF_TRANSPORT == RTSP_RTP_OVER_RTSP
		r->b_interleavedMode = RTSP_RTP_OVER_RTSP;
		r->low_transport = RTP_TRANSPORT_TCP;
#else
		r->b_interleavedMode = RTSP_RTP_OVER_UDP;
		r->low_transport = RTP_TRANSPORT_UDP;
#endif
	} else if (r->transport == RTP_UDP) {
		r->b_interleavedMode = RTSP_RTP_OVER_UDP;
		r->low_transport = RTP_TRANSPORT_UDP;
	} else {
		r->b_interleavedMode = RTSP_RTP_OVER_RTSP;
		r->low_transport = RTP_TRANSPORT_TCP;
	}

	strncpy(stream_url, r->stream, sizeof(stream_url) -1);
	if ((ptr = strstr(stream_url, "?")) != NULL) {
		*ptr = '\0';
	}
	
TRY_INTERLEAVED_MODE:
	if(memcmp(control,"rtsp://",strlen("rtsp://"))==0){
		sprintf(tmp,"%s",control);
	}else{
		sprintf(tmp,"rtsp://%s:%d/%s/%s",r->ip_me,r->port,stream_url,control);
	}
	if(r->b_interleavedMode == true){
		if (0 == strcmp(media_type, "audio")) {
			r->channel = 2;
		}else{
			r->channel = 0;
		}
		r->client_port = r->channel;
		r->server_port = r->channel;
	}else{
		PORT_MANAGE_apply2_port3(&chn_port_tmp);
		r->client_port = chn_port_tmp;
	}
	rtsp_setup_transport(r,tmp2);
	if(r->session_id[0] == 0){
		sprintf(r->payload,format,tmp,RTSP_VERSION,
			++r->cseq,
			RTSP_USER_AGENT,
			tmp2);
	}else{
		sprintf(r->payload,format2,tmp,RTSP_VERSION,
			++r->cseq,
			RTSP_USER_AGENT,
			tmp2,
			r->session_id);
	}
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// get response
	if(RTSP_read_message(r)==RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// parse response
	if(rtsp_parse_response(r,&status_code,tmp)==RTSP_RET_FAIL){
		if((status_code == RTSP_RSC_UNSUPPORTED_TRANSPORT) && 
			(r->transport == RTSP_RTP_AUTO) &&
			(r->b_interleavedMode == RTSP_RTP_OVER_UDP)){
#if RTSP_RTP_DEF_TRANSPORT == RTSP_RTP_OVER_RTSP
			if (r->b_interleavedMode == RTSP_RTP_OVER_RTSP) {
				r->b_interleavedMode = RTSP_RTP_OVER_UDP;			
				r->low_transport = RTP_TRANSPORT_UDP;
				goto TRY_INTERLEAVED_MODE;
			}		
#else		
			if (r->b_interleavedMode == RTSP_RTP_OVER_UDP) {
				r->b_interleavedMode = RTSP_RTP_OVER_RTSP;			
				r->low_transport = RTP_TRANSPORT_TCP;
				goto TRY_INTERLEAVED_MODE;
			}
#endif
		}
		return RTSP_RET_FAIL;
	}
	// setup network
	//init rtp 
	
	if (0 == strcmp(media_type, "audio")) {
	//if(type == SDP_PAYLOAD_TYPE_ALAW || type == SDP_PAYLOAD_TYPE_ULAW){
		rtp = (Rtp_t **)(&r->rtp_audio);
		rtcp = (Rtcp_t **)&r->rtcp_audio;
		b_video = false;
	}else if (0 == strcmp(media_type, "video")) {
		rtp = (Rtp_t **)(&r->rtp_video);
		rtcp = (Rtcp_t **)&r->rtcp_video;
		b_video = true;
	}else{
		VLOG(VLOG_ERROR,"SETUP: unsupport payload type:%s - %d",media_type, type);
		return RTSP_RET_FAIL;
	}
	VLOG(VLOG_DEBUG,"%s setup: c_port:%d s_port:%d",control,r->client_port,r->server_port);
	if(r->b_interleavedMode){
		if((r->channel % 2) == 0){
			rtp_chn_port = r->channel;
			rtcp_chn_port = r->channel + 1;
		}else{
			rtcp_chn_port = r->channel;
			rtp_chn_port = r->channel + 1;
		}
		rtp_sock = r->sock;
	}else{
		if((r->server_port % 2) == 0){
			rtp_chn_port = r->server_port;
			rtcp_chn_port = r->server_port + 1;
		}else{
			rtcp_chn_port = r->server_port;
			rtp_chn_port = r->server_port + 1;
		}
		rtp_sock = SOCK_udp_init(NULL, r->client_port,RTSP_SOCK_TIMEOUT);
		if(rtp_sock == -1) return RTSP_RET_FAIL;
	}
	*rtp = RTP_client_new(r->low_transport,r->b_interleavedMode,rtp_sock, type, real_type, r->peername,rtp_chn_port,r->buffer_time);
	if(*rtp == NULL) return RTSP_RET_FAIL;
	//init rtcp 
	ret=RTCP_init(rtcp,r->role,r->ssrc,r->low_transport,r->cast_type,
		r->b_interleavedMode,r->sock,rtcp_chn_port,r->client_port+1,*rtp);
	if(ret == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;

	// nat detect	
	SOCK_getsockname(r->sock,sockname);
	if(SOCK_isreservedip(sockname)==true && SOCK_isreservedip((*rtp)->peername)!=true && r->b_interleavedMode==false)
		rtsp_request_nat_detect(*rtp);
	//
	r->state = RTSP_STATE_READY;
	//
	return RTSP_RET_OK;
}

int RTSP_request_teardown(Rtsp_t *r)
{
	char tmp[256];
	const char format[]=
		"TEARDOWN %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Session: %s\r\n"\
		"\r\n";
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);
	sprintf(r->payload,format,tmp,RTSP_VERSION,
		++r->cseq,
		RTSP_USER_AGENT,
		r->session_id);
	r->payload_size=strlen(r->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",r->payload_size,r->payload);
	// send request
	if(rtsp_send_packet(r) == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	// get response
	//if(RTSP_read_message(r)==RTSP_RET_FAIL)
	//	return RTSP_RET_FAIL;
	// parse response
	//if(rtsp_parse_response(r,&status_code,tmp)==RTSP_RET_FAIL)
	//	return RTSP_RET_FAIL;
	//
	r->state = RTSP_STATE_INIT;
	//
	return RTSP_RET_OK;
}

int RTSP_keep_liveness(Rtsp_t *r)
{
	char tmp[256];
	char payload[RTSP_BUF_SIZE];
	int payload_size;
	const char format[]=
		"OPTIONS %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"\r\n";
	
	const char format2[]=
		"GET_PARAMETER %s %s\r\n"\
		"CSeq: %d\r\n"\
		"User-Agent: %s\r\n"\
		"Session: %s\r\n"\
		"\r\n";
	sprintf(tmp,"rtsp://%s:%d/%s",r->ip_me,r->port,r->stream);

	if(strstr(r->allow_method, "GET_PARAMETER")  != NULL){
		sprintf(payload,format2,tmp,RTSP_VERSION,
			++r->cseq,
			RTSP_USER_AGENT,
			r->session_id);
		payload_size=strlen(payload);
		VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n", payload_size, payload);
		// send request
		if(SOCK_send(r->sock, payload, payload_size) < 0) {
			return RTSP_RET_FAIL;
		}
	}
	
	sprintf(payload,format,tmp,RTSP_VERSION,
		++r->cseq,
		RTSP_USER_AGENT);
	payload_size=strlen(payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n", payload_size, payload);
	// send request
	if(SOCK_send(r->sock, payload, payload_size) < 0) {
		return RTSP_RET_FAIL;
	}	
	VLOG(VLOG_DEBUG,"rtsp session:%s keep alive!", r->ip_me);
	return RTSP_RET_OK;
}

Rtsp_t* RTSP_SERVER_init(int fd,int bAudio)
{
	Rtsp_t *r=NULL;
	printf("sock:%d\n",fd);
	r=(Rtsp_t *)malloc(sizeof(Rtsp_t));
	if(r == NULL){
		VLOG(VLOG_ERROR,"maloc for rtsp failed");
		return NULL;
	}
	memset(r,0,sizeof(Rtsp_t));
	r->toggle = true;
	r->data_available = 0;
	r->role = RTSP_SERVER;
	r->sock = fd;
	r->state = RTSP_STATE_INIT;
	r->buffer_time = 0;
	r->cseq = 0;
	r->readed = 0;
	r->b_interleavedMode = 0;
	r->client_port = 0;
	r->server_port = 0;
	r->channel = RTSP_CHANNEL_BEGIN;
	r->cast_type = RTP_UNICAST;
	r->work_mode = RTSP_MODE_PLAY;
	r->low_transport = RTP_TRANSPORT_UDP;
	SOCK_getpeername(fd,r->peername);
	if(bAudio == true){
		r->stream_type = RTSP_STREAM_VIDEO | RTSP_STREAM_AUDIO;
	}else{
		r->stream_type = RTSP_STREAM_VIDEO;
	}
	r->sdp = NULL;
	r->rtp_audio = NULL;
	r->rtp_video = NULL;
	r->rtcp_audio = NULL;
	r->rtcp_video = NULL;
	sprintf(r->session_id,"%d",rand());
	strcpy(r->allow_method,RTSP_ALLOW_METHODS);
	if(SOCK_tcp_init(fd,RTSP_SOCK_TIMEOUT)== -1){
		free(r);
		return NULL;
	}
#if RTSP_ENABLE_AUTHTICATION
	HTTP_AUTH_server_init(&r->auth,HTTP_AUTH_DEFAULT_TYPE);
	r->bLogin = false;
#else
	r->auth = NULL;
	r->bLogin = true;
#endif

	r->get_avc = NULL;
	r->eventHook = fRTSP_DEFAULT_EVENT_HOOK;
	r->eventCustomCtx = NULL;
	
	VLOG(VLOG_CRIT,"rtsp server init done.");
	return r;
}

Rtsp_t* RTSP_CLIENT_init(const char *url,char *user,char *pwd,enRTP_TRANSPORT transport,int iBufferTime, int bAudio)
{
	Rtsp_t *r=NULL;
	r=(Rtsp_t *)calloc(1, sizeof(Rtsp_t));
	if(r == NULL){
		VLOG(VLOG_ERROR,"maloc for rtsp failed");
		exit(0);
	}
	r->toggle = RTSPC_RUNNING;
	r->data_available = 0;
	r->role = RTSP_CLIENT;
	r->sock = -1;
	r->state = RTSP_STATE_INIT;
	r->cseq = 0;
	r->buffer_time = iBufferTime;
	r->client_port = 0;
	r->server_port = 0;
	r->channel = RTSP_CHANNEL_BEGIN;
	r->cast_type = RTP_UNICAST;
	r->work_mode = RTSP_MODE_PLAY;
	r->blocal = true;	// note .....,with rtpbuf
	r->transport = transport;

	r->readed = 0;

	r->stream_type = RTSP_STREAM_VIDEO;
	if(bAudio){
		r->stream_type |= RTSP_STREAM_AUDIO;
	}
	r->sdp = NULL;
	r->rtp_audio = NULL;
	r->rtp_video = NULL;
	r->rtcp_audio = NULL;
	r->rtcp_video = NULL;

	r->auth = NULL;
	r->bLogin = true;
	if(user) strcpy(r->user,user);
	if(pwd) strcpy(r->pwd,pwd);
	
	r->get_avc = NULL;
	r->eventHook = fRTSP_DEFAULT_EVENT_HOOK;
	r->eventCustomCtx = NULL;

	http_get_url(url,r->ip_me,&r->port,r->stream);
	
	return r;
}

static Rtsp_t* RTSP_CLIENT_init2(Rtsp_t *rtsp,int fd,char *user,char *pwd,enRTP_TRANSPORT transport,int iBufferTime)
{
	Rtsp_t *r=NULL;
	if(rtsp == NULL){
		r=(Rtsp_t *)malloc(sizeof(Rtsp_t));
		if(r == NULL){
			VLOG(VLOG_ERROR,"maloc for rtsp failed");
			exit(0);
		}
	}else{
		r = rtsp;
	}
	//memset(r,0,sizeof(Rtsp_t));
	r->toggle = RTSPC_RUNNING;
	r->data_available = 0;
	r->role = RTSP_CLIENT;
	r->sock = fd;
	r->state = RTSP_STATE_INIT;
	r->cseq = 0;
	r->buffer_time = iBufferTime;
	r->client_port = 0;
	r->server_port = 0;
	r->channel = RTSP_CHANNEL_BEGIN;
	r->cast_type = RTP_UNICAST;
	r->work_mode = RTSP_MODE_PLAY;
	r->blocal = false;
	r->session_id[0] = 0;
	r->session_timeout = 0;
	r->transport = transport;
	SOCK_getpeername(fd,r->peername);
	r->sdp = NULL;
	r->rtp_audio = NULL;
	r->rtp_video = NULL;
	r->rtcp_audio = NULL;
	r->rtcp_video = NULL;
	r->readed = 0;

	r->auth = NULL;
	r->bLogin = true;
	if(user && (user != r->user)) strcpy(r->user,user);
	if(pwd && (pwd != r->pwd)) strcpy(r->pwd,pwd);
	
	return r;
}


int RTSP_destroy(Rtsp_t *r)
{
	if (r == NULL)
		return -1;
	printf("rtsp %s destroy ...", r->ip_me);
	if(r->role == RTSP_CLIENT){ 
		if((r->state == RTSP_STATE_PLAYING) || (r->state == RTSP_STATE_RECORDING)){
			RTSP_request_teardown(r);
		}
	}else{
		//RTSP_STREAM_destroy(&r->s);
		RTSP_session_del(r->peername,r->low_transport,r->stream);
	}
	r->toggle = false;
	r->data_available = 0;
	r->state = RTSP_STATE_INIT;
	if(r->sock != -1){
		SOCK_close(r->sock);
		r->sock = -1;
	}
	r->cseq = 0;
	if(r->sdp){
		SDP_cleanup(r->sdp);
		r->sdp = NULL;
	}
	
	if(r->rtcp_audio){
		RTCP_destroy(r->rtcp_audio);
		r->rtcp_audio = NULL;
	}
	if(r->rtcp_video){
		RTCP_destroy(r->rtcp_video);
		r->rtcp_video=NULL;
	}
	if(r->rtp_audio){			
		RTP_destroy(r->rtp_audio);
		r->rtp_audio = NULL;
	}
	if(r->rtp_video){
		RTP_destroy(r->rtp_video);
		r->rtp_video = NULL;
	}

	if(r->auth){
		HTTP_AUTH_destroy(r->auth);
		r->auth = NULL;
	}
	r->bLogin = false;
		
	free(r);
	return TRUE;
}

int RTSP_cleanup(Rtsp_t *r)
{
	printf("rtsp %s cleanup ...", r->ip_me);
	if(r->sock != (-1)){	
		RTSP_request_teardown(r);
		SOCK_close(r->sock);
	}
	r->sock = -1;
	r->toggle = RTSPC_RUNNING;
	r->data_available = false;
	r->state = RTSP_STATE_INIT;
	r->sock = -1;
	r->cseq = 0;
	r->session_id[0] = 0;
	r->session_timeout = 0;
	if(r->sdp){
		SDP_cleanup(r->sdp);
		r->sdp = NULL;
	}
	if(r->rtcp_audio){
		RTCP_destroy(r->rtcp_audio);
		r->rtcp_audio = NULL;
	}
	if(r->rtcp_video){
		RTCP_destroy(r->rtcp_video);
		r->rtcp_video=NULL;
	}
	if(r->rtp_audio){
		RTP_destroy(r->rtp_audio);
		r->rtp_audio = NULL;
	}
	if(r->rtp_video){
		RTP_destroy(r->rtp_video);
		r->rtp_video = NULL;
	}

	if(r->auth){
		HTTP_AUTH_destroy(r->auth);
		r->auth = NULL;
	}
	//if(r->fflag != 0){
	///	VLOG(VLOG_ERROR, "EXCEED RTSP BUFFER!");
	//	exit(0);
	//}
	r->bLogin = false;
	r->blocal = false;
	
	r->get_avc = NULL;
			
	return TRUE;
}

static int rtsp_handle_unknown_chn_data(Rtsp_t *r, unsigned int len)
{
	char buf[0xffff];
	char *pbuf = buf;
	if (len >= sizeof(buf)) {
		pbuf = (char *)malloc(len);
		if(pbuf == NULL)
			return -1;
		
	}
	if (SOCK_recv3(r->sock, pbuf, len, 0) != len)
		return -1;
	return 0;
}

static int rtsp_handle_intreleaved(Rtsp_t *r, fRtpParsePacket frtp, fRtcpParsePacket frtcp)
{
	Rtp_t *rtp=NULL;
	Rtcp_t *rtcp=NULL;
	RtspInterHeader_t *interHeader=NULL;

	// check and parse interleaved packet
	interHeader = (RtspInterHeader_t *)r->payload;
	if(r->rtp_audio){
		if(interHeader->channel == r->rtp_audio->peer_chn_port){
			rtp=r->rtp_audio;
		}
	}
	if(r->rtp_video){
		if(interHeader->channel == r->rtp_video->peer_chn_port){
			rtp=r->rtp_video;
		}
	}
	if(r->rtcp_audio){
		if(interHeader->channel == r->rtcp_audio->chn_port_c){
			rtcp=r->rtcp_audio;
		}
	}
	if(r->rtcp_video){
		if(interHeader->channel == r->rtcp_video->chn_port_c){
			rtcp=r->rtcp_video;
		}
	}
	if(rtp){
		return frtp(rtp, NULL, ntohs(interHeader->length));
	}else if(rtcp){
		return frtcp(rtcp, NULL, ntohs(interHeader->length));
	}else{
		VLOG(VLOG_WARNING,"unknown packet (interleaved channel:%d magic:%c) ,check it.\n", interHeader->channel, interHeader->magic);
		if (rtsp_handle_unknown_chn_data(r, ntohs(interHeader->length)) < 0) {
			return RTSP_RET_FAIL;
		}
		return 0;
	}
}

static int is_rtsp_packet(const void* msg, int msg_sz)
{
	if((memcmp(msg,"OPTIONS",strlen("OPTIONS")) == 0)
		|| (memcmp(msg,"DESCRIBE",strlen("DESCRIBE")) == 0)
		|| (memcmp(msg,"SETUP",strlen("SETUP")) == 0)
		|| (memcmp(msg,"PLAY",strlen("PLAY")) == 0)
		|| (memcmp(msg,"PAUSE",strlen("PAUSE")) == 0)
		|| (memcmp(msg,"TEARDOWN",strlen("TEARDOWN")) == 0)
		|| (memcmp(msg,"RECORD",strlen("RECORD")) == 0)
		|| (memcmp(msg,"REDIRECT",strlen("REDIRECT")) == 0)
		|| (memcmp(msg,"ANNOUNCE",strlen("ANNOUNCE")) == 0)
		|| (memcmp(msg,"SET_PARAMETER",strlen("SET_PARAMETER")) == 0)
		|| (memcmp(msg,"GET_PARAMETER",strlen("GET_PARAMETER")) == 0)
		|| (memcmp(msg,"RTSP/1.0",strlen("RTSP/1.0")) == 0)){
		return true;
	}
	return false;
}


/************************************************************************
*function: read a complete packet from socket *
*note: 1. support rtsp method message and rtsp interleaved message
	2. can support multi-frame(packet) sended in a message
	3 .can support multi-message in a network frame(packet)
*************************************************************************/
int RTSP_read_message2(Rtsp_t *r, fRtpParsePacket frtp, fRtcpParsePacket frtcp)
{
	char *ptr=r->payload,*q;
	int ret = 0,readed=0,expected= 4;
	int content_len=0,packet_size=-1;

	do{
		if((readed + expected) > RTSP_BUF_SIZE){
			VLOG(VLOG_ERROR, "exceed rtsp buffer size,readed:%d expected:%d", readed, expected);
			return RTSP_RET_FAIL;
		}
		if(expected == 0){
			VLOG(VLOG_WARNING, "chn:ipret:%d readed:%d expected:%d packet_size:%d content:%d",
				ret, readed, expected, packet_size, content_len);
			return RTSP_RET_FAIL;
			//VLOG_Hex(VLOG_CRIT, r->payload, (readed > 512) ? 512 : readed);
			//assert(0);
		}
		if (r->readed == 0) {
			ret = recv(r->sock, ptr, expected, 0);
			if(ret < 0){
				if (SOCK_ERR == SOCK_EINTR) {
					VLOG(VLOG_WARNING, "# tcp recv error %d", SOCK_ERR);
					continue;
				}else if (SOCK_ERR == SOCK_EAGAIN) {
					r->readed = readed;
					//if (readed > 0) 
					//	VLOG(VLOG_DEBUG, "tcp recv SOCK_EAGAIN for %s..., %d/%d %c", r->peername, readed, packet_size, r->payload[0]);
					return SOCK_EAGAIN;
				} else if (SOCK_ERR == SOCK_ETIMEOUT) {
					VLOG(VLOG_ERROR, "# tcp recv time out");
					return RTSP_RET_FAIL;
				}
				VLOG(VLOG_ERROR, "### tcp recv error @%d ###", SOCK_ERR);
				return RTSP_RET_FAIL;
			}else if(ret == 0){
				VLOG(VLOG_CRIT, "network is shutdown by peer, expected:%d", expected);
				return RTSP_RET_FAIL;
			} else if (ret != expected) {
				VLOG(VLOG_DEBUG, "tcp recv not enough for %s..., %d/%d", r->peername, ret, expected);
				expected -=  ret;
				readed += ret;
				ptr += ret;
				continue;
			}
		}else if (r->readed < 4) {
			readed += r->readed;
			ptr += r->readed;
			expected = 4 - r->readed;
			r->readed = 0;
			continue;
		}else {
			ret = r->readed;
			expected = 1;
		}
		ptr[ret] = 0;
		readed += ret;
		if (expected > 1) 
		VLOG(VLOG_DEBUG, "chn:ipret:%d readed:%d expected:%d packet_size:%d content:%d",
			ret, readed, expected, packet_size, content_len);
		if(readed == packet_size)
			break;
		// check packet complete here ....
		// 
		if(r->payload[0] == RTSP_INTERLEAVED_MAGIC){
			int hret = 0;
			if(readed < sizeof(RtspInterHeader_t)){//not enouth 
				expected = sizeof(RtspInterHeader_t) - readed; 
			}else{
				hret = rtsp_handle_intreleaved(r, frtp, frtcp);
				if ( hret == SOCK_EAGAIN) {
					r->readed = 4;
					VLOG_HexString(VLOG_INFO, (unsigned char *)r->payload, 4);
				} else {
					r->readed = 0;
				}
				return hret;
			}
		}else{//rtsp message
			r->readed = 0;
			if(ptr[0] >= 0x80) {
				VLOG(VLOG_WARNING, "rtsp msg, but recv not ascii char %x", ptr[0]);
				return -1;
			}
			if(http_get_number(r->payload, "Content-length:", &content_len)==
				RTSP_RET_FAIL){//without content
				if(strstr(r->payload, "\r\n\r\n") != NULL){//end of http response header
					break;
				}else{
					expected = 1;
				}
			}else{
				if((q = strstr(r->payload,"\r\n\r\n")) != NULL){//end of http response header
					packet_size = content_len + (q - r->payload) + 4;
					if(readed == packet_size){
						break;
					}else{
						expected = packet_size - readed; 
					}
				}else{
					expected = 1; 
				}
			}
		}

		//		
		ptr += ret;
	}while(1);

	// check if rtsp packet combine with rtp/rtcp packet
	
	r->payload[readed] = 0;
	r->payload_size = readed;
	VLOG(VLOG_DEBUG,"read rtsp packet done,size:%d",readed);
	if(r->payload[0] != RTSP_INTERLEAVED_MAGIC)
		VLOG(VLOG_DEBUG,"payload :\r\n:%s",r->payload);
	
	return RTSP_RET_OK;
}

int RTSP_read_message(Rtsp_t *r)
{
	char *ptr=r->payload,*q;
	int ret,readed=0,expected=1;
	int content_len=0,packet_size=-1;
	do{
		if((readed + expected) > RTSP_BUF_SIZE){
			VLOG(VLOG_ERROR, "exceed rtsp buffer size,readed:%d expected:%d", readed, expected);
			return RTSP_RET_FAIL;
		}
		ret = recv(r->sock, ptr, expected, 0);
		if(ret < 0){
			if (SOCK_ERR == SOCK_EINTR) {
				VLOG(VLOG_WARNING, "# tcp recv error SOCK_EINTR");
				continue;
			}else if (SOCK_ERR == SOCK_EAGAIN) {
				VLOG(VLOG_ERROR, "tcp recv SOCK_EAGAIN in nonblocking mode");
				return RTSP_RET_FAIL;
			} else if (SOCK_ERR == SOCK_ETIMEOUT) {
				VLOG(VLOG_ERROR, "# tcp recv time out");
				return RTSP_RET_FAIL;
			}
			VLOG(VLOG_ERROR, "### tcp recv error @%d ###", SOCK_ERR);
			return RTSP_RET_FAIL;
		}else if(ret == 0){
			VLOG(VLOG_CRIT, "network is shutdown by peer");
			return RTSP_RET_FAIL;
		}
		ptr[ret] = 0;
		readed += ret;
		VLOG(VLOG_DEBUG, "ret:%d readed:%d expected:%d packet_size:%d content:%d",
			ret, readed, expected, packet_size, content_len);
		if(readed == packet_size)
			break;
		// check packet complete here ....
		// 
		if(r->payload[0] == RTSP_INTERLEAVED_MAGIC){
			RtspInterHeader_t *interHeader = NULL;
			if(readed < sizeof(RtspInterHeader_t)){//not enouth 
				expected = sizeof(RtspInterHeader_t) - readed; 
			}else{
				//char *pp=r->payload;
				//RTSP_LOG_HEX(pp,16);
				interHeader=(RtspInterHeader_t *)r->payload;
				packet_size =  ntohs(interHeader->length) + sizeof(RtspInterHeader_t);
				if(readed < packet_size){//not enought
					expected = packet_size - readed;
				}else if(readed == packet_size){
					break;
				}else{
					VLOG(VLOG_ERROR,"occur unhandle error,please check it");
					return RTSP_RET_FAIL;
				}
			}
		}else{//rtsp message
			if(http_get_number(r->payload, "Content-length:", &content_len)==
				RTSP_RET_FAIL){//without content
				if(strstr(ptr, "\r\n\r\n") != NULL){//end of http response header
					break;
				}else{
					expected = RTSP_BUF_SIZE - readed - 1; 
				}
			}else{
				if((q = strstr(ptr,"\r\n\r\n")) != NULL){//end of http response header
					packet_size = content_len + (q - r->payload) + 4;
					if(readed == packet_size){
						break;
					}else{
						expected = packet_size - readed; 
					}
				}else{
					expected = RTSP_BUF_SIZE - readed - 1; 
				}
			}
		}

		//		
		ptr += ret;
	}while(1);

	
	r->payload[readed] = 0;
	r->payload_size = readed;
	VLOG(VLOG_DEBUG,"read rtsp packet done,size:%d",readed);
	if(r->payload[0] != RTSP_INTERLEAVED_MAGIC)
		VLOG(VLOG_DEBUG,"payload :\r\n:%s",r->payload);
	
	return RTSP_RET_OK;
}


/************************************************************************
*function: parse a rtsp packet *
*note: 1. the src data is in rtsp->payload
	2. parameter fRTP and fRTCP is only use for interleaved mode(RTP over RTSP),else 
	   these parameter must given NULL
	3 .fRTP and fRTCP is formated like: int (*function)(void *structure,void *payload,int size)
*************************************************************************/
int RTSP_parse_message(Rtsp_t *r,fRtpParsePacket fRTP,fRtcpParsePacket fRTCP)
{
	int i,ret;
	Rtp_t *rtp=NULL;
	Rtcp_t *rtcp=NULL;
	RtspInterHeader_t *interHeader=NULL;
	// check and parse interleaved packet
	if((r->b_interleavedMode == true) && (r->payload[0] == RTSP_INTERLEAVED_MAGIC)){
		interHeader = (RtspInterHeader_t *)r->payload;
		if(r->rtp_audio){
			//VLOG(VLOG_DEBUG,"audio chn:%d inter-chn:%d",r->rtp_audio->chn_port_c,interHeader->channel);
			if(interHeader->channel == r->rtp_audio->peer_chn_port){
				rtp=r->rtp_audio;
			}
		}
		if(r->rtp_video){
			//VLOG(VLOG_DEBUG,"vido chn:%d inter-chn:%d",r->rtp_video->chn_port_c,interHeader->channel);
			if(interHeader->channel == r->rtp_video->peer_chn_port){
				rtp=r->rtp_video;
			}
		}
		if(r->rtcp_audio){
			//VLOG(VLOG_DEBUG,"A rtcp chn:%d inter-chn:%d",r->rtcp_audio->chn_port_c,interHeader->channel);
			if(interHeader->channel == r->rtcp_audio->chn_port_c){
				rtcp=r->rtcp_audio;
			}
		}
		if(r->rtcp_video){
			//VLOG(VLOG_DEBUG,"V rtcp chn:%d inter-chn:%d",r->rtcp_video->chn_port_c,interHeader->channel);
			if(interHeader->channel == r->rtcp_video->chn_port_c){
				rtcp=r->rtcp_video;
			}
		}
		if(rtp){
			if(fRTP == NULL){
				VLOG(VLOG_ERROR,"fRTP is null");
				return RTSP_RET_FAIL;
			}
			ret=fRTP(rtp,r->payload,r->payload_size);
		}else if(rtcp){
#if defined(RTCP_SENDER)
			if(fRTCP == NULL){
				VLOG(VLOG_ERROR,"fRTCP is null");
				return RTSP_RET_FAIL;
			}
			ret=fRTCP(rtcp,r->payload,r->payload_size);
#else
			VLOG(VLOG_DEBUG,"got rtcp packet!!!!");
			ret = RTSP_RET_OK;
#endif
		}else{
			ret=RTSP_RET_FAIL;
			r->payload[r->payload_size] = 0;
			VLOG(VLOG_ERROR,"unknown packet format,check it.\n%s",r->payload);
		}
		return ret;
	}
	// not interleaved packet
	for(i=0;i<RTSP_METHOD_CNT;i++){
		if(memcmp(r->payload,rtspMethods[i],strlen(rtspMethods[i])) == 0){
			break;
		}
	}
	if(i == RTSP_METHOD_CNT){
		VLOG(VLOG_ERROR,"unknow method: %d(%s)", i, r->payload);
		return RTSP_RET_FAIL;
	}
	if((ret = http_get_url(r->payload,r->ip_me,&r->port,r->stream))==RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	} else if (ret == (-2)) {
		if ( i  != RTSP_METHOD_OPTIONS) {
			VLOG(VLOG_ERROR,"unknow support URI of * for method: %s", rtspMethods[i]);
			return RTSP_RET_FAIL;
		}
	}
	if(http_get_number(r->payload,"CSeq:",&r->cseq)==-1){
		VLOG(VLOG_ERROR,"invaild request format,not found cseq");
		return RTSP_RET_FAIL;
	}
	http_get_string(r->payload, "User-Agent:", r->agent);
	VLOG(VLOG_DEBUG,"rtsp request %d <<%s>> cseq:%d agent:%s",i,rtspMethods[i],r->cseq, r->agent);
	switch(i){
		case RTSP_METHOD_DESCRIBE:
			if(RTSP_find_stream(r->stream,NULL)==RTSP_RET_OK){
				ret=rtsp_handle_describe(r);
			}else{
				ret=rtsp_response_error(r,RTSP_RSC_NOT_FOUND);
			}
			break;
		//case RTSP_METHOD_ANNOUNCE:
		//	ret=rtsp_handle_announce(r);
		//	break;
		case RTSP_METHOD_GET_PARAMETER:			
			ret=rtsp_handle_get_parameter(r);
			break;
		case RTSP_METHOD_OPTIONS:			
			ret=rtsp_handle_options(r);
			break;
		//case RTSP_METHOD_PAUSE:
		//	ret=rtsp_handle_pause(r);
		//	break;
		case RTSP_METHOD_PLAY:
			ret=rtsp_handle_play(r);

			//check rtsp sessions
			#if 0
			if((sess=RTSP_session_find(r->peername,r->low_transport,r->stream))!= NULL){
				// disable multi connection from the same devices
				//RTSP_destroy((Rtsp_t *)sess->data.rtsp);
				((Rtsp_t *)sess->data.rtsp)->toggle = false;
				MSLEEP(300);
			}
			#endif
			RTSP_session_add(r->peername,r->low_transport,r->stream,(void *)r);
			r->state = RTSP_STATE_PLAYING;
			break;
		//case RTSP_METHOD_RECORD:
		//	ret=rtsp_handle_record(r);
		//	break;
		//case RTSP_METHOD_REDIRECT:
		//	ret=rtsp_handle_redirect(r);
		//	break;
		case RTSP_METHOD_SETUP:
			ret=rtsp_handle_setup(r);
			r->state = RTSP_STATE_READY;
			break;
		case RTSP_METHOD_SET_PARAMETER:
			//rtsp_handle_set_parameter(r);
			break;	
		//case RTSP_METHOD_SET_PARAMETER:
		//	rtsp_handle_set_parameter(r);
		//	break;
		case RTSP_METHOD_TEARDOWN:
			ret=rtsp_handle_teardown(r);
			r->state = RTSP_STATE_INIT;
			break;
		default:
			ret=rtsp_handle_notallowed_method(r);
			break;
	}
	return ret;
}

/*
Rtsp_t* RTSP_connect_server(Rtsp_t *rtsp,char *url,char *user,char *pwd,int bInterleaved,int iBufferTime)
{
	Rtsp_t *r=NULL;
	char sockname[20];
	char ip_dst[20],stream[128];
	int port;
	SOCK_t sock;
	int i;
	Attribute_t attr;
	int iSetupMedia=0;

	if(http_get_url(url,ip_dst,&port,stream)== RTSP_RET_FAIL)
		return NULL;
	sock = SOCK_tcp_connect2(ip_dst,port, 4000,RTSP_SOCK_TIMEOUT);
	if(sock == -1)
		return NULL;
	// judge the socket buffer if necessary
	SOCK_getsockname(sock,sockname);
#ifdef RTSP_ENABLE_LAN_CHECK
	if(SOCK_isreservedip(sockname)!=true || SOCK_isreservedip(ip_dst)!=true){
		if(iBufferTime < RTSP_PLAYER_BUFFER_TIME) iBufferTime = RTSP_PLAYER_BUFFER_TIME;
#ifdef RTSP_FORCE_RTP_OVER_RTSP
		bInterleaved = true;
#endif
	}else{
		iBufferTime = 1000;
	}
#endif
	//
	r=RTSP_CLIENT_init2(rtsp,sock,user,pwd,bInterleaved,iBufferTime);
	if(r==NULL) return NULL;
	r->port = port;
	strcpy(r->ip_me,ip_dst);
	strcpy(r->stream,stream);
	//
	if(RTSP_request_options(r)==RTSP_RET_FAIL)
		goto CONNECT_ERR_EXIT;
	if(RTSP_request_describe(r)==RTSP_RET_FAIL)
		goto CONNECT_ERR_EXIT;
	for(i=0;i<r->sdp->media_num;i++){
		if(strcmp(r->sdp->media[i].media_n.type, "audio") == 0){
			//
			// (not rtsp protocal) do something for compatibility for old bug
			//if(strstr(r->agent, "minirtsp") == NULL){ // enable the compatibility of Zentao IPC PROJECT BUG #409
			//	continue;
			//}
			//
			// compability for our DVR/NVR/IPCAM project, not rtsp protocal's work
			//
			if(0 == (r->stream_type & RTSP_STREAM_AUDIO)){
				continue;
			}
			
			if(r->sdp->media[i].media_n.format == 0 || //pcma or pcmu
				r->sdp->media[i].media_n.format == 8){
				if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
					SDP_ATTR_CONTROL,(void *)&attr)==RTSP_RET_FAIL){
					continue;
				}
				
				if(RTSP_request_setup(r,attr.value, r->sdp->media[i].media_n.type, 
					r->sdp->media[i].media_n.format)==RTSP_RET_FAIL)
					goto CONNECT_ERR_EXIT;
			}else if (r->sdp->media[i].media_n.format >= 96) { 
				//dynamic payload type , need to get detail media type by rtpmap attribute
				if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
						SDP_ATTR_RTP_MAP,(void *)&attr) == RTSP_RET_OK){
					if(strncmp(attr.rtpmap.codec_type,"PCMU",strlen("PCMU")) == 0 ||
						strncmp(attr.rtpmap.codec_type,"PCMA",strlen("PCMA")) == 0) {
						// get audio control uri
						if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
							SDP_ATTR_CONTROL,(void *)&attr)==RTSP_RET_FAIL){
							continue;
						}
						// setup audio 
						if(RTSP_request_setup(r,attr.value,r->sdp->media[i].media_n.type, 
							r->sdp->media[i].media_n.format)==RTSP_RET_FAIL)
							goto CONNECT_ERR_EXIT;
					} else {
						VLOG(VLOG_WARNING, "UNKNOWN AUDIO CODEC TYPE: %s", attr.rtpmap.codec_type);
						continue;
					}
				}else{
					continue;
				}

			}else{
				VLOG(VLOG_WARNING, "UNKNOWN AUDIO TYPE: %d", r->sdp->media[i].media_n.format);
			}

		}else if(strcmp(r->sdp->media[i].media_n.type, "video") == 0){
			//check video codec type
			if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
					SDP_ATTR_RTP_MAP,(void *)&attr) == RTSP_RET_OK){
				if(strncmp(attr.rtpmap.codec_type,"H264",strlen("H264")) != 0){
					VLOG(VLOG_WARNING, "UNKNOWN VIDEO CODEC TYPE: %s", attr.rtpmap.codec_type);
					goto CONNECT_ERR_EXIT;
				}
			}else{
				goto CONNECT_ERR_EXIT;
			}
			// check video payload type
			if(r->sdp->media[i].media_n.format >= 96){
				if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
					SDP_ATTR_CONTROL,(void *)&attr)==RTSP_RET_FAIL){
					goto CONNECT_ERR_EXIT;
				}
				if(RTSP_request_setup(r,attr.value,r->sdp->media[i].media_n.type, 
					r->sdp->media[i].media_n.format)==RTSP_RET_FAIL)
					goto CONNECT_ERR_EXIT;
				iSetupMedia ++;
			}else{
				VLOG(VLOG_WARNING, "UNKNOWN VIDEO TYPE: %d", r->sdp->media[i].media_n.format);
			}
		}else{
			VLOG(VLOG_WARNING, "UNKNOWN MEDIA TYPE: %s", r->sdp->media[i].media_n.type);
		}
	}
	if(iSetupMedia == 0){
		MSLEEP(2000); // I CAN'T FIND ANY MEDIA which is usable for me; 
		goto CONNECT_ERR_EXIT;
	}
	if(RTSP_request_play(r)==RTSP_RET_FAIL)
		goto CONNECT_ERR_EXIT;
	
	return r;

CONNECT_ERR_EXIT:
	RTSP_destroy(r);
	return NULL;
}
*/

int RTSP_connect_server2(Rtsp_t *rtsp,char *url,char *user,char *pwd,enRTP_TRANSPORT transport,int iBufferTime)
{
	Rtsp_t *r=NULL;
	char sockname[20];
	char ip_dst[20],stream[128];
	int port;
	SOCK_t sock;
	int i;
	Attribute_t attr;
	int iSetupMedia=0;
	int real_type = 0;

	if(http_get_url(url,ip_dst,&port,stream)== RTSP_RET_FAIL)
		goto CONNECT_ERR_EXIT;
	sock = SOCK_tcp_connect2(ip_dst,port, 4000,RTSP_SOCK_TIMEOUT);
	if(sock == -1)
		goto CONNECT_ERR_EXIT;
	// judge the socket buffer if necessary
	SOCK_getsockname(sock,sockname);
#ifdef RTSP_ENABLE_LAN_CHECK
	if(SOCK_isreservedip(sockname)!=true || SOCK_isreservedip(ip_dst)!=true){
		if(iBufferTime < RTSP_PLAYER_BUFFER_TIME) iBufferTime = RTSP_PLAYER_BUFFER_TIME;
#ifdef RTSP_FORCE_RTP_OVER_RTSP
		transport = RTP_TCP;
#endif
	}else{
		iBufferTime = 1000;
	}
#endif
	//
	r=RTSP_CLIENT_init2(rtsp,sock,user,pwd,transport,iBufferTime);
	r->port = port;
	strcpy(r->ip_me,ip_dst);
	strcpy(r->stream,stream);
	//
	if(RTSP_request_options(r)==RTSP_RET_FAIL)
		goto CONNECT_ERR_EXIT;
	if(RTSP_request_describe(r)==RTSP_RET_FAIL)
		goto CONNECT_ERR_EXIT;
	for(i=0;i<r->sdp->media_num;i++){
		if(strcmp(r->sdp->media[i].media_n.type, "audio") == 0){			
			//
			// (not rtsp protocal) do something for compatibility for old bug
			//if(strstr(r->agent, "minirtsp") == NULL){ // enable the compatibility of Zentao IPC PROJECT BUG #409
			//	continue;
			//}
			//
			// compability for our DVR/NVR/IPCAM project, not rtsp protocal's work
			//
			if(0 == (r->stream_type & RTSP_STREAM_AUDIO)){
				continue;
			}
			//
			if(r->sdp->media[i].media_n.format == 0 || //pcma or pcmu
				r->sdp->media[i].media_n.format == 8){
				if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
					SDP_ATTR_CONTROL,(void *)&attr)==RTSP_RET_FAIL){
					continue;
				}
				real_type = r->sdp->media[i].media_n.format;
				if(RTSP_request_setup(r,attr.value,r->sdp->media[i].media_n.type, 
					r->sdp->media[i].media_n.format, real_type)==RTSP_RET_FAIL)
					goto CONNECT_ERR_EXIT;
			}else if (r->sdp->media[i].media_n.format >= 96
				|| r->sdp->media[i].media_n.format == 20
				|| r->sdp->media[i].media_n.format == 21
				|| r->sdp->media[i].media_n.format == 22
				|| r->sdp->media[i].media_n.format == 23) { 
				// 20 ~ 23 , unsigned payload type
				// >= 96: dynamic payload type , need to get detail media type by rtpmap attribute
				if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
						SDP_ATTR_RTP_MAP,(void *)&attr) == RTSP_RET_OK){
					if(strncmp(attr.rtpmap.codec_type,"PCMU",strlen("PCMU")) == 0 ||
						strncmp(attr.rtpmap.codec_type,"PCMA",strlen("PCMA")) == 0) {
						if (strncmp(attr.rtpmap.codec_type,"PCMA",strlen("PCMA")) == 0 ){
							real_type = RTP_TYPE_PCMA;
						} 
						else if (strncmp(attr.rtpmap.codec_type,"PCMU",strlen("PCMU")) == 0 ){
							real_type = RTP_TYPE_PCMU;
						}
						// get audio control uri
						if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
							SDP_ATTR_CONTROL,(void *)&attr)==RTSP_RET_FAIL){
							continue;
						}
						// setup audio 
						if(RTSP_request_setup(r,attr.value,r->sdp->media[i].media_n.type, 
							r->sdp->media[i].media_n.format, real_type)==RTSP_RET_FAIL)
							goto CONNECT_ERR_EXIT;
					} else {
						VLOG(VLOG_WARNING, "UNKNOWN AUDIO CODEC TYPE: %s", attr.rtpmap.codec_type);
						continue;
					}
				}else{
					continue;
				}

			}else{
				VLOG(VLOG_WARNING, "UNKNOWN AUDIO TYPE: %d", r->sdp->media[i].media_n.format);
			}
		}else if(strcmp(r->sdp->media[i].media_n.type, "video") == 0){
			//check video codec type
			if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
					SDP_ATTR_RTP_MAP,(void *)&attr) == RTSP_RET_OK){
				if(strncmp(attr.rtpmap.codec_type,"H264",strlen("H264")) != 0){
					VLOG(VLOG_WARNING, "UNKNOWN VIDEO CODEC TYPE: %s", attr.rtpmap.codec_type);
					goto CONNECT_ERR_EXIT;
				} else {
					real_type = RTP_TYPE_H264;
				}
			}else{
				goto CONNECT_ERR_EXIT;
			}
			// check video payload type
			if(r->sdp->media[i].media_n.format >= 96
				|| (r->sdp->media[i].media_n.format == 24)
				||  (r->sdp->media[i].media_n.format == 27)
				||  (r->sdp->media[i].media_n.format == 29)
				||  (r->sdp->media[i].media_n.format == 30)
				||  (r->sdp->media[i].media_n.format >= 35 && r->sdp->media[i].media_n.format <= 71)
				||  (r->sdp->media[i].media_n.format >= 77 && r->sdp->media[i].media_n.format <= 95)){
				if(SDP_get_media_attr(r->sdp,r->sdp->media[i].media_n.type,
					SDP_ATTR_CONTROL,(void *)&attr)==RTSP_RET_FAIL){
					goto CONNECT_ERR_EXIT;
				}
				if(RTSP_request_setup(r,attr.value,r->sdp->media[i].media_n.type, 
					r->sdp->media[i].media_n.format, real_type)==RTSP_RET_FAIL)
					goto CONNECT_ERR_EXIT;
				iSetupMedia ++;
			}else{
				VLOG(VLOG_WARNING, "UNKNOWN VIDEO TYPE: %d", r->sdp->media[i].media_n.format);
			}
		}else{
			VLOG(VLOG_WARNING, "UNKNOWN MEDIA TYPE: %s", r->sdp->media[i].media_n.type);
			//goto CONNECT_ERR_EXIT;
		}
	}
	
	if(iSetupMedia == 0){ // not video found
		goto CONNECT_ERR_EXIT;
	}
	
	return RTSP_RET_OK;

CONNECT_ERR_EXIT:
	MSLEEP(500);
	return RTSP_RET_FAIL;
}

/**********************************************************************
* interface for rtsp stream table *
***********************************************************************/
int RTSP_find_stream(const char *stream_name,char *media_name)
{
	int i,flag=false;
	for(i=0;i<g_RtspStreamTable.entries;i++){
		if(strcmp(g_RtspStreamTable.stream[i],stream_name) == 0){
			flag=true;
			break;
		}
	}
	if(flag == true){
		if(media_name)
			strcpy(media_name,g_RtspStreamTable.media[i]);
		return RTSP_RET_OK;
	}

	VLOG(VLOG_ERROR,"invalid stream: %s",stream_name);
	return RTSP_RET_FAIL;
}

int RTSP_add_stream(const char *stream_name,const char *media_name)
{
	if((strlen(stream_name) > RTSP_MAX_STREAM_LEN) || (strlen(media_name) > RTSP_MAX_STREAM_LEN)){
		VLOG(VLOG_ERROR,"strlen(stream-name) exceed the buffer size");
		assert(0);
	}
	if(g_RtspStreamTable.entries > RTSP_MAX_STREAM){
		VLOG(VLOG_ERROR,"the entries %d(%d) of streams exceed the buffer size", g_RtspStreamTable.entries, RTSP_MAX_STREAM);
		assert(0);
	}
	strcpy(g_RtspStreamTable.stream[g_RtspStreamTable.entries],stream_name);
	strcpy(g_RtspStreamTable.media[g_RtspStreamTable.entries],media_name);
	g_RtspStreamTable.entries++;
	return RTSP_RET_OK;
}

int RTSP_remove_stream(const char *stream_name)
{
	int i,flag=false;
	for(i=0;i<g_RtspStreamTable.entries;i++){
		if(strcmp(g_RtspStreamTable.stream[i],stream_name) == 0){
			flag=true;
		}
	}
	if(flag == true){
		for(;i<g_RtspStreamTable.entries;i++){
			strcpy(g_RtspStreamTable.stream[i],g_RtspStreamTable.stream[i+1]);
			strcpy(g_RtspStreamTable.media[i],g_RtspStreamTable.media[i+1]);
		}
		g_RtspStreamTable.entries--;
	}
	return RTSP_RET_OK;
}

void RTSP_set_event_hook(Rtsp_t *thiz, fRTSP_EVENT_HOOK hook, void *customCtx)
{
	if (thiz && hook) {
		thiz->eventHook = hook;
		thiz->eventCustomCtx = customCtx;
	}
}


