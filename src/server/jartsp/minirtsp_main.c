/*************************************************************************
	> File Name: minirtsp_main.c
	> Author: kaga
	> Mail: kejiazhw@163.com 
	> Created Time: 2014年11月06日 星期四 10时04分01秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<stdint.h>

#include "minirtsp.h"


typedef struct _test_264
{
	uint32_t flag;
	uint32_t size;
	uint32_t isidr;
}Test264Frame_t;

static void* file_new(const char *name)
{
	FILE *f=fopen(name,"wb+");
	if(f==NULL){
		printf("open file failed\n");
		return NULL;
	}
	printf("create file:%s success\n",name);
	return (void *)f;
}

static int file_write(FILE *f,char *buf,int size)
{
	if(fwrite(buf,size,1,f)!=1){
		printf("write file failed\n");
		return -1;
	}
	return 0;
}

static inline int _h264_is_idr(unsigned char *data)
{
	int rlt = 0;

	if( ((data[4] & 0x1f) == 7) || ((data[4] & 0x1f) == 8) )
		rlt = 1;
	else
		rlt = 0;

	return rlt;
}

static FILE *f_video = NULL;
static FILE *f_audio = NULL;

static void rtsp_data_hook(void *pdata, uint32_t dataSize, 
	uint32_t timestamp,
	int dataType, void *customCtx)
{
	Test264Frame_t header;

	if (dataType == MD_TYPE_H264) {
		header.flag = 0x7d22628c;
		header.isidr = _h264_is_idr(pdata);
		header.size = dataSize;
		if (f_video == NULL) {
			f_video = file_new("./video.h264");
		}
		file_write(f_video, &header, sizeof(Test264Frame_t));
		file_write(f_video, pdata, dataSize);
	} else if (dataType == MD_TYPE_ULAW || dataType == MD_TYPE_ALAW) {
		;
	}
	//printf("rtsp Got %d data(size:%u timestamp:%u)\n", dataType, dataSize, timestamp);
}

static void rtsp_event_hook(int eventType, int lParam, void *rParam,
	void *customCtx)
{
	printf("rtsp Got EVENT[%d] param(%p)\n", eventType, customCtx);
	switch (eventType)
	{
		case MINIRTSP_EVENT_CONNECTED:
		case MINIRTSP_EVENT_DISCONNECTED:
		case MINIRTSP_EVENT_DESTROYED:
		case MINIRTSP_EVENT_PLAY:
		case MINIRTSP_EVENT_RECORD:
		case MINIRTSP_EVENT_PAUSE:
		case MINIRTSP_EVENT_DATA_RECEIVED:
			break;
		case MINIRTSP_EVENT_AUTH_REQUIRE:
			{
				// auth require , input username and password
				if (rParam) sprintf((char *)rParam, "%s:%s", "admin", "12345");
			}
			break;
		case MINIRTSP_EVENT_AUTH_FAILED:
		case MINIRTSP_EVENT_RTCP_SENDER_REPORT:
		case MINIRTSP_EVENT_RTCP_RECEIVER_REPORT:
		case MINIRTSP_EVENT_CHECK_ALIVE_FAILED:
			break;
		default:
			printf("rtsp Got INVALID EVENT[%d] param(%p)\n", eventType, customCtx);
			break;
	}
}

lpMINIRTSP rtsp_client_daemon()
{
	const char *streamUrl = "rtsp://192.168.1.218:554/Streaming/Channels/1"; //hikvision
	//const char *streamUrl = "rtsp://192.168.1.205:80/ch0_1.264";
	lpMINIRTSP rtsp = MINIRTSP_client_new(streamUrl, MINIRTSP_TRANSPORT_AUTO,
		"admin","",
		false, false);
	if (rtsp == NULL)
		return NULL;
	MINIRTSP_set_data_hook(rtsp,rtsp_data_hook,NULL, 0);
	MINIRTSP_set_event_hook(rtsp,rtsp_event_hook,NULL, 0);
	//MINIRTSP_set_loglevel(rtsp,5);
	if (MINIRTSP_connect(rtsp) < 0) {
		MINIRTSP_delete(rtsp);
		return NULL;
	}
	//if (MINIRTSP_play(rtsp) < 0){
	//	MINIRTSP_delete(rtsp);
	//	return NULL;
	//}
	return rtsp;
}

lpMINIRTSP rtsp_server_daemon()
{
	return NULL;
}

static lpMINIRTSP rtsp = NULL;

void rtsp_daemon_exit()
{
	MINIRTSP_delete(rtsp);
	rtsp = NULL;
	if (f_video) {
		fclose(f_video);
		f_video = NULL;
	}
	if (f_audio) {
		fclose(f_audio);
		f_audio = NULL;
	}
}

int main(int argc, char *argv[])
{
	char ch;
	stMINIRTSP_DATA_PROPERTY property;
	const char *usage="Usage :%s [-c|-s] [streamUrl | streamFilePath]\n";

	if (argc == 1) {
		printf(usage, argv[0]);
		return 0;
	}
	if (strcmp(argv[1], "-c") == 0)
		rtsp = rtsp_client_daemon();
	else
		rtsp = rtsp_server_daemon();
	
	if (rtsp == NULL)
		return -1;

	atexit(rtsp_daemon_exit);
	
	for(;;) {
		ch = getchar();
		switch (ch) {
			case 'q': // exit programe
				exit(0);
			case 'p': // play
				MINIRTSP_play(rtsp);
				break;
			case '=': // pause
				MINIRTSP_pause(rtsp);
				break;
			case 'r': // reconnect
				MINIRTSP_disconnect(rtsp);
				MINIRTSP_connect(rtsp);
				break;
			case 'a': // data attribute
				MINIRTSP_lookup_data(rtsp, MD_TYPE_H264, &property);
				MINIRTSP_lookup_data(rtsp, MD_TYPE_ALAW, &property);
				MINIRTSP_lookup_data(rtsp, MD_TYPE_ULAW, &property);
				break;
		}
	}

	
	return 0;
}
