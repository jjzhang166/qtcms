#ifndef __ONVIF_H__
#define __ONVIF_H__
#include "stdinc.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "nvp_define.h"
#include "onvif_common.h"
#include "env_common.h"
#include "stack.h"
#include "cross_lock.h"
#include "cross_thread.h"
/*
extern int ONVIF_search();
extern int ONVIF_set_network_interface(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether);
*/

typedef struct ONVIF_C_CONTEXT
{
	// onvif event
	char event_bindip[20];
	int event_port;
	pthread_t event_pid;
	HMUTEX mutex;
	bool event_trigger;
	int event_user; // online event user count
	lpONVIF_EVENT_SUBSCRIBE event_list;
	//
	THREAD_HANDLE pid_discover;
	bool trigger_discover;

	// wsdd device list
	bool check_with_arp;
	int online_timeout;
	JStack_t *devices;
	//
	int connect_timeout;
	int send_timeout;
	int recv_timeout;
	char uuid[32];
	int bind_flag;
	//
	stNVP_ENV env;
}stONVIF_C_CONTEXT, *lpONVIF_C_CONTEXT;

void ONVIFC_wsdd_event_hook(char *dev_type, char *xaddr, char *scopes, int wsdd_event_type);

#ifdef __cplusplus
}
#endif
#endif



