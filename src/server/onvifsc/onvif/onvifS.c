#include "stdinc.h"

#include "onvif_common.h"
#include "soapH.h"
#include "onvif.h"
#include "onvifS.h"
#include "soapStub.h"
#include "onvif_debug.h"
#include "sys/socket.h"
#include "generic.h"
#include "app_debug.h"
#include "ezxml.h"

#ifdef SOAP_SERVER

/***********************************************************************
**
************************************************************************/
lpONVIF_S_CONTEXT g_OnvifServerCxt = NULL;

static enum xsd__boolean  nfalse = xsd__boolean__false_;
static enum xsd__boolean  ntrue = xsd__boolean__true_;

static time_t _motion_detection_timestamp[1];

/***********************************************************************
**
************************************************************************/
static void onvif_mark_motion_detection()
{
	time(&_motion_detection_timestamp[0]);
}

static bool onvif_md_status(int vin, bool clearFlag)
{
	vin = 0; // FIXME:
	time_t const t = time(NULL);
	
	if(clearFlag){
		// clear the timestamp of motion detection
		_motion_detection_timestamp[vin] = 0;
	}else{
		int diff = t - _motion_detection_timestamp[vin];
		//APP_TRACE("ONVIF Last Motion Detection(%d) is %d", vin, diff);
		if(abs(diff) <= 5){
			return true;
		}else{
			return false;
		}
	}
	return false;
}

void ONVIF_S_env_load(const char *module, int keyid)
{
	NVP_env_load(&g_OnvifServerCxt->env, module, keyid);
}

void ONVIF_S_env_save(const char *module, int keyid)
{
	NVP_env_save(&g_OnvifServerCxt->env, module, keyid);
}


void ONVIF_notify_event(int event)
{
	lpONVIF_S_CONTEXT onvif = g_OnvifServerCxt;
	onvif_mark_motion_detection();
	write(onvif->event_fd[1], &event, sizeof(event));
	ONVIF_TRACE(" event:%d occur", event);
}

static void *onvif_event_daemon(void *param)
{
#define ONVIF_EVENT_DELAY_DELETE	(2)
	int ret, notify_ret = 0;
	lpONVIF_S_CONTEXT onvif = (lpONVIF_S_CONTEXT)param;
	lpONVIF_EVENT_SUBSCRIBE event = onvif->event_list;
	lpONVIF_EVENT_SUBSCRIBE pre = NULL;
	fd_set rset;
	time_t t_now;
	struct timeval timeout;

	ONVIF_INFO("onvif event_s daemon start, pid(0x%lx)...", (long)pthread_self());
	while(onvif->event_trigger)
	{
		int i;
		notify_ret = 0;
		
		FD_ZERO(&rset);
		FD_SET(onvif->event_fd[0], &rset);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		ret = select(onvif->event_fd[0] +1, &rset, NULL, NULL, &timeout);
		
		mutex_lock(onvif->mutex);

		if (ret <= 0) {
		} else {
			if (FD_ISSET(onvif->event_fd[0], &rset)) {
				int event_type;
				if (read(onvif->event_fd[0], &event_type, sizeof(event_type)) == sizeof(event_type)) {
					//printf("get event type : %x\n", event_type);
					//
					event = onvif->event_list;
					while(event) 
					{
						ONVIF_TRACE("event type: %d(%d) from %s", event_type, event->event_type, event->peer_reference);
				
						if (event->event_mode == ONVIF_EVENT_MODE_NOTIFY) {
							if (event->event_type == event_type
								|| event->event_type == 0) {
								//printf("-- %s %x\n", event->peer_reference, event->event_type);
								// send notify
								if (event->event_state[event_type] == ONVIF_EVENT_STATE_DELETE) {
									event->event_state[event_type] = ONVIF_EVENT_STATE_INIT;
								}
								ONVIF_TRACE("MD event notify to %s", event->peer_reference);
								notify_ret = ONVIF_send_notify(event_type, event->event_state[event_type] , event->local_reference, event->peer_reference);
								if (notify_ret == NVP_RET_CONNECT_REFUSED) {
									goto __NOTIFY_PEER_REFUSED;
								} else {
									if (event->event_state[event_type] == ONVIF_EVENT_STATE_INIT) {
										notify_ret = ONVIF_send_notify(event_type, ONVIF_EVENT_STATE_CHANGED , event->local_reference, event->peer_reference);
									}
								}
								event->event_state[event_type] = ONVIF_EVENT_STATE_CHANGED;
								time(&event->m_occur[event_type]);
								if (notify_ret == NVP_RET_CONNECT_REFUSED) {
									goto __NOTIFY_PEER_REFUSED;
								}
							} 
						}
						event = event->next;
					}
				}
			}
		}
		//// check if subscribe timeout
		//
		event = onvif->event_list;
		pre = NULL;
		while(event) 
		{
			lpONVIF_EVENT_SUBSCRIBE tmp = event->next;
			
			time(&t_now);
			if ((t_now - event->m_sync) > event->timeout) {
				// subcrube timeout , unscribe it...
				ONVIF_INFO("%s timeout , delete it", event->local_reference);
				if (event->trigger && event->pid != 0) {
					event->trigger = false;
					pthread_join(event->pid, NULL);
				}
				free(event);
				if (pre == NULL) // delete first node
					onvif->event_list = tmp;
				else
					pre->next = tmp;
				event = pre;
				
				onvif->event_user--;
			} else {
				// check event state
				if (event->event_mode == ONVIF_EVENT_MODE_NOTIFY) {
					if (event->event_type == NVP_EVENT_ALL) {
						for (i = 1; i < NVP_EVENT_CNT; i++) {
							if ((unsigned int)((t_now - event->m_occur[i])) > ONVIF_EVENT_DELAY_DELETE) {
								//ONVIF_TRACE("event all %lu > %lu state:%d", t_now, event->m_occur[i], event->event_state[i]);
								if (event->event_state[i] == ONVIF_EVENT_STATE_CHANGED) {
									event->event_state[i] = ONVIF_EVENT_STATE_DELETE;
									notify_ret = ONVIF_send_notify(i, event->event_state[i] , event->local_reference, event->peer_reference);
									if (notify_ret == NVP_RET_CONNECT_REFUSED) {
										goto __NOTIFY_PEER_REFUSED;
									}
								} 
							}
						}
					} else {
						if ((t_now - event->m_occur[event->event_type]) > ONVIF_EVENT_DELAY_DELETE) {
							if (event->event_state[event->event_type] == ONVIF_EVENT_STATE_CHANGED) {
								event->event_state[event->event_type] = ONVIF_EVENT_STATE_DELETE;
								notify_ret = ONVIF_send_notify(event->event_type, event->event_state[event->event_type] , event->local_reference, event->peer_reference);
								if (notify_ret == NVP_RET_CONNECT_REFUSED) {
									goto __NOTIFY_PEER_REFUSED;
								}
							} 
						}
					}
				}
			}
			pre = event;
			event = tmp;
		}
		
		mutex_unlock(onvif->mutex);
		
__NOTIFY_PEER_REFUSED:
		mutex_unlock(onvif->mutex);
		if (event)
			ONVIF_event_unsubscribe(event->local_reference);
	}
	ONVIF_INFO("onvif event_s daemon stop.");

	pthread_exit(NULL);
}

static void onvif_init_name_token(lpONVIF_S_CONTEXT onvif)
{
#define ONVIF_SET_NT(ptr, value)			snprintf(ptr, sizeof(ptr), "%s", value)
#define ONVIF_SET_NT_CHN(ptr, value,chn)	snprintf(ptr, sizeof(ptr), "%s%d", value, chn)
#define ONVIF_SET_NT_ID(ptr, value,chn, id)	snprintf(ptr, sizeof(ptr), "%s%d_%d", value, chn, id)
	int i, n;

	if (onvif == NULL)
		return;

	memset(&onvif->env.devinfo, 0, sizeof(stNVP_DEV_INFO));
	memset(&onvif->env.systime, 0, sizeof(stNVP_DATE_TIME));
	memset(&onvif->env.ether, 0, sizeof(stNVP_ETHER_CONFIG));
	memset(&onvif->env.profiles, 0, sizeof(stNVP_PROFILE));

	onvif->env.profiles.chn =  NVP_CH_NR;
	for ( n = 0; n < NVP_MAX_CH; n++)
	{
		onvif->env.profiles.profile[n].index = n;
		onvif->env.profiles.profile[n].profile_nr = NVP_VENC_NR;
		for ( i = 0; i < NVP_MAX_VENC; i++) {
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].name[i], "Profile",n,  i);
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].token[i], "ProfileToken", n,i);
			
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].ain_name[i], "AIN", n,i);
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].ain_token[i], "AINToken", n,i);
		}

		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].v_source.name, "VIN", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].v_source.token, "VINToken", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].v_source.image.name, "IMG", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].v_source.image.token, "IMGToken", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].v_source.image.src_token, "VINToken", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].v_source.image.color.src_token, "VINToken", n);
		
		onvif->env.profiles.profile[n].venc_nr = NVP_VENC_NR;
		for ( i = 0; i < NVP_MAX_VENC; i++) {
			onvif->env.profiles.profile[n].venc[i].index = i;
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].venc[i].name, "Profile", n, i);
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].venc[i].token, "ProfileToken", n, i);
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].venc[i].enc_name, "VENC", n, i);
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].venc[i].enc_token, "VENCToken", n, i);
		}

		onvif->env.profiles.profile[n].vin_conf_nr = NVP_VIN_IN_A_SOURCE;
		for ( i = 0; i < NVP_MAX_VIN_IN_A_SOURCE; i++) {
			onvif->env.profiles.profile[n].venc[i].index = i;
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].vin[i].name, "VIN", n, i);
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].vin[i].token, "VINToken", n, i);
		}

		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].ain.name, "AIN", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].ain.token, "AINToken", n);

		onvif->env.profiles.profile[n].aenc_nr = NVP_AENC_NR;
		for ( i = 0; i < NVP_MAX_AENC; i++) {
			onvif->env.profiles.profile[n].aenc[i].index = i;
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].aenc[i].name, "AENC", n, i);
			ONVIF_SET_NT_ID(onvif->env.profiles.profile[n].aenc[i].token, "AENCToken", n, i);
		}

		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].van.name, "VAN", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].van.token, "VANToken", n);

		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].md.rule_name, "MDRule", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].md.module_name, "MDModule", n);

		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].ptz.name, "PTZ", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].ptz.token, "PTZToken", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].ptz.node_name, "PTZNode", n);
		ONVIF_SET_NT_CHN(onvif->env.profiles.profile[n].ptz.node_token, "PTZNodeToken", n);
		onvif->env.profiles.profile[n].ptz.preset_nr = NVP_MAX_PTZ_PRESET;
		for ( i  = 0; i < NVP_MAX_PTZ_PRESET; i++) {
			onvif->env.profiles.profile[n].ptz.preset[i].index = i;
			memset(onvif->env.profiles.profile[n].ptz.preset[i].name, 0, sizeof(onvif->env.profiles.profile[n].ptz.name));
			sprintf(onvif->env.profiles.profile[n].ptz.preset[i].token, "PresetToken%d", i + 1);
			onvif->env.profiles.profile[n].ptz.preset[i].in_use = false;
		}
		onvif->env.profiles.profile[n].ptz.default_pan_speed = 0.5;
		onvif->env.profiles.profile[n].ptz.default_tilt_speed = 0.5;
		onvif->env.profiles.profile[n].ptz.default_zoom_speed = 0.5;
		onvif->env.profiles.profile[n].ptz.tour_nr = 0;
		for ( i  = 0; i < NVP_MAX_PTZ_TOUR; i++) {
			onvif->env.profiles.profile[n].ptz.tour[i].index = i;
			memset(onvif->env.profiles.profile[n].ptz.tour[i].name, 0, sizeof(onvif->env.profiles.profile[n].ptz.tour[i].name));
			sprintf(onvif->env.profiles.profile[n].ptz.tour[i].token, "TourToken%d", i + 1);
		}
	}

	
	ONVIF_SET_NT(onvif->env.ether.name, "eth0");
	ONVIF_SET_NT(onvif->env.ether.token, "Eth0Token");

}



static void *onvif_server_proc(void *arg)
{
	int sock = *((int *)arg);
	
	free(arg);
	//signal(SIG_PIPE, SIG_IGN);
	pthread_detach(pthread_self());
	ONVIF_SERVER_daemon(sock);
	return NULL;
}

static void *onvif_server_listen_proc(void *arg)
{
	lpONVIF_S_CONTEXT onvif = (lpONVIF_S_CONTEXT)arg;
	int m, s = 0; /* master and slave sockets */
	struct soap *add_soap = soap_new();
	fd_set rset;
	struct timeval timeout;
	int ret;

	if(add_soap == NULL){
		ONVIF_INFO("soap new failed!");
		onvif->web_trigger = false;
		pthread_exit(NULL);
	}

	soap_set_namespaces(add_soap, namespaces);
	add_soap->bind_flags |= SO_REUSEADDR;

	m = soap_bind(add_soap, NULL, onvif->web_port, 32);
	if (m < 0)
	{
		soap_print_fault(add_soap, stderr);
		onvif->web_trigger = false;
		ONVIF_INFO("soap bind failed!");
		pthread_exit(NULL);
	}

	add_soap->recv_timeout = 3;
	add_soap->send_timeout = 3;

	ONVIF_INFO("onvif server daemon start, pid(0x%lx)...", (long)pthread_self());
	
	while(onvif->web_trigger == true) {
		FD_ZERO(&rset);
		FD_SET(add_soap->master, &rset);

		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		ret  = select(add_soap->master + 1, &rset, NULL, NULL, &timeout);
		if (ret < 0) {
			//ONVIF_TRACE("select error.");
		}else if (ret == 0) {
			//ONVIF_TRACE("select timeout, trigger: %d.", (int)onvif->search_trigger);
		} else {
			if (FD_ISSET(add_soap->master, &rset)) {
				//ONVIF_TRACE("select available.");
				s = soap_accept(add_soap); 
				if (s <= 0)
				{ 
					soap_print_fault(add_soap, stderr);
				} else {
					pthread_t pid;
					int *sock = (int *)malloc(sizeof(int));
					/*
					//ONVIF_TRACE("Socket connection successful: slave socket = %d", add_soap->socket);
					if ((ret = soap_serve(add_soap)) != SOAP_OK)
						ONVIF_INFO("soap serve error : %d!", ret);
					//soap_destroy(add_soap);
					
					soap_end(add_soap);
					*/
					*sock = (int)s;
					pthread_create(&pid, NULL, onvif_server_proc, (void *)sock);
				}
			}
		}
	}

	soap_free(add_soap);	 
	ONVIF_INFO("onvif server daemon stop!");
	pthread_exit(NULL);
}


void ONVIF_SERVER_start(int port)
{
	lpONVIF_S_CONTEXT onvif  = g_OnvifServerCxt;
	if (onvif)
	{
		if (onvif->web_pid == 0) {
			if (port > 0)
				onvif->web_port = port;
			onvif->web_trigger = true;
			pthread_create(&onvif->web_pid, NULL, onvif_server_listen_proc, onvif);
		}
	}
}

void ONVIF_SERVER_stop()
{
	lpONVIF_S_CONTEXT onvif  = g_OnvifServerCxt;
	if (onvif)
	{
		if (onvif->web_pid ) {
			onvif->web_trigger = false;
			pthread_join(onvif->web_pid, NULL);
			onvif->web_pid = 0;
		}
	}
}


static lpONVIF_S_CONTEXT ONVIF_SERVER_new(int device_type, const char *device_name)
{
	int i;
	lpONVIF_S_CONTEXT onvif = (lpONVIF_S_CONTEXT)calloc(1, sizeof(stONVIF_S_CONTEXT));
	APP_ASSERT(onvif != NULL, "onvif new failed!");

	ONVIF_INFO("onvif new success (%p)", onvif);

	onvif->need_auth = false;
	onvif->device_type = device_type;
	strcpy(onvif->device_name, device_name);
	onvif->event_user = 0;
	onvif->event_list = NULL;
	onvif->event_pid = 0;
	onvif->event_trigger = true;
	if (pipe(onvif->event_fd) < 0) {
		APP_TRACE("onvif event pipe new failed!");
		free(onvif);
		return NULL;
	}

	if (onvif->device_type == ONVIF_DEV_NVT)
		strcpy(onvif->dev_type, "dn:NetworkVideoTransmitter");
	else if (onvif->device_type == ONVIF_DEV_NVD)
		strcpy(onvif->dev_type, "dn:NetworkVideoDisplay");
	else if (onvif->device_type == ONVIF_DEV_NVS)
		strcpy(onvif->dev_type, "dn:NetworkVideoStorage");
	else if (onvif->device_type == ONVIF_DEV_NVA)
		strcpy(onvif->dev_type, "dn:NetworkVideoAnalytics");
	// init scope
	onvif->scope_nr = 7;
	onvif->scope_list = (char **)calloc(onvif->scope_nr, sizeof(char *));
	for ( i = 0; i < onvif->scope_nr; i++) {
		onvif->scope_list[i] = (char *)malloc(80);
	}
	strcpy(onvif->scope_list[0], "onvif://www.onvif.org/type/video_encoder");
 	strcpy(onvif->scope_list[1] , "onvif://www.onvif.org/type/audio_encoder");
 	strcpy(onvif->scope_list[2], "onvif://www.onvif.org/type/video_analytics");
 	strcpy(onvif->scope_list[3], "onvif://www.onvif.org/type/Network_Video_Transmitter");
 	strcpy(onvif->scope_list[4], "onvif://www.onvif.org/hardware/IPC-model");
	sprintf(onvif->scope_list[5], "onvif://www.onvif.org/name/%s", onvif->device_name);
 	strcpy(onvif->scope_list[6], "onvif://www.onvif.org/location/country/china");
 
	onvif_init_name_token(onvif);

	onvif->web_pid = 0;
	onvif->web_trigger = false;
	onvif->web_port = 8080;

	onvif->search_pid = 0;
	onvif->search_trigger = false;
	
	onvif->mutex = mutex_create();
	pthread_create(&onvif->event_pid, NULL, onvif_event_daemon, onvif);

	return onvif;
}

static void ONVIF_SERVER_del(lpONVIF_S_CONTEXT onvif)
{
	if (onvif) {
		int i;
		lpONVIF_EVENT_SUBSCRIBE event = onvif->event_list;		
		// delete onvif search daemon
		if (onvif->search_pid) {
			onvif->search_trigger = false;
			ONVIF_INFO("try to stop search thread (0x%lx) ", (long)onvif->search_pid);
			pthread_join(onvif->search_pid, NULL);
			onvif->search_pid = 0;
		}
		// delete onvif web server daemon
		if (onvif->web_pid ) {
			onvif->web_trigger = false;
			ONVIF_INFO("try to stop web thread(0x%lx) ", (long)onvif->web_pid);
			pthread_join(onvif->web_pid, NULL);
			onvif->web_pid = 0;
		}

		// delete events subscribe manager
		if (onvif->event_trigger && onvif->event_pid != 0) {
			onvif->event_trigger = false;
			pthread_join(onvif->event_pid, NULL);
		}
		while(event)
		{
			lpONVIF_EVENT_SUBSCRIBE tmp = event->next;
			if (event->trigger && event->pid != 0) {
				event->trigger = false;
				pthread_join(event->pid, NULL);
			}
			free(event);
			event = tmp;
		}		
		onvif->event_user = 0;
		onvif->event_list = NULL;
		//
		for (i = 0; i < onvif->scope_nr; i++)
			free(onvif->scope_list[i]);
		free(onvif->scope_list);
		onvif->scope_list = NULL;
		onvif->scope_nr = 0;
		
		mutex_destroy(onvif->mutex);
		
		free(onvif);
		onvif = NULL;
		
		ONVIF_INFO("%s done ", __FUNCTION__);

	}
}

void ONVIF_SERVER_init(int device_type, const char *device_name)
{
	if (g_OnvifServerCxt == NULL) {
		g_OnvifServerCxt = ONVIF_SERVER_new(device_type, device_name);		
		ONVIF_send_hello();
	}
}

void ONVIF_SERVER_deinit()
{
	if (g_OnvifServerCxt) {
		ONVIF_send_bye();
		ONVIF_SERVER_del(g_OnvifServerCxt);
	}
}

int ONVIF_event_subscribe(int event_mode, int event_type, char *peer_reference, char *local_reference, int timeout)
{
	int i;
	lpONVIF_S_CONTEXT onvif = g_OnvifServerCxt;
	lpONVIF_EVENT_SUBSCRIBE event = onvif->event_list;
	lpONVIF_EVENT_SUBSCRIBE e = NULL;
	unsigned char ipv4[4];
	unsigned short port;
	char uri[128];
	time_t t_now;
		
	ONVIF_S_env_load(OM_NET, 0);

	mutex_lock(onvif->mutex);

	time(&t_now);
	if (local_reference[0] == 0) {
		snprintf(local_reference, 200, "http://%s:%d%s/%ld_%d",
			_ip_2string(onvif->env.ether.ip, NULL), onvif->env.ether.http_port, 
			ONVIF_EVENT_PULLPOINT_URI_PREFIX,
			t_now, onvif->event_user + 1);
	}
	
	http_parse_url(ipv4, &port, uri, peer_reference);
	if (http_parse_url(ipv4, &port, uri, peer_reference) < -1) 
	{
		mutex_lock(onvif->mutex);
		return -1;		
	} else {
		memset(ipv4, 0, sizeof(ipv4));
		port = 0;
		uri[0] = 0;
	}
	
	while(event && event->next) event= event->next;
	e = (lpONVIF_EVENT_SUBSCRIBE)calloc(1, sizeof(stONVIF_EVENT_SUBSCRIBE));
	e->pid = 0;
	e->sock = 0;
	e->event_type = event_type;
	e->event_mode = event_mode;
	for (i  = 0; i < NVP_EVENT_CNT; i++) {
		e->event_state[i] = ONVIF_EVENT_STATE_INIT;
		e->m_occur[i] = 0;
	}
	strcpy(e->event_topic, g_onvif_topic[event_type]);
	sprintf(e->peer_addr, "%d.%d.%d.%d", ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
	e->peer_port = port;
	strncpy(e->peer_reference, peer_reference, sizeof(e->peer_reference) -1);
	strncpy(e->local_reference, local_reference, sizeof(e->local_reference) -1);
	e->timeout = timeout;
	time(&e->m_sync);
	e->trigger = false;
	e->next = NULL;

	if (onvif->event_list == NULL) {
		onvif->event_list = e;
	} else {
		event->next = e;
	}

	onvif->event_user++;
	
	mutex_unlock(onvif->mutex);

	return 0;
}

int ONVIF_event_renew(char *reference, int64_t *terminal_duration)
{
	lpONVIF_S_CONTEXT onvif = g_OnvifServerCxt;
	lpONVIF_EVENT_SUBSCRIBE event = onvif->event_list;
	char uri_local[128];
	char uri[128];

	//printf("event : %p user: %d\n", event, onvif->event_user);
	
	mutex_lock(onvif->mutex);
	while( event) {
		http_parse_url(NULL, NULL, uri_local, event->local_reference);
		http_parse_url(NULL, NULL, uri, reference);
		if (strncmp(uri_local, uri, strlen(uri)) == 0) {
			time(&event->m_sync);
			if (*terminal_duration > 0) {
				event->timeout = *terminal_duration;
			} else {
				*terminal_duration = event->timeout;
			}
			mutex_unlock(onvif->mutex);
			return 0;
		}
		event = event->next;
	}
	
	mutex_unlock(onvif->mutex);
	return -1;
}


int ONVIF_event_find(char *reference, lpONVIF_EVENT_SUBSCRIBE subscribe)
{
	lpONVIF_S_CONTEXT onvif = g_OnvifServerCxt;
	lpONVIF_EVENT_SUBSCRIBE event = onvif->event_list;
	char uri_local[128];
	char uri[128];

	//printf("event : %p user: %d\n", event, onvif->event_user);
	
	mutex_lock(onvif->mutex);
	while( event) {
		http_parse_url(NULL, NULL, uri_local, event->local_reference);
		http_parse_url(NULL, NULL, uri, reference);
		if (strncmp(uri_local, uri, strlen(uri)) == 0) {
			if (subscribe) {
				memcpy(subscribe, event, sizeof(stONVIF_EVENT_SUBSCRIBE));
			}
			mutex_unlock(onvif->mutex);
			return 0;
		}
		event = event->next;
	}
	
	mutex_unlock(onvif->mutex);
	return -1;
}

int ONVIF_event_unsubscribe(char *reference)
{
	lpONVIF_S_CONTEXT onvif = g_OnvifServerCxt;
	lpONVIF_EVENT_SUBSCRIBE event = onvif->event_list;
	lpONVIF_EVENT_SUBSCRIBE pre = NULL;
	char uri_local[128];
	char uri[128];

	mutex_lock(onvif->mutex);

	while(event) 
	{
		lpONVIF_EVENT_SUBSCRIBE tmp = event->next;
		
		http_parse_url(NULL, NULL, uri_local, event->local_reference);
		http_parse_url(NULL, NULL, uri, reference);
		if (strncmp(uri_local, uri, strlen(uri)) == 0) {
			ONVIF_TRACE("\t%s unsubscribe ", event->local_reference);
			if (event->trigger && event->pid != 0) {
				event->trigger = false;
				pthread_join(event->pid, NULL);
			}
			free(event);
			if (pre == NULL) // delete first node
				onvif->event_list = tmp;
			else
				pre->next = tmp;
			event = pre;
			
			onvif->event_user--;
			
			mutex_unlock(onvif->mutex);
			return 0;
		}
		pre = event;
		event = tmp;
	}

	mutex_unlock(onvif->mutex);

	return -1;
}


/***********************************************************************
**
************************************************************************/




static inline void onvif_put_int(struct soap *soap, char **pbuf, int value)
{
	*pbuf = ONVIF_MALLOC_SIZE(char, 12);
	snprintf(*pbuf, 12, "%d", value);
}

void onvif_fault(struct soap *soap,char *value1,char *value2)
{
	soap->fault = (struct SOAP_ENV__Fault*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Fault)));
	soap->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Code)));
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = "SOAP-ENV:Sender";
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode = (struct SOAP_ENV__Code*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Code)));;
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value = value1;
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode = (struct SOAP_ENV__Code*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Code)));
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Value = value2;
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Subcode = NULL;
	soap->fault->faultcode = NULL;
	soap->fault->faultstring = NULL;
	soap->fault->faultactor = NULL;
	soap->fault->detail = NULL;
	soap->fault->SOAP_ENV__Reason = NULL;
	//soap->fault->SOAP_ENV__Reason = (struct SOAP_ENV__Reason*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Reason)));
	//soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text = (char*)soap_malloc(soap,100);
	//strcpy(soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text,"The requested service is not supported by the NVT");
	
	soap->fault->SOAP_ENV__Node = NULL;
	soap->fault->SOAP_ENV__Role = NULL;
	soap->fault->SOAP_ENV__Detail = NULL;
}

void onvif_fault2(struct soap *soap, int role,char *value1)
{
	soap->fault = (struct SOAP_ENV__Fault*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Fault)));
	soap->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Code)));
	if (role == 0)
		soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = "SOAP-ENV::Sender";
	else 
		soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = "SOAP-ENV::Receiver";
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode = NULL;
	soap->fault->faultcode = NULL;
	soap->fault->faultstring = NULL;
	soap->fault->faultactor = NULL;
	soap->fault->detail = NULL;
	soap->fault->SOAP_ENV__Reason = (struct SOAP_ENV__Reason*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Reason)));
	soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text = (char*)soap_malloc(soap,100);
	strcpy(soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text, value1);
	
	soap->fault->SOAP_ENV__Node = NULL;
	soap->fault->SOAP_ENV__Role = NULL;
	soap->fault->SOAP_ENV__Detail = NULL;
}


SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __dn__Hello(struct soap* soap, int dn__Hello, int *dn__HelloResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __dn__Bye(struct soap* soap, int dn__Bye, int *dn__ByeResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __dn__Probe(struct soap* soap, int dn__Probe, int *dn__ProbeResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServices(struct soap *soap, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
	char _IPAddr[NVP_MAX_URI_SIZE];

	ONVIF_S_env_load(OM_NET, 0);
	
	sprintf(_IPAddr, "http://%s:%d", _ip_2string(g_OnvifServerCxt->env.ether.ip, NULL), g_OnvifServerCxt->env.ether.http_port);

	tds__GetServicesResponse->__sizeService = 7;
	tds__GetServicesResponse->Service = ONVIF_MALLOC_SIZE(struct tds__Service,tds__GetServicesResponse->__sizeService);

	tds__GetServicesResponse->Service[0].XAddr = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tds__GetServicesResponse->Service[0].Namespace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	strcpy(tds__GetServicesResponse->Service[0].Namespace, "http://www.onvif.org/ver10/device/wsdl");
	sprintf(tds__GetServicesResponse->Service[0].XAddr,"%s%s", _IPAddr,"/onvif/device_service");
	tds__GetServicesResponse->Service[0].Capabilities = NULL;
	tds__GetServicesResponse->Service[0].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[0].Version->Major = 2;
	tds__GetServicesResponse->Service[0].Version->Minor = 3;
	tds__GetServicesResponse->Service[0].__size = 0;
	tds__GetServicesResponse->Service[0].__any = NULL;
	tds__GetServicesResponse->Service[0].__anyAttribute = NULL;

	tds__GetServicesResponse->Service[1].XAddr = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tds__GetServicesResponse->Service[1].Namespace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	strcpy(tds__GetServicesResponse->Service[1].Namespace, "http://www.onvif.org/ver10/media/wsdl");
	sprintf(tds__GetServicesResponse->Service[1].XAddr,"%s%s", _IPAddr,"/onvif/Media");
	tds__GetServicesResponse->Service[1].Capabilities = NULL;
	tds__GetServicesResponse->Service[1].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[1].Version->Major = 2;
	tds__GetServicesResponse->Service[1].Version->Minor = 4;
	tds__GetServicesResponse->Service[1].__size = 0;
	tds__GetServicesResponse->Service[1].__any = NULL;
	tds__GetServicesResponse->Service[1].__anyAttribute = NULL;	

	tds__GetServicesResponse->Service[2].XAddr = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tds__GetServicesResponse->Service[2].Namespace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	strcpy(tds__GetServicesResponse->Service[2].Namespace, "http://www.onvif.org/ver10/events/wsdl");
	sprintf(tds__GetServicesResponse->Service[2].XAddr,"%s%s", _IPAddr,"/onvif/Events");
	tds__GetServicesResponse->Service[2].Capabilities = NULL;
	tds__GetServicesResponse->Service[2].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[2].Version->Major = 2;
	tds__GetServicesResponse->Service[2].Version->Minor = 4;
	tds__GetServicesResponse->Service[2].__size = 0;
	tds__GetServicesResponse->Service[2].__any = NULL;
	tds__GetServicesResponse->Service[2].__anyAttribute = NULL;	

	tds__GetServicesResponse->Service[3].XAddr = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tds__GetServicesResponse->Service[3].Namespace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	strcpy(tds__GetServicesResponse->Service[3].Namespace, "http://www.onvif.org/ver20/imaging/wsdl");
	sprintf(tds__GetServicesResponse->Service[3].XAddr,"%s%s", _IPAddr,"/onvif/Imaging");
	tds__GetServicesResponse->Service[3].Capabilities = NULL;
	tds__GetServicesResponse->Service[3].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[3].Version->Major = 2;
	tds__GetServicesResponse->Service[3].Version->Minor = 2;
	tds__GetServicesResponse->Service[3].__size = 0;
	tds__GetServicesResponse->Service[3].__any = NULL;
	tds__GetServicesResponse->Service[3].__anyAttribute = NULL;	

	tds__GetServicesResponse->Service[4].XAddr = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tds__GetServicesResponse->Service[4].Namespace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	strcpy(tds__GetServicesResponse->Service[4].Namespace, "http://www.onvif.org/ver10/deviceIO/wsdl");
	sprintf(tds__GetServicesResponse->Service[4].XAddr,"%s%s", _IPAddr,"/onvif/DeviceIO");
	tds__GetServicesResponse->Service[4].Capabilities = NULL;
	tds__GetServicesResponse->Service[4].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[4].Version->Major = 2;
	tds__GetServicesResponse->Service[4].Version->Minor = 2;
	tds__GetServicesResponse->Service[4].__size = 0;
	tds__GetServicesResponse->Service[4].__any = NULL;
	tds__GetServicesResponse->Service[4].__anyAttribute = NULL;	

	tds__GetServicesResponse->Service[5].XAddr = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tds__GetServicesResponse->Service[5].Namespace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	strcpy(tds__GetServicesResponse->Service[5].Namespace, "http://www.onvif.org/ver20/analytics/wsdl");
	sprintf(tds__GetServicesResponse->Service[5].XAddr,"%s%s", _IPAddr,"/onvif/Analytics");
	tds__GetServicesResponse->Service[5].Capabilities = NULL;
	tds__GetServicesResponse->Service[5].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[5].Version->Major = 2;
	tds__GetServicesResponse->Service[5].Version->Minor = 2;
	tds__GetServicesResponse->Service[5].__size = 0;
	tds__GetServicesResponse->Service[5].__any = NULL;
	tds__GetServicesResponse->Service[5].__anyAttribute = NULL;	

	tds__GetServicesResponse->Service[6].XAddr = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tds__GetServicesResponse->Service[6].Namespace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	strcpy(tds__GetServicesResponse->Service[6].Namespace, "http://www.onvif.org/ver20/ptz/wsdl");
	sprintf(tds__GetServicesResponse->Service[6].XAddr,"%s%s", _IPAddr,"/onvif/ptz");
	tds__GetServicesResponse->Service[6].Capabilities = NULL;
	tds__GetServicesResponse->Service[6].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[6].Version->Major = 2;
	tds__GetServicesResponse->Service[6].Version->Minor = 4;
	tds__GetServicesResponse->Service[6].__size = 0;
	tds__GetServicesResponse->Service[6].__any = NULL;
	tds__GetServicesResponse->Service[6].__anyAttribute = NULL;		
	
	if(tds__GetServices->IncludeCapability){
		
		tds__GetServicesResponse->Service[0].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[0].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[0].Capabilities->__any,
		"<tds:Capabilities>"
			"<tds:Network IPFilter=\"false\" ZeroConfiguration=\"false\" IPVersion6=\"false\" DynDNS=\"false\" Dot11Configuration=\"false\" HostnameFromDHCP=\"false\" NTP=\"1\"></tds:Network>"
			"<tds:Security TLS1.0=\"false\" TLS1.1=\"false\" TLS1.2=\"false\" OnboardKeyGeneration=\"false\" AccessPolicyConfig=\"false\" Dot1X=\"false\" RemoteUserHandling=\"false\" X.509Token=\"false\" SAMLToken=\"false\" KerberosToken=\"false\" UsernameToken=\"false\" HttpDigest=\"false\" RELToken=\"false\"></tds:Security>"
			"<tds:System DiscoveryResolve=\"false\" DiscoveryBye=\"false\" RemoteDiscovery=\"false\" SystemBackup=\"false\" SystemLogging=\"false\" FirmwareUpgrade=\"false\" HttpFirmwareUpgrade=\"false\" HttpSystemBackup=\"false\" HttpSystemLogging=\"false\" HttpSupportInformation=\"false\"></tds:System>"
		"</tds:Capabilities>"
		);

		tds__GetServicesResponse->Service[1].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[1].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[1].Capabilities->__any,
        //"<tds:Capabilities>"
            "<trt:Capabilities>"
                "<trt:ProfileCapabilities MaximumNumberOfProfiles=\"2\"></trt:ProfileCapabilities>"
                "<trt:StreamingCapabilities RTPMulticast=\"true\" RTP_TCP=\"true\" RTP_RTSP_TCP=\"true\" NonAggregateControl=\"false\"></trt:StreamingCapabilities>"
            "</trt:Capabilities>"
        //"</tds:Capabilities>"
		);	

		tds__GetServicesResponse->Service[2].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[2].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[2].Capabilities->__any,
        //"<tds:Capabilities>"
            "<tev:Capabilities WSSubscriptionPolicySupport=\"true\" WSPullPointSupport=\"true\" "
            	"WSPausableSubscriptionManagerInterfaceSupport=\"false\"></tev:Capabilities>"
        //"</tds:Capabilities>"
		);			

		tds__GetServicesResponse->Service[3].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[3].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[3].Capabilities->__any,
		//"<tds:Capabilities>"
			"<timg:Capabilities/>"
		//"</tds:Capabilities>"
		);
		
		tds__GetServicesResponse->Service[4].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[4].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[4].Capabilities->__any,
		//"<tds:Capabilities>"
			"<tmd:Capabilities VideoSources=\"1\" VideoOutputs=\"0\" AudioSources=\"0\" AudioOutputs=\"0\" RelayOutputs=\"0\"></tmd:Capabilities>"
		//"</tds:Capabilities>"
		);

		tds__GetServicesResponse->Service[5].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[5].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[5].Capabilities->__any,
		//"<tds:Capabilities>"
			"<tan:Capabilities/>"
		//"</tds:Capabilities>"	
		);	

		tds__GetServicesResponse->Service[6].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[6].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[6].Capabilities->__any,
		//"<tds:Capabilities>"
		 	"<tptz:Capabilities xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\" EFlip=\"false\" Reverse=\"false\" />"
		//"</tds:Capabilities>"	
		);
	}
	return SOAP_OK;		
}
	
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServiceCapabilities(struct soap *soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
{
	tds__GetServiceCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct tds__DeviceServiceCapabilities);
	/* NETWORK */
	tds__GetServiceCapabilitiesResponse->Capabilities->Network = ONVIF_MALLOC(struct tds__NetworkCapabilities);
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPFilter = &nfalse;	
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->ZeroConfiguration = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPVersion6 = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->DynDNS = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot11Configuration = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->HostnameFromDHCP= &nfalse;	
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP= (int *)soap_malloc(soap, sizeof(int));
	*tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP= 1;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->__anyAttribute = NULL;

	/* SYSTEM */
	tds__GetServiceCapabilitiesResponse->Capabilities->System = ONVIF_MALLOC(struct tds__SystemCapabilities);
	tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryResolve = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryBye = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->RemoteDiscovery = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemBackup = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->FirmwareUpgrade = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemLogging = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemBackup = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpFirmwareUpgrade = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemLogging = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSupportInformation = &nfalse;

	/* SECURITY */
	tds__GetServiceCapabilitiesResponse->Capabilities->Security = ONVIF_MALLOC(struct tds__SecurityCapabilities);
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e0 = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e1 = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e2 = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->OnboardKeyGeneration = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->AccessPolicyConfig = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->Dot1X = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->RemoteUserHandling = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->X_x002e509Token = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->SAMLToken = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->KerberosToken = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->UsernameToken = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->HttpDigest = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->RELToken = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->SupportedEAPMethods = NULL;
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDeviceInformation(struct soap *soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
	ONVIF_S_env_load(OM_INFO, 0);

	tds__GetDeviceInformationResponse->FirmwareVersion = g_OnvifServerCxt->env.devinfo.sw_version;
	tds__GetDeviceInformationResponse->HardwareId = g_OnvifServerCxt->env.devinfo.hw_version;
	tds__GetDeviceInformationResponse->Manufacturer = g_OnvifServerCxt->env.devinfo.manufacturer;
	tds__GetDeviceInformationResponse->Model = g_OnvifServerCxt->env.devinfo.model;
	tds__GetDeviceInformationResponse->SerialNumber = g_OnvifServerCxt->env.devinfo.sn;
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemDateAndTime(struct soap *soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{
	int tzone = 0;
	int h, m, s;
	int y, M, d;

	ONVIF_S_env_load(OM_DTIME, 0);
	
	if (	tds__SetSystemDateAndTime->DateTimeType == tt__SetDateTimeType__NTP) {
		g_OnvifServerCxt->env.systime.ntp_enable = true;	
	} else {
		if (tds__SetSystemDateAndTime->UTCDateTime == NULL) {
			onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidDateTime");
			return SOAP_FAULT;
		}
		
		
		h = tds__SetSystemDateAndTime->UTCDateTime->Time->Hour;
		m = tds__SetSystemDateAndTime->UTCDateTime->Time->Minute;
		s = tds__SetSystemDateAndTime->UTCDateTime->Time->Second;
		y = tds__SetSystemDateAndTime->UTCDateTime->Date->Year - 1900;
		M = tds__SetSystemDateAndTime->UTCDateTime->Date->Month -1;
		d = tds__SetSystemDateAndTime->UTCDateTime->Date->Day;


		if ( h < 0 || h >= 24
			|| m < 0 || m >= 60
			|| s < 0 || s >= 60
			|| y < 0 || y > 255
			|| M < 0 || M > 11
			|| d <= 0 || d > 31) {
			onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidDateTime");
			return SOAP_FAULT;
		}
		
		g_OnvifServerCxt->env.systime.ntp_enable = false;	
		g_OnvifServerCxt->env.systime.gm_time.time.hour = h;
		g_OnvifServerCxt->env.systime.gm_time.time.minute = m;
		g_OnvifServerCxt->env.systime.gm_time.time.second = s;
		g_OnvifServerCxt->env.systime.gm_time.date.year = y;
		g_OnvifServerCxt->env.systime.gm_time.date.month = M;
		g_OnvifServerCxt->env.systime.gm_time.date.day = d;
	}
	ONVIF_INFO("datetime %d-%d-%d %d:%d:%d \n",
		y + 1900, M + 1, d, h, m, s);


	if(tds__SetSystemDateAndTime->TimeZone != NULL &&
		tds__SetSystemDateAndTime->TimeZone->TZ != NULL){
		int timezone_hour = 0, timezone_min = 0;

		if (tzone_s2int(tds__SetSystemDateAndTime->TimeZone->TZ , &tzone) < 0) {
			ONVIF_INFO("invalid tz value: %s", tds__SetSystemDateAndTime->TimeZone->TZ);
			tzone = 0;
			onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidTimeZone");
			return SOAP_FAULT;
		}
		timezone_hour = abs(tzone) / 3600;
		timezone_min = (abs(tzone)  % 3600) / 60;
		
		g_OnvifServerCxt->env.systime.tzone = tzone;

		g_OnvifServerCxt->env.systime.tzone = timezone_hour * 100 + timezone_min;
		if (tzone < 0) {
			g_OnvifServerCxt->env.systime.tzone *= -1;
		}
		ONVIF_INFO("set timezone: %d (%s)", g_OnvifServerCxt->env.systime.tzone, tds__SetSystemDateAndTime->TimeZone->TZ);
	}

	
	ONVIF_S_env_save(OM_DTIME, 0);
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemDateAndTime(struct soap *soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{   
	int tz_in_sec = 0;
	ONVIF_S_env_load(OM_DTIME, 0);

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime = ONVIF_MALLOC(struct tt__SystemDateTime);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType = tt__SetDateTimeType__Manual;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DaylightSavings = nfalse;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone = ONVIF_MALLOC(struct tt__TimeZone);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ = ONVIF_MALLOC_SIZE(char, 16);
	tz_in_sec = (g_OnvifServerCxt->env.systime.tzone / 100) * 3600 + (g_OnvifServerCxt->env.systime.tzone % 100) * 60;
	//GMT_STRING(g_OnvifServerCxt->env.systime.tzone, 
	//	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ, 16);
	tzone_int2s(tz_in_sec, 
		tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ, 16);
	ONVIF_INFO("timezone:%s", tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime = ONVIF_MALLOC(struct tt__DateTime);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date = ONVIF_MALLOC(struct tt__Date);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time = ONVIF_MALLOC(struct tt__Time);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Year = 
		g_OnvifServerCxt->env.systime.local_time.date.year+ 1900;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Month =
		g_OnvifServerCxt->env.systime.local_time.date.month +1;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Day = 
		g_OnvifServerCxt->env.systime.local_time.date.day;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Hour = 
		g_OnvifServerCxt->env.systime.local_time.time.hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Minute = 
		g_OnvifServerCxt->env.systime.local_time.time.minute;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Second = 
		g_OnvifServerCxt->env.systime.local_time.time.second;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime = ONVIF_MALLOC(struct tt__DateTime);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date = ONVIF_MALLOC(struct tt__Date);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time = ONVIF_MALLOC(struct tt__Time);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Year =  
		g_OnvifServerCxt->env.systime.gm_time.date.year + 1900;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Month = 
		g_OnvifServerCxt->env.systime.gm_time.date.month + 1;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Day = 
		g_OnvifServerCxt->env.systime.gm_time.date.day;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Hour = 
		g_OnvifServerCxt->env.systime.gm_time.time.hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Minute = 
		g_OnvifServerCxt->env.systime.gm_time.time.minute;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Second = 
		g_OnvifServerCxt->env.systime.gm_time.time.second;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemFactoryDefault(struct soap *soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
{
	return SOAP_FAULT;
 }

SOAP_FMAC5 int SOAP_FMAC6 __tds__UpgradeSystemFirmware(struct soap *soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SystemReboot(struct soap *soap, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
{
	tds__SystemRebootResponse->Message = "System reboot finished!";
	NVP_env_cmd(NULL, OM_REBOOT, 0);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RestoreSystem(struct soap *soap, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemBackup(struct soap *soap, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemLog(struct soap *soap, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemSupportInformation(struct soap *soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetScopes(struct soap *soap, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
	
 	int i;

	ONVIF_TRACE("Scopes count :%d", g_OnvifServerCxt->scope_nr);
	tds__GetScopesResponse->__sizeScopes = g_OnvifServerCxt->scope_nr;
	tds__GetScopesResponse->Scopes = ONVIF_MALLOC_SIZE(struct tt__Scope, g_OnvifServerCxt->scope_nr);

	for ( i = 0; i < g_OnvifServerCxt->scope_nr; i++ ){
		tds__GetScopesResponse->Scopes[i].ScopeDef = tt__ScopeDefinition__Fixed;
		tds__GetScopesResponse->Scopes[i].ScopeItem = ONVIF_MALLOC_SIZE(char, 80);
		strncpy(tds__GetScopesResponse->Scopes[i].ScopeItem,  g_OnvifServerCxt->scope_list[i], 80);
	}

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetScopes(struct soap *soap, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddScopes(struct soap *soap, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveScopes(struct soap *soap, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDiscoveryMode(struct soap *soap, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDiscoveryMode(struct soap *soap, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteDiscoveryMode(struct soap *soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteDiscoveryMode(struct soap *soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDPAddresses(struct soap *soap, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetEndpointReference(struct soap *soap, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteUser(struct soap *soap, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteUser(struct soap *soap, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetUsers(struct soap *soap, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateUsers(struct soap *soap, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteUsers(struct soap *soap, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetUser(struct soap *soap, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetWsdlUrl(struct soap *soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{	
	tds__GetWsdlUrlResponse->WsdlUrl = "http://www.onvif.org/Documents/Specifications.aspx";

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCapabilities(struct soap *soap, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
#define ONVIF_CAPABILITIES_LEN (80)
	char szip[20];
	int port;
	int _Category;

	ONVIF_TRACE("Category(%d)", tds__GetCapabilities->Category ? (*tds__GetCapabilities->Category) : 0xffff);

	ONVIF_S_env_load(OM_NET, 0);
		
	_ip_2string(g_OnvifServerCxt->env.ether.ip, szip);
	port = g_OnvifServerCxt->env.ether.http_port;

	if(tds__GetCapabilities->Category == NULL)
	{
		tds__GetCapabilities->Category= ONVIF_MALLOC(enum tt__CapabilityCategory);
		*tds__GetCapabilities->Category = tt__CapabilityCategory__All;
		_Category = tt__CapabilityCategory__All;
	}
	else
	{
		_Category = *tds__GetCapabilities->Category;
	}

	tds__GetCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct tt__Capabilities);
	tds__GetCapabilitiesResponse->Capabilities->Analytics = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Device = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Events = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Imaging = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Media = NULL;
	tds__GetCapabilitiesResponse->Capabilities->PTZ = NULL;

	tds__GetCapabilitiesResponse->Capabilities->Extension = ONVIF_MALLOC(struct tt__CapabilitiesExtension);
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO = ONVIF_MALLOC(struct tt__DeviceIOCapabilities);
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr = ONVIF_MALLOC_SIZE(char,ONVIF_CAPABILITIES_LEN);
	snprintf(tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr, ONVIF_CAPABILITIES_LEN, 
		"http://%s:%d/onvif/deivceIO", szip, port);
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoSources = 1;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoOutputs = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioSources = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioOutputs = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->RelayOutputs = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->__size = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->__any = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->__anyAttribute = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Display = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Recording = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Search = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Replay = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->AnalyticsDevice = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Extensions = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->__size = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->__any = NULL;
	
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Analytics))
	{
		tds__GetCapabilitiesResponse->Capabilities->Analytics = ONVIF_MALLOC(struct tt__AnalyticsCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr, ONVIF_CAPABILITIES_LEN, 
			"http://%s:%d/onvif/analytics",szip, port);
		tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport = ntrue;
		tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport = ntrue;

	}		
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Device))
	{
		tds__GetCapabilitiesResponse->Capabilities->Device = ONVIF_MALLOC(struct tt__DeviceCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Device->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Device->XAddr, ONVIF_CAPABILITIES_LEN, 
			"http://%s:%d/onvif/device", szip, port);
		tds__GetCapabilitiesResponse->Capabilities->Device->Network = ONVIF_MALLOC(struct tt__NetworkCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter = &nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration = &nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 = &nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS = &ntrue;
		tds__GetCapabilitiesResponse->Capabilities->Device->System = ONVIF_MALLOC(struct tt__SystemCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery = ntrue;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions = 4;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions = 
			ONVIF_MALLOC_SIZE(struct tt__OnvifVersion, 4);
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[0].Major = 2;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[0].Minor = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[1].Major = 2;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[1].Minor = 1;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[2].Major = 2;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[2].Minor = 2;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[3].Major = 2;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions[3].Minor = 3;
		tds__GetCapabilitiesResponse->Capabilities->Device->IO = ONVIF_MALLOC(struct tt__IOCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Device->Security = ONVIF_MALLOC(struct tt__SecurityCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1 = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2 = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken  = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken = nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken = nfalse;
		
	}		
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Events))
	{	
		tds__GetCapabilitiesResponse->Capabilities->Events = ONVIF_MALLOC(struct tt__EventCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Events->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Events->XAddr, ONVIF_CAPABILITIES_LEN, 
			"http://%s:%d/onvif/events", szip, port);
		tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport = ntrue;
		tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport = ntrue;
		tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport = nfalse;
	}		
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Imaging))
	{	
		tds__GetCapabilitiesResponse->Capabilities->Imaging = ONVIF_MALLOC(struct tt__ImagingCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, ONVIF_CAPABILITIES_LEN, 
			"http://%s:%d/onvif/imaging", szip, port);
	}	
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Media))
	{		
		tds__GetCapabilitiesResponse->Capabilities->Media = ONVIF_MALLOC(struct tt__MediaCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Media->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, ONVIF_CAPABILITIES_LEN, 
			"http://%s:%d/onvif/media", szip, port);
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities = ONVIF_MALLOC(struct tt__RealTimeStreamingCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast = &nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP = &nfalse;
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = &nfalse;
	}
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__PTZ))
	{
		tds__GetCapabilitiesResponse->Capabilities->PTZ = ONVIF_MALLOC(struct tt__PTZCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr, ONVIF_CAPABILITIES_LEN, 
			"http://%s:%d/onvif/ptz", szip, port);
		tds__GetCapabilitiesResponse->Capabilities->PTZ->__size = 0;
		tds__GetCapabilitiesResponse->Capabilities->PTZ->__any = NULL;
		tds__GetCapabilitiesResponse->Capabilities->PTZ->__anyAttribute = NULL;
	}
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDPAddresses(struct soap *soap, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetHostname(struct soap *soap, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
	tds__GetHostnameResponse->HostnameInformation = ONVIF_MALLOC(struct tt__HostnameInformation);
	tds__GetHostnameResponse->HostnameInformation->Name = ONVIF_MALLOC_SIZE(char, 64);
	gethostname(tds__GetHostnameResponse->HostnameInformation->Name, 64);
	tds__GetHostnameResponse->HostnameInformation->FromDHCP = nfalse;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostname(struct soap *soap, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
	if (tds__SetHostname->Name) {
		if (sethostname(tds__SetHostname->Name, strlen(tds__SetHostname->Name)) < 0) {
			onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidHostname");
			return SOAP_FAULT;
		}
	} else {
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidHostname");
		return SOAP_FAULT;
	}

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostnameFromDHCP(struct soap *soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDNS(struct soap *soap, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
{
	char _dns[32];	

	ONVIF_S_env_load(OM_NET, 0);
	_ip_2string(g_OnvifServerCxt->env.ether.dns1, _dns);
	
	tds__GetDNSResponse->DNSInformation = ONVIF_MALLOC(struct tt__DNSInformation);
	if (g_OnvifServerCxt->env.ether.dhcp == true)
		tds__GetDNSResponse->DNSInformation->FromDHCP = ntrue;
	else
		tds__GetDNSResponse->DNSInformation->FromDHCP = nfalse;
	tds__GetDNSResponse->DNSInformation->__sizeSearchDomain = 0;
	tds__GetDNSResponse->DNSInformation->SearchDomain = NULL;
	tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP = 0;
	tds__GetDNSResponse->DNSInformation->DNSFromDHCP = NULL;
	tds__GetDNSResponse->DNSInformation->__sizeDNSManual = 0;
	tds__GetDNSResponse->DNSInformation->Extension = NULL;

	if((tds__GetDNSResponse->DNSInformation->FromDHCP) == 1) 
	{
		tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP = 1;
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP = ONVIF_MALLOC(struct tt__IPAddress);
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->Type = 0; 
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv4Address = ONVIF_MALLOC_SIZE(char, 20);
		strncpy(tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv4Address, _dns, 20);
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv6Address = NULL;
	}
	else 
	{
		tds__GetDNSResponse->DNSInformation->__sizeDNSManual = 1;
		tds__GetDNSResponse->DNSInformation->DNSManual = ONVIF_MALLOC_SIZE(struct tt__IPAddress, 1);
		tds__GetDNSResponse->DNSInformation->DNSManual[0].Type = tt__IPType__IPv4; 
		tds__GetDNSResponse->DNSInformation->DNSManual[0].IPv4Address = ONVIF_MALLOC_SIZE(char, 20);
		strncpy(tds__GetDNSResponse->DNSInformation->DNSManual[0].IPv4Address,  _dns, 20);
		tds__GetDNSResponse->DNSInformation->DNSManual[0].IPv6Address = NULL;
	}
	return SOAP_OK;
	
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDNS(struct soap *soap, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse)
{	
	ONVIF_S_env_load(OM_NET, 0);

	if (tds__SetDNS->FromDHCP == nfalse && tds__SetDNS->DNSManual == NULL) 
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ja:InvalidDNSManual");
		return SOAP_FAULT;
	}

	if(tds__SetDNS->DNSManual->Type != tt__IPType__IPv4) 
	{
		onvif_fault(soap,"ja:NotSupported", "ja:UnsupportedIPV6");
		return SOAP_FAULT;
	}

	if (tds__SetDNS->DNSManual) {
		if(isValidIp4(tds__SetDNS->DNSManual->IPv4Address) == 0)
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidIPv4Address");
			return SOAP_FAULT;
		}

		NVP_IP_INIT_FROM_STRING(g_OnvifServerCxt->env.ether.dns1, tds__SetDNS->DNSManual->IPv4Address);
	}
	
	ONVIF_S_env_save(OM_NET, 0);

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNTP(struct soap *soap, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
	ONVIF_S_env_load(OM_DTIME, 0);

	tds__GetNTPResponse->NTPInformation = ONVIF_MALLOC(struct tt__NTPInformation);
	tds__GetNTPResponse->NTPInformation->FromDHCP = nfalse;
	tds__GetNTPResponse->NTPInformation->__sizeNTPManual = 1;
	tds__GetNTPResponse->NTPInformation->NTPManual = ONVIF_MALLOC(struct tt__NetworkHost);
	tds__GetNTPResponse->NTPInformation->NTPManual->Type = tt__NetworkHostType__DNS;
	tds__GetNTPResponse->NTPInformation->NTPManual->DNSname = ONVIF_MALLOC_SIZE(char, 128);
	strncpy(tds__GetNTPResponse->NTPInformation->NTPManual->DNSname, 
		g_OnvifServerCxt->env.systime.ntp_server, 128);

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNTP(struct soap *soap, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse)
{
	ONVIF_S_env_load(OM_DTIME, 0);
	
	if (tds__SetNTP->FromDHCP == true) {
		
	} else {
		if (tds__SetNTP->__sizeNTPManual == 0 ||
			tds__SetNTP->NTPManual == NULL) {
			onvif_fault(soap, "ter:InvalidArgVal", "");
			return SOAP_FAULT;
		}

		if (tds__SetNTP->NTPManual[0].Type == tt__NetworkHostType__IPv4) {
			if (tds__SetNTP->NTPManual[0].IPv4Address &&
				check_ipv4_addr(tds__SetNTP->NTPManual[0].IPv4Address) == 0) {
				strcpy( g_OnvifServerCxt->env.systime.ntp_server, tds__SetNTP->NTPManual[0].IPv4Address);
			} else {
				onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidIPv4Address");
				return SOAP_FAULT;
			}
		}
		else if (tds__SetNTP->NTPManual[0].Type == tt__NetworkHostType__IPv6) {
			onvif_fault2(soap, 1, "ja:NotSupportIPV6");
			return SOAP_FAULT;
		}
		else if (tds__SetNTP->NTPManual[0].Type == tt__NetworkHostType__DNS) {
			if (tds__SetNTP->NTPManual[0].DNSname) {
				strcpy( g_OnvifServerCxt->env.systime.ntp_server, tds__SetNTP->NTPManual[0].DNSname);
			} else {
				onvif_fault(soap, "ter:InvalidArgVal", "ter:TimeSyncedToNtp");
				return SOAP_FAULT;
			}
		} else {
			onvif_fault(soap, "ter:InvalidArgVal", "ter:TimeSyncedToNtp");
			return SOAP_FAULT;
		}
	}

	g_OnvifServerCxt->env.systime.ntp_enable = true;
	ONVIF_S_env_save(OM_DTIME, 0);
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDynamicDNS(struct soap *soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDynamicDNS(struct soap *soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
{
	return SOAP_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkInterfaces(struct soap *soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{	
	ONVIF_S_env_load(OM_NET, 0);

	tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = 1;    /* required attribute of type tt:ReferenceToken */
	tds__GetNetworkInterfacesResponse->NetworkInterfaces = ONVIF_MALLOC(struct tt__NetworkInterface);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->token = ONVIF_MALLOC_SIZE(char, 32);
	strcpy(tds__GetNetworkInterfacesResponse->NetworkInterfaces->token, g_OnvifServerCxt->env.ether.token);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Enabled = xsd__boolean__true_;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info = ONVIF_MALLOC(struct tt__NetworkInterfaceInfo);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress = ONVIF_MALLOC_SIZE(char, 32);
	snprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress, 32, "%s", 
		_mac_2string(g_OnvifServerCxt->env.ether.mac, NULL));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->MTU = ONVIF_MALLOC(int);
	*tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->MTU = 1500;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->Name = ONVIF_MALLOC_SIZE(char, 32);
	strcpy(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->Name, g_OnvifServerCxt->env.ether.name);

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4 = ONVIF_MALLOC(struct tt__IPv4NetworkInterface);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Enabled = xsd__boolean__true_;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config = ONVIF_MALLOC(struct tt__IPv4Configuration);
	if (g_OnvifServerCxt->env.ether.dhcp == true) 
		tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->DHCP = xsd__boolean__true_;
	else	
		tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->DHCP = xsd__boolean__false_;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual = ONVIF_MALLOC(struct tt__PrefixedIPv4Address);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address = ONVIF_MALLOC_SIZE(char, 20);
	snprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address, 20, "%s", 
		_ip_2string(g_OnvifServerCxt->env.ether.ip, NULL));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->PrefixLength = 
		netmask_to_prefixlength(g_OnvifServerCxt->env.ether.netmask);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal = ONVIF_MALLOC(struct tt__PrefixedIPv4Address);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal->Address = ONVIF_MALLOC_SIZE(char, 20);
	snprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal->Address, 20, "%s", 
		_ip_2string(g_OnvifServerCxt->env.ether.ip, NULL));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->LinkLocal->PrefixLength = 
		netmask_to_prefixlength(g_OnvifServerCxt->env.ether.netmask);

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->__sizeManual = 1;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkInterfaces(struct soap *soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
{	
	ONVIF_S_env_load(OM_NET, 0);

	if (tds__SetNetworkInterfaces->InterfaceToken == NULL ||
		(tds__SetNetworkInterfaces->InterfaceToken 
			&& strcmp(tds__SetNetworkInterfaces->InterfaceToken, g_OnvifServerCxt->env.ether.token) != 0)){
		ONVIF_TRACE("SetNetworkInterfaces failed\n");
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidNetworkInterface");
		return SOAP_FAULT;
	}
	if (tds__SetNetworkInterfaces->NetworkInterface == NULL ||
		tds__SetNetworkInterfaces->NetworkInterface->IPv4 == NULL ||
		(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual == NULL || 
			tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP == NULL)) {
		ONVIF_TRACE("SetNetworkInterfaces failed 2\n");
		return SOAP_FAULT;
	}
	if ((*tds__SetNetworkInterfaces->NetworkInterface->IPv4->DHCP) ==  nfalse) {
		g_OnvifServerCxt->env.ether.dhcp = false;
		
		if (isValidIp4(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual ->Address) == 0
			|| tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual ->PrefixLength < 8
			|| tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual ->PrefixLength > 24) {
			ONVIF_TRACE("SetNetworkInterfaces failed 3\n");
			onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidIPv4Address");
			return SOAP_FAULT;
		}

		NVP_IP_INIT_FROM_STRING(g_OnvifServerCxt->env.ether.ip, tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual->Address);
		netmask_from_prefixlength(tds__SetNetworkInterfaces->NetworkInterface->IPv4->Manual ->PrefixLength,
			g_OnvifServerCxt->env.ether.netmask);
	} else {
		g_OnvifServerCxt->env.ether.dhcp = true;
	}

	tds__SetNetworkInterfacesResponse->RebootNeeded  = ntrue;

	ONVIF_S_env_save(OM_NET, 0);
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkProtocols(struct soap *soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
{
	ONVIF_S_env_load(OM_NET, 0);
	
	tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols = 2;
	tds__GetNetworkProtocolsResponse->NetworkProtocols = ONVIF_MALLOC_SIZE(struct tt__NetworkProtocol, 2);
	tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Name = tt__NetworkProtocolType__HTTP;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Enabled = xsd__boolean__true_;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Port = ONVIF_MALLOC_SIZE(int, 1);
	tds__GetNetworkProtocolsResponse->NetworkProtocols[0].__sizePort = 1;
	*tds__GetNetworkProtocolsResponse->NetworkProtocols[0].Port = g_OnvifServerCxt->env.ether.http_port;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Name = tt__NetworkProtocolType__RTSP;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Enabled = xsd__boolean__true_;
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port = ONVIF_MALLOC_SIZE(int, 1);
	tds__GetNetworkProtocolsResponse->NetworkProtocols[1].__sizePort = 1;
	*tds__GetNetworkProtocolsResponse->NetworkProtocols[1].Port = g_OnvifServerCxt->env.ether.rtsp_port;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkProtocols(struct soap *soap, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
{
	int i;
	int spook_port = 0;
	int rtsp_port = 0, http_port = 0;
	
	ONVIF_TRACE("");
	ONVIF_S_env_load(OM_NET, 0);
	
	if (tds__SetNetworkProtocols->NetworkProtocols == NULL || tds__SetNetworkProtocols->NetworkProtocols->Port == NULL){
		return SOAP_FAULT;
	}

	for ( i = 0; i <  tds__SetNetworkProtocols->__sizeNetworkProtocols; i ++ ) {
		if (!(tds__SetNetworkProtocols->NetworkProtocols[i].Name == tt__NetworkProtocolType__HTTP || 
			tds__SetNetworkProtocols->NetworkProtocols[i].Name == tt__NetworkProtocolType__RTSP)) {
			if (tds__SetNetworkProtocols->__sizeNetworkProtocols == 1) {
				ONVIF_INFO("SetNetWorkProtocols failed\n");
				onvif_fault(soap,"ter:InvalidArgVal", "ter:ServiceNotSupported");
				return SOAP_FAULT;
			}
		}
		if (tds__SetNetworkProtocols->NetworkProtocols[i].__sizePort > 1) {
			ONVIF_INFO("SetNetWorkProtocols failed 2(size port:%d)\n", tds__SetNetworkProtocols->NetworkProtocols[i].__sizePort);
			return SOAP_FAULT;
		} else if (tds__SetNetworkProtocols->NetworkProtocols[i].__sizePort == 0) {
			continue;
		}
		if (tds__SetNetworkProtocols->NetworkProtocols[i].Port[0] < 80) {
			ONVIF_INFO("SetNetWorkProtocols failed 3\n");
			onvif_fault(soap,"ter:InvalidArgVal", "ter:PortAlreadyInUse");
			return SOAP_FAULT;
		}
		spook_port = tds__SetNetworkProtocols->NetworkProtocols[i].Port[0];
	}
	// check httpport is same with rtsp port
	for ( i = 0; i <  tds__SetNetworkProtocols->__sizeNetworkProtocols; i ++ ) {
		if (tds__SetNetworkProtocols->NetworkProtocols[i].Name == tt__NetworkProtocolType__HTTP) {
			http_port = tds__SetNetworkProtocols->NetworkProtocols[i].Port[0];
		}else if ( tds__SetNetworkProtocols->NetworkProtocols[i].Name == tt__NetworkProtocolType__RTSP) {
			rtsp_port =  tds__SetNetworkProtocols->NetworkProtocols[i].Port[0];
		}
	}
	
	if (http_port > 0 && rtsp_port > 0) {
		if (http_port != rtsp_port) {
			ONVIF_INFO("SetNetWorkProtocols failed 3\n");
			onvif_fault(soap,"ter:InvalidArgVal", "ja:HttpPortAndRtspPortNotEqual");
			return SOAP_FAULT;
		}
	}
	
	ONVIF_INFO("SetNetWorkProtocols, %d -> %d", g_OnvifServerCxt->env.ether.http_port , spook_port);
	if (spook_port > 0) {		
		g_OnvifServerCxt->env.ether.http_port = spook_port;
		ONVIF_S_env_save(OM_NET, 0);		
		// reboot myself
		NVP_env_cmd(NULL, OM_REBOOT, 0);
	}
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkDefaultGateway(struct soap *soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
	ONVIF_S_env_load(OM_NET, 0);

	tds__GetNetworkDefaultGatewayResponse->NetworkGateway = ONVIF_MALLOC(struct tt__NetworkGateway);
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address = 1;
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address = ONVIF_MALLOC_SIZE(char *, 1);
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address[0]  = ONVIF_MALLOC_SIZE(char, 20);
	snprintf(tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address[0] , 20, "%s",
		_ip_2string(g_OnvifServerCxt->env.ether.gateway, NULL));
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv6Address = 0;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkDefaultGateway(struct soap *soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
{	
	ONVIF_TRACE("");
	
	ONVIF_S_env_load(OM_NET, 0);

	if (tds__SetNetworkDefaultGateway->__sizeIPv4Address == 0 || tds__SetNetworkDefaultGateway->IPv4Address == NULL) {
		ONVIF_INFO("SetNetWorkDefaultGateway failed\n");
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidGatewayAddress");
		return SOAP_FAULT;
	}
	if (isValidIp4( tds__SetNetworkDefaultGateway->IPv4Address[0]) == 0) {
		ONVIF_INFO("SetNetWorkDefaultGateway failed2\n");
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidIPv4Address");
		return SOAP_FAULT;
	}
	
	NVP_IP_INIT_FROM_STRING(g_OnvifServerCxt->env.ether.gateway,  tds__SetNetworkDefaultGateway->IPv4Address[0]);

	ONVIF_S_env_save(OM_NET, 0);

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetZeroConfiguration(struct soap *soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetZeroConfiguration(struct soap *soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetIPAddressFilter(struct soap *soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetIPAddressFilter(struct soap *soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddIPAddressFilter(struct soap *soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveIPAddressFilter(struct soap *soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetAccessPolicy(struct soap *soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetAccessPolicy(struct soap *soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateCertificate(struct soap *soap, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificates(struct soap *soap, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificatesStatus(struct soap *soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetCertificatesStatus(struct soap *soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteCertificates(struct soap *soap, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetPkcs10Request(struct soap *soap, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificates(struct soap *soap, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetClientCertificateMode(struct soap *soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetClientCertificateMode(struct soap *soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRelayOutputs(struct soap *soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
/*	static struct tt__RelayOutput RelayOutputs[0];
	static struct tt__RelayOutputSettings Properties;
	Properties.DelayTime = "PT1S";
	Properties.IdleState = tt__RelayIdleState__closed;
	Properties.Mode = tt__RelayMode__Bistable;
	RelayOutputs[0].Properties = &Properties;
	RelayOutputs[0].token = "RelayOutputs_token_1";
*/
	tds__GetRelayOutputsResponse->RelayOutputs = ONVIF_MALLOC(struct tt__RelayOutput);
	tds__GetRelayOutputsResponse->RelayOutputs->Properties = ONVIF_MALLOC(struct tt__RelayOutputSettings);
	tds__GetRelayOutputsResponse->RelayOutputs->Properties->DelayTime = 1;
	tds__GetRelayOutputsResponse->RelayOutputs->Properties->IdleState = tt__RelayIdleState__closed;
	tds__GetRelayOutputsResponse->RelayOutputs->Properties->Mode = tt__RelayMode__Bistable;
	tds__GetRelayOutputsResponse->RelayOutputs->token = "RelayOutputs_token_1";
	tds__GetRelayOutputsResponse->__sizeRelayOutputs = 1;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputSettings(struct soap *soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputState(struct soap *soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SendAuxiliaryCommand(struct soap *soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCACertificates(struct soap *soap, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificateWithPrivateKey(struct soap *soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificateInformation(struct soap *soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCACertificates(struct soap *soap, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateDot1XConfiguration(struct soap *soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDot1XConfiguration(struct soap *soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfiguration(struct soap *soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfigurations(struct soap *soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteDot1XConfiguration(struct soap *soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Capabilities(struct soap *soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Status(struct soap *soap, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__ScanAvailableDot11Networks(struct soap *soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemUris(struct soap *soap, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartFirmwareUpgrade(struct soap *soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartSystemRestore(struct soap *soap, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetServiceCapabilities(struct soap *soap, struct _timg__GetServiceCapabilities *timg__GetServiceCapabilities, struct _timg__GetServiceCapabilitiesResponse *timg__GetServiceCapabilitiesResponse)
{
	timg__GetServiceCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct timg__Capabilities);
	timg__GetServiceCapabilitiesResponse->Capabilities->__size = 0;
	timg__GetServiceCapabilitiesResponse->Capabilities->__any = NULL;
	timg__GetServiceCapabilitiesResponse->Capabilities->__anyAttribute = NULL;
	timg__GetServiceCapabilitiesResponse->Capabilities->ImageStabilization = ONVIF_MALLOC(enum xsd__boolean);
	*timg__GetServiceCapabilitiesResponse->Capabilities->ImageStabilization = nfalse;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetImagingSettings(struct soap *soap, struct _timg__GetImagingSettings *timg__GetImagingSettings, struct _timg__GetImagingSettingsResponse *timg__GetImagingSettingsResponse)
{
	int token_exist = 0;
	int chn = 0;
	int i;

	ONVIF_TRACE(" VideoSourceToken:%s", timg__GetImagingSettings->VideoSourceToken);

	for(i = 0; i < g_OnvifServerCxt->env.profiles.chn; i++)
	{
		if(strcmp(timg__GetImagingSettings->VideoSourceToken, g_OnvifServerCxt->env.profiles.profile[i].v_source.token) == 0)
		{
			token_exist = 1;
			chn = i;
			break;
		}
	}

	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoSource");
		return SOAP_FAULT;
	}
	
	ONVIF_S_env_load(OM_IMG, chn * 100);

	timg__GetImagingSettingsResponse->ImagingSettings = ONVIF_MALLOC(struct tt__ImagingSettings20);
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation = ONVIF_MALLOC(struct tt__BacklightCompensation20);
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
	timg__GetImagingSettingsResponse->ImagingSettings->Brightness = ONVIF_MALLOC(float);
	*timg__GetImagingSettingsResponse->ImagingSettings->Brightness = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.brightness;
	timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = ONVIF_MALLOC(float);
	*timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation =g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.saturation;
	timg__GetImagingSettingsResponse->ImagingSettings->Contrast = ONVIF_MALLOC(float);
	*timg__GetImagingSettingsResponse->ImagingSettings->Contrast = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.contrast;
	timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = ONVIF_MALLOC(float);
	*timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.sharpness;
	timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter = ONVIF_MALLOC(enum tt__IrCutFilterMode);
	if (g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.ircut.ircut_mode == NVP_IRCUT_MODE_AUTO) {
		*timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter  = tt__IrCutFilterMode__AUTO;
	} else if (g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.ircut.ircut_mode == NVP_IRCUT_MODE_NIGHT) {
		*timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter  = tt__IrCutFilterMode__ON;
	} else{
		*timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter  = tt__IrCutFilterMode__OFF;
	}

	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange = NULL;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__SetImagingSettings(struct soap *soap, struct _timg__SetImagingSettings *timg__SetImagingSettings, struct _timg__SetImagingSettingsResponse *timg__SetImagingSettingsResponse)
{
	int token_exist = 0;
	int i, chn = 0;

	ONVIF_TRACE("reqVideoSourceToken:%s::rep:%s\n", timg__SetImagingSettings->VideoSourceToken,
		g_OnvifServerCxt->env.profiles.profile[chn].v_source.token);


	for(i = 0; i < g_OnvifServerCxt->env.profiles.chn; i++)
	{
		if(strcmp(timg__SetImagingSettings->VideoSourceToken, g_OnvifServerCxt->env.profiles.profile[i].v_source.token) == 0)
		{
			token_exist = 1;
			chn = i;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoSource");
		return SOAP_FAULT;
	}
	
	ONVIF_S_env_load(OM_IMG, chn * 100);

	if(NULL != timg__SetImagingSettings->ImagingSettings->Brightness){
		g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.brightness = 
			*timg__SetImagingSettings->ImagingSettings->Brightness;
	}
	if(NULL != timg__SetImagingSettings->ImagingSettings->ColorSaturation){
		g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.saturation = 
			*timg__SetImagingSettings->ImagingSettings->ColorSaturation;
	}
	if(NULL != timg__SetImagingSettings->ImagingSettings->Contrast){
		g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.contrast = 
			*timg__SetImagingSettings->ImagingSettings->Contrast;
	}
	if(NULL != timg__SetImagingSettings->ImagingSettings->Sharpness){
		g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.sharpness = 
			*timg__SetImagingSettings->ImagingSettings->Sharpness;
	}

	ONVIF_TRACE("%s brightness:%f, saturation:%f, contrast:%f, sharpen:%f\n", 
									timg__SetImagingSettings->VideoSourceToken,
									g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.brightness, 
									g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.saturation, 
									g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.contrast, 
									g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.color.sharpness);	
	
	if(NULL != timg__SetImagingSettings->ImagingSettings->IrCutFilter){
		if(tt__IrCutFilterMode__ON == *timg__SetImagingSettings->ImagingSettings->IrCutFilter){
			g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.ircut.ircut_mode = NVP_IRCUT_MODE_NIGHT;
		}
		else if(tt__IrCutFilterMode__AUTO == *timg__SetImagingSettings->ImagingSettings->IrCutFilter){
			g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.ircut.ircut_mode = NVP_IRCUT_MODE_AUTO;
		}
		else{
			g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.ircut.ircut_mode = NVP_IRCUT_MODE_DAYLIGHT;
		}
	}
	
	ONVIF_S_env_save(OM_IMG, chn * 100);
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetOptions(struct soap *soap, struct _timg__GetOptions *timg__GetOptions, struct _timg__GetOptionsResponse *timg__GetOptionsResponse)
{
	enum tt__IrCutFilterMode *IrCutFilterMode;
	int token_exist = 0;
	int i, chn = 0;

	ONVIF_TRACE("ImagingGetOptions, VideoSourceToken:%s", timg__GetOptions->VideoSourceToken);

	for(i = 0; i < g_OnvifServerCxt->env.profiles.chn; i++)
	{
		if(strcmp(timg__GetOptions->VideoSourceToken, g_OnvifServerCxt->env.profiles.profile[i].v_source.token) == 0)
		{
			token_exist = 1;
			chn = i;
			break;
		}
	}

	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoSource");
		return SOAP_FAULT;
	}

	ONVIF_S_env_load(OM_IMG, chn * 100);

	timg__GetOptionsResponse->ImagingOptions = ONVIF_MALLOC(struct tt__ImagingOptions20);
	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation = ONVIF_MALLOC(struct tt__BacklightCompensationOptions20);
	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Mode = ONVIF_MALLOC(enum tt__BacklightCompensationMode);
	*timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->__sizeMode = 1;

	timg__GetOptionsResponse->ImagingOptions->Brightness = ONVIF_MALLOC(struct tt__FloatRange);
	timg__GetOptionsResponse->ImagingOptions->Brightness->Min = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.brightness.min;
	timg__GetOptionsResponse->ImagingOptions->Brightness->Max = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.brightness.max;
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation = ONVIF_MALLOC(struct tt__FloatRange);
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Min = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.saturation.min;
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Max = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.saturation.max;
	timg__GetOptionsResponse->ImagingOptions->Contrast = ONVIF_MALLOC(struct tt__FloatRange);
	timg__GetOptionsResponse->ImagingOptions->Contrast->Min = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.contrast.min;
	timg__GetOptionsResponse->ImagingOptions->Contrast->Max = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.contrast.max;
	timg__GetOptionsResponse->ImagingOptions->Sharpness = ONVIF_MALLOC(struct tt__FloatRange);
	timg__GetOptionsResponse->ImagingOptions->Sharpness->Min = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.sharpness.min;
	timg__GetOptionsResponse->ImagingOptions->Sharpness->Max = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.sharpness.max;

	IrCutFilterMode = ONVIF_MALLOC_SIZE(enum tt__IrCutFilterMode, g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.ircut_mode.nr);
	for(i = 0; i < g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.ircut_mode.nr; ++i){
		*(i + IrCutFilterMode) = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.ircut_mode.list[i];	
	}
	timg__GetOptionsResponse->ImagingOptions->__sizeIrCutFilterModes = g_OnvifServerCxt->env.profiles.profile[chn].v_source.image.option.ircut_mode.nr;
	timg__GetOptionsResponse->ImagingOptions->IrCutFilterModes = IrCutFilterMode;
	timg__GetOptionsResponse->ImagingOptions->WideDynamicRange = NULL;
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Move(struct soap *soap, struct _timg__Move *timg__Move, struct _timg__MoveResponse *timg__MoveResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Stop(struct soap *soap, struct _timg__Stop *timg__Stop, struct _timg__StopResponse *timg__StopResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetStatus(struct soap *soap, struct _timg__GetStatus *timg__GetStatus, struct _timg__GetStatusResponse *timg__GetStatusResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetMoveOptions(struct soap *soap, struct _timg__GetMoveOptions *timg__GetMoveOptions, struct _timg__GetMoveOptionsResponse *timg__GetMoveOptionsResponse)
{
	int token_exist = 0;
	int i;

	for(i = 0; i < g_OnvifServerCxt->env.profiles.chn; i++)
	{
		if(strcmp(timg__GetMoveOptions->VideoSourceToken, g_OnvifServerCxt->env.profiles.profile[i].v_source.token) == 0)
		{
			token_exist = 1;
			break;
		}
	}

	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoSource");
		return SOAP_FAULT;
	}
	
	timg__GetMoveOptionsResponse->MoveOptions = ONVIF_MALLOC(struct tt__MoveOptions20);
	timg__GetMoveOptionsResponse->MoveOptions->Absolute = NULL;
	timg__GetMoveOptionsResponse->MoveOptions->Relative = NULL;
	timg__GetMoveOptionsResponse->MoveOptions->Continuous = NULL;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetServiceCapabilities(struct soap *soap, struct _tptz__GetServiceCapabilities *tptz__GetServiceCapabilities, struct _tptz__GetServiceCapabilitiesResponse *tptz__GetServiceCapabilitiesResponse)
{
	tptz__GetServiceCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct tptz__Capabilities);
	tptz__GetServiceCapabilitiesResponse->Capabilities->EFlip = &nfalse;
	tptz__GetServiceCapabilitiesResponse->Capabilities->Reverse = &nfalse;
	tptz__GetServiceCapabilitiesResponse->Capabilities->__size = 0;
	tptz__GetServiceCapabilitiesResponse->Capabilities->__any = NULL;
	tptz__GetServiceCapabilitiesResponse->Capabilities->GetCompatibleConfigurations = &nfalse;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurations(struct soap *soap, struct _tptz__GetConfigurations *tptz__GetConfigurations, struct _tptz__GetConfigurationsResponse *tptz__GetConfigurationsResponse)
{
	int n;

	tptz__GetConfigurationsResponse->__sizePTZConfiguration = g_OnvifServerCxt->env.profiles.chn;
	tptz__GetConfigurationsResponse->PTZConfiguration = ONVIF_MALLOC_SIZE(struct tt__PTZConfiguration, g_OnvifServerCxt->env.profiles.chn);

	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		lpNVP_PROFILE_CHN profile = &g_OnvifServerCxt->env.profiles.profile[n];

		ONVIF_S_env_load(OM_PTZ, 100 * n);

		tptz__GetConfigurationsResponse->PTZConfiguration[n].Name = profile->ptz.name;
		tptz__GetConfigurationsResponse->PTZConfiguration[n].UseCount = 1;
		tptz__GetConfigurationsResponse->PTZConfiguration[n].token = profile->ptz.token;
		tptz__GetConfigurationsResponse->PTZConfiguration[n].NodeToken = profile->ptz.node_token;
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultAbsolutePantTiltPositionSpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultAbsolutePantTiltPositionSpace = 
			"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultAbsoluteZoomPositionSpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultAbsoluteZoomPositionSpace = 
			"http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultRelativePanTiltTranslationSpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultRelativePanTiltTranslationSpace = 
			"http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultRelativeZoomTranslationSpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultRelativeZoomTranslationSpace = 
			"http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultContinuousPanTiltVelocitySpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultContinuousPanTiltVelocitySpace = 
			"http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultContinuousZoomVelocitySpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultContinuousZoomVelocitySpace = 
			"http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultPTZSpeed = ONVIF_MALLOC(struct tt__PTZSpeed);
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultPTZSpeed->PanTilt = ONVIF_MALLOC(struct tt__Vector2D);
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultPTZSpeed ->PanTilt->x = profile->ptz.default_pan_speed;
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultPTZSpeed ->PanTilt->y = profile->ptz.default_tilt_speed;
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultPTZSpeed->Zoom = ONVIF_MALLOC(struct tt__Vector1D);
		tptz__GetConfigurationsResponse->PTZConfiguration[n].DefaultPTZSpeed ->Zoom->x = profile->ptz.default_zoom_speed;
	}
		

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresets(struct soap *soap, struct _tptz__GetPresets *tptz__GetPresets, struct _tptz__GetPresetsResponse *tptz__GetPresetsResponse)
{
	bool found = false;
	int i, n, chn, id;
	lpNVP_PROFILE_CHN profile;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for ( i  = 0;  i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++) {
			if (strcmp(tptz__GetPresets->ProfileToken, g_OnvifServerCxt->env.profiles.profile[n].token[i]) == 0) {
				found = true;
				chn  = n;
				id = i;
				break;
			}
		}
	}

	if (found == false) {
		onvif_fault(soap, "ter:InvaldArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}

	ONVIF_S_env_load(OM_PTZ, chn * 100 + id);
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];

	tptz__GetPresetsResponse->__sizePreset = profile->ptz.preset_nr;
	tptz__GetPresetsResponse->Preset = ONVIF_MALLOC_SIZE(struct tt__PTZPreset, tptz__GetPresetsResponse->__sizePreset);
	for ( i  = 0; i < tptz__GetPresetsResponse->__sizePreset; i++) {
		if (profile->ptz.preset[i].name[0] == 0)
			tptz__GetPresetsResponse->Preset[i].Name = NULL;
		else
			tptz__GetPresetsResponse->Preset[i].Name = profile->ptz.preset[i].name;
		tptz__GetPresetsResponse->Preset[i].token = profile->ptz.preset[i].token;
		tptz__GetPresetsResponse->Preset[i].PTZPosition = NULL;
	}
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetPreset(struct soap *soap, struct _tptz__SetPreset *tptz__SetPreset, struct _tptz__SetPresetResponse *tptz__SetPresetResponse)
{
	bool found = false;
	bool preset_found = false;
	int i, n, chn, id;
	int preset_id;
	lpNVP_PROFILE_CHN profile;
	stNVP_CMD cmd;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for ( i  = 0;  i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++) {
			if (strcmp(tptz__SetPreset->ProfileToken, g_OnvifServerCxt->env.profiles.profile[n].token[i]) == 0) {
				found = true;
				chn  = n;
				id = i;
				break;
			}
		}
	}

	if (found == false) {
		onvif_fault(soap, "ter:InvaldArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	

	ONVIF_S_env_load(OM_PTZ, chn * 100 + id);
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];

	if (tptz__SetPreset->PresetToken == NULL) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoToken");
		return SOAP_FAULT;
	}

	for ( i  = 0; i < profile->ptz.preset_nr; i++) {
		if (strcmp(tptz__SetPreset->PresetToken, profile->ptz.preset[i].token) == 0) {
			preset_found = true;
			preset_id = i;
			break;
		}
	}

	if (preset_found == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoToken");
		return SOAP_FAULT;
	}

	if (tptz__SetPreset->PresetName)
		strcpy(profile->ptz.preset[preset_id].name, tptz__SetPreset->PresetName);
	profile->ptz.preset[preset_id].in_use = true;
	ONVIF_S_env_save(OM_PTZ, chn * 100 + id);

	cmd.ptz.index =  preset_id;
	cmd.ptz.cmd = NVP_PTZ_CMD_SET_PRESET;
	NVP_env_cmd(&cmd, OM_PTZ, chn * 100 + id);

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePreset(struct soap *soap, struct _tptz__RemovePreset *tptz__RemovePreset, struct _tptz__RemovePresetResponse *tptz__RemovePresetResponse)
{
	bool found = false;
	bool preset_found = false;
	int i, n, chn, id;
	int preset_id;
	lpNVP_PROFILE_CHN profile;
	stNVP_CMD cmd;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for ( i  = 0;  i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++) {
			if (strcmp(tptz__RemovePreset->ProfileToken, g_OnvifServerCxt->env.profiles.profile[n].token[i]) == 0) {
				found = true;
				chn  = n;
				id = i;
				break;
			}
		}
	}

	if (found == false) {
		onvif_fault(soap, "ter:InvaldArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	

	ONVIF_S_env_load(OM_PTZ, chn * 100 + id);
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];

	if (tptz__RemovePreset->PresetToken == NULL) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoToken");
		return SOAP_FAULT;
	}

	for ( i  = 0; i < profile->ptz.preset_nr; i++) {
		if (strcmp(tptz__RemovePreset->PresetToken, profile->ptz.preset[i].token) == 0) {
			preset_found = true;
			preset_id = i;
			break;
		}
	}

	if (preset_found == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoToken");
		return SOAP_FAULT;
	}

	memset(profile->ptz.preset[preset_id].name, 0, sizeof(profile->ptz.preset[preset_id].name));
	profile->ptz.preset[preset_id].in_use = false;
	ONVIF_S_env_save(OM_PTZ, chn * 100 + id);

	cmd.ptz.index =  preset_id;
	cmd.ptz.cmd = NVP_PTZ_CMD_CLEAR_PRESET;
	NVP_env_cmd(&cmd, OM_PTZ, chn * 100 + id);

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoPreset(struct soap *soap, struct _tptz__GotoPreset *tptz__GotoPreset, struct _tptz__GotoPresetResponse *tptz__GotoPresetResponse)
{
	bool found = false;
	bool preset_found = false;
	int i, n, chn, id;
	int preset_id;
	lpNVP_PROFILE_CHN profile;
	stNVP_CMD cmd;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for ( i  = 0;  i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++) {
			if (strcmp(tptz__GotoPreset->ProfileToken, g_OnvifServerCxt->env.profiles.profile[n].token[i]) == 0) {
				found = true;
				chn  = n;
				id = i;
				break;
			}
		}
	}

	if (found == false) {
		onvif_fault(soap, "ter:InvaldArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	

	ONVIF_S_env_load(OM_PTZ, chn * 100 + id);
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];

	if (tptz__GotoPreset->PresetToken == NULL) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoToken");
		return SOAP_FAULT;
	}

	for ( i  = 0; i < profile->ptz.preset_nr; i++) {
		if (strcmp(tptz__GotoPreset->PresetToken, profile->ptz.preset[i].token) == 0) {
			preset_found = true;
			preset_id = i;
			break;
		}
	}

	if (preset_found == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoToken");
		return SOAP_FAULT;
	}

	cmd.ptz.index =  preset_id;
	cmd.ptz.cmd = NVP_PTZ_CMD_GOTO_PRESET;
	NVP_env_cmd(&cmd, OM_PTZ, chn * 100 + id);

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetStatus(struct soap *soap, struct _tptz__GetStatus *tptz__GetStatus, struct _tptz__GetStatusResponse *tptz__GetStatusResponse)
{
	bool found = false;
	int i, n, chn, id;
	lpNVP_PROFILE_CHN profile;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for ( i  = 0;  i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++) {
			if (strcmp(tptz__GetStatus->ProfileToken, g_OnvifServerCxt->env.profiles.profile[n].token[i]) == 0) {
				found = true;
				chn  = n;
				id = i;
				break;
			}
		}
	}

	if (found == false) {
		onvif_fault(soap, "ter:InvaldArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}

	ONVIF_S_env_load(OM_PTZ, chn * 100 + id);
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];

	tptz__GetStatusResponse->PTZStatus = ONVIF_MALLOC(struct tt__PTZStatus);
	time(&tptz__GetStatusResponse->PTZStatus->UtcTime);
	// FIX ME , ptz status
	tptz__GetStatusResponse->PTZStatus->MoveStatus = ONVIF_MALLOC(struct tt__PTZMoveStatus);
	tptz__GetStatusResponse->PTZStatus->MoveStatus->PanTilt = ONVIF_MALLOC(enum tt__MoveStatus);
	*tptz__GetStatusResponse->PTZStatus->MoveStatus->PanTilt = tt__MoveStatus__UNKNOWN;
		//profile->ptz.pantile_ismoving ? tt__MoveStatus__MOVING : tt__MoveStatus__IDLE;
	tptz__GetStatusResponse->PTZStatus->MoveStatus->Zoom = ONVIF_MALLOC(enum tt__MoveStatus);
	*tptz__GetStatusResponse->PTZStatus->MoveStatus->Zoom = tt__MoveStatus__UNKNOWN;
		//profile->ptz.zoom_ismoving ? tt__MoveStatus__MOVING : tt__MoveStatus__IDLE;
	tptz__GetStatusResponse->PTZStatus->Position = NULL;
	tptz__GetStatusResponse->PTZStatus->__size = 0;
	tptz__GetStatusResponse->PTZStatus->__any = NULL;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfiguration(struct soap *soap, struct _tptz__GetConfiguration *tptz__GetConfiguration, struct _tptz__GetConfigurationResponse *tptz__GetConfigurationResponse)
{
	int n;
	bool found = false;
	lpNVP_PROFILE_CHN profile;

	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		if (strcmp(g_OnvifServerCxt->env.profiles.profile[n].ptz.token, tptz__GetConfiguration->PTZConfigurationToken) == 0) {
			found = true;
			break;
		}
	}

	if (found == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	
	ONVIF_S_env_load(OM_PTZ, 100 * n);
	profile = &g_OnvifServerCxt->env.profiles.profile[n];

	tptz__GetConfigurationResponse->PTZConfiguration = ONVIF_MALLOC(struct tt__PTZConfiguration);

	tptz__GetConfigurationResponse->PTZConfiguration->Name = profile->ptz.name;
	tptz__GetConfigurationResponse->PTZConfiguration->UseCount = 1;
	tptz__GetConfigurationResponse->PTZConfiguration->token = profile->ptz.token;
	tptz__GetConfigurationResponse->PTZConfiguration->NodeToken = profile->ptz.node_token;
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultAbsolutePantTiltPositionSpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultAbsolutePantTiltPositionSpace = 
		"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultAbsoluteZoomPositionSpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultAbsoluteZoomPositionSpace = 
		"http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultRelativePanTiltTranslationSpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultRelativePanTiltTranslationSpace = 
		"http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultRelativeZoomTranslationSpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultRelativeZoomTranslationSpace = 
		"http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace = 
		"http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultContinuousZoomVelocitySpace = ONVIF_MALLOC_SIZE(char, NVP_MAX_URI_SIZE);
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultContinuousZoomVelocitySpace = 
		"http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultPTZSpeed = ONVIF_MALLOC(struct tt__PTZSpeed);
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultPTZSpeed->PanTilt = ONVIF_MALLOC(struct tt__Vector2D);
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultPTZSpeed ->PanTilt->x = profile->ptz.default_pan_speed;
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultPTZSpeed ->PanTilt->y = profile->ptz.default_tilt_speed;
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultPTZSpeed->Zoom = ONVIF_MALLOC(struct tt__Vector1D);
	tptz__GetConfigurationResponse->PTZConfiguration->DefaultPTZSpeed ->Zoom->x = profile->ptz.default_zoom_speed;
		

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNodes(struct soap *soap, struct _tptz__GetNodes *tptz__GetNodes, struct _tptz__GetNodesResponse *tptz__GetNodesResponse)
{
	int n;

	tptz__GetNodesResponse->__sizePTZNode= g_OnvifServerCxt->env.profiles.chn;
	tptz__GetNodesResponse->PTZNode = ONVIF_MALLOC_SIZE(struct tt__PTZNode, g_OnvifServerCxt->env.profiles.chn);

	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		lpNVP_PROFILE_CHN profile = &g_OnvifServerCxt->env.profiles.profile[n];

		ONVIF_S_env_load(OM_PTZ, 100 * n);

		tptz__GetNodesResponse->PTZNode[n].Name = profile->ptz.node_name;
		tptz__GetNodesResponse->PTZNode[n].token = profile->ptz.node_token;
		tptz__GetNodesResponse->PTZNode[n].MaximumNumberOfPresets = profile->ptz.preset_nr;
		tptz__GetNodesResponse->PTZNode[n].HomeSupported = nfalse;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces = ONVIF_MALLOC(struct tt__PTZSpaces);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsolutePanTiltPositionSpace = 
			ONVIF_MALLOC_SIZE(struct tt__Space2DDescription, 1);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsolutePanTiltPositionSpace->URI = 
			"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsolutePanTiltPositionSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsolutePanTiltPositionSpace->XRange->Min = -1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsolutePanTiltPositionSpace->XRange->Max = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsolutePanTiltPositionSpace->YRange = ONVIF_MALLOC(struct tt__FloatRange);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsolutePanTiltPositionSpace->YRange->Min = -1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsolutePanTiltPositionSpace->YRange->Max = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsoluteZoomPositionSpace = 
			ONVIF_MALLOC_SIZE(struct tt__Space1DDescription, 1);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsoluteZoomPositionSpace->URI = 
			"http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsoluteZoomPositionSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsoluteZoomPositionSpace->XRange->Min = 0;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->AbsoluteZoomPositionSpace->XRange->Max = 1;

		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativePanTiltTranslationSpace = 
			ONVIF_MALLOC_SIZE(struct tt__Space2DDescription, 1);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativePanTiltTranslationSpace->URI = 
			"http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativePanTiltTranslationSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativePanTiltTranslationSpace->XRange->Min = -1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativePanTiltTranslationSpace->XRange->Max = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativePanTiltTranslationSpace->YRange = ONVIF_MALLOC(struct tt__FloatRange);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativePanTiltTranslationSpace->YRange->Min = -1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativePanTiltTranslationSpace->YRange->Max = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativeZoomTranslationSpace = 
			ONVIF_MALLOC_SIZE(struct tt__Space1DDescription, 1);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativeZoomTranslationSpace->URI = 
			"http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativeZoomTranslationSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativeZoomTranslationSpace->XRange->Min = -1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->RelativeZoomTranslationSpace->XRange->Max = 1;

				tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace= 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace = 
			ONVIF_MALLOC_SIZE(struct tt__Space2DDescription, 1);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->URI = 
			"http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->XRange->Min = -1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->XRange->Max = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->YRange = ONVIF_MALLOC(struct tt__FloatRange);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->YRange->Min = -1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->YRange->Max = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->__sizeContinuousZoomVelocitySpace = 1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousZoomVelocitySpace = 
			ONVIF_MALLOC_SIZE(struct tt__Space1DDescription, 1);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousZoomVelocitySpace->URI = 
			"http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousZoomVelocitySpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousZoomVelocitySpace->XRange->Min = -1;
		tptz__GetNodesResponse->PTZNode[n].SupportedPTZSpaces->ContinuousZoomVelocitySpace->XRange->Max = 1;

		tptz__GetNodesResponse->PTZNode[n].__sizeAuxiliaryCommands = 0;
		tptz__GetNodesResponse->PTZNode[n].AuxiliaryCommands = NULL;
		
	}
		

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNode(struct soap *soap, struct _tptz__GetNode *tptz__GetNode, struct _tptz__GetNodeResponse *tptz__GetNodeResponse)
{
	int n;
	bool found_node = false;
	lpNVP_PROFILE_CHN profile;
	
	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		if (strcmp(g_OnvifServerCxt->env.profiles.profile[n].ptz.node_token, tptz__GetNode->NodeToken) == 0) {
			found_node = true;
			break;
		}
	}

	if (found_node == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoEntity");
		return SOAP_FAULT;
	}
	
	profile = &g_OnvifServerCxt->env.profiles.profile[n];

	ONVIF_S_env_load(OM_PTZ, 100 * n);

	tptz__GetNodeResponse->PTZNode = ONVIF_MALLOC(struct tt__PTZNode);

	tptz__GetNodeResponse->PTZNode->Name = profile->ptz.node_name;
	tptz__GetNodeResponse->PTZNode->token = profile->ptz.node_token;
	tptz__GetNodeResponse->PTZNode->MaximumNumberOfPresets = profile->ptz.preset_nr;
	tptz__GetNodeResponse->PTZNode->HomeSupported = nfalse;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces = ONVIF_MALLOC(struct tt__PTZSpaces);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space2DDescription, 1);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace->URI = 
		"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace->XRange->Min = -1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace->XRange->Max = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace->YRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace->YRange->Min = -1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace->YRange->Max = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space1DDescription, 1);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace->URI = 
		"http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace->XRange->Min = 0;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace->XRange->Max = 1;

	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space2DDescription, 1);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace->URI = 
		"http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace->XRange->Min = -1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace->XRange->Max = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace->YRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace->YRange->Min = -1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace->YRange->Max = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space1DDescription, 1);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace->URI = 
		"http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace->XRange->Min = -1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace->XRange->Max = 1;

			tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace= 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space2DDescription, 1);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->URI = 
		"http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->XRange->Min = -1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->XRange->Max = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->YRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->YRange->Min = -1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace->YRange->Max = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeContinuousZoomVelocitySpace = 1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space1DDescription, 1);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace->URI = 
		"http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace->XRange->Min = -1;
	tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace->XRange->Max = 1;

	tptz__GetNodeResponse->PTZNode->__sizeAuxiliaryCommands = 0;
	tptz__GetNodeResponse->PTZNode->AuxiliaryCommands = NULL;
		
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetConfiguration(struct soap *soap, struct _tptz__SetConfiguration *tptz__SetConfiguration, struct _tptz__SetConfigurationResponse *tptz__SetConfigurationResponse)
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurationOptions(struct soap *soap, struct _tptz__GetConfigurationOptions *tptz__GetConfigurationOptions, struct _tptz__GetConfigurationOptionsResponse *tptz__GetConfigurationOptionsResponse)
{
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions = ONVIF_MALLOC(struct tt__PTZConfigurationOptions);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout = ONVIF_MALLOC(struct tt__DurationRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout->Max = 3600;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout->Min = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces = ONVIF_MALLOC(struct tt__PTZSpaces);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsolutePanTiltPositionSpace = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space2DDescription, 1);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->URI = 
		"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->XRange->Min = -1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->XRange->Max = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->YRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->YRange->Min = -1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace->YRange->Max = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsoluteZoomPositionSpace = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space1DDescription, 1);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace->URI = 
		"http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace->XRange->Min = 0;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace->XRange->Max = 1;

	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativePanTiltTranslationSpace = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space2DDescription, 1);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->URI = 
		"http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->XRange->Min = -1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->XRange->Max = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->YRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->YRange->Min = -1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace->YRange->Max = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativeZoomTranslationSpace = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space1DDescription, 1);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace->URI = 
		"http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace->XRange->Min = -1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace->XRange->Max = 1;

			tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousPanTiltVelocitySpace= 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space2DDescription, 1);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->URI = 
		"http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->XRange->Min = -1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->XRange->Max = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->YRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->YRange->Min = -1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace->YRange->Max = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousZoomVelocitySpace = 1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace = 
		ONVIF_MALLOC_SIZE(struct tt__Space1DDescription, 1);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace->URI = 
		"http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace->XRange->Min = -1;
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace->XRange->Max = 1;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoHomePosition(struct soap *soap, struct _tptz__GotoHomePosition *tptz__GotoHomePosition, struct _tptz__GotoHomePositionResponse *tptz__GotoHomePositionResponse)
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetHomePosition(struct soap *soap, struct _tptz__SetHomePosition *tptz__SetHomePosition, struct _tptz__SetHomePositionResponse *tptz__SetHomePositionResponse)
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__ContinuousMove(struct soap *soap, struct _tptz__ContinuousMove *tptz__ContinuousMove, struct _tptz__ContinuousMoveResponse *tptz__ContinuousMoveResponse)
{
	bool found = false;
	int i, n, chn, id;
	stNVP_CMD cmd;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for ( i  = 0;  i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++) {
			if (strcmp(tptz__ContinuousMove->ProfileToken, g_OnvifServerCxt->env.profiles.profile[n].token[i]) == 0) {
				found = true;
				chn  = n;
				id = i;
				break;
			}
		}
	}

	if (found == false) {
		onvif_fault(soap, "ter:InvaldArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}

	if (tptz__ContinuousMove->Velocity == NULL) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidVelocity");
		return SOAP_FAULT;
	}

	if (tptz__ContinuousMove->Velocity->PanTilt) {
		if (tptz__ContinuousMove->Velocity->PanTilt->x < 0)
			cmd.ptz.cmd = NVP_PTZ_CMD_LEFT;
		else if (tptz__ContinuousMove->Velocity->PanTilt->x > 0)
			cmd.ptz.cmd = NVP_PTZ_CMD_RIGHT;
		if (tptz__ContinuousMove->Velocity->PanTilt->y < 0)
			cmd.ptz.cmd = NVP_PTZ_CMD_DOWN;
		else if (tptz__ContinuousMove->Velocity->PanTilt->y > 0)
			cmd.ptz.cmd = NVP_PTZ_CMD_UP;
	}
	if (tptz__ContinuousMove->Velocity->Zoom) {
		if (tptz__ContinuousMove->Velocity->Zoom->x < 0)
			cmd.ptz.cmd = NVP_PTZ_CMD_ZOOM_OUT;
		else if (tptz__ContinuousMove->Velocity->Zoom->x > 0)
			cmd.ptz.cmd = NVP_PTZ_CMD_ZOOM_IN;
	}

	NVP_env_cmd(&cmd, OM_PTZ, chn * 100 + id);
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RelativeMove(struct soap *soap, struct _tptz__RelativeMove *tptz__RelativeMove, struct _tptz__RelativeMoveResponse *tptz__RelativeMoveResponse)
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SendAuxiliaryCommand(struct soap *soap, struct _tptz__SendAuxiliaryCommand *tptz__SendAuxiliaryCommand, struct _tptz__SendAuxiliaryCommandResponse *tptz__SendAuxiliaryCommandResponse)
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__AbsoluteMove(struct soap *soap, struct _tptz__AbsoluteMove *tptz__AbsoluteMove, struct _tptz__AbsoluteMoveResponse *tptz__AbsoluteMoveResponse)
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__Stop(struct soap *soap, struct _tptz__Stop *tptz__Stop, struct _tptz__StopResponse *tptz__StopResponse)
{
	bool found = false;
	int i, n, chn, id;
	stNVP_CMD cmd;

	memset(&cmd, 0, sizeof(cmd));

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for ( i  = 0;  i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++) {
			if (strcmp(tptz__Stop->ProfileToken, g_OnvifServerCxt->env.profiles.profile[n].token[i]) == 0) {
				found = true;
				chn  = n;
				id = i;
				break;
			}
		}
	}

	if (found == false) {
		onvif_fault(soap, "ter:InvaldArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}

	if (tptz__Stop->PanTilt) {
		if (*tptz__Stop->PanTilt == ntrue) {
			cmd.ptz.cmd = NVP_PTZ_CMD_STOP;
		}
	}
	if (tptz__Stop->Zoom) {
		if (*tptz__Stop->Zoom == ntrue) {
			cmd.ptz.cmd = NVP_PTZ_CMD_STOP;
		}
	}

	if (cmd.ptz.cmd == NVP_PTZ_CMD_STOP)
		NVP_env_cmd(&cmd, OM_PTZ, chn * 100 + id);
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTours(struct soap *soap, struct _tptz__GetPresetTours *tptz__GetPresetTours, struct _tptz__GetPresetToursResponse *tptz__GetPresetToursResponse) 
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTour(struct soap *soap, struct _tptz__GetPresetTour *tptz__GetPresetTour, struct _tptz__GetPresetTourResponse *tptz__GetPresetTourResponse) 
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTourOptions(struct soap *soap, struct _tptz__GetPresetTourOptions *tptz__GetPresetTourOptions, struct _tptz__GetPresetTourOptionsResponse *tptz__GetPresetTourOptionsResponse) 
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__CreatePresetTour(struct soap *soap, struct _tptz__CreatePresetTour *tptz__CreatePresetTour, struct _tptz__CreatePresetTourResponse *tptz__CreatePresetTourResponse) 
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__ModifyPresetTour(struct soap *soap, struct _tptz__ModifyPresetTour *tptz__ModifyPresetTour, struct _tptz__ModifyPresetTourResponse *tptz__ModifyPresetTourResponse) 
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__OperatePresetTour(struct soap *soap, struct _tptz__OperatePresetTour *tptz__OperatePresetTour, struct _tptz__OperatePresetTourResponse *tptz__OperatePresetTourResponse) 
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePresetTour(struct soap *soap, struct _tptz__RemovePresetTour *tptz__RemovePresetTour, struct _tptz__RemovePresetTourResponse *tptz__RemovePresetTourResponse) 
{
	onvif_fault(soap, "ter:ActionNotSupported", "ter:PTZNotSupported");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetCompatibleConfigurations(struct soap *soap, struct _tptz__GetCompatibleConfigurations *tptz__GetCompatibleConfigurations, struct _tptz__GetCompatibleConfigurationsResponse *tptz__GetCompatibleConfigurationsResponse) 
{
	return SOAP_FAULT; 
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetServiceCapabilities(struct soap *soap, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities, struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
{
	ONVIF_TRACE("nProfileCount:%d\n", g_OnvifServerCxt->env.profiles.chn);

	trt__GetServiceCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct trt__Capabilities);
	trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities = ONVIF_MALLOC(struct trt__ProfileCapabilities);
	trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles = ONVIF_MALLOC(int);
	*trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles = g_OnvifServerCxt->env.profiles.chn;

	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities = ONVIF_MALLOC(struct trt__StreamingCapabilities);
	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTPMulticast = &nfalse;
	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORETCP = &ntrue;
	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = &ntrue;
	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NonAggregateControl = &nfalse;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources(struct soap *soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
	int n;

	trt__GetVideoSourcesResponse->__sizeVideoSources = g_OnvifServerCxt->env.profiles.chn;
	trt__GetVideoSourcesResponse->VideoSources = ONVIF_MALLOC_SIZE(struct tt__VideoSource, g_OnvifServerCxt->env.profiles.chn);

	for ( n  = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		//
		ONVIF_S_env_load(OM_VSRC, n * 100);

		trt__GetVideoSourcesResponse->VideoSources[n].token = g_OnvifServerCxt->env.profiles.profile[n].v_source.token;
		trt__GetVideoSourcesResponse->VideoSources[n].Framerate = g_OnvifServerCxt->env.profiles.profile[n].v_source.fps; 
		trt__GetVideoSourcesResponse->VideoSources[n].Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
		trt__GetVideoSourcesResponse->VideoSources[n].Resolution->Width = g_OnvifServerCxt->env.profiles.profile[n].v_source.resolution.width;
		trt__GetVideoSourcesResponse->VideoSources[n].Resolution->Height = g_OnvifServerCxt->env.profiles.profile[n].v_source.resolution.height;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging = ONVIF_MALLOC(struct tt__ImagingSettings);
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->BacklightCompensation = ONVIF_MALLOC(struct tt__BacklightCompensation);
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->BacklightCompensation->Level = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Brightness = ONVIF_MALLOC_SIZE(float, 1);
		*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Brightness = g_OnvifServerCxt->env.profiles.profile[n].v_source.image.color.brightness;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->ColorSaturation = ONVIF_MALLOC_SIZE(float, 1);
		*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->ColorSaturation = g_OnvifServerCxt->env.profiles.profile[n].v_source.image.color.saturation;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Contrast = ONVIF_MALLOC_SIZE(float, 1);
		*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Contrast = g_OnvifServerCxt->env.profiles.profile[n].v_source.image.color.contrast;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Sharpness = ONVIF_MALLOC_SIZE(float, 1);
		*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Sharpness = g_OnvifServerCxt->env.profiles.profile[n].v_source.image.color.sharpness;

		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure = ONVIF_MALLOC(struct tt__Exposure);
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Mode =  tt__ExposureMode__MANUAL;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Priority = tt__ExposurePriority__FrameRate;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Window = ONVIF_MALLOC(struct tt__Rectangle);
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Window->bottom  = ONVIF_MALLOC(float);
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Window->top = ONVIF_MALLOC(float);
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Window->right = ONVIF_MALLOC(float);
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Window->left = ONVIF_MALLOC(float);
		*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Window->bottom = 1;
		*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Window->top = 0;
		*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Window->right = 1;
		*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Window->left = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->MinExposureTime = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->MaxExposureTime = 1;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->MinGain = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->MaxGain = 1;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->MinIris = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->MaxIris = 1;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->ExposureTime = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Gain = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Exposure->Iris = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Focus = ONVIF_MALLOC(struct tt__FocusConfiguration);
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Focus->AutoFocusMode = tt__AutoFocusMode__MANUAL;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Focus->DefaultSpeed = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Focus->NearLimit = 0;
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->Focus->FarLimit = 0;

		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->IrCutFilter = ONVIF_MALLOC(enum tt__IrCutFilterMode);
		if(NVP_IRCUT_MODE_AUTO == g_OnvifServerCxt->env.profiles.profile[n].v_source.image.ircut.ircut_mode){
			*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->IrCutFilter = tt__IrCutFilterMode__AUTO;
		}
		else if(NVP_IRCUT_MODE_NIGHT == g_OnvifServerCxt->env.profiles.profile[n].v_source.image.ircut.ircut_mode){
			*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->IrCutFilter  = tt__IrCutFilterMode__ON;
		}
		else{
			*trt__GetVideoSourcesResponse->VideoSources[n].Imaging->IrCutFilter = tt__IrCutFilterMode__OFF;
		}
		trt__GetVideoSourcesResponse->VideoSources[n].Imaging->WideDynamicRange = NULL;
	}

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources(struct soap *soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
	int n;

	trt__GetAudioSourcesResponse->__sizeAudioSources = g_OnvifServerCxt->env.profiles.chn;
	trt__GetAudioSourcesResponse->AudioSources = ONVIF_MALLOC_SIZE(struct tt__AudioSource, g_OnvifServerCxt->env.profiles.chn);

	for ( n  = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		//
		ONVIF_S_env_load(OM_AIN, n * 100);

		trt__GetAudioSourcesResponse->AudioSources[n].token = g_OnvifServerCxt->env.profiles.profile[n].ain.token;
		trt__GetAudioSourcesResponse->AudioSources[n].Channels = 1;
	}
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs(struct soap *soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile(struct soap *soap, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile(struct soap *soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
	lpNVP_PROFILE_CHN profile;
	int n,i, chn = 0, id = 0;
	int Ptoken_status = 0;
	
	ONVIF_TRACE("GetProfile:%s\n", trt__GetProfile->ProfileToken);

	if((trt__GetProfile->ProfileToken) == NULL)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidInputToken"); 
		return SOAP_FAULT;
	}
	if(strcmp(trt__GetProfile->ProfileToken, "") == 0)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile"); 
		return SOAP_FAULT;
	}

	/* Check if ProfileToken Exist or Not */	
	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for(i = 0; i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++)
		{
			if(strcmp(trt__GetProfile->ProfileToken, g_OnvifServerCxt->env.profiles.profile[n].token[i]) == 0)
			{
				Ptoken_status = 1;
				chn = n;
				id = i;
				break;
			}
		}
	}
	if(!Ptoken_status)
	{
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoProfile"); 
		return SOAP_FAULT;
	}

	ONVIF_S_env_load(OM_PROFILE, 100 * chn + id);
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];
	
	trt__GetProfileResponse->Profile = ONVIF_MALLOC(struct tt__Profile);
	trt__GetProfileResponse->Profile->Name = profile->name[id]; 
	trt__GetProfileResponse->Profile->token = profile->token[id];
	trt__GetProfileResponse->Profile->fixed = ONVIF_MALLOC(enum xsd__boolean);
	*(trt__GetProfileResponse->Profile->fixed) = ntrue;
	/* VideoSourceConfiguration */
	trt__GetProfileResponse->Profile->VideoSourceConfiguration = ONVIF_MALLOC(struct tt__VideoSourceConfiguration);
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name = profile->vin[id].name;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->token = profile->vin[id].token;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken = profile->v_source.token;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->UseCount = 1;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->x = profile->vin[id].rect.nX;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->y = profile->vin[id].rect.nY;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->width = profile->vin[id].rect.width;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->height = profile->vin[id].rect.height;
	/* AudioSourceConfiguration */
	trt__GetProfileResponse->Profile->AudioSourceConfiguration = ONVIF_MALLOC(struct tt__AudioSourceConfiguration);
	trt__GetProfileResponse->Profile->AudioSourceConfiguration->Name = profile->ain_name[id];
	trt__GetProfileResponse->Profile->AudioSourceConfiguration->token = profile->ain_token[id];
	trt__GetProfileResponse->Profile->AudioSourceConfiguration->SourceToken = profile->ain.token;
	/*VideoEncoderConfiguration */
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration = ONVIF_MALLOC(struct tt__VideoEncoderConfiguration);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name =profile->venc[id].enc_name;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token = profile->venc[id].enc_token;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->UseCount = 1;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->SessionTimeout = ONVIF_SESSION_TIMEOUT;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Quality = (float)profile->venc[id].enc_quality;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);

	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Width = profile->venc[id].width;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Height = profile->venc[id].height;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit = profile->venc[id].enc_fps;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->EncodingInterval = profile->venc[id].enc_interval;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->BitrateLimit = profile->venc[id].enc_bps;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264 = ONVIF_MALLOC(struct tt__H264Configuration);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->GovLength = profile->venc[id].enc_gov;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = profile->venc[id].enc_profile;

	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 20);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
	snprintf(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address, 20, "235.255.255.255");
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Port = 3703;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->TTL = 0;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->AutoStart = nfalse;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Encoding = profile->venc[id].enc_type;
	/* AudioEncoderConfiguration */
	if (id < profile->aenc_nr) {
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration = ONVIF_MALLOC(struct tt__AudioEncoderConfiguration);
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Name = profile->aenc[id].name;
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->token = profile->aenc[id].token;
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Encoding = profile->aenc[id].enc_type;
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->UseCount = 1;
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Bitrate = profile->aenc[id].sample_size;
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->SampleRate = profile->aenc[id].sample_rate;
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->SessionTimeout = ONVIF_SESSION_TIMEOUT;
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 20);
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
		snprintf(trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address->IPv4Address, 20, "235.255.255.255");
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Port = 3703;
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->TTL = 0;
		trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->AutoStart = nfalse;	
	}
	/* PTZConfiguration */ 
	trt__GetProfileResponse->Profile->PTZConfiguration = ONVIF_MALLOC(struct tt__PTZConfiguration);
	trt__GetProfileResponse->Profile->PTZConfiguration->Name = profile->ptz.name;
	trt__GetProfileResponse->Profile->PTZConfiguration->UseCount = 1;
	trt__GetProfileResponse->Profile->PTZConfiguration->token = profile->ptz.token;
	trt__GetProfileResponse->Profile->PTZConfiguration->NodeToken = profile->ptz.node_token;
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultAbsolutePantTiltPositionSpace="http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultAbsoluteZoomPositionSpace="http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultRelativePanTiltTranslationSpace="http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultRelativeZoomTranslationSpace="http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace="http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultContinuousZoomVelocitySpace="http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed = ONVIF_MALLOC(struct tt__PTZSpeed);
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt = ONVIF_MALLOC(struct tt__Vector2D);
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt->x = 0.5;
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt->y = 0.5;
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt->space = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->Zoom = ONVIF_MALLOC(struct tt__Vector1D);
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->Zoom->x = 0.5;
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->Zoom->space = "http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZTimeout = ONVIF_MALLOC(LONG64);
	*trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZTimeout = 60000; //60s
	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits = ONVIF_MALLOC(struct tt__PanTiltLimits);
	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range = ONVIF_MALLOC(struct tt__Space2DDescription);
	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->URI = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->XRange->Min = -1;
	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->XRange->Max = 1;
	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->YRange = ONVIF_MALLOC(struct tt__FloatRange);
	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->YRange->Min = -1;
	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->YRange->Max = 1;
	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits = ONVIF_MALLOC(struct tt__ZoomLimits);
	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range = ONVIF_MALLOC(struct tt__Space1DDescription);
	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->URI = "http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->XRange = ONVIF_MALLOC(struct tt__FloatRange);
	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->XRange->Min = -1;
	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->XRange->Max = 1;
	/* MetadataConfiguration */
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles(struct soap *soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
	const char *layout_xml = 
		"<tt:CellLayout Columns=\"%d\" Rows=\"%d\"><tt:Transformation><tt:Translate x=\"%f\" y=\"%f\"/>"
		"<tt:Scale x=\"%f\" y=\"%f\"/>"
		"</tt:Transformation>"
		"</tt:CellLayout>";
	lpNVP_PROFILE_CHN profile;
	int nr_profile = 0;
	int i, n = 0, id;
	char md_layout[1500];
	struct tt__Profile *profile_list = NULL;
	

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		nr_profile += g_OnvifServerCxt->env.profiles.profile[n].profile_nr;
	}
	ONVIF_TRACE(" nprofile:%d",  nr_profile);

	ONVIF_S_env_load(OM_PROFILES, 0);
	
	profile_list = ONVIF_MALLOC_SIZE(struct tt__Profile,  nr_profile);
	trt__GetProfilesResponse->__sizeProfiles = nr_profile;

	id  = 0;
	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		profile = &g_OnvifServerCxt->env.profiles.profile[n];
		//
		md_celllayout_hex2s(md_layout, sizeof(md_layout), (uint8_t *)profile->md.grid.granularity, 
			(profile->md.grid.rowGranularity * profile->md.grid.columnGranularity + 7)/8);

		for (i = 0; i < profile->profile_nr; i++) {
			
			profile_list[id].Name = profile->name[i]; 
			profile_list[id].token = profile->token[i];
			profile_list[id].fixed = ONVIF_MALLOC(enum xsd__boolean);
			*(profile_list[id].fixed) = ntrue;
			/* VideoSourceConfiguration */
			profile_list[id].VideoSourceConfiguration = ONVIF_MALLOC(struct tt__VideoSourceConfiguration);
			profile_list[id].VideoSourceConfiguration->Name = profile->vin[i].name;
			profile_list[id].VideoSourceConfiguration->token = profile->vin[i].token;
			profile_list[id].VideoSourceConfiguration->SourceToken = profile->v_source.token;
			profile_list[id].VideoSourceConfiguration->UseCount = 1;
			
			profile_list[id].VideoSourceConfiguration->Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
			profile_list[id].VideoSourceConfiguration->Bounds->x = profile->vin[id].rect.nX;
			profile_list[id].VideoSourceConfiguration->Bounds->y = profile->vin[id].rect.nY;
			profile_list[id].VideoSourceConfiguration->Bounds->width = profile->vin[id].rect.width;
			profile_list[id].VideoSourceConfiguration->Bounds->height = profile->vin[id].rect.height;

			/* AudioSourceConfiguration */
			/*
			profile_list[id].AudioSourceConfiguration = ONVIF_MALLOC(struct tt__AudioSourceConfiguration);
			profile_list[id].AudioSourceConfiguration->Name = g_OnvifConf.Profile[i].ASCName;
			profile_list[id].AudioSourceConfiguration->token = g_OnvifConf.Profile[i].ASCToken;
			profile_list[id].AudioSourceConfiguration->SourceToken = g_OnvifConf.Profile[i].ASCSourceToken;
			*/
			/* VideoEncoderConfiguration */
			profile_list[id].VideoEncoderConfiguration = ONVIF_MALLOC(struct tt__VideoEncoderConfiguration);
			profile_list[id].VideoEncoderConfiguration->Name = profile->venc[i].enc_name;
			profile_list[id].VideoEncoderConfiguration->token = profile->venc[i].enc_token;
			profile_list[id].VideoEncoderConfiguration->UseCount = profile->venc[i].user_count;
			profile_list[id].VideoEncoderConfiguration->SessionTimeout = ONVIF_SESSION_TIMEOUT;
			profile_list[id].VideoEncoderConfiguration->Quality = profile->venc[i].enc_quality;
			profile_list[id].VideoEncoderConfiguration->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
			profile_list[id].VideoEncoderConfiguration->Resolution->Width = profile->venc[i].width;
			profile_list[id].VideoEncoderConfiguration->Resolution->Height = profile->venc[i].height;
			profile_list[id].VideoEncoderConfiguration->RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
			profile_list[id].VideoEncoderConfiguration->RateControl->FrameRateLimit = profile->venc[i].enc_fps;
			profile_list[id].VideoEncoderConfiguration->RateControl->EncodingInterval = profile->venc[i].enc_interval;
			profile_list[id].VideoEncoderConfiguration->RateControl->BitrateLimit = profile->venc[i].enc_bps;
			profile_list[id].VideoEncoderConfiguration->H264 = ONVIF_MALLOC(struct tt__H264Configuration);
			profile_list[id].VideoEncoderConfiguration->H264->GovLength = profile->venc[i].enc_gov;
			profile_list[id].VideoEncoderConfiguration->H264->H264Profile = profile->venc[i].enc_profile;
			profile_list[id].VideoEncoderConfiguration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
			profile_list[id].VideoEncoderConfiguration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
			profile_list[id].VideoEncoderConfiguration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
			profile_list[id].VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
			snprintf(profile_list[id].VideoEncoderConfiguration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
			profile_list[id].VideoEncoderConfiguration->Multicast->Port = 3703;
			profile_list[id].VideoEncoderConfiguration->Multicast->TTL = 0;
			profile_list[id].VideoEncoderConfiguration->Multicast->AutoStart = nfalse;
			profile_list[id].VideoEncoderConfiguration->Encoding = profile->venc[i].enc_type;
			/* AudioEncoderConfiguration 
			profile_list[id].AudioEncoderConfiguration = ONVIF_MALLOC(struct tt__AudioEncoderConfiguration);
			profile_list[id].AudioEncoderConfiguration->Name = g_OnvifConf.Profile[i].AECName;
			profile_list[id].AudioEncoderConfiguration->token = g_OnvifConf.Profile[i].AECToken;
			profile_list[id].AudioEncoderConfiguration->Encoding = g_OnvifConf.Profile[i].AECEncoding;
			profile_list[id].AudioEncoderConfiguration->UseCount = g_OnvifConf.Profile[i].AECUseCount;
			profile_list[id].AudioEncoderConfiguration->Bitrate = 8;
			profile_list[id].AudioEncoderConfiguration->SampleRate = g_OnvifConf.Profile[i].AECSampleRate;
			profile_list[id].AudioEncoderConfiguration->SessionTimeout = ONVIF_SESSION_TIMEOUT;
			profile_list[id].AudioEncoderConfiguration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
			profile_list[id].AudioEncoderConfiguration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
			profile_list[id].AudioEncoderConfiguration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
			profile_list[id].AudioEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
			snprintf(profile_list[id].AudioEncoderConfiguration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
			profile_list[id].AudioEncoderConfiguration->Multicast->Port = 3703;
			profile_list[id].AudioEncoderConfiguration->Multicast->TTL = 0;
			profile_list[id].AudioEncoderConfiguration->Multicast->AutoStart = nfalse;
			*/
			/* Video Analytics configuration */		
			profile_list[id].VideoAnalyticsConfiguration = ONVIF_MALLOC(struct tt__VideoAnalyticsConfiguration);
			profile_list[id].VideoAnalyticsConfiguration->UseCount = 2;
			profile_list[id].VideoAnalyticsConfiguration->Name = profile->van.name;
			profile_list[id].VideoAnalyticsConfiguration->token = profile->van.token;
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration = ONVIF_MALLOC(struct tt__AnalyticsEngineConfiguration);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__sizeAnalyticsModule = 1;
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule = ONVIF_MALLOC_SIZE(struct tt__Config, 1);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Name = profile->md.module_name;
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Type = "tt:CellMotionEngine";
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters = ONVIF_MALLOC(struct tt__ItemList);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeSimpleItem = 1;
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem = ONVIF_MALLOC(struct  _tt__ItemList_SimpleItem); 
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Name = "Sensitivity";
			onvif_put_int(soap, &(profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem->Value),
				(int)profile->md.grid.sensitivity);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__sizeElementItem = 1;
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem = ONVIF_MALLOC(struct _tt__ItemList_ElementItem);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->Name = "Layout";
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->__any =  ONVIF_MALLOC_SIZE(char , 1024);
			snprintf(profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->ElementItem->__any, 1024,
				layout_xml, profile->md.grid.columnGranularity, profile->md.grid.rowGranularity, 
				(float)-1,(float) -1,
				(float)2.0/profile->v_source.resolution.width, (float)2.0/profile->v_source.resolution.height);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->__anyAttribute = NULL;
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->Extension = NULL;
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->__anyAttribute = NULL;
			profile_list[id].VideoAnalyticsConfiguration->RuleEngineConfiguration = ONVIF_MALLOC(struct tt__RuleEngineConfiguration);
			profile_list[id].VideoAnalyticsConfiguration->RuleEngineConfiguration->__sizeRule = 1;
			profile_list[id].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule = ONVIF_MALLOC(struct tt__Config);
			profile_list[id].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule->Name = profile->md.rule_name;
			profile_list[id].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule->Type = "tt:CellMotionDetector";
			profile_list[id].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule->Parameters = ONVIF_MALLOC(struct tt__ItemList);
			profile_list[id].VideoAnalyticsConfiguration->RuleEngineConfiguration->Rule->Parameters->__sizeSimpleItem = 4;
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem = ONVIF_MALLOC_SIZE(struct _tt__ItemList_SimpleItem, 4);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Name = "MinCount";
			onvif_put_int(soap, &(profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[0].Value),
				profile->md.grid.threshold);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[1].Name = "AlarmOnDelay";
			onvif_put_int(soap, &(profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[1].Value),
				profile->md.delay_on_alarm);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[2].Name = "AlarmOffDelay";
			onvif_put_int(soap, &(profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[2].Value),
				profile->md.delay_off_alarm);
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[3].Name = "ActiveCells";
			profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[3].Value = (char *)ONVIF_MALLOC_SIZE(char, 2000);
			//
			snprintf(profile_list[id].VideoAnalyticsConfiguration->AnalyticsEngineConfiguration->AnalyticsModule->Parameters->SimpleItem[3].Value , 2000,
				"%s",
				md_layout);
			profile_list[id].VideoAnalyticsConfiguration->__size = 0;
			profile_list[id].VideoAnalyticsConfiguration->__any = NULL;
			profile_list[id].VideoAnalyticsConfiguration->__anyAttribute = NULL;
			
			/* PTZConfiguration */
			profile_list[id].PTZConfiguration = ONVIF_MALLOC(struct tt__PTZConfiguration);
			profile_list[id].PTZConfiguration->Name = profile->ptz.name;
			profile_list[id].PTZConfiguration->UseCount = 1;
			profile_list[id].PTZConfiguration->token = profile->ptz.token;
			profile_list[id].PTZConfiguration->NodeToken = profile->ptz.node_token;
		 	profile_list[id].PTZConfiguration->DefaultAbsolutePantTiltPositionSpace="http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
		 	profile_list[id].PTZConfiguration->DefaultAbsoluteZoomPositionSpace="http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
		 	profile_list[id].PTZConfiguration->DefaultRelativePanTiltTranslationSpace="http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
		 	profile_list[id].PTZConfiguration->DefaultRelativeZoomTranslationSpace="http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
		 	profile_list[id].PTZConfiguration->DefaultContinuousPanTiltVelocitySpace="http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
		 	profile_list[id].PTZConfiguration->DefaultContinuousZoomVelocitySpace="http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
		 	profile_list[id].PTZConfiguration->DefaultPTZSpeed = ONVIF_MALLOC(struct tt__PTZSpeed);
		 	profile_list[id].PTZConfiguration->DefaultPTZSpeed->PanTilt = ONVIF_MALLOC(struct tt__Vector2D);
		 	profile_list[id].PTZConfiguration->DefaultPTZSpeed->PanTilt->x = 0.5;
		 	profile_list[id].PTZConfiguration->DefaultPTZSpeed->PanTilt->y = 0.5;
		 	profile_list[id].PTZConfiguration->DefaultPTZSpeed->PanTilt->space = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace";
		 	profile_list[id].PTZConfiguration->DefaultPTZSpeed->Zoom = ONVIF_MALLOC(struct tt__Vector1D);
		 	profile_list[id].PTZConfiguration->DefaultPTZSpeed->Zoom->x = 0.5;
		 	profile_list[id].PTZConfiguration->DefaultPTZSpeed->Zoom->space = "http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace";
		 	profile_list[id].PTZConfiguration->DefaultPTZTimeout = ONVIF_MALLOC(LONG64);
			*profile_list[id].PTZConfiguration->DefaultPTZTimeout = 60000; //60s
		 	profile_list[id].PTZConfiguration->PanTiltLimits = ONVIF_MALLOC(struct tt__PanTiltLimits);
		 	profile_list[id].PTZConfiguration->PanTiltLimits->Range = ONVIF_MALLOC(struct tt__Space2DDescription);
		 	profile_list[id].PTZConfiguration->PanTiltLimits->Range->URI = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
		 	profile_list[id].PTZConfiguration->PanTiltLimits->Range->XRange = ONVIF_MALLOC(struct tt__FloatRange);
			profile_list[id].PTZConfiguration->PanTiltLimits->Range->XRange->Min = -1;
			profile_list[id].PTZConfiguration->PanTiltLimits->Range->XRange->Max = 1;
		 	profile_list[id].PTZConfiguration->PanTiltLimits->Range->YRange = ONVIF_MALLOC(struct tt__FloatRange);
			profile_list[id].PTZConfiguration->PanTiltLimits->Range->YRange->Min = -1;
			profile_list[id].PTZConfiguration->PanTiltLimits->Range->YRange->Max = 1;
		 	profile_list[id].PTZConfiguration->ZoomLimits = ONVIF_MALLOC(struct tt__ZoomLimits);
		 	profile_list[id].PTZConfiguration->ZoomLimits->Range = ONVIF_MALLOC(struct tt__Space1DDescription);
		 	profile_list[id].PTZConfiguration->ZoomLimits->Range->URI = "http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
		 	profile_list[id].PTZConfiguration->ZoomLimits->Range->XRange = ONVIF_MALLOC(struct tt__FloatRange);
			profile_list[id].PTZConfiguration->ZoomLimits->Range->XRange->Min = -1;
			profile_list[id].PTZConfiguration->ZoomLimits->Range->XRange->Max = 1;

			//////////////
			id ++;
		}	
	}
	
	trt__GetProfilesResponse->Profiles = profile_list;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration(struct soap *soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration(struct soap *soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioEncoderConfiguration(struct soap *soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioSourceConfiguration(struct soap *soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddPTZConfiguration(struct soap *soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse)
{
	ONVIF_TRACE("undone AddPTZConf, ProfileToken:%s, ConfigurationToken:%s\n", trt__AddPTZConfiguration->ProfileToken, trt__AddPTZConfiguration->ConfigurationToken);
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported"); 
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoAnalyticsConfiguration(struct soap *soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddMetadataConfiguration(struct soap *soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioOutputConfiguration(struct soap *soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioDecoderConfiguration(struct soap *soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration(struct soap *soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration(struct soap *soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioEncoderConfiguration(struct soap *soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioSourceConfiguration(struct soap *soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemovePTZConfiguration(struct soap *soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse)
{
	ONVIF_TRACE("unknown%d\n", 153);
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported"); 
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoAnalyticsConfiguration(struct soap *soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration(struct soap *soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioOutputConfiguration(struct soap *soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioDecoderConfiguration(struct soap *soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile(struct soap *soap, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations(struct soap *soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
	int i, n , id;
	int nr_profile = 0;
	struct tt__VideoSourceConfiguration *VideoSourceConf = NULL;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		nr_profile += g_OnvifServerCxt->env.profiles.profile[n].profile_nr;
	}
	ONVIF_TRACE(" nprofile:%d",  nr_profile);

	ONVIF_S_env_load(OM_PROFILES, 0);

	VideoSourceConf = ONVIF_MALLOC_SIZE(struct tt__VideoSourceConfiguration, nr_profile);

	id  = 0;
	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		lpNVP_PROFILE_CHN profile = &g_OnvifServerCxt->env.profiles.profile[n];
		
		for(i = 0; i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; ++i){
			VideoSourceConf[id].Name = profile->vin[i].name;
			VideoSourceConf[id].token = profile->vin[i].token;
			VideoSourceConf[id].SourceToken = profile->v_source.token;
			VideoSourceConf[id].UseCount = 1;
			VideoSourceConf[id].Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
			VideoSourceConf[id].Bounds->x = profile->vin[i].rect.nX;
			VideoSourceConf[id].Bounds->y = profile->vin[i].rect.nY;
			VideoSourceConf[id].Bounds->width = profile->vin[i].rect.width;
			VideoSourceConf[id].Bounds->height = profile->vin[i].rect.height;

			//
			id++;
		}
	}
	trt__GetVideoSourceConfigurationsResponse->Configurations = VideoSourceConf;
	trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations = nr_profile;	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations(struct soap *soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
	int i, n , id;
	int nr_profile = 0;
	struct tt__VideoEncoderConfiguration *VideoEncoderConf = NULL;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		nr_profile += g_OnvifServerCxt->env.profiles.profile[n].profile_nr;
	}
	ONVIF_TRACE(" nprofile:%d",  nr_profile);

	ONVIF_S_env_load(OM_PROFILES, 0);
	
	VideoEncoderConf = ONVIF_MALLOC_SIZE(struct tt__VideoEncoderConfiguration, nr_profile);

	id  = 0;
	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		lpNVP_PROFILE_CHN profile = &g_OnvifServerCxt->env.profiles.profile[n];

		for(i = 0; i < profile->profile_nr; ++i){
			VideoEncoderConf[id].Name = profile->venc[i].enc_name;
			VideoEncoderConf[id].token = profile->venc[i].enc_token;
			VideoEncoderConf[id].UseCount = profile->venc[i].user_count;
			VideoEncoderConf[id].SessionTimeout = ONVIF_SESSION_TIMEOUT;
			VideoEncoderConf[id].Quality = profile->venc[i].enc_quality;
			VideoEncoderConf[id].Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
			VideoEncoderConf[id].Resolution->Width = profile->venc[i].width;
			VideoEncoderConf[id].Resolution->Height = profile->venc[i].height;
			VideoEncoderConf[id].RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
			VideoEncoderConf[id].RateControl->FrameRateLimit = profile->venc[i].enc_fps;
			VideoEncoderConf[id].RateControl->EncodingInterval = profile->venc[i].enc_interval;
			VideoEncoderConf[id].RateControl->BitrateLimit = profile->venc[i].enc_bps;
			VideoEncoderConf[id].H264 = ONVIF_MALLOC(struct tt__H264Configuration);
			VideoEncoderConf[id].H264->GovLength = profile->venc[i].enc_gov;
			VideoEncoderConf[id].H264->H264Profile = profile->venc[i].enc_profile;
			VideoEncoderConf[id].Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
			VideoEncoderConf[id].Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
			VideoEncoderConf[id].Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
			VideoEncoderConf[id].Multicast->Address->Type = tt__IPType__IPv4;
			snprintf(VideoEncoderConf[id].Multicast->Address->IPv4Address, 16, "235.255.255.255");
			VideoEncoderConf[id].Multicast->Port = 3703;
			VideoEncoderConf[id].Multicast->TTL = 0;
			VideoEncoderConf[id].Multicast->AutoStart = nfalse;
			VideoEncoderConf[id].Encoding = profile->venc[i].enc_type;		

			//
			id++;
		}	
	}
	trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = nr_profile;
	trt__GetVideoEncoderConfigurationsResponse->Configurations = VideoEncoderConf;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations(struct soap *soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
{
	int i, n , id;
	int nr_profile = 0;
	struct tt__AudioSourceConfiguration *AudioSourceConf = NULL;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		nr_profile += g_OnvifServerCxt->env.profiles.profile[n].profile_nr;
	}
	ONVIF_TRACE(" nprofile:%d",  nr_profile);

	ONVIF_S_env_load(OM_PROFILES, 0);
		
	AudioSourceConf = ONVIF_MALLOC_SIZE(struct tt__AudioSourceConfiguration,  nr_profile);
	
	id  = 0;
	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		lpNVP_PROFILE_CHN profile = &g_OnvifServerCxt->env.profiles.profile[n];

		for(i = 0; i < profile->profile_nr; ++i){
			AudioSourceConf[id].Name = profile->ain_name[i];
			AudioSourceConf[id].token = profile->ain_token[i];
			AudioSourceConf[id].SourceToken = profile->ain.token;
			AudioSourceConf[id].UseCount = 1;
			AudioSourceConf[id].__size = 0;
			AudioSourceConf[id].__any  = NULL;
			AudioSourceConf[id].__anyAttribute = NULL;
			/////
			id++;
		}
	}


	trt__GetAudioSourceConfigurationsResponse->__sizeConfigurations = nr_profile;
	trt__GetAudioSourceConfigurationsResponse->Configurations = AudioSourceConf;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurations(struct soap *soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse)
{
	int i, n , id;
	int nr_profile = 0;
	struct tt__AudioEncoderConfiguration *AudioEncoderConf = NULL;

	for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		nr_profile += g_OnvifServerCxt->env.profiles.profile[n].profile_nr;
	}
	ONVIF_TRACE(" nprofile:%d",  nr_profile);

	ONVIF_S_env_load(OM_PROFILES, 0);

	AudioEncoderConf = ONVIF_MALLOC_SIZE(struct tt__AudioEncoderConfiguration, nr_profile);

	id  = 0;
	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		lpNVP_PROFILE_CHN profile = &g_OnvifServerCxt->env.profiles.profile[n];

		for(i = 0; i < profile->aenc_nr; ++i){
			AudioEncoderConf[id].Name = profile->aenc[i].name;
			AudioEncoderConf[id].token = profile->aenc[i].token;
			AudioEncoderConf[id].Encoding = profile->aenc[i].enc_type;
			AudioEncoderConf[id].UseCount = profile->aenc[i].user_count;
			AudioEncoderConf[id].Bitrate = profile->aenc[i].sample_size;
			AudioEncoderConf[id].SampleRate = profile->aenc[i].sample_rate;
			AudioEncoderConf[id].SessionTimeout = ONVIF_SESSION_TIMEOUT;
			AudioEncoderConf[id].Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
			AudioEncoderConf[id].Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
			AudioEncoderConf[id].Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
			AudioEncoderConf[id].Multicast->Address->Type = tt__IPType__IPv4;
			snprintf(AudioEncoderConf[id].Multicast->Address->IPv4Address, 16, "235.255.255.255");
			AudioEncoderConf[id].Multicast->Port = 3703;
			AudioEncoderConf[id].Multicast->TTL = 0;
			AudioEncoderConf[id].Multicast->AutoStart = nfalse;

			////
			id++;
		}
	}
	trt__GetAudioEncoderConfigurationsResponse->Configurations = AudioEncoderConf;
	trt__GetAudioEncoderConfigurationsResponse->__sizeConfigurations = nr_profile;
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations(struct soap *soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurations(struct soap *soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse)
{
	ONVIF_TRACE("unknown%d\n", 162);
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported"); 
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurations(struct soap *soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration(struct soap *soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
	bool found = false;
	int i, n, chn, id;
	lpNVP_PROFILE_CHN profile;

	ONVIF_TRACE("CToken:%s", trt__GetVideoSourceConfiguration->ConfigurationToken);

	trt__GetVideoSourceConfigurationResponse->Configuration = ONVIF_MALLOC(struct tt__VideoSourceConfiguration);
	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for(i = 0; i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; ++i){
			if(0 == strcmp(trt__GetVideoSourceConfiguration->ConfigurationToken,  
				g_OnvifServerCxt->env.profiles.profile[n].vin[i].token)){
				found = true;
				chn = n;
				id = i;
				break;
			}		
		}
	}
	if (found == false) {
		ONVIF_TRACE("Specific token %s not found\n", trt__GetVideoSourceConfiguration->ConfigurationToken);
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	ONVIF_S_env_load(OM_VINC, chn * 100 + id);
	
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];

	trt__GetVideoSourceConfigurationResponse->Configuration->Name = profile->vin[id].name;
	trt__GetVideoSourceConfigurationResponse->Configuration->token = profile->vin[id].token;
	trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken = profile->v_source.token;
	trt__GetVideoSourceConfigurationResponse->Configuration->UseCount = 1;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->x = profile->vin[id].rect.nX;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->y = profile->vin[id].rect.nY;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->width = profile->vin[id].rect.width;
	trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->height = profile->vin[id].rect.height;			

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration(struct soap *soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
	bool found = false;
	int i, n, chn, id;
	lpNVP_PROFILE_CHN profile;
	
	ONVIF_TRACE("CToken:%s", trt__GetVideoEncoderConfiguration->ConfigurationToken);
	
	trt__GetVideoEncoderConfigurationResponse->Configuration = ONVIF_MALLOC(struct tt__VideoEncoderConfiguration);
	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for(i = 0; i < g_OnvifServerCxt->env.profiles.profile[n].venc_nr; ++i){
			if(0 == strcmp(trt__GetVideoEncoderConfiguration->ConfigurationToken,  
				g_OnvifServerCxt->env.profiles.profile[n].venc[i].enc_token)){
				found = true;
				chn = n;
				id = i;
				break;
			}		
		}
	}
	if (found == false) {
		ONVIF_TRACE("Specific token %s not found\n", trt__GetVideoEncoderConfiguration->ConfigurationToken);
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	
	ONVIF_S_env_load(OM_VENC, chn * 100 + id);
	
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];

	trt__GetVideoEncoderConfigurationResponse->Configuration->Name = profile->venc[id].enc_name;
	trt__GetVideoEncoderConfigurationResponse->Configuration->token = profile->venc[id].enc_token;
	trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount = profile->venc[id].user_count;
	trt__GetVideoEncoderConfigurationResponse->Configuration->SessionTimeout = ONVIF_SESSION_TIMEOUT;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Quality = profile->venc[id].enc_quality;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width = profile->venc[id].width;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height =profile->venc[id].height;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit =profile->venc[id].enc_fps;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval = profile->venc[id].enc_interval;
	trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit = profile->venc[id].enc_bps;
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264 = ONVIF_MALLOC(struct tt__H264Configuration);
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength = profile->venc[id].enc_gov;
	trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = profile->venc[id].enc_profile;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->Type = tt__IPType__IPv4;
	snprintf(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Port = 3703;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->TTL = 0;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->AutoStart = nfalse;
	trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding = profile->venc[id].enc_type;

	ONVIF_TRACE("GetVenc %d Response ok:%s", i, trt__GetVideoEncoderConfiguration->ConfigurationToken);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfiguration(struct soap *soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfiguration(struct soap *soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
{
	bool found = false;
	int i, n, chn, id;
	lpNVP_PROFILE_CHN profile;
		
	trt__GetAudioEncoderConfigurationResponse->Configuration = ONVIF_MALLOC(struct tt__AudioEncoderConfiguration);
	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for(i = 0; i < g_OnvifServerCxt->env.profiles.profile[n].aenc_nr; ++i){
			if(0 == strcmp(trt__GetAudioEncoderConfiguration->ConfigurationToken,  
				g_OnvifServerCxt->env.profiles.profile[n].aenc[i].token)){
				found = true;
				chn = n;
				id = i;
				break;
			}		
		}
	}
	if (found == false) {
		ONVIF_TRACE("Specific token %s not found", trt__GetAudioEncoderConfiguration->ConfigurationToken);
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	
	ONVIF_S_env_load(OM_AENC, chn * 100 + id);
	
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];

	trt__GetAudioEncoderConfigurationResponse->Configuration->Name = profile->aenc[id].name;
	trt__GetAudioEncoderConfigurationResponse->Configuration->token = profile->aenc[id].token;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Encoding = profile->aenc[id].enc_type;
	trt__GetAudioEncoderConfigurationResponse->Configuration->UseCount = profile->aenc[id].user_count;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Bitrate = profile->aenc[id].sample_size;
	trt__GetAudioEncoderConfigurationResponse->Configuration->SampleRate = profile->aenc[id].sample_rate;
	trt__GetAudioEncoderConfigurationResponse->Configuration->SessionTimeout = ONVIF_SESSION_TIMEOUT;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->Type = tt__IPType__IPv4;
	snprintf(trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Port = 3703;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->TTL = 0;
	trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->AutoStart = nfalse;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfiguration(struct soap *soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfiguration(struct soap *soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfiguration(struct soap *soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
	/*
	char _IPAddr[LARGE_INFO_LENGTH];
	int i = 0;
	int flag = 0;
	int Ptoken_exist = 0;

	sprintf(_IPAddr, "http://%03d.%03d.%d.%d/onvif/services", g_OnvifConf.lan.lan.staticIP);
	
	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken, g_OnvifConf.Profile[i].ProfileToken) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations = 1;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations = ONVIF_MALLOC(struct tt__VideoEncoderConfiguration);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Name = g_OnvifConf.Profile[i].VECName;//"VideoEncoderConfiguration";
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->UseCount = 1;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->token = g_OnvifConf.Profile[i].VECToken;//"VideoEncoderConfiguration";
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Encoding = g_OnvifConf.Profile[i].VECEncoding;// {tt__VideoEncoding__JPEG = 0, onv__VideoEncoding__MPEG4 = 1, tt__VideoEncoding__H264 = 2}
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Quality = g_OnvifConf.Profile[i].VECQuality; // float
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->SessionTimeout = ONVIF_SESSION_TIMEOUT;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Resolution->Width = g_OnvifConf.Profile[i].VECWidth;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Resolution->Height = g_OnvifConf.Profile[i].VECHeight;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->RateControl->FrameRateLimit = g_OnvifConf.Profile[i].VECfps;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->RateControl->EncodingInterval = g_OnvifConf.Profile[i].VENCInterval;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->RateControl->BitrateLimit = g_OnvifConf.Profile[i].VECbps;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->MPEG4 = ONVIF_MALLOC(struct tt__Mpeg4Configuration);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->MPEG4->GovLength = g_OnvifConf.Profile[i].VENCH264Gop;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->MPEG4->Mpeg4Profile = 0;//{onv__Mpeg4Profile__SP = 0, onv__Mpeg4Profile__ASP = 1}
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->H264 = ONVIF_MALLOC(struct tt__H264Configuration);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->H264->GovLength = g_OnvifConf.Profile[i].VENCH264Gop;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->H264->H264Profile = g_OnvifConf.Profile[i].VECH264Profile;//Baseline = 0, Main = 1, High = 3
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Port = 80;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->TTL = 1;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->AutoStart = 0; 
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address  = ONVIF_MALLOC(struct tt__IPAddress);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address->Type = 0;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 20);
	strcpy(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv4Address, _IPAddr);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv6Address = NULL;
*/
		return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
	/*
	int i=0;
	int Ptoken_exist = NOT_EXIST;
	uint32_t src_width, src_height;
	SENSOR_get_resolution(&src_width, &src_height);

	for(i=0;i< g_OnvifConf.nProfileCount;i++)
	{
		if(strcmp(trt__GetCompatibleVideoSourceConfigurations->ProfileToken,g_OnvifConf.Profile[i].ProfileToken) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	trt__GetCompatibleVideoSourceConfigurationsResponse->__sizeConfigurations = 1; //MPEG4 | H264 | JPEG

	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations = ONVIF_MALLOC(struct tt__VideoSourceConfiguration);
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Name = g_OnvifConf.Profile[i].VSCName;//"JPEG";
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->UseCount = 1;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->token = g_OnvifConf.Profile[i].VSCToken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->SourceToken = g_OnvifConf.Profile[i].VSCSourceToken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds->x = 0;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds->y =  0;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds->width = src_width;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds->height = src_height;
*/
		return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse)
{
/*
	trt__GetCompatibleAudioSourceConfigurationsResponse->__sizeConfigurations = 2;

	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations = ONVIF_MALLOC_SIZE(struct tt__AudioSourceConfiguration, trt__GetCompatibleAudioSourceConfigurationsResponse->__sizeConfigurations);
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].Name = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].Name, "G711");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].UseCount = 1;
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].token = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].token, "G711");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].SourceToken = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].SourceToken, "G711");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].Name = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].Name, "AAC");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].UseCount = 1;
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].token = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].token, "AAC");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].SourceToken = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].SourceToken, "AAC");
*/
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations(struct soap *soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioOutputConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioDecoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration(struct soap *soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration(struct soap *soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{
	int i=0, n , chn, id;
	bool found = false;
	unsigned char encodinginterval = 0;
	unsigned char frameratelimit = 0;
	int _width = 0;
	int _height = 0;
	int bitrate = 0;
	bool rightArg = true;
	lpNVP_PROFILE_CHN profile;

	
	ONVIF_TRACE("encoding:%s", trt__SetVideoEncoderConfiguration->Configuration->token);

	if (trt__SetVideoEncoderConfiguration->Configuration == NULL) {
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	if (trt__SetVideoEncoderConfiguration->Configuration->Encoding != tt__VideoEncoding__H264) {
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	for (n  = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for ( i = 0; i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++) {
			if(strcmp(trt__SetVideoEncoderConfiguration->Configuration->token, 
				g_OnvifServerCxt->env.profiles.profile[n].venc[i].enc_token)==0)
	      		{
	  			found = true;
				chn = n;
				id = i;
				break;
			}
		}
	}


	if( found == false) 
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoSource");
		return SOAP_FAULT;
	}	

	ONVIF_S_env_load(OM_VENC, chn *100 + id);
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];
	
	if(trt__SetVideoEncoderConfiguration->Configuration->Resolution != NULL)
	{
		_width = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width;
		_height = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height;
		//
		if( ( (_width % 2) != 0 || (_height % 2) != 0 ) || ((_width * _height) % 16 != 0))	
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
			return SOAP_FAULT;
		}
		profile->venc[id].width = _width;
		profile->venc[id].height = _height;
	}
	if(trt__SetVideoEncoderConfiguration->Configuration->RateControl != NULL)
	{
		encodinginterval = trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
		bitrate = trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit;
		frameratelimit = trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
		profile->venc[id].enc_interval = trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
		profile->venc[id].enc_fps  = frameratelimit;
		//profile->venc[id].quant_mode = NVP_QUANT_CBR;
		profile->venc[id].enc_bps  = bitrate;
	}

	profile->venc[id].user_count = trt__SetVideoEncoderConfiguration->Configuration->UseCount;
	profile->venc[id].enc_quality = trt__SetVideoEncoderConfiguration->Configuration->Quality;
	profile->venc[id].enc_type = trt__SetVideoEncoderConfiguration->Configuration->Encoding;
	if(trt__SetVideoEncoderConfiguration->Configuration->H264 != NULL)
	{
		profile->venc[id].enc_gov = trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength;
		profile->venc[id].enc_profile = trt__SetVideoEncoderConfiguration->Configuration->H264->H264Profile;
	}

	if(trt__SetVideoEncoderConfiguration->Configuration->Multicast != NULL)
	{
		ONVIF_TRACE("trt__SetVideoEncoderConfiguration->Configuration->Multicast.");
	}

	if (id == 0) {
		if (profile->venc[id].enc_bps > 6000 || profile->venc[id].enc_bps < 32) {
			rightArg = false;
			profile->venc[id].enc_bps = 3000;
		} 
	} else if (id == 1) {
		if (profile->venc[id].enc_bps > 1500 || profile->venc[id].enc_bps < 32) {
			profile->venc[id].enc_bps = 1000;
			rightArg = false;
		} 
	} else {
		if (profile->venc[id].enc_bps >512 || profile->venc[id].enc_bps < 32) {
			profile->venc[id].enc_bps = 300;
			rightArg = false;
		} 
	}
	if ( profile->venc[id].enc_fps > 30 ||  profile->venc[id].enc_fps < 3){
		rightArg = false;
		 profile->venc[id].enc_fps = 25;
	} 
	if ( profile->venc[id].enc_gov  > 50 || profile->venc[id].enc_gov  < 1){
		rightArg = false;
		profile->venc[id].enc_gov  = 30;
	}

	if (rightArg == false) {
		ONVIF_INFO("VENC CONFIG some args out of range!");
		//onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
		//return SOAP_FAULT;
	}

	ONVIF_INFO("set %d fps:%d bps:%d (%dx%d)", (chn + 1)* 100 + id + 1, profile->venc[id].enc_fps, profile->venc[id].enc_bps,
		_width, _height);
	ONVIF_S_env_save(OM_VENC, chn * 100 + id);	

	// disable send 
	// kaga
	if (g_OnvifServerCxt->need_auth == false) {
		soap->header = NULL;
	}

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioSourceConfiguration(struct soap *soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse)
{
	/*
	char _Token[SMALL_INFO_LENGTH]; 
	char _Name[SMALL_INFO_LENGTH]; 
	char _SourceToken[SMALL_INFO_LENGTH]; 
	int i;
	int flag = 0;
	unsigned char _Encoding;

	strcpy(_SourceToken, trt__SetAudioSourceConfiguration->Configuration->SourceToken);
	strcpy(_Token, trt__SetAudioSourceConfiguration->Configuration->token);
	strcpy(_Name, trt__SetAudioSourceConfiguration->Configuration->Name);

	if(strcmp(_Token,"G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{ 
		onvif_fault(soap,"ter:NoConfig", "ter:InvalidArgValue");
		return SOAP_FAULT;
	}
	
	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(_SourceToken, g_OnvifConf.Profile[i].ASCSourceToken) == 0)
		{
			flag = EXIST;
			break;
		}
	}
	if(!flag)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	if(strcmp(_SourceToken, "G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_SourceToken, "G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_SourceToken, "AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{ 
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	//if(oSysInfo->audio_config.codectype != _Encoding)
	//{
	//	ControlSystemData(SFIELD_SET_AUDIO_ENCODE, (void *)&_Encoding, sizeof(_Encoding));
	//}
	strcpy(g_OnvifConf.Profile[i].ASCName, _Name);
	strcpy(g_OnvifConf.Profile[i].ASCToken, _Token);
	strcpy(g_OnvifConf.Profile[i].ASCSourceToken, _SourceToken);
	flag = NOT_EXIST;
	for(i = 0; i <= g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(_Token, g_OnvifConf.Profile[i].ASCToken) == 0)
		{
			flag = EXIST;
			//Add_Audio_Conf.position = i;
			break;
		}
	}
	*/
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioEncoderConfiguration(struct soap *soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetMetadataConfiguration(struct soap *soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioOutputConfiguration(struct soap *soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioDecoderConfiguration(struct soap *soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(struct soap *soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{
	return SOAP_FAULT;
	/*
	int i;
	int num_token = 0;
	int j;
	int flag = NOT_EXIST;
	int flg = 0;
	int index = 0;
	int num = 0;
	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		for(j = 0; j <= i; j++)
		{
			if(strcmp(g_OnvifConf.Profile[j].VSCToken,g_OnvifConf.Profile[i].VSCToken) == 0);
			{
				flg = 1;		
				break;
			}
		}
		if(flg == 0)
		{
			num_token++;
		}
	}
	if(trt__GetVideoSourceConfigurationOptions->ConfigurationToken != NULL)
	{
		for(i = 0; i <= g_OnvifConf.nProfileCount ; i++)
		{
			if(strcmp(trt__GetVideoSourceConfigurationOptions->ConfigurationToken,g_OnvifConf.Profile[i].VSCToken) == 0);
			{
				flag = EXIST;
				index = j;
				break;
			}
		}
	}
		
	if(trt__GetVideoSourceConfigurationOptions->ProfileToken != NULL)
	{
		for(i = 0; i <= g_OnvifConf.nProfileCount; i++)
		{
			if(strcmp(trt__GetVideoSourceConfigurationOptions->ProfileToken, g_OnvifConf.Profile[i].ProfileToken)==0);
			{
				flag = EXIST;
				index = j;
				break;
			}
		}
	}
	
	if(!flag)
	{
		return SOAP_FAULT;
	}
	else
	{
		trt__GetVideoSourceConfigurationOptionsResponse->Options = ONVIF_MALLOC(struct tt__VideoSourceConfigurationOptions);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange = ONVIF_MALLOC(struct tt__IntRectangleRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Min = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Max = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Min = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Max = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Min = 320;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Max = 1920;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Min = 192;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Max = 1080;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable = (char **)soap_malloc(soap, sizeof(char *) * 1);
		if(trt__GetVideoSourceConfigurationOptions->ProfileToken == NULL && trt__GetVideoSourceConfigurationOptions->ConfigurationToken == NULL)
		{
			trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable = num_token;
			for(i = 0;i < g_OnvifConf.nProfileCount; i++)
			{
				for(j = 0; j < i; j++)
				{
					if(strcmp(g_OnvifConf.Profile[j].VSCToken, g_OnvifConf.Profile[i].VSCToken) == 0)
					{
						flg = 1;
						break;
					}
				}
				if(flg == 0)
				{

				trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[num] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
				strcpy(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[num], g_OnvifConf.Profile[i].VSCSourceToken);
				num++;
				}
				flg = 0;
			}
		}
		else
		{
			trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable = 1;
			trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
			strcpy(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0], g_OnvifConf.Profile[index].VSCSourceToken);
		}
	}

	return SOAP_OK; 
	*/
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions(struct soap *soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{
	ONVIF_TRACE("GetVideoEncoderConfigurationOptions, CToken:%s, PToken:%s", trt__GetVideoEncoderConfigurationOptions->ConfigurationToken, trt__GetVideoEncoderConfigurationOptions->ProfileToken);

	int i, n, chn, id;
	char JPEG_profile = 0;
	char MPEG4_profile = 0;
	char H264_profile =0;
	bool tokenGot = false;
	char PToken[NVP_MAX_NT_SIZE] = {0, };
	char CToken[NVP_MAX_NT_SIZE] = {0, };
	lpNVP_PROFILE_CHN profile;
	struct tt__H264Options *h264 = NULL;
	struct tt__H264Options2 *h264_2 = NULL;

	if(trt__GetVideoEncoderConfigurationOptions->ProfileToken)
	{
		strcpy(PToken, trt__GetVideoEncoderConfigurationOptions->ProfileToken);
	}
	if(trt__GetVideoEncoderConfigurationOptions->ConfigurationToken)
	{
		strcpy(CToken, trt__GetVideoEncoderConfigurationOptions->ConfigurationToken);
	}

	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for(i = 0; i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++)
		{
			if((strcmp(PToken, g_OnvifServerCxt->env.profiles.profile[n].venc[i].token) == 0) 
				|| (strcmp(CToken, g_OnvifServerCxt->env.profiles.profile[n].venc[i].enc_token) == 0))
			{
				tokenGot = true;
				chn = n; 
				id  = i;
				break;
			}
		}
	}
	
	if (tokenGot == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}

	ONVIF_S_env_load(OM_VENC, 100 * chn + id);
	
	profile = &g_OnvifServerCxt->env.profiles.profile[chn];
	
	if(profile->venc[id].enc_type == NVP_VENC_JPEG)
	{	
		JPEG_profile = 1;
	}
	else if(profile->venc[id].enc_type == NVP_VENC_MPEG4)
	{	
		MPEG4_profile = 1;
	}
	else if(profile->venc[id].enc_type == NVP_VENC_H264)
	{	
		H264_profile = 1;
	}
	
	trt__GetVideoEncoderConfigurationOptionsResponse->Options = ONVIF_MALLOC(struct tt__VideoEncoderConfigurationOptions);
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange = ONVIF_MALLOC(struct tt__IntRange);
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max = profile->venc[id].option.enc_quality.max;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min = profile->venc[id].option.enc_quality.min;

	if (profile->venc[id].enc_type == NVP_VENC_H264)
	{
		
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264 = ONVIF_MALLOC(struct tt__H264Options);
		h264 = trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264;
		h264->__sizeResolutionsAvailable = profile->venc[id].option.resolution_nr;
		h264->ResolutionsAvailable = ONVIF_MALLOC_SIZE(struct tt__VideoResolution, profile->venc[id].option.resolution_nr);
		for ( i = 0 ; i < profile->venc[id].option.resolution_nr; i++) {
			h264->ResolutionsAvailable[i].Width =  profile->venc[id].option.resolution[i].width;
			h264->ResolutionsAvailable[i].Height = profile->venc[id].option.resolution[i].height;
		}
		h264->GovLengthRange = ONVIF_MALLOC(struct tt__IntRange);
		h264->GovLengthRange->Min = profile->venc[id].option.enc_gov.min;
		h264->GovLengthRange->Max = profile->venc[id].option.enc_gov.max;
		h264->FrameRateRange = ONVIF_MALLOC(struct tt__IntRange);
		h264->FrameRateRange->Min = profile->venc[id].option.enc_fps.min;
		h264->FrameRateRange->Max = profile->venc[id].option.enc_fps.max;
		h264->EncodingIntervalRange = ONVIF_MALLOC(struct tt__IntRange);
		h264->EncodingIntervalRange->Min = profile->venc[id].option.enc_interval.min;
		h264->EncodingIntervalRange->Max = profile->venc[id].option.enc_interval.max;
		h264->__sizeH264ProfilesSupported = profile->venc[id].option.enc_profile_nr;
		h264->H264ProfilesSupported = ONVIF_MALLOC_SIZE(enum tt__H264Profile, profile->venc[id].option.enc_profile_nr);
		for ( i = 0; i < profile->venc[id].option.enc_profile_nr; i++) {
			h264->H264ProfilesSupported[i] = profile->venc[id].option.enc_profile[i]; //Baseline = 0, Main = 1, High = 3}
		}

		{
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension = ONVIF_MALLOC(struct tt__VideoEncoderOptionsExtension);
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->__size = 0;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->__any = NULL;
			trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264 = ONVIF_MALLOC(struct tt__H264Options2);
			h264_2 = trt__GetVideoEncoderConfigurationOptionsResponse->Options->Extension->H264;


			h264_2->__sizeResolutionsAvailable = profile->venc[id].option.resolution_nr;
			h264_2->ResolutionsAvailable = ONVIF_MALLOC_SIZE(struct tt__VideoResolution, profile->venc[id].option.resolution_nr);
			for ( i = 0 ; i < profile->venc[id].option.resolution_nr; i++) {
				h264_2->ResolutionsAvailable[i].Width =  profile->venc[id].option.resolution[i].width;
				h264_2->ResolutionsAvailable[i].Height = profile->venc[id].option.resolution[i].height;
			}

			h264_2->GovLengthRange = ONVIF_MALLOC(struct tt__IntRange);
			h264_2->GovLengthRange->Min = profile->venc[id].option.enc_gov.min;
			h264_2->GovLengthRange->Max = profile->venc[id].option.enc_gov.max;
			h264_2->FrameRateRange = ONVIF_MALLOC(struct tt__IntRange);
			h264_2->FrameRateRange->Min = profile->venc[id].option.enc_fps.min;
			h264_2->FrameRateRange->Max = profile->venc[id].option.enc_fps.max;
			h264_2->EncodingIntervalRange = ONVIF_MALLOC(struct tt__IntRange);
			h264_2->EncodingIntervalRange->Min = profile->venc[id].option.enc_interval.min;
			h264_2->EncodingIntervalRange->Max = profile->venc[id].option.enc_interval.max;
			h264_2->__sizeH264ProfilesSupported = profile->venc[id].option.enc_profile_nr;
			h264_2->H264ProfilesSupported = ONVIF_MALLOC_SIZE(enum tt__H264Profile, profile->venc[id].option.enc_profile_nr);
			for ( i = 0; i < profile->venc[id].option.enc_profile_nr; i++) {
				h264_2->H264ProfilesSupported[i] = profile->venc[id].option.enc_profile[i]; //Baseline = 0, Main = 1, High = 3}
			}
			h264_2->BitrateRange = ONVIF_MALLOC(struct tt__IntRange);
			h264_2->BitrateRange->Min = profile->venc[id].option.enc_bps.min;
			h264_2->BitrateRange->Max = profile->venc[id].option.enc_bps.max;
		}
	}

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurationOptions(struct soap *soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse)
{
	int profile_exist = 0;
	int i , n, chn, id;
	int _Encoding;
	char _Token[NVP_MAX_NT_SIZE] = "";

	/* Response */
	if(trt__GetAudioSourceConfigurationOptions->ProfileToken != NULL)
	{
		for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
			for(i = 0; i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++)
			{
				if((strcmp(trt__GetAudioSourceConfigurationOptions->ProfileToken, 
					g_OnvifServerCxt->env.profiles.profile[n].venc[i].token) == 0) )
				{
					profile_exist = true;
					chn = n; 
					id	= i;
					break;
				}
			}
		}

		if(!profile_exist)
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
			return SOAP_FAULT;
		}
	}

	if(trt__GetAudioSourceConfigurationOptions->ConfigurationToken != NULL)
	{
		strcpy(_Token,trt__GetAudioSourceConfigurationOptions->ConfigurationToken);
	}

	if(_Token[0] == 0 || strcmp(_Token, "G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token, "G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token, "AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{ 
		onvif_fault(soap, "ter:NoConfig", "ter:InvalidArgValue");
		return SOAP_FAULT;
	}

	trt__GetAudioSourceConfigurationOptionsResponse->Options = ONVIF_MALLOC(struct tt__AudioSourceConfigurationOptions); 
	trt__GetAudioSourceConfigurationOptionsResponse->Options->__sizeInputTokensAvailable = 3; 
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable = ONVIF_MALLOC_SIZE(char *, trt__GetAudioSourceConfigurationOptionsResponse->Options->__sizeInputTokensAvailable);
	
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[0] = ONVIF_MALLOC_SIZE(char, 16);
	strcpy(trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[0], "G711");
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[1] = ONVIF_MALLOC_SIZE(char, 16); 
	strcpy(trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[1], "G726"); 
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[2] = ONVIF_MALLOC_SIZE(char, 16); 
	strcpy(trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[2], "AAC"); 
	trt__GetAudioSourceConfigurationOptionsResponse->Options->Extension = NULL;
	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions(struct soap *soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurationOptions(struct soap *soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
	ONVIF_TRACE("GetGuaranteedNumberOfVideoEncoderInstances\n");

	trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse->TotalNumber = 1;

	return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri(struct soap *soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
	char file[16] = {0};
	int i, n, chn, id;
	bool TokenExit = false;
	
	for ( n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
		for(i = 0; i < g_OnvifServerCxt->env.profiles.profile[n].profile_nr; i++)
		{
			if((strcmp(trt__GetStreamUri->ProfileToken, 
				g_OnvifServerCxt->env.profiles.profile[n].venc[i].token) == 0) )
			{
				TokenExit = true;
				chn = n; 
				id	= i;
				break;
			}
		}
	}

	if(!TokenExit)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}

	if(trt__GetStreamUri->StreamSetup != NULL)
	{
		if(trt__GetStreamUri->StreamSetup->Stream == tt__StreamType__RTP_Multicast)
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:InvalidStreamSetup");
			return SOAP_FAULT;
		}
	}

	ONVIF_S_env_load(OM_NET, 0);
	
	snprintf(file, sizeof(file), "ch%d_%d.264", chn , id);
	
	trt__GetStreamUriResponse->MediaUri = ONVIF_MALLOC(struct tt__MediaUri);
	trt__GetStreamUriResponse->MediaUri->Uri = ONVIF_MALLOC_SIZE(char , 50);
	trt__GetStreamUriResponse->MediaUri->InvalidAfterReboot = nfalse;
	trt__GetStreamUriResponse->MediaUri->InvalidAfterConnect = nfalse;
	trt__GetStreamUriResponse->MediaUri->Timeout = 75;
	snprintf(trt__GetStreamUriResponse->MediaUri->Uri, 50, "rtsp://%s:%d/%s",
		_ip_2string( g_OnvifServerCxt->env.ether.ip, NULL), g_OnvifServerCxt->env.ether.http_port, file);
	ONVIF_INFO("GetStreamUri, stream setup:%p, profile token:%s, stream uri:%s", 
		trt__GetStreamUri->StreamSetup, trt__GetStreamUri->ProfileToken,
		trt__GetStreamUriResponse->MediaUri->Uri);
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__StartMulticastStreaming(struct soap *soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__StopMulticastStreaming(struct soap *soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetSynchronizationPoint(struct soap *soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri(struct soap *soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
	ONVIF_S_env_load(OM_NET, 0);

	trt__GetSnapshotUriResponse->MediaUri = ONVIF_MALLOC(struct tt__MediaUri);
	trt__GetSnapshotUriResponse->MediaUri->Uri = ONVIF_MALLOC_SIZE(char , 50);
	trt__GetSnapshotUriResponse->MediaUri->InvalidAfterReboot = nfalse;
	trt__GetSnapshotUriResponse->MediaUri->InvalidAfterConnect = nfalse;
	trt__GetSnapshotUriResponse->MediaUri->Timeout = 100;
	snprintf(trt__GetSnapshotUriResponse->MediaUri->Uri, 50, "http://%s:%d/%s",
		_ip_2string( g_OnvifServerCxt->env.ether.ip, NULL), g_OnvifServerCxt->env.ether.http_port, "snapshot.jpg");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceModes(struct soap *soap, struct _trt__GetVideoSourceModes *trt__GetVideoSourceModes, struct _trt__GetVideoSourceModesResponse *trt__GetVideoSourceModesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceMode(struct soap *soap, struct _trt__SetVideoSourceMode *trt__SetVideoSourceMode, struct _trt__SetVideoSourceModeResponse *trt__SetVideoSourceModeResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDs(struct soap *soap, struct _trt__GetOSDs *trt__GetOSDs, struct _trt__GetOSDsResponse *trt__GetOSDsResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSD(struct soap *soap, struct _trt__GetOSD *trt__GetOSD, struct _trt__GetOSDResponse *trt__GetOSDResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetOSDOptions(struct soap *soap, struct _trt__GetOSDOptions *trt__GetOSDOptions, struct _trt__GetOSDOptionsResponse *trt__GetOSDOptionsResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetOSD(struct soap *soap, struct _trt__SetOSD *trt__SetOSD, struct _trt__SetOSDResponse *trt__SetOSDResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateOSD(struct soap *soap, struct _trt__CreateOSD *trt__CreateOSD, struct _trt__CreateOSDResponse *trt__CreateOSDResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteOSD(struct soap *soap, struct _trt__DeleteOSD *trt__DeleteOSD, struct _trt__DeleteOSDResponse *trt__DeleteOSDResponse) { return SOAP_FAULT; }


SOAP_FMAC5 int SOAP_FMAC6 __tev__PullMessages(struct soap *soap, struct _tev__PullMessages *tev__PullMessages, struct _tev__PullMessagesResponse *tev__PullMessagesResponse) 
{
	int chn, id;
	char sztime[40];
	bool ismotion = false;
	int i, event_num = 1;
	LONG64 tt;
	int event_index[NVP_EVENT_CNT] = {NVP_EVENT_MD, NVP_EVENT_MD_EX};
	struct wsnt__NotificationMessageHolderType *msg = NULL;
	lpNVP_PROFILE_CHN profile = NULL;
	stONVIF_EVENT_SUBSCRIBE subscribe;

	ONVIF_TRACE("tev__PullMessages timeout: %lld, messagelimit: %d", tev__PullMessages->Timeout, tev__PullMessages->MessageLimit);

	// FIM me
	chn = 0;
	id  = 0;
	memset(&subscribe, 0, sizeof(subscribe));
	profile =  &g_OnvifServerCxt->env.profiles.profile[chn];

	tt = tev__PullMessages->Timeout;
	if (ONVIF_event_find(soap->endpoint, &subscribe) < 0) {
		if (soap->header)
			soap->header->wsa5__Action = "http://www.w3.org/2005/08/addressing/soap/fault";
		return SOAP_FAULT;
	}

	if ( subscribe.event_type == NVP_EVENT_ALL) {
		event_num = 2;
	} else {
		event_num = 1;
		event_index[0] = subscribe.event_type;
	}
	
	msg = ONVIF_MALLOC_SIZE(struct wsnt__NotificationMessageHolderType, event_num);

	for ( i  = 0; i < event_num; i++) {
		msg[i].Topic = soap_malloc(soap, sizeof(struct wsnt__TopicExpressionType));
		msg[i].Topic->Dialect = "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet";
		msg[i].Topic->__any = (char *)soap_malloc(soap, 128);
		strcpy(msg[i].Topic->__any, g_onvif_topic[event_index[i]]);
		msg[i].Topic->__mixed = NULL;
		msg[i].Topic->__anyAttribute = NULL;
		get_current_time(sztime);
		msg[i].Message.__any = (char *)soap_malloc(soap, 2000);

		if (event_index[i] == NVP_EVENT_MD) {
			ismotion = onvif_md_status(0, false);
			snprintf(msg[i].Message.__any, 2000, g_onvif_event_msg[event_index[i]], 
				sztime, "Initialized",
				profile->vin[id].token, profile->van.token, profile->md.rule_name,
				ismotion ? "true" : "false");
		} else if (event_index[i] == NVP_EVENT_MD_EX) {
			ismotion = onvif_md_status(0, false);
			snprintf(msg[i].Message.__any, 2000, g_onvif_event_msg[event_index[i]], 
				sztime, "Initialized", profile->v_source.token,
				ismotion ? "true" : "false");
		} else {
			sprintf(msg[i].Message.__any, "");
		}
		msg[i].SubscriptionReference = NULL;
		msg[i].ProducerReference = NULL;
	}

	time(&tev__PullMessagesResponse->CurrentTime);
	tev__PullMessagesResponse->TerminationTime = 
		tev__PullMessagesResponse->CurrentTime + subscribe.timeout;
	tev__PullMessagesResponse->__sizeNotificationMessage = event_num;
	tev__PullMessagesResponse->wsnt__NotificationMessage = msg;

	ONVIF_event_renew(soap->endpoint, &tt);

	if (soap->header)
		soap->header->wsa5__Action = "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesResponse";

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Seek(struct soap *soap, struct _tev__Seek *tev__Seek, struct _tev__SeekResponse *tev__SeekResponse)
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__SetSynchronizationPoint(struct soap *soap, struct _tev__SetSynchronizationPoint *tev__SetSynchronizationPoint, struct _tev__SetSynchronizationPointResponse *tev__SetSynchronizationPointResponse) 
{
	if (soap->header)
		soap->header->wsa5__Action = "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/SetSynchronizationPointResponse";
	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetServiceCapabilities(struct soap *soap, struct _tev__GetServiceCapabilities *tev__GetServiceCapabilities, struct _tev__GetServiceCapabilitiesResponse *tev__GetServiceCapabilitiesResponse) 
{
	tev__GetServiceCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct tev__Capabilities);
	tev__GetServiceCapabilitiesResponse->Capabilities->__size = 0;
	tev__GetServiceCapabilitiesResponse->Capabilities->__any = NULL;
	tev__GetServiceCapabilitiesResponse->Capabilities->WSSubscriptionPolicySupport = ONVIF_MALLOC(enum xsd__boolean);
	*tev__GetServiceCapabilitiesResponse->Capabilities->WSSubscriptionPolicySupport = ntrue;
	tev__GetServiceCapabilitiesResponse->Capabilities->WSPullPointSupport = ONVIF_MALLOC(enum xsd__boolean);
	*tev__GetServiceCapabilitiesResponse->Capabilities->WSPullPointSupport = ntrue;
	tev__GetServiceCapabilitiesResponse->Capabilities->WSPausableSubscriptionManagerInterfaceSupport = ONVIF_MALLOC(enum xsd__boolean);
	*tev__GetServiceCapabilitiesResponse->Capabilities->WSPausableSubscriptionManagerInterfaceSupport = nfalse;
	tev__GetServiceCapabilitiesResponse->Capabilities->PersistentNotificationStorage = ONVIF_MALLOC(enum xsd__boolean);
	*tev__GetServiceCapabilitiesResponse->Capabilities->PersistentNotificationStorage = nfalse;
	tev__GetServiceCapabilitiesResponse->Capabilities->MaxNotificationProducers = ONVIF_MALLOC(int);
	*tev__GetServiceCapabilitiesResponse->Capabilities->MaxNotificationProducers = 4;
	tev__GetServiceCapabilitiesResponse->Capabilities->MaxPullPoints = ONVIF_MALLOC(int);
	*tev__GetServiceCapabilitiesResponse->Capabilities->MaxPullPoints = 4;
	tev__GetServiceCapabilitiesResponse->Capabilities->__anyAttribute = NULL;
	
	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPointSubscription(struct soap *soap, struct _tev__CreatePullPointSubscription *tev__CreatePullPointSubscription, struct _tev__CreatePullPointSubscriptionResponse *tev__CreatePullPointSubscriptionResponse) 
{
	int event_type = 0;
	int timeout = 0;
	char currenttime[64];
	char reference[200];
	
	if (tev__CreatePullPointSubscription->Filter) {
		ezxml_t filter_xml = NULL, node = NULL;
		/*
		<wsnt:TopicExpression Dialect=\"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet\">
			{ any } ?
		</wsnt:TopicExpression>"\
		"<wsnt:MessageContent Dialect=\"http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter\">
			{ any } ?
		</wsnt:MessageContent>"
		*/
		if (tev__CreatePullPointSubscription->Filter->__any[0] ) {
			printf("filter (%d): %s\n", tev__CreatePullPointSubscription->Filter->__size, tev__CreatePullPointSubscription->Filter->__any[0] );
			if((filter_xml= ezxml_parse_str(tev__CreatePullPointSubscription->Filter->__any[0] , 
				strlen(tev__CreatePullPointSubscription->Filter->__any[0] ))) != NULL){
				node = ezxml_idx(filter_xml, 0);
				while(node)
				{
					char *name , *dialect = NULL, *content = NULL;
					name = ezxml_name(node);
					dialect = (char *)ezxml_attr(node, "Dialect");
					content = (char *)ezxml_txt(node);
					printf("name : %s dialect: %s\n", name, dialect);
					if (strstr(name, "TopicExpression") != NULL) {
						if (dialect == NULL
							|| strcmp(dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet") != 0) {
							onvif_fault2(soap, 1, "wsnt:TopicExpressionDialectUnknownFault");
							goto __FAULT_EXIT;
						}
						if (strlen(content) == 0) {
							event_type = 0;
						}
						else if (content &&  strstr(content, "RuleEngine/CellMotionDetector/Motion") != NULL) {
							event_type = NVP_EVENT_MD;
						}else if (content &&  strstr(content, "VideoSource/MotionAlarm") != NULL) {
							event_type = NVP_EVENT_MD_EX;
						}else if (content &&  strstr(content, "VideoSource/SignalLoss") != NULL) {
							event_type = NVP_EVENT_VIDEO_LOSS;
						}else if (content &&  strstr(content, "Device/Trigger/DigitalInput") != NULL) {
							event_type = NVP_EVENT_IO_IN;
						}else if (content &&  strstr(content, "Device/Trigger/Relay") != NULL) {
							event_type = NVP_EVENT_IO_OUT;
						} else {
							ONVIF_INFO("unknown topic: %s", content ? content : "null");
							onvif_fault2(soap, 1, "wsnt:TopicNotSupportedFault");
							goto __FAULT_EXIT;
						}
					} else if (strstr(name, "MessageContent") != NULL) {
						if (dialect == NULL
							|| strcmp(dialect, "http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter") != 0) {
							onvif_fault2(soap, 1, "wsnt:TopicExpressionDialectUnknownFault");
							goto __FAULT_EXIT;
						}

						//// FIX ME
					}
					//
					node = ezxml_next(filter_xml);
				}
			}
		}
	}

	if (tev__CreatePullPointSubscription->InitialTerminationTime)
		timeout = sztime_to_int(tev__CreatePullPointSubscription->InitialTerminationTime);
	if (timeout  == 0)
		timeout = 120;

	get_current_time(currenttime);
	reference[0] = 0;
	if (ONVIF_event_subscribe(ONVIF_EVENT_MODE_PULL, event_type, "", reference, timeout) < 0) {
		ONVIF_TRACE("\t add subscribe entry failed!");
		onvif_fault2(soap, 1, "wsnt:SubscribeCreationFailedFault");
		goto __FAULT_EXIT;
	}

	time(&tev__CreatePullPointSubscriptionResponse->wsnt__CurrentTime);
	tev__CreatePullPointSubscriptionResponse->wsnt__TerminationTime = tev__CreatePullPointSubscriptionResponse->wsnt__CurrentTime + timeout;
	tev__CreatePullPointSubscriptionResponse->SubscriptionReference.Address = ONVIF_MALLOC_SIZE(char, 200);
	strcpy(tev__CreatePullPointSubscriptionResponse->SubscriptionReference.Address, reference);
	tev__CreatePullPointSubscriptionResponse->SubscriptionReference.Metadata = NULL;
	tev__CreatePullPointSubscriptionResponse->SubscriptionReference.ReferenceParameters = NULL;
	tev__CreatePullPointSubscriptionResponse->SubscriptionReference.__size = 0;
	tev__CreatePullPointSubscriptionResponse->SubscriptionReference.__anyAttribute = NULL;
	tev__CreatePullPointSubscriptionResponse->SubscriptionReference.__any = NULL;
	tev__CreatePullPointSubscriptionResponse->__size = 0;
	tev__CreatePullPointSubscriptionResponse->__any = NULL;

	
	if (soap->header)
		soap->header->wsa5__Action = "http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionResponse";
	
	ONVIF_INFO("\tpull  subscribe(%s) type: %x ok!", reference, event_type);
	return SOAP_OK;


__FAULT_EXIT:
	if (soap->header)
		soap->header->wsa5__Action = "http://www.w3.org/2005/08/addressing/soap/fault";

	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetEventProperties(struct soap *soap, 
	struct _tev__GetEventProperties *tev__GetEventProperties, 
	struct _tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse) 
{
const char *md_e_msg_desc = 
	"<tns1:RuleEngine wstop:topic=\"true\">"
	"<CellMotionDetector wstop:topic=\"true\">"
	"<Motion wstop:topic=\"true\">"
	"  <tt:MessageDescription IsProperty=\"true\">"
	"    <tt:Source>"
	"      <tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\" Type=\"tt:ReferenceToken\"/>" 
	"      <tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
	"      <tt:SimpleItemDescription Name=\"Rule\" Type=\"xsd:string\"/>"
	"    </tt:Source>"
	"    <tt:Data>"
	"      <tt:SimpleItemDescription Name=\"IsMotion\" Type=\"xsd:boolean\"/>"
	"    </tt:Data>"
	"  </tt:MessageDescription>"
	"</Motion>"
	"</CellMotionDetector>"
	"</tns1:RuleEngine>";
const char *vsource_e_msg_desc = 
	"<tns1:VideoSource wstop:topic=\"true\">"
	"<MotionAlarm wstop:topic=\"true\">"
	"  <tt:MessageDescription IsProperty=\"true\">"
	"	 <tt:Source>"
	"	   <tt:SimpleItemDescription Name=\"Source\" Type=\"tt:ReferenceToken\"/>" 
	"	 </tt:Source>"
	"	 <tt:Data>"
	"	   <tt:SimpleItemDescription Name=\"State\" Type=\"xsd:boolean\"/>"
	"	 </tt:Data>"
	"  </tt:MessageDescription>"
	"</MotionAlarm>"
	"<SignalLoss wstop:topic=\"true\">"
	"  <tt:MessageDescription IsProperty=\"true\">"
	"	 <tt:Source>"
	"	   <tt:SimpleItemDescription Name=\"Source\" Type=\"tt:ReferenceToken\"/>" 
	"	 </tt:Source>"
	"	 <tt:Data>"
	"	   <tt:SimpleItemDescription Name=\"State\" Type=\"xsd:boolean\"/>"
	"	 </tt:Data>"
	"  </tt:MessageDescription>"
	"</SignalLoss>"
	"</tns1:VideoSource>";
const char *device_msg_desc = 
	"<tns1:Device wstop:topic=\"true\">"
	"<Trigger wstop:topic=\"true\">"
	"<DigitalInput wstop:topic=\"true\">"
	"  <tt:MessageDescription IsProperty=\"true\">"
	"	 <tt:Source>"
	"	   <tt:SimpleItemDescription Name=\"InputToken\" Type=\"tt:ReferenceToken\"/>" 
	"	 </tt:Source>"
	"	 <tt:Data>"
	"	   <tt:SimpleItemDescription Name=\"LogicalState\" Type=\"xsd:boolean\"/>"
	"	 </tt:Data>"
	"  </tt:MessageDescription>"
	"</DigitalInput>"
	"<Relay wstop:topic=\"true\">"
	"  <tt:MessageDescription IsProperty=\"true\">"
	"	 <tt:Source>"
	"	   <tt:SimpleItemDescription Name=\"RelayToken\" Type=\"tt:ReferenceToken\"/>" 
	"	 </tt:Source>"
	"	 <tt:Data>"
	"	   <tt:SimpleItemDescription Name=\"LogicalState\" Type=\"tt:RelayLogicalState\"/>"
	"	 </tt:Data>"
	"  </tt:MessageDescription>"
	"</Relay>"	
	"</Trigger>"
	"</tns1:Device>";

	int i;
	const char *  msg_descs[] = {vsource_e_msg_desc, device_msg_desc, md_e_msg_desc};
	//const char *  msg_descs[] = {md_e_msg_desc,  vsource_e_msg_desc, device_msg_desc};
	//const char *  msg_descs[] = {md_e_msg_desc};
	
	ONVIF_TRACE("EVENT MSG DESC NUM: %d", sizeof(msg_descs)/sizeof(msg_descs[0]));
	if (soap->header)
		soap->header->wsa5__Action = "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesResponse";

	tev__GetEventPropertiesResponse->__sizeTopicNamespaceLocation = 1;
	tev__GetEventPropertiesResponse->TopicNamespaceLocation = ONVIF_MALLOC_SIZE(char *, 1);
	tev__GetEventPropertiesResponse->TopicNamespaceLocation[0] = ONVIF_MALLOC_SIZE(char, 128);
	snprintf(tev__GetEventPropertiesResponse->TopicNamespaceLocation[0], 128,
		"http://www.onvif.org/onvif/ver10/topics/topicns.xml");
	tev__GetEventPropertiesResponse->wsnt__FixedTopicSet = ntrue;
#if 0
	tev__GetEventPropertiesResponse->wstop__TopicSet->__any = NULL;
#else
	tev__GetEventPropertiesResponse->wstop__TopicSet = ONVIF_MALLOC(struct wstop__TopicSetType);	
	tev__GetEventPropertiesResponse->wstop__TopicSet->__size = sizeof(msg_descs)/sizeof(msg_descs[0]);
	tev__GetEventPropertiesResponse->wstop__TopicSet->__any = ONVIF_MALLOC_SIZE(char *, 
		sizeof(msg_descs)/sizeof(msg_descs[0]));
	for ( i = 0; i < tev__GetEventPropertiesResponse->wstop__TopicSet->__size; i++) {
		tev__GetEventPropertiesResponse->wstop__TopicSet->__any[i] = ONVIF_MALLOC_SIZE(char, 2000);
		snprintf(tev__GetEventPropertiesResponse->wstop__TopicSet->__any[0], 2000, "%s",
			msg_descs[i]);
	}
#endif
	tev__GetEventPropertiesResponse->wstop__TopicSet->documentation = NULL;
	
	tev__GetEventPropertiesResponse->__sizeTopicExpressionDialect = 2;
	tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect = ONVIF_MALLOC_SIZE(char *, 2);
	tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[0] = ONVIF_MALLOC_SIZE(char, 128);
	snprintf(tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[0], 128,
		"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
	tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[1] = ONVIF_MALLOC_SIZE(char, 128);
	snprintf(tev__GetEventPropertiesResponse->wsnt__TopicExpressionDialect[1], 128,
		"http://docs.oasis-open.org/wsn/t-1/TopicExpression/Concrete");
	
	tev__GetEventPropertiesResponse->__sizeMessageContentFilterDialect = 1;
	tev__GetEventPropertiesResponse->MessageContentFilterDialect = ONVIF_MALLOC_SIZE(char *, 1);
	tev__GetEventPropertiesResponse->MessageContentFilterDialect[0] = ONVIF_MALLOC_SIZE(char, 128);
	snprintf(tev__GetEventPropertiesResponse->MessageContentFilterDialect[0], 128,
		"http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter");
	
	tev__GetEventPropertiesResponse->__sizeMessageContentSchemaLocation = 1;
	tev__GetEventPropertiesResponse->MessageContentSchemaLocation = ONVIF_MALLOC_SIZE(char *, 1);
	tev__GetEventPropertiesResponse->MessageContentSchemaLocation[0] = ONVIF_MALLOC_SIZE(char, 128);
	snprintf(tev__GetEventPropertiesResponse->MessageContentSchemaLocation[0], 128,
		"http://www.onvif.org/onvif/ver10/schema/onvif.xsd");
	
	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew(struct soap *soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse) 
{
	char *ptr = NULL;
	LONG64 tt = 0;
	ONVIF_TRACE("soap renew endpoint : %s", soap->endpoint);

	ptr = wsnt__Renew->TerminationTime;
	if (ptr[0] == 'P'){
		if( soap_s2xsd__duration(soap, ptr, &tt) != SOAP_OK) {
			onvif_fault2(soap, 1,"wsntw:UnacceptableTerminationTimeFault");
			return SOAP_FAULT;
		}
		tt = tt/1000;
	} else {
		time_t terminal = 0, t_now = 0;
		struct tm ptm;
		time(&t_now);
		//printf("timenow : %s ", ctime(&t_now));
		// change to gmtime
		memcpy(&ptm , gmtime(&t_now), sizeof(struct tm));
		t_now = mktime(&ptm);
		//printf("timenow 2 : %s ", ctime(&t_now));
		if (datetime_s2tm(ptr, NULL, &terminal) < 0
			|| terminal  <= t_now) {
			printf("terminal is not in future (%ld < %ld)\n", terminal, t_now);
			onvif_fault2(soap, 1,"wsntw:UnacceptableTerminationTimeFault");
			return SOAP_FAULT;
		}
		tt = (LONG64)terminal - (LONG64)t_now;
		//printf("get duration time: %lld s\n", tt);
	}

	if (ONVIF_event_renew(soap->endpoint, &tt) < 0){
		if (soap->header)
			soap->header->wsa5__Action = "http://www.w3.org/2005/08/addressing/soap/fault";
		
		onvif_fault(soap,"wsrf-r:ResourceUnknownFault", NULL);
		return SOAP_FAULT;
	}

	wsnt__RenewResponse->CurrentTime = ONVIF_MALLOC(time_t);
	time(wsnt__RenewResponse->CurrentTime);
	wsnt__RenewResponse->TerminationTime = *wsnt__RenewResponse->CurrentTime + tt;
	wsnt__RenewResponse->__size = 0;
	wsnt__RenewResponse->__any = NULL;
	
	if (soap->header)
		soap->header->wsa5__Action = "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewResponse";

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse) 
{
	if (ONVIF_event_unsubscribe(soap->endpoint) < 0) {
		if (soap->header)
			soap->header->wsa5__Action = "http://www.w3.org/2005/08/addressing/soap/fault";
		return SOAP_FAULT;
	}
	
	if (soap->header)
		soap->header->wsa5__Action = "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeResponse";

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Subscribe(struct soap *soap, struct _wsnt__Subscribe *wsnt__Subscribe, struct _wsnt__SubscribeResponse *wsnt__SubscribeResponse) 
{ 
	int event_type = 0;
	int timeout = 0;
	char reference[200];

	
	ONVIF_INFO("\t subscribe(%s \n\t%s)...", reference, wsnt__Subscribe->ConsumerReference.Address);
	if (wsnt__Subscribe->Filter) {
		ezxml_t filter_xml = NULL, node = NULL;
		/*
		<wsnt:TopicExpression Dialect=\"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet\">
			{ any } ?
		</wsnt:TopicExpression>"\
		"<wsnt:MessageContent Dialect=\"http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter\">
			{ any } ?
		</wsnt:MessageContent>"
		*/
		if (wsnt__Subscribe->Filter->__any[0] ) {
			ONVIF_INFO("filter (%d): %s", wsnt__Subscribe->Filter->__size, wsnt__Subscribe->Filter->__any[0] );
			if((filter_xml= ezxml_parse_str(wsnt__Subscribe->Filter->__any[0] , 
				strlen(wsnt__Subscribe->Filter->__any[0] ))) != NULL){
				node = ezxml_idx(filter_xml, 0);
				while(node)
				{
					char *name , *dialect = NULL, *content = NULL;
					name = ezxml_name(node);
					dialect = (char *)ezxml_attr(node, "Dialect");
					content = (char *)ezxml_txt(node);
					ONVIF_INFO("name : %s dialect: %s content:%s", name, dialect, content ? content : "null");
					if (strstr(name, "TopicExpression") != NULL) {
						if (dialect == NULL
							|| strcmp(dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet") != 0) {
							ONVIF_INFO("invalid dialect: %s", dialect ? dialect : "null");
							onvif_fault2(soap, 1, "wsnt:TopicExpressionDialectUnknownFault");
							goto __FAULT_EXIT;
						}
						if (content && strlen(content) == 0) {
							event_type = 0;
						}
						else if (content &&  strstr(content, "RuleEngine/CellMotionDetector/Motion") != NULL) {
							event_type = NVP_EVENT_MD;
						}else if (content &&  strstr(content, "VideoSource/MotionAlarm") != NULL) {
							event_type = NVP_EVENT_MD_EX;
						}else if (content &&  strstr(content, "VideoSource/SignalLoss") != NULL) {
							event_type = NVP_EVENT_VIDEO_LOSS;
						}else if (content &&  strstr(content, "Device/Trigger/DigitalInput") != NULL) {
							event_type = NVP_EVENT_IO_IN;
						}else if (content &&  strstr(content, "Device/Trigger/Relay") != NULL) {
							event_type = NVP_EVENT_IO_OUT;
						} else {
							ONVIF_INFO("unknown topic: %s", content ? content : "null");
							onvif_fault2(soap, 1, "wsnt:TopicNotSupportedFault");
							goto __FAULT_EXIT;
						}
					} else if (strstr(name, "MessageContent") != NULL) {
						if (dialect == NULL
							|| strcmp(dialect, "http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter") != 0) {
							ONVIF_INFO("invalid MessageContent dialect: %s", dialect ? dialect : "null");
							onvif_fault2(soap, 1, "wsnt:TopicExpressionDialectUnknownFault");
							goto __FAULT_EXIT;
						}

						//// FIX ME
					}
					//
					node = ezxml_next(filter_xml);
				}
			}
		}
	}
	
	if (wsnt__Subscribe->ConsumerReference.Address == NULL) {
		ONVIF_INFO("\t subscribe address is empty");
		goto __FAULT_EXIT;
	}
	if (wsnt__Subscribe->InitialTerminationTime)
		timeout = sztime_to_int(wsnt__Subscribe->InitialTerminationTime);
	if (timeout  == 0)
		timeout = 3600;

	reference[0] = 0;
	if (ONVIF_event_subscribe(ONVIF_EVENT_MODE_NOTIFY, event_type, wsnt__Subscribe->ConsumerReference.Address, reference, timeout) < 0) {
		ONVIF_INFO("\t add subscribe entry failed!");
		onvif_fault2(soap, 1, "wsnt:SubscribeCreationFailedFault");
		goto __FAULT_EXIT;
	}

	wsnt__SubscribeResponse->CurrentTime = ONVIF_MALLOC_SIZE(time_t , 1);
	time(wsnt__SubscribeResponse->CurrentTime);
	wsnt__SubscribeResponse->TerminationTime = ONVIF_MALLOC_SIZE(time_t, 1);
	*wsnt__SubscribeResponse->TerminationTime = *wsnt__SubscribeResponse->CurrentTime + timeout;
	wsnt__SubscribeResponse->SubscriptionReference.Address = ONVIF_MALLOC_SIZE(char, 200);
	strcpy(wsnt__SubscribeResponse->SubscriptionReference.Address, reference);
	wsnt__SubscribeResponse->SubscriptionReference.Metadata = NULL;
	wsnt__SubscribeResponse->SubscriptionReference.ReferenceParameters = NULL;
	wsnt__SubscribeResponse->SubscriptionReference.__size = 0;
	wsnt__SubscribeResponse->SubscriptionReference.__anyAttribute = NULL;
	wsnt__SubscribeResponse->SubscriptionReference.__any = NULL;
	wsnt__SubscribeResponse->__size = 0;
	wsnt__SubscribeResponse->__any = NULL;
	
	ONVIF_INFO("\t notify subscribe(%s \n\t%s) type: %x ok!", reference, wsnt__Subscribe->ConsumerReference.Address, event_type);
	return SOAP_OK;

__FAULT_EXIT:
	if (soap->header)
		soap->header->wsa5__Action = "http://www.w3.org/2005/08/addressing/soap/fault";

	return SOAP_FAULT;	
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetCurrentMessage(struct soap *soap, struct _wsnt__GetCurrentMessage *wsnt__GetCurrentMessage, struct _wsnt__GetCurrentMessageResponse *wsnt__GetCurrentMessageResponse) 
{
	return SOAP_FAULT;
}

/*
SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify(struct soap *soap, struct _wsnt__Notify *wsnt__Notify) 
{
	if (wsnt__Notify->__sizeNotificationMessage <= 0 || wsnt__Notify->NotificationMessage == NULL) {
		ONVIF_INFO("event notify, but no event carried 2");
		return SOAP_FAULT;
	}

	ONVIF_INFO("event notify 2, reference: %s", soap->endpoint);
	return SOAP_FAULT;
}
*/

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetMessages(struct soap *soap, struct _wsnt__GetMessages *wsnt__GetMessages, struct _wsnt__GetMessagesResponse *wsnt__GetMessagesResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__DestroyPullPoint(struct soap *soap, struct _wsnt__DestroyPullPoint *wsnt__DestroyPullPoint, struct _wsnt__DestroyPullPointResponse *wsnt__DestroyPullPointResponse) 
{
	if (ONVIF_event_unsubscribe(soap->endpoint) < 0) {
		if (soap->header)
			soap->header->wsa5__Action = "http://www.w3.org/2005/08/addressing/soap/fault";
		return SOAP_FAULT;
	}
	
	if (soap->header)
		soap->header->wsa5__Action = "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/DestroyPullPointResponse";

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify_(struct soap *soap, struct _wsnt__Notify *wsnt__Notify) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPoint(struct soap *soap, struct _wsnt__CreatePullPoint *wsnt__CreatePullPoint, struct _wsnt__CreatePullPointResponse *wsnt__CreatePullPointResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew_(struct soap *soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse) 
{ 
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe_(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__PauseSubscription(struct soap *soap, struct _wsnt__PauseSubscription *wsnt__PauseSubscription, struct _wsnt__PauseSubscriptionResponse *wsnt__PauseSubscriptionResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__ResumeSubscription(struct soap *soap, struct _wsnt__ResumeSubscription *wsnt__ResumeSubscription, struct _wsnt__ResumeSubscriptionResponse *wsnt__ResumeSubscriptionResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__PullMessages_(struct soap *soap, struct _tev__PullMessages *tev__PullMessages, struct _tev__PullMessagesResponse *tev__PullMessagesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__Seek_(struct soap *soap, struct _tev__Seek *tev__Seek, struct _tev__SeekResponse *tev__SeekResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__SetSynchronizationPoint_(struct soap *soap, struct _tev__SetSynchronizationPoint *tev__SetSynchronizationPoint, struct _tev__SetSynchronizationPointResponse *tev__SetSynchronizationPointResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetServiceCapabilities_(struct soap *soap, struct _tev__GetServiceCapabilities *tev__GetServiceCapabilities, struct _tev__GetServiceCapabilitiesResponse *tev__GetServiceCapabilitiesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPointSubscription_(struct soap *soap, struct _tev__CreatePullPointSubscription *tev__CreatePullPointSubscription, struct _tev__CreatePullPointSubscriptionResponse *tev__CreatePullPointSubscriptionResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetEventProperties_(struct soap *soap, struct _tev__GetEventProperties *tev__GetEventProperties, struct _tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew__(struct soap *soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe__(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__Subscribe_(struct soap *soap, struct _wsnt__Subscribe *wsnt__Subscribe, struct _wsnt__SubscribeResponse *wsnt__SubscribeResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetCurrentMessage_(struct soap *soap, struct _wsnt__GetCurrentMessage *wsnt__GetCurrentMessage, struct _wsnt__GetCurrentMessageResponse *wsnt__GetCurrentMessageResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify__(struct soap *soap, struct _wsnt__Notify *wsnt__Notify) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__GetMessages_(struct soap *soap, struct _wsnt__GetMessages *wsnt__GetMessages, struct _wsnt__GetMessagesResponse *wsnt__GetMessagesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__DestroyPullPoint_(struct soap *soap, struct _wsnt__DestroyPullPoint *wsnt__DestroyPullPoint, struct _wsnt__DestroyPullPointResponse *wsnt__DestroyPullPointResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__Notify___(struct soap *soap, struct _wsnt__Notify *wsnt__Notify) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__CreatePullPoint_(struct soap *soap, struct _wsnt__CreatePullPoint *wsnt__CreatePullPoint, struct _wsnt__CreatePullPointResponse *wsnt__CreatePullPointResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__Renew___(struct soap *soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse) 
{
	ONVIF_TRACE("");
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tev__Unsubscribe___(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__PauseSubscription_(struct soap *soap, struct _wsnt__PauseSubscription *wsnt__PauseSubscription, struct _wsnt__PauseSubscriptionResponse *wsnt__PauseSubscriptionResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tev__ResumeSubscription_(struct soap *soap, struct _wsnt__ResumeSubscription *wsnt__ResumeSubscription, struct _wsnt__ResumeSubscriptionResponse *wsnt__ResumeSubscriptionResponse) { return SOAP_FAULT; }


SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedRules(struct soap *soap, struct _tan__GetSupportedRules *tan__GetSupportedRules, struct _tan__GetSupportedRulesResponse *tan__GetSupportedRulesResponse) 
{ 
	bool found = false;
	int n, chn = 0, id = 0;

	if (tan__GetSupportedRules->ConfigurationToken ) {
		for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
			if (strcmp(tan__GetSupportedRules->ConfigurationToken, g_OnvifServerCxt->env.profiles.profile[n].van.token) ==0) {
				found = true;
				chn = n;
				id = 0;
			}
		}
	}
	
	
	if (found == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	tan__GetSupportedRulesResponse->SupportedRules = ONVIF_MALLOC(struct tt__SupportedRules);
	tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleContentSchemaLocation = 1;
	tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation = ONVIF_MALLOC_SIZE(char *, 1);
	tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[0] = ONVIF_MALLOC_SIZE(char, 128);
	snprintf(tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[0], 128,
		"%s", "http://www.w3.org/2001/XMLSchema");
	//tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[1] = ONVIF_MALLOC_SIZE(char, 128);
	//snprintf(tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[1], 128,
	//	"%s", "http://www.onvif.org/ver10/schema");
	tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleDescription = 1;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription = ONVIF_MALLOC_SIZE(struct tt__ConfigDescription, 1);
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters = ONVIF_MALLOC(struct tt__ItemListDescription);
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->__sizeSimpleItemDescription = 4;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->SimpleItemDescription = ONVIF_MALLOC_SIZE(struct _tt__ItemListDescription_SimpleItemDescription, 4);
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->SimpleItemDescription[0].Name = "MinCount";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->SimpleItemDescription[0].Type = "xsd:integer";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->SimpleItemDescription[1].Name = "AlarmOnDelay";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->SimpleItemDescription[1].Type = "xsd:integer";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->SimpleItemDescription[2].Name = "AlarmOffDelay";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->SimpleItemDescription[2].Type = "xsd:integer";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->SimpleItemDescription[3].Name = "ActiveCells";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->SimpleItemDescription[3].Type = "xsd:base64Binary";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters ->__sizeElementItemDescription = 0;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Name = "tt:CellMotionDetector";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].__sizeMessages = 1;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages = ONVIF_MALLOC_SIZE(struct _tt__ConfigDescription_Messages, 1);
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source = ONVIF_MALLOC(struct tt__ItemListDescription);
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source ->__sizeSimpleItemDescription = 3;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source ->SimpleItemDescription = ONVIF_MALLOC_SIZE(struct _tt__ItemListDescription_SimpleItemDescription, 3);
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source->SimpleItemDescription[0].Name = "VideoSourceConfigurationToken";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source ->SimpleItemDescription[0].Type = "tt:ReferenceToken";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source ->SimpleItemDescription[1].Name = "VideoAnalyticsConfigurationToken";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source->SimpleItemDescription[1].Type = "tt:ReferenceToken";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source ->SimpleItemDescription[2].Name = "Rule";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source->SimpleItemDescription[2].Type = "xsd:string";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Source->__sizeElementItemDescription = 0;

	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Data = ONVIF_MALLOC(struct tt__ItemListDescription);
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Data ->__sizeSimpleItemDescription = 1;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Data ->SimpleItemDescription = ONVIF_MALLOC_SIZE(struct _tt__ItemListDescription_SimpleItemDescription, 1);
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Data->SimpleItemDescription[0].Name = "IsMotion";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Data ->SimpleItemDescription[0].Type = "xsd:boolean";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Data->__sizeElementItemDescription = 0;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].ParentTopic = "tns1:RuleEngine/CellMotionDetector/Motion";
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].IsProperty = ONVIF_MALLOC(enum xsd__boolean);
	*tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].IsProperty = ntrue;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Key = NULL;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].Extension = NULL;
	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages[0].__anyAttribute = NULL;

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateRules(struct soap *soap, struct _tan__CreateRules *tan__CreateRules, struct _tan__CreateRulesResponse *tan__CreateRulesResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteRules(struct soap *soap, struct _tan__DeleteRules *tan__DeleteRules, struct _tan__DeleteRulesResponse *tan__DeleteRulesResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRules(struct soap *soap, struct _tan__GetRules *tan__GetRules, struct _tan__GetRulesResponse *tan__GetRulesResponse) 
{
	char md_layout[1024];
	
	bool found = false;
	int n,chn = 0, id = 0;

	if (tan__GetRules->ConfigurationToken ) {
		for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
			if (strcmp(tan__GetRules->ConfigurationToken, g_OnvifServerCxt->env.profiles.profile[n].van.token) ==0) {
				found = true;
				chn = n;
				id = 0;
			}
		}
	}
		
	if (found == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	
	ONVIF_S_env_load(OM_MD, 100 * chn + id);

	ONVIF_TRACE("delay_on: %d, delay_off:%d %d/%d", g_OnvifServerCxt->env.profiles.profile[chn].md.delay_on_alarm,
		g_OnvifServerCxt->env.profiles.profile[chn].md.delay_off_alarm,
		chn, id);
	
	md_celllayout_hex2s(md_layout, sizeof(md_layout), (uint8_t *)g_OnvifServerCxt->env.profiles.profile[chn].md.grid.granularity, 
		(g_OnvifServerCxt->env.profiles.profile[chn].md.grid.columnGranularity* g_OnvifServerCxt->env.profiles.profile[chn].md.grid.rowGranularity + 7)/8);

	tan__GetRulesResponse->__sizeRule = 1;
	tan__GetRulesResponse->Rule= ONVIF_MALLOC_SIZE(struct tt__Config, 1);
	tan__GetRulesResponse->Rule[0].Parameters = ONVIF_MALLOC(struct tt__ItemList);
	tan__GetRulesResponse->Rule[0].Name = g_OnvifServerCxt->env.profiles.profile[chn].md.rule_name;
	tan__GetRulesResponse->Rule[0].Type = "tt:CellMotionDetector";
	tan__GetRulesResponse->Rule[0].Parameters ->__sizeSimpleItem = 4;
	tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem = ONVIF_MALLOC_SIZE(struct _tt__ItemList_SimpleItem, 4);
	// FIX ME, Min Count: min adjacent cell number covered by moving object
	// AlarmOnDlay / AlarmOffDelay unit : millisecond
	tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem[0].Name = "MinCount";
	onvif_put_int(soap, &tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem[0].Value,
		g_OnvifServerCxt->env.profiles.profile[chn].md.grid.threshold);
	tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem[1].Name = "AlarmOnDelay";
	onvif_put_int(soap, &tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem[1].Value,
		g_OnvifServerCxt->env.profiles.profile[chn].md.delay_on_alarm);
	tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem[2].Name = "AlarmOffDelay";
	onvif_put_int(soap, &tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem[2].Value,
		g_OnvifServerCxt->env.profiles.profile[chn].md.delay_off_alarm);
	tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem[3].Name = "ActiveCells";
	tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem[3].Value = ONVIF_MALLOC_SIZE(char , 1024);
	snprintf(tan__GetRulesResponse->Rule[0].Parameters ->SimpleItem[3].Value, 1024,
		"%s", md_layout);
	tan__GetRulesResponse->Rule[0].Parameters ->__sizeElementItem = 0;
	tan__GetRulesResponse->Rule[0].Parameters ->ElementItem = NULL;
	tan__GetRulesResponse->Rule[0].Parameters ->Extension = NULL;
	tan__GetRulesResponse->Rule[0].Parameters ->__anyAttribute = NULL;

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyRules(struct soap *soap, struct _tan__ModifyRules *tan__ModifyRules, struct _tan__ModifyRulesResponse *tan__ModifyRulesResponse) 
{
	bool found = false;
	int n, i, chn = 0, id = 0;

	if (tan__ModifyRules->ConfigurationToken ) {
		for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
			if (strcmp(tan__ModifyRules->ConfigurationToken, g_OnvifServerCxt->env.profiles.profile[n].van.token) ==0) {
				found = true;
				chn = n;
				id = 0;
			}
		}
	}
	
	if (found == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}
	if (tan__ModifyRules->__sizeRule == 0 || tan__ModifyRules->Rule == NULL) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidRule");
		return SOAP_FAULT;
	}

	ONVIF_S_env_load(OM_MD, chn * 100 + id);

	for ( i = 0; i < tan__ModifyRules->__sizeRule ; i++){
		if ((&tan__ModifyRules->Rule[i]) == NULL 
			|| tan__ModifyRules->Rule[i].Type == NULL || tan__ModifyRules->Rule[i].Name == NULL) {
			onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidRule");
			return SOAP_FAULT;
		}
		if (strcmp(tan__ModifyRules->Rule[i].Name, g_OnvifServerCxt->env.profiles.profile[chn].md.rule_name)) {
			onvif_fault(soap, "ter:InvalidArgVal", "ter:RuleNotExistent");
			return SOAP_FAULT;
		}
		
		//if (strstr(tan__ModifyRules->Rule[i].Type, "RuleEngine/CellMotionDetector/Motion") == NULL) {
		if (strstr(tan__ModifyRules->Rule[i].Type, "CellMotionDetector") == NULL) {
			onvif_fault(soap, "ter:InvalidArgVal", "ter:RuleNotExistent");
			return SOAP_FAULT;
		}
		if (tan__ModifyRules->Rule[i].Parameters == NULL) {
			onvif_fault(soap, "ter:InvalidArgVal", "ter:InvalidRule");
			return SOAP_FAULT;
		}

		if (strstr(tan__ModifyRules->Rule[i].Type, "CellMotionDetector") != NULL) {
			for (n = 0; n < tan__ModifyRules->Rule[i].Parameters->__sizeSimpleItem; n++) {
				if (strcmp(tan__ModifyRules->Rule[i].Parameters->SimpleItem[i].Name, "MinCount") == 0){
					g_OnvifServerCxt->env.profiles.profile[chn].md.grid.threshold = atoi(tan__ModifyRules->Rule[i].Parameters->SimpleItem[i].Value);
				}else if (strcmp(tan__ModifyRules->Rule[i].Parameters->SimpleItem[i].Name, "AlarmOnDelay") == 0){
					g_OnvifServerCxt->env.profiles.profile[chn].md.delay_on_alarm = atoi(tan__ModifyRules->Rule[i].Parameters->SimpleItem[i].Value);
				}else if (strcmp(tan__ModifyRules->Rule[i].Parameters->SimpleItem[i].Name, "AlarmOffDelay") == 0){
					g_OnvifServerCxt->env.profiles.profile[chn].md.delay_off_alarm = atoi(tan__ModifyRules->Rule[i].Parameters->SimpleItem[i].Value);
				}else if (strcmp(tan__ModifyRules->Rule[i].Parameters->SimpleItem[i].Name, "ActiveCells") == 0){
					unsigned char temp[1024];
					int ret = md_celllayout_s2hex(tan__ModifyRules->Rule[i].Parameters->SimpleItem[i].Value,
						temp, sizeof(temp));
					if (ret > 0)
						memcpy(g_OnvifServerCxt->env.profiles.profile[chn].md.grid.granularity, temp, ret);
					ONVIF_INFO("set md granularity: %s", tan__ModifyRules->Rule[i].Parameters->SimpleItem[i].Value);
				}
			}
			ONVIF_INFO("set md !");
			ONVIF_S_env_save(OM_MD, chn * 100 + id);
		}
	}

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetServiceCapabilities(struct soap *soap, struct _tan__GetServiceCapabilities *tan__GetServiceCapabilities, struct _tan__GetServiceCapabilitiesResponse *tan__GetServiceCapabilitiesResponse) 
{
	tan__GetServiceCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct tan__Capabilities);
	tan__GetServiceCapabilitiesResponse->Capabilities->RuleSupport = ONVIF_MALLOC(enum xsd__boolean);
	*tan__GetServiceCapabilitiesResponse->Capabilities->RuleSupport = ntrue;
	tan__GetServiceCapabilitiesResponse->Capabilities->AnalyticsModuleSupport = ONVIF_MALLOC(enum xsd__boolean);
	*tan__GetServiceCapabilitiesResponse->Capabilities->AnalyticsModuleSupport = ntrue;
	tan__GetServiceCapabilitiesResponse->Capabilities->CellBasedSceneDescriptionSupported = ONVIF_MALLOC(enum xsd__boolean);
	*tan__GetServiceCapabilitiesResponse->Capabilities->CellBasedSceneDescriptionSupported = nfalse;
	tan__GetServiceCapabilitiesResponse->Capabilities->__anyAttribute = NULL;
	tan__GetServiceCapabilitiesResponse->Capabilities->__size = 0;
	tan__GetServiceCapabilitiesResponse->Capabilities->__any = NULL;
	
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedAnalyticsModules(struct soap *soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules, struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse) 
{
	struct tt__SupportedAnalyticsModules *ptr = NULL;
	bool found = false;
	int n, chn = 0, id = 0;

	if (tan__GetSupportedAnalyticsModules->ConfigurationToken ) {
		for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
			if (strcmp(tan__GetSupportedAnalyticsModules->ConfigurationToken, g_OnvifServerCxt->env.profiles.profile[n].van.token) ==0) {
				found = true;
				chn = n;
				id = 0;
			}
		}
	}
	
	ONVIF_TRACE("\ttoken: %s", tan__GetSupportedAnalyticsModules->ConfigurationToken ? tan__GetSupportedAnalyticsModules->ConfigurationToken : "NULL");
	if (found == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules = ONVIF_MALLOC(struct tt__SupportedAnalyticsModules);
	ptr  = tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules;
	ptr->__sizeAnalyticsModuleContentSchemaLocation = 1;
	ptr->AnalyticsModuleContentSchemaLocation = ONVIF_MALLOC_SIZE(char *, 2);
	ptr->AnalyticsModuleContentSchemaLocation[0] = ONVIF_MALLOC_SIZE(char , 128);
	snprintf(ptr->AnalyticsModuleContentSchemaLocation[0], 128, "%s",
		"http://www.w3.org/2001/XMLSchema");
	ptr->AnalyticsModuleContentSchemaLocation[1] = ONVIF_MALLOC_SIZE(char , 128);
	snprintf(ptr->AnalyticsModuleContentSchemaLocation[1], 128, "%s",
		"http://www.onvif.org/ver10/schema");
	ptr->__sizeAnalyticsModuleDescription = 1;
	ptr->AnalyticsModuleDescription = ONVIF_MALLOC_SIZE(struct tt__ConfigDescription, 1);
	ptr->AnalyticsModuleDescription[0].Parameters = ONVIF_MALLOC(struct tt__ItemListDescription);
		
	ptr->AnalyticsModuleDescription[0].Parameters ->__sizeSimpleItemDescription = 1;
	ptr->AnalyticsModuleDescription[0].Parameters ->SimpleItemDescription = ONVIF_MALLOC_SIZE(struct _tt__ItemListDescription_SimpleItemDescription, 1);
	ptr->AnalyticsModuleDescription[0].Parameters ->SimpleItemDescription[0].Name = "Sensitivity";
	ptr->AnalyticsModuleDescription[0].Parameters ->SimpleItemDescription[0].Type = "xsd:integer";
	ptr->AnalyticsModuleDescription[0].Parameters ->__sizeElementItemDescription = 1;
	ptr->AnalyticsModuleDescription[0].Parameters->ElementItemDescription = ONVIF_MALLOC_SIZE(struct _tt__ItemListDescription_ElementItemDescription, 1);
	ptr->AnalyticsModuleDescription[0].Parameters->ElementItemDescription[0].Name = "Layout";
	ptr->AnalyticsModuleDescription[0].Parameters->ElementItemDescription[0].Type = "tt:CellLayout";
	ptr->AnalyticsModuleDescription[0].Parameters->Extension = NULL;
	ptr->AnalyticsModuleDescription[0].Parameters->__anyAttribute = NULL;
	ptr->AnalyticsModuleDescription[0].Name = "tt:CellMotionEngine";
	ptr->AnalyticsModuleDescription[0].Extension = NULL;
	ptr->AnalyticsModuleDescription[0].__anyAttribute = NULL;
#if 0
	ptr->AnalyticsModuleDescription[0].__sizeMessages = 0;
	ptr->AnalyticsModuleDescription[0].Messages = NULL;
#else
	ptr->AnalyticsModuleDescription[0].__sizeMessages = 1;
	ptr->AnalyticsModuleDescription[0].Messages = ONVIF_MALLOC_SIZE(struct _tt__ConfigDescription_Messages, 1);
	ptr->AnalyticsModuleDescription[0].Messages[0].Source = ONVIF_MALLOC(struct tt__ItemListDescription);
	ptr->AnalyticsModuleDescription[0].Messages[0].Source ->__sizeSimpleItemDescription = 3;
	ptr->AnalyticsModuleDescription[0].Messages[0].Source ->SimpleItemDescription = ONVIF_MALLOC_SIZE(struct _tt__ItemListDescription_SimpleItemDescription, 3);
	ptr->AnalyticsModuleDescription[0].Messages[0].Source->SimpleItemDescription[0].Name = "VideoSourceConfigurationToken";
	ptr->AnalyticsModuleDescription[0].Messages[0].Source ->SimpleItemDescription[0].Type = "tt:ReferenceToken";
	ptr->AnalyticsModuleDescription[0].Messages[0].Source ->SimpleItemDescription[1].Name = "VideoAnalyticsConfigurationToken";
	ptr->AnalyticsModuleDescription[0].Messages[0].Source->SimpleItemDescription[1].Type = "tt:ReferenceToken";
	ptr->AnalyticsModuleDescription[0].Messages[0].Source ->SimpleItemDescription[2].Name = "Rule";
	ptr->AnalyticsModuleDescription[0].Messages[0].Source->SimpleItemDescription[2].Type = "xsd:string";
	ptr->AnalyticsModuleDescription[0].Messages[0].Source->__sizeElementItemDescription = 0;

	ptr->AnalyticsModuleDescription[0].Messages[0].Data = ONVIF_MALLOC(struct tt__ItemListDescription);
	ptr->AnalyticsModuleDescription[0].Messages[0].Data ->__sizeSimpleItemDescription = 1;
	ptr->AnalyticsModuleDescription[0].Messages[0].Data ->SimpleItemDescription = ONVIF_MALLOC_SIZE(struct _tt__ItemListDescription_SimpleItemDescription, 1);
	ptr->AnalyticsModuleDescription[0].Messages[0].Data->SimpleItemDescription[0].Name = "IsMotion";
	ptr->AnalyticsModuleDescription[0].Messages[0].Data ->SimpleItemDescription[0].Type = "xsd:boolean";
	ptr->AnalyticsModuleDescription[0].Messages[0].Data->__sizeElementItemDescription = 0;
	ptr->AnalyticsModuleDescription[0].Messages[0].ParentTopic = "tns1:RuleEngine/CellMotionDetector/Motion";
	ptr->AnalyticsModuleDescription[0].Messages[0].IsProperty = ONVIF_MALLOC(enum xsd__boolean);
	*ptr->AnalyticsModuleDescription[0].Messages[0].IsProperty = ntrue;
	ptr->AnalyticsModuleDescription[0].Messages[0].Key = NULL;
	ptr->AnalyticsModuleDescription[0].Messages[0].Extension = NULL;
	ptr->AnalyticsModuleDescription[0].Messages[0].__anyAttribute = NULL;
#endif
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateAnalyticsModules(struct soap *soap, struct _tan__CreateAnalyticsModules *tan__CreateAnalyticsModules, struct _tan__CreateAnalyticsModulesResponse *tan__CreateAnalyticsModulesResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteAnalyticsModules(struct soap *soap, struct _tan__DeleteAnalyticsModules *tan__DeleteAnalyticsModules, struct _tan__DeleteAnalyticsModulesResponse *tan__DeleteAnalyticsModulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModules(struct soap *soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules, struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse) 
{
	const char *celllayout_fmt = 
		"<tt:CellLayout Columns=\"%d\" Rows=\"%d\">"
		"<tt:Transformation>"
		"<tt:Translate x=\"%f\" y=\"%f\"/>"
		"<tt:Scale x=\"%f\" y=\"%f\"/>"
		"</tt:Transformation>"
		"</tt:CellLayout>";
	bool found = false;
	int n, chn = 0, id = 0;

	if (tan__GetAnalyticsModules->ConfigurationToken ) {
		for (n = 0; n < g_OnvifServerCxt->env.profiles.chn; n++) {
			if (strcmp(tan__GetAnalyticsModules->ConfigurationToken, g_OnvifServerCxt->env.profiles.profile[n].van.token) ==0) {
				found = true;
				chn = n;
				id = 0;
			}
		}
	}
	
	ONVIF_TRACE("\ttoken: %s", tan__GetAnalyticsModules->ConfigurationToken ? tan__GetAnalyticsModules->ConfigurationToken : "NULL");
	
	if (found == false) {
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	ONVIF_S_env_load(OM_MD OM_AND OM_VSRC, chn * 100 + id);
	
	tan__GetAnalyticsModulesResponse->__sizeAnalyticsModule = 1;
	tan__GetAnalyticsModulesResponse->AnalyticsModule = ONVIF_MALLOC_SIZE(struct tt__Config, 1);
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters = ONVIF_MALLOC(struct tt__ItemList);
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Name = g_OnvifServerCxt->env.profiles.profile[chn].md.module_name;
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Type = "tt:CellMotionEngine";
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->__sizeSimpleItem = 1;
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->SimpleItem = ONVIF_MALLOC_SIZE(struct _tt__ItemList_SimpleItem, 1);
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->SimpleItem[0].Name = "Sensitivity";
	onvif_put_int(soap, &tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->SimpleItem[0].Value,
		g_OnvifServerCxt->env.profiles.profile[chn].md.grid.sensitivity);
	
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->__sizeElementItem = 1;
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->ElementItem = ONVIF_MALLOC_SIZE(struct _tt__ItemList_ElementItem, 1);
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->ElementItem[0].Name = "Layout";
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->ElementItem[0].__any = ONVIF_MALLOC_SIZE(char, 2000);;
	snprintf(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->ElementItem[0].__any, 2000,
		celllayout_fmt, 
		g_OnvifServerCxt->env.profiles.profile[chn].md.grid.columnGranularity, 
		g_OnvifServerCxt->env.profiles.profile[chn].md.grid.rowGranularity, 
		(float)-1.0, (float)-1.0,
		(float)2.0/g_OnvifServerCxt->env.profiles.profile[chn].v_source.resolution.width, 
		(float)2.0/g_OnvifServerCxt->env.profiles.profile[chn].v_source.resolution.height);
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->Extension = NULL;
	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters ->__anyAttribute = NULL;

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyAnalyticsModules(struct soap *soap, struct _tan__ModifyAnalyticsModules *tan__ModifyAnalyticsModules, struct _tan__ModifyAnalyticsModulesResponse *tan__ModifyAnalyticsModulesResponse) 
{
	return SOAP_FAULT; 
}

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedRules_(struct soap *soap, struct _tan__GetSupportedRules *tan__GetSupportedRules, struct _tan__GetSupportedRulesResponse *tan__GetSupportedRulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateRules_(struct soap *soap, struct _tan__CreateRules *tan__CreateRules, struct _tan__CreateRulesResponse *tan__CreateRulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteRules_(struct soap *soap, struct _tan__DeleteRules *tan__DeleteRules, struct _tan__DeleteRulesResponse *tan__DeleteRulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRules_(struct soap *soap, struct _tan__GetRules *tan__GetRules, struct _tan__GetRulesResponse *tan__GetRulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyRules_(struct soap *soap, struct _tan__ModifyRules *tan__ModifyRules, struct _tan__ModifyRulesResponse *tan__ModifyRulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetServiceCapabilities_(struct soap *soap, struct _tan__GetServiceCapabilities *tan__GetServiceCapabilities, struct _tan__GetServiceCapabilitiesResponse *tan__GetServiceCapabilitiesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedAnalyticsModules_(struct soap *soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules, struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateAnalyticsModules_(struct soap *soap, struct _tan__CreateAnalyticsModules *tan__CreateAnalyticsModules, struct _tan__CreateAnalyticsModulesResponse *tan__CreateAnalyticsModulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteAnalyticsModules_(struct soap *soap, struct _tan__DeleteAnalyticsModules *tan__DeleteAnalyticsModules, struct _tan__DeleteAnalyticsModulesResponse *tan__DeleteAnalyticsModulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModules_(struct soap *soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules, struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse) { return SOAP_FAULT; }

SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyAnalyticsModules_(struct soap *soap, struct _tan__ModifyAnalyticsModules *tan__ModifyAnalyticsModules, struct _tan__ModifyAnalyticsModulesResponse *tan__ModifyAnalyticsModulesResponse) { return SOAP_FAULT; }


SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetServiceCapabilities(struct soap *soap, struct _tmd__GetServiceCapabilities *tmd__GetServiceCapabilities, struct _tmd__GetServiceCapabilitiesResponse *tmd__GetServiceCapabilitiesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetRelayOutputOptions(struct soap *soap, struct _tmd__GetRelayOutputOptions *tmd__GetRelayOutputOptions, struct _tmd__GetRelayOutputOptionsResponse *tmd__GetRelayOutputOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSources(struct soap *soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputs(struct soap *soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSources(struct soap *soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
	return __trt__GetVideoSources(soap, trt__GetVideoSources, trt__GetVideoSourcesResponse);
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputs(struct soap *soap, struct _tmd__GetVideoOutputs *tmd__GetVideoOutputs, struct _tmd__GetVideoOutputsResponse *tmd__GetVideoOutputsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSourceConfiguration(struct soap *soap, struct _tmd__GetVideoSourceConfiguration *tmd__GetVideoSourceConfiguration, struct _tmd__GetVideoSourceConfigurationResponse *tmd__GetVideoSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputConfiguration(struct soap *soap, struct _tmd__GetVideoOutputConfiguration *tmd__GetVideoOutputConfiguration, struct _tmd__GetVideoOutputConfigurationResponse *tmd__GetVideoOutputConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSourceConfiguration(struct soap *soap, struct _tmd__GetAudioSourceConfiguration *tmd__GetAudioSourceConfiguration, struct _tmd__GetAudioSourceConfigurationResponse *tmd__GetAudioSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputConfiguration(struct soap *soap, struct _tmd__GetAudioOutputConfiguration *tmd__GetAudioOutputConfiguration, struct _tmd__GetAudioOutputConfigurationResponse *tmd__GetAudioOutputConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetVideoSourceConfiguration(struct soap *soap, struct _tmd__SetVideoSourceConfiguration *tmd__SetVideoSourceConfiguration, struct _tmd__SetVideoSourceConfigurationResponse *tmd__SetVideoSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetVideoOutputConfiguration(struct soap *soap, struct _tmd__SetVideoOutputConfiguration *tmd__SetVideoOutputConfiguration, struct _tmd__SetVideoOutputConfigurationResponse *tmd__SetVideoOutputConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetAudioSourceConfiguration(struct soap *soap, struct _tmd__SetAudioSourceConfiguration *tmd__SetAudioSourceConfiguration, struct _tmd__SetAudioSourceConfigurationResponse *tmd__SetAudioSourceConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetAudioOutputConfiguration(struct soap *soap, struct _tmd__SetAudioOutputConfiguration *tmd__SetAudioOutputConfiguration, struct _tmd__SetAudioOutputConfigurationResponse *tmd__SetAudioOutputConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoSourceConfigurationOptions(struct soap *soap, struct _tmd__GetVideoSourceConfigurationOptions *tmd__GetVideoSourceConfigurationOptions, struct _tmd__GetVideoSourceConfigurationOptionsResponse *tmd__GetVideoSourceConfigurationOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetVideoOutputConfigurationOptions(struct soap *soap, struct _tmd__GetVideoOutputConfigurationOptions *tmd__GetVideoOutputConfigurationOptions, struct _tmd__GetVideoOutputConfigurationOptionsResponse *tmd__GetVideoOutputConfigurationOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioSourceConfigurationOptions(struct soap *soap, struct _tmd__GetAudioSourceConfigurationOptions *tmd__GetAudioSourceConfigurationOptions, struct _tmd__GetAudioSourceConfigurationOptionsResponse *tmd__GetAudioSourceConfigurationOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetAudioOutputConfigurationOptions(struct soap *soap, struct _tmd__GetAudioOutputConfigurationOptions *tmd__GetAudioOutputConfigurationOptions, struct _tmd__GetAudioOutputConfigurationOptionsResponse *tmd__GetAudioOutputConfigurationOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetRelayOutputs(struct soap *soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetRelayOutputSettings(struct soap *soap, struct _tmd__SetRelayOutputSettings *tmd__SetRelayOutputSettings, struct _tmd__SetRelayOutputSettingsResponse *tmd__SetRelayOutputSettingsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetRelayOutputState(struct soap *soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetDigitalInputs(struct soap *soap, struct _tmd__GetDigitalInputs *tmd__GetDigitalInputs, struct _tmd__GetDigitalInputsResponse *tmd__GetDigitalInputsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPorts(struct soap *soap, struct _tmd__GetSerialPorts *tmd__GetSerialPorts, struct _tmd__GetSerialPortsResponse *tmd__GetSerialPortsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPortConfiguration(struct soap *soap, struct _tmd__GetSerialPortConfiguration *tmd__GetSerialPortConfiguration, struct _tmd__GetSerialPortConfigurationResponse *tmd__GetSerialPortConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SetSerialPortConfiguration(struct soap *soap, struct _tmd__SetSerialPortConfiguration *tmd__SetSerialPortConfiguration, struct _tmd__SetSerialPortConfigurationResponse *tmd__SetSerialPortConfigurationResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__GetSerialPortConfigurationOptions(struct soap *soap, struct _tmd__GetSerialPortConfigurationOptions *tmd__GetSerialPortConfigurationOptions, struct _tmd__GetSerialPortConfigurationOptionsResponse *tmd__GetSerialPortConfigurationOptionsResponse)
{
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __tmd__SendReceiveSerialCommand(struct soap *soap, struct _tmd__SendReceiveSerialCommand *tmd__SendReceiveSerialCommand, struct _tmd__SendReceiveSerialCommandResponse *tmd__SendReceiveSerialCommandResponse)
{
	return SOAP_FAULT;
}

#endif // #ifdef SOAP_SERVER

