#ifndef __ONVIF_SERVER_H__
#define __ONVIF_SERVER_H__

#include "stdinc.h"
#include "cross.h"
#include "cross_lock.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "env_common.h"
#include "onvif_common.h"


typedef struct ONVIF_S_CONTEXT
{
	bool need_auth;
 	// onvif search
	pthread_t search_pid;
	bool search_trigger;
	// onvif web server
	int web_port;
	pthread_t web_pid;
	bool web_trigger;
	// onvif event	
	int event_fd[2]; // pipe
	pthread_t event_pid;
	HMUTEX mutex;
	bool event_trigger;
	int event_user; // online event user count
	lpONVIF_EVENT_SUBSCRIBE event_list;
	//
	int device_type;
	char device_name[32];
	char dev_type[32];
	char scope_nr;
	char **scope_list;
	//
	// common environment
	stNVP_ENV env;
}stONVIF_S_CONTEXT, *lpONVIF_S_CONTEXT;

#ifdef __cplusplus
};
#endif

#endif


