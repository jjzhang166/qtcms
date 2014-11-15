#include "stdinc.h"

#include "stdsoap2.h"
#include "wsddapi.h"
#include "soapStub.h"
#include "onvif_common.h"
#include "onvifC.h"

#include "onvif.h"
#include "generic.h"
#include "ezxml.h"
#include "stack.h"
#include "nvp_define.h"
#include "sha1.h"
#include "_base64.h"
#include "sock.h"
#include "onvif_debug.h"
#ifdef HAVE_ARP
#include "arp.h"
#endif // HAVE_ARP
#include "cross.h"
#include "cross_thread.h"

#ifdef SOAP_CLIENT

#define ONVIF_MAX_SEARCH_NUM	(255)
#define ONVIF_TIMEOUT		(6)
#define ONVIF_SERVER_URI	"http://%s:%d/onvif/device_service"


typedef enum{
	ONVIF_MODEULE_ALL = 0,
	ONVIF_ANALYTICS,
	ONVIF_DEVICE,
	ONVIF_EVENTS,
	ONVIF_IMAGING,
	ONVIF_MEDIA,
	ONVIF_PTZ,	
	ONVIF_MODULE_CNT
}enONVIF_MODULE;

typedef int (*fSOAP_CALL)();
//typedef int (*fSOAP_CALL)(struct soap *soap, const char *soap_endpoint, const char *soap_action, void *in, void *out);

typedef struct ONVIF_PRIV_CONTEXT
{
	stNVP_ARGS args;
	//
	bool sync;
	time_t sync_time;
	//
	//
	bool auth;
	bool module_support[ONVIF_MODULE_CNT];
	char endpoint[ONVIF_MODULE_CNT][64];

	fNVPEventHook nvp_hook[NVP_HOOK_EVENT_NR];
	void *nvp_hook_custom[NVP_HOOK_EVENT_NR];

	bool event_subscribe;
	bool event_pull;
	fNVPEventHook md_hook;
	void *md_hook_custom;
	char md_reference[128];

	lpNVP_DEV_INFO info;
	int chn;
	lpNVP_PROFILE_CHN profiles;	
}stONVIF_PRIV_CONTEXT, *lpONVIF_PRIV_CONTEXT;

#define ONVIF_TRUE		xsd__boolean__true_
#define ONVIF_FALSE	xsd__boolean__false_
static enum xsd__boolean m_NTRUE = xsd__boolean__true_;
static enum xsd__boolean m_NFALSE = xsd__boolean__false_;

lpONVIF_C_CONTEXT g_OnvifClientCxt = NULL;

typedef struct ONVIF_DEV
{
	char ip[20];
	int port;
	char name[32];
	char firmware[32];
	char location[32];
	int dev_type;
	time_t t_online;
}stONVIF_DEVICE, *lpONVIF_DEVICE;

int ONVIF_renew_event_ex(lpNVP_ARGS args, lpNVP_SUBSCRIBE subs) ;
int ONVIF_get_devinfo(lpNVP_ARGS args, lpNVP_DEV_INFO info);

static inline int RTSP_URI_PARSE(char *src,unsigned char *ip,unsigned short *port,char *stream)
{
	char tmp[128],*ptr;
	char *p=NULL;
	int _port,_ip[4];
	if(strncmp(src,"rtsp://",strlen("rtsp://"))!=0){
		return -1;
	}else{
		ptr=src;
	}
	if((p = strstr(ptr+strlen("rtsp://"),":")) ==NULL){
		_port = 554;
		sscanf(ptr,"rtsp://%[^/]/%s",tmp,stream);
	}else{
		sscanf(ptr,"rtsp://%[^:]:%d/%s",tmp,&_port,stream);
	}
	*port = (unsigned short)_port;
	if(sscanf(tmp,"%d.%d.%d.%d",&_ip[0],&_ip[1],&_ip[2],&_ip[3])!=4){
		return -1;
	}
	ip[0] = _ip[0];
	ip[1] = _ip[1];
	ip[2] = _ip[2];
	ip[3] = _ip[3];

	//ONVIF_TRACE("ip:%s stream:%s port:%d",tmp,stream,_port);
	return 0;
}

#ifndef WIN32
static int local_ip(char *eth, char *szip)
{
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;

	szip[0] = 0;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		perror("socket");
		return -1;   
	}  

	strncpy(ifr.ifr_name, eth, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0; 
   	
	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) 
	{
		perror("ioctl");
		close(sock);
		return -1;
	}
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	strcpy(szip,inet_ntoa(sin.sin_addr));
	close(sock);
	return 0;
}

static int is_same_network(char *eth,unsigned char *ipv4_dst)
{
	int sock;
	char szip[20];
	unsigned char ipv4_host[4],netmask[4];
	//
	unsigned int tmp1,tmp2,tmp3;
	struct sockaddr_in sin;
	struct ifreq ifr;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		perror("socket");
		return -1;   
	}  

	strncpy(ifr.ifr_name, eth, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0; 
   	
	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) 
	{
		perror("ioctl");
		close(sock);
		return -1;
	}
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	strcpy(szip,inet_ntoa(sin.sin_addr));
	if (ipstr2uint8(ipv4_host,szip) < 0){
		perror("ipstr2uint8");
		close(sock);
		return -1;
	}
	
	if (ioctl(sock, SIOCGIFNETMASK, &ifr)< 0) 
	{
		perror("ioctl");
		close(sock);
		return -1;
	}
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));  
	strcpy(szip,inet_ntoa(sin.sin_addr));
	if (ipstr2uint8(netmask,szip) < 0){
		perror("ipstr2uint8");
		close(sock);
		return -1;
	}

	tmp1 = (ipv4_host[0] << 24) | (ipv4_host[1] << 16) | (ipv4_host[2] << 8) | ipv4_host[3];
	tmp2 = (ipv4_dst[0] << 24) | (ipv4_dst[1] << 16) | (ipv4_dst[2] << 8) | ipv4_dst[3];
	tmp3 = (netmask[0] << 24) | (netmask[1] << 16) | (netmask[2] << 8) | netmask[3];
	if((tmp1 & tmp3) == (tmp2 & tmp3)){
		close(sock);
		return 1;
	}
	//ONVIF_TRACE("not same network");
	close(sock);
	return 0;
}
#endif // WIN32

static int compute_password_digest(char *password,char *nonce,
	char *created, char *out, int out_size)
{
	SHA1_CTX sha;
	unsigned char u8sha1[SHA1_MAC_LEN];
	char tmp[512];
	int ret;
	
	// Password_Digest = Base64 ( SHA-1 ( nonce + created + password ) ) 
	//strcpy(tmp,nonce);
	ret = BASE64_decode(nonce, strlen(nonce), tmp, sizeof(tmp));
	tmp[ret] = 0;
	//strcat(tmp,created);
	//strcat(tmp,password);
	//printf("result: %s count:%d\n",tmp, ret);
	SHA1Reset(&sha);
	SHA1Input(&sha, (unsigned char *)tmp, ret);
	SHA1Input(&sha, (unsigned char *)created, strlen(created));
	SHA1Input(&sha, (unsigned char *)password, strlen(password));
	SHA1Result(&sha, u8sha1);
	ret = BASE64_encode(u8sha1, sizeof(u8sha1), out, out_size);
	out[ret] = 0;
	//ONVIF_TRACE("PasswordDigest is: %s", out);
	return ret;
}

static int compute_password_digest2(char *password,char *nonce,
	char *created, char *out, int out_size)
{
	SHA1_CTX sha;
	unsigned char u8sha1[SHA1_MAC_LEN];
	int ret;
	
	SHA1Reset(&sha);
	SHA1Input(&sha, (unsigned char *)nonce, strlen(nonce));
	SHA1Input(&sha, (unsigned char *)created, strlen(created));
	SHA1Input(&sha, (unsigned char *)password, strlen(password));
	SHA1Result(&sha, u8sha1);
	ret = BASE64_encode(u8sha1, sizeof(u8sha1), out, out_size);
	out[ret] = 0;
	ONVIF_TRACE("PasswordDigest is: %s", out);
	return ret;
}


static int update_change(char *pwd, char *nonce, char *created, char *digest, int size)
{
	time_t t;
	struct tm *ptm;
	char sznonce[64];

	time(&t);
	ptm = gmtime(&t);
	sprintf(created, "%04d-%02d-%02dT%02d:%02d:%02d.0Z",ptm->tm_year + 1900,ptm->tm_mon,
		ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);
	sprintf(sznonce,"%lu%lu%lu", t & 0x379a5a, t,  t & 0x754aaa);
	BASE64_encode(sznonce, strlen(sznonce), nonce, size);
	return compute_password_digest2(pwd, sznonce, created, digest, size);
}

static int  _onvif_device_compare(StackItem_t aa, StackItem_t bb)
{
	int i;
	uint8_t a_ip[4], b_ip[4];
	stONVIF_DEVICE *A = (stONVIF_DEVICE *)aa;
	stONVIF_DEVICE *B = (stONVIF_DEVICE *)bb;
	if ((ipstr2uint8(a_ip, A->ip) < 0) ||
		(ipstr2uint8(b_ip, B->ip) < 0)) {
		ONVIF_INFO("compare invalid ip (%s, %s)", A->ip, B->ip);
		return -1;
	}
	for ( i = 0; i < 4 ; i++) {
		if (a_ip[i] < b_ip[i]) {
			return -1;
		} else if (a_ip[i] > b_ip[i]) {
			return 1;
		}		
	}
	return 0;
}

void ONVIFC_wsdd_event_hook(char *dev_type, char *xaddr, char *scopes, int wsdd_event_type)
{
	stONVIF_DEVICE device;
	unsigned char u_ip[4];
	unsigned short _port;
	char _url[64];
	char *ptr;
	char httpaddr[200];
	char temp[512];
	
	if (g_OnvifClientCxt == NULL)
		return;
	if ((g_OnvifClientCxt->check_with_arp == false) &&
		(g_OnvifClientCxt->online_timeout <= 0)) {
		return;
	}

	if (xaddr == NULL) return;
	memset(&device, 0, sizeof(device));

	ptr = strstr(xaddr,  "http://");
	if(ptr == NULL) return;
	if(ptr[strlen("http://")] == '['){//ipv6
		ptr = strstr(ptr+strlen("http://"),  "http://");
		if(ptr == NULL) return;
		if(ptr[strlen("http://")] == '['){//ipv6
			return;
		}
	}
	// get ipv4 address
	sscanf(ptr, "%s", httpaddr);
	
	if (http_parse_url(u_ip, &_port, _url, httpaddr) < 0) {
		return ;
	}
	_ip_2string(u_ip, device.ip);
	device.port = _port;
	
	if (dev_type) {
		if (strstr(dev_type, "NetworkVideoTransmitter") != NULL) {
			device.dev_type = ONVIF_DEV_NVT;
		}else if (strstr(dev_type, "NetworkVideoDisplay") != NULL) {
			device.dev_type = ONVIF_DEV_NVD;
		}else if (strstr(dev_type, "NetworkVideoStorage") != NULL) {
			device.dev_type = ONVIF_DEV_NVS;
		}else if (strstr(dev_type, "NetworkVideoAnalytics") != NULL) {
			device.dev_type = ONVIF_DEV_NVA;
		} else {
			return;
		}
	} else {
		return;
	}
	
	if (scopes) {
		ptr = strstr(scopes,"onvif://www.onvif.org/name/");
		if(ptr){
			ptr+=strlen("onvif://www.onvif.org/name/");
			sscanf(ptr,"%s", temp);
			if ((ptr = strstr(temp, "onvif://")) != NULL) {
				*ptr = 0;
			}
			strcpy(device.name, temp);
		}
		ptr = strstr(scopes,"onvif://www.onvif.org/hardware/");
		if(ptr){
			ptr+=strlen("onvif://www.onvif.org/hardware/");
			sscanf(ptr,"%s", temp);
			if ((ptr = strstr(temp, "onvif://")) != NULL) {
				*ptr = 0;
			}
			strcpy(device.firmware, temp);
		}

		ptr = strstr(scopes,"onvif://www.onvif.org/location/country/");
		if(ptr){
			ptr+=strlen("onvif://www.onvif.org/location/country/");
			sscanf(ptr,"%s", temp);
			if ((ptr = strstr(temp, "onvif://")) != NULL) {
				*ptr = 0;
			}
			strcpy(device.location, temp);
		}
		ptr = strstr(scopes,"onvif://www.onvif.org/location/city/");
		if(ptr){
			ptr+=strlen("onvif://www.onvif.org/location/city/");
			sscanf(ptr,"%s", temp);
			if ((ptr = strstr(temp, "onvif://")) != NULL) {
				*ptr = 0;
			}
			strcpy(device.location, temp);
		}
	}

	if (g_OnvifClientCxt->devices) {
		if (wsdd_event_type == WSDD_EVENT_HELLO){
			time(&device.t_online);
			STACK_push_to_bottom(g_OnvifClientCxt->devices, &device);
		}else if (wsdd_event_type == WSDD_EVENT_BYE)
			STACK_del(g_OnvifClientCxt->devices, &device);
	}
}

static void  _onvif_device_alive_check(StackItem_t aa)
{
	lpONVIF_DEVICE device = (lpONVIF_DEVICE)aa;
#ifdef WIN32
	unsigned long mac[6];
	unsigned long uLen = 6;
	IPAddr srcAddr = inet_addr(device->ip);
	DWORD dwRet = SendARP(srcAddr,0,mac,&uLen);
	if (0 != dwRet)
	{
		uLen = 6;
		dwRet = SendARP(srcAddr,0,mac,&uLen);
		if (0 != dwRet)
		{
			ONVIF_INFO("not found device:%s , delete it", device->ip);
			STACK_del(g_OnvifClientCxt->devices, device);		}
	}
#else // WIN32
#	if 0	
	stNVP_ARGS args;
	stNVP_DEV_INFO info;
	unsigned char u_ipv4[4];
	int ret;
		
	if (g_OnvifClientCxt== NULL)
		return;

	memset(&info, 0, sizeof(info));
	if (ipstr2uint8(u_ipv4, device->ip) < 0) {
		ONVIF_INFO("invalid ip: %s", device->ip);
		return;
	}
	if (is_same_network(ONVIF_DEFAULT_ETHER, u_ipv4) == 1) {
		NVP_INIT_ARGS(args, device->ip, device->port, "admin", "");
		ONVIF_TRACE("_onvif_device_alive_check %s:%d", device->ip, device->port);
		if ((ret = ONVIF_get_devinfo(&args, &info)) < 0) {
			if (ret != NVP_RET_AUTH_FAILED) {
				ONVIF_INFO("not found device:%s , delete it", device->ip);
				STACK_del(g_OnvifClientCxt->devices, device);
			}
		} 
	}
#	else // 0
#		ifdef HAVE_ARP
	char mac[18];
	int ret;
	ONVIF_TRACE("_onvif_device_alive_check %s:%d", device->ip, device->port);
	if ((ret = ARP_query(device->ip, mac))< 0) {
		;
	} else if (ret == 0) {
		// try again
		if ((ret = ARP_query(device->ip, mac)) == 0) {
			ONVIF_INFO("not found device:%s , delete it", device->ip);
			STACK_del(g_OnvifClientCxt->devices, device);
		}
	}
#		else // HAVE_ARP
	if (device) {
		STACK_del(g_OnvifClientCxt->devices, device);
	}
#		endif // HAVE_ARP
#	endif // 0
#endif // WIN32
}

static THREAD_RETURN _discover_proc(void *param)
{
	lpONVIF_C_CONTEXT onvif = (lpONVIF_C_CONTEXT)param;
	stONVIF_DEVICE device;
	time_t tt = 0;
	
	if (onvif == NULL)
		return (THREAD_RETURN)NULL;
	
	ONVIF_INFO("onvif discover thread (0x%lx) start...", (long)onvif->pid_discover);

	while(onvif->trigger_discover) {
		JStack_t *stack = NULL;
		time_t t_now;
		time(&t_now);
		if (onvif->devices) {
			if (onvif->check_with_arp){
				if ((t_now - tt) >= 30) {
					stack = STACK_dup(onvif->devices);
					if (stack != NULL) {
						while(STACK_pop(stack, &device) == 0) {
							_onvif_device_alive_check(&device);
						}
						STACK_destroy(stack);
					}
					time(&tt);
				}
			} else {
				stack = STACK_dup(onvif->devices);
				if (stack != NULL) {
					while(STACK_pop(stack, &device) == 0) {
						if ((t_now - device.t_online) > onvif->online_timeout){
							ONVIF_TRACE("check online timeout(%d), delete %s", onvif->online_timeout,
								device.ip);
							STACK_del(g_OnvifClientCxt->devices, &device);
						}
					}
					STACK_destroy(stack);
				}
			}
		}
		Sleep_c(2);
	}
	
	ONVIF_INFO("onvif discover thread (0x%lx) stop.", (long)onvif->pid_discover);
	return (THREAD_RETURN)NULL;
}

static void start_discover_proc(lpONVIF_C_CONTEXT onvif)
{
	if (onvif == NULL)
		return;
	if (onvif->pid_discover == 0) {
		onvif->trigger_discover = true;
		initThread_c(&onvif->pid_discover,_discover_proc,onvif);
	}
}

static void stop_discover_proc(lpONVIF_C_CONTEXT onvif)
{
	if (onvif == NULL)
		return;
	if (onvif->pid_discover) {
		onvif->trigger_discover = false;
		ONVIF_INFO("try to stop discover thread (0x%lx) ", (long)onvif->pid_discover);
		joinThread_c(onvif->pid_discover);
	}
}


void ONVIF_C_env_load(const char *module, int keyid)
{
	NVP_env_load(&g_OnvifClientCxt->env, module, keyid);
}

void ONVIF_C_env_save(const char *module, int keyid)
{
	NVP_env_save(&g_OnvifClientCxt->env, module, keyid);
}



int ONVIF_C_event_subscribe(int event_type, char *peer_reference, char *local_reference, int timeout,
	char *username, char *password,
	fNVPEventHook hook, void *param, void *private)
{
	lpONVIF_C_CONTEXT onvif = g_OnvifClientCxt;
	lpONVIF_EVENT_SUBSCRIBE event = onvif->event_list;
	lpONVIF_EVENT_SUBSCRIBE e = NULL;
	unsigned char ipv4[4];
	unsigned short port;
	char uri[128];
	time_t t_now;
		
	mutex_lock(onvif->mutex);

	time(&t_now);
	if (local_reference[0] == 0) {
		SNPRINTF(local_reference, 200, "http://%s:%d%s/%ld_%d",
			onvif->event_bindip, onvif->event_port, 
			ONVIF_EVENT_PULLPOINT_URI_PREFIX,
			t_now, onvif->event_user + 1);
	}
	
	if (http_parse_url(ipv4, &port, uri, peer_reference) < -1) 
	{
		mutex_lock(onvif->mutex);
		return -1;		
	}
	
	while(event && event->next) event= event->next;
	e = (lpONVIF_EVENT_SUBSCRIBE)calloc(1, sizeof(stONVIF_EVENT_SUBSCRIBE));
	e->pid = 0;
	e->sock = 0;
	e->event_type = event_type;
	sprintf(e->peer_addr, "%d.%d.%d.%d", ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
	e->peer_port = port;
	strcpy(e->username, username);
	strcpy(e->userpwd, password);
	e->hook = hook;
	e->hook_custom = param;
	strncpy(e->peer_reference, peer_reference, sizeof(e->peer_reference) -1);
	strncpy(e->local_reference, local_reference, sizeof(e->local_reference) -1);
	e->timeout = timeout;
	time(&e->m_sync);
	e->trigger = false;
	e->private = private;
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

int ONVIF_C_event_renew(char *reference, int64_t *terminal_duration)
{
	lpONVIF_C_CONTEXT onvif = g_OnvifClientCxt;
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


int ONVIF_C_event_find(char *reference, lpONVIF_EVENT_SUBSCRIBE entry)
{
	lpONVIF_C_CONTEXT onvif = g_OnvifClientCxt;
	lpONVIF_EVENT_SUBSCRIBE event = onvif->event_list;
	char uri_local[128];
	char uri[128];

	//printf("event : %p user: %d\n", event, onvif->event_user);
	
	mutex_lock(onvif->mutex);
	while( event) {
		http_parse_url(NULL, NULL, uri_local, event->local_reference);
		http_parse_url(NULL, NULL, uri, reference);
		if (strncmp(uri_local, uri, strlen(uri)) == 0) {
			memcpy(entry, event, sizeof(stONVIF_EVENT_SUBSCRIBE));
			mutex_unlock(onvif->mutex);
			return 0;
		}
		event = event->next;
	}
	
	mutex_unlock(onvif->mutex);
	return -1;
}

int ONVIF_C_event_unsubscribe(char *reference)
{
	lpONVIF_C_CONTEXT onvif = g_OnvifClientCxt;
	lpONVIF_EVENT_SUBSCRIBE event = onvif->event_list;
	lpONVIF_EVENT_SUBSCRIBE pre = NULL;
	char uri_peer[128];
	char uri[128];

	mutex_lock(onvif->mutex);

	while(event) 
	{
		lpONVIF_EVENT_SUBSCRIBE tmp = event->next;
		
		http_parse_url(NULL, NULL, uri_peer, event->peer_reference);
		http_parse_url(NULL, NULL, uri, reference);
		if (strncmp(uri_peer, uri, strlen(uri)) == 0) {
			ONVIF_TRACE("\t%s unsubscribe ", event->peer_reference);
			if (event->trigger && event->pid != 0) {
				event->trigger = false;
				joinThread_c(event->pid);
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



int onvif_set_nvp_event_hook(lpNVP_ARGS args, NVP_HOOK_EVENT event, fNVPEventHook hook, void *custom)
{
	lpNVP_INTERFACE iface = (lpNVP_INTERFACE)args->thiz;
	
	if(args->thiz){
		lpONVIF_PRIV_CONTEXT context = (lpONVIF_PRIV_CONTEXT)iface->private_ctx;
		if (context == NULL) 
			return -1;
		ONVIF_ASSERT(event < NVP_HOOK_EVENT_NR, "unknown nvp hook event");
		context->nvp_hook[event] = hook;
		context->nvp_hook_custom[event] = custom;
		return 0;
	}
	return -1;
}


static THREAD_RETURN onvif_c_event_proc(void *param)
{
	int sock = *((int *)param);

	free(param);
	detatchThread_c(currentThreadId_c());
	ONVIF_SERVER_daemon( sock);
	return (THREAD_RETURN)NULL;
}

static THREAD_RETURN onvif_c_event_renew_proc(void *param)
{
#define RENEW_MAX_RETRY	(4)
	stONVIF_EVENT_SUBSCRIBE event;
	stNVP_ARGS args;
	stNVP_SUBSCRIBE subscribe;
	int error_occur = 0;
	
	detatchThread_c(currentThreadId_c());
	
	memcpy(&event, param, sizeof(event));
	NVP_INIT_ARGS(args, event.peer_addr, event.peer_port, event.username, event.userpwd);
	memset(&subscribe, 0, sizeof(subscribe));
	subscribe.keeplive_time = event.timeout;
	strcpy(subscribe.reference, event.peer_reference);
	for(; error_occur < RENEW_MAX_RETRY; error_occur++) {
		if (ONVIF_renew_event_ex(&args, &subscribe) == 0){
			break;
		} else {
			sleep_c(500);
		}
	}
	// FIX ME
	if (error_occur == RENEW_MAX_RETRY) {
		ONVIF_INFO("onvif event renew failed!");
		ONVIF_C_event_unsubscribe(event.local_reference);
		if (event.private) {
			lpONVIF_PRIV_CONTEXT context = (lpONVIF_PRIV_CONTEXT)event.private;
			if (context->nvp_hook[NVP_EVENT_RENEW_FAILED]) {
				context->nvp_hook[NVP_EVENT_RENEW_FAILED](NVP_EVENT_RENEW_FAILED, 0, 0, 
					context->nvp_hook_custom[NVP_EVENT_RENEW_FAILED]);
			}
		}
	}
	
	return (THREAD_RETURN)NULL;
}


/*
static void *onvif_c_event_listen_proc(void *param)
{
	int sock;
	fd_set rset;
	struct timeval timeout;
	lpONVIF_C_CONTEXT onvif = (lpONVIF_C_CONTEXT)param;

	if (onvif == NULL) {
		return NULL;
	}

	// init listen socket
	sock = SOCK_tcp_listen(onvif->event_port);
	if (sock <= 0) {
		ONVIF_INFO("create listen socket@%d failed!", onvif->event_port);
		return NULL;
	}

	while(onvif->event_trigger) 
	{
		int ret;
		pthread_t pid;
		int client_sock = -1;
		
		FD_ZERO(&rset);
		FD_SET(sock, &rset);
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		
		ret = select(sock + 1, &rset, NULL, NULL, &timeout);
		if (ret < 0) {
		} else if (ret == 0){
		} else  {
			if (FD_ISSET(sock, &rset)) {
				ONVIF_TRACE("got a event notified connection");
				//
				assept(
			}
		}
	}
}
*/

static THREAD_RETURN onvif_c_event_listen_proc(void *arg)
{
	lpONVIF_C_CONTEXT onvif = (lpONVIF_C_CONTEXT)arg;
	lpONVIF_EVENT_SUBSCRIBE event;
	int m, s = 0; /* master and slave sockets */
	struct soap *add_soap = soap_new();
	fd_set rset;
	pthread_t pid;
	time_t t_now;
	struct timeval timeout;
	int ret;

	if(add_soap == NULL){
		ONVIF_INFO("soap new failed!");
		onvif->event_trigger = false;
		exitThread_c(NULL);
	}

	soap_set_namespaces(add_soap, namespaces);
	add_soap->bind_flags |= SO_REUSEADDR;

	m = soap_bind(add_soap, NULL, onvif->event_port, 32);
	if (m < 0)
	{
		soap_print_fault(add_soap, stderr);
		onvif->event_trigger = false;
		ONVIF_INFO("soap bind failed!");
		exitThread_c(NULL);
	}

	add_soap->recv_timeout = 3;
	add_soap->send_timeout = 3;

	ONVIF_INFO("onvif event_c server start, pid(0x%lx)...", (long)currentThreadId_c());
	
	while(onvif->event_trigger == true) {
		FD_ZERO(&rset);
		FD_SET(add_soap->master, &rset);

		timeout.tv_sec = 1;
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
					int *sock = (int *)malloc(sizeof(int));
					/*
					//ONVIF_TRACE("Socket connection successful: slave socket = %d", add_soap->socket);
					if ((ret = soap_serve(add_soap)) != SOAP_OK)
						ONVIF_INFO("soap serve error : %d!", ret);
					//soap_destroy(add_soap);
					
					soap_end(add_soap);
					*/
					*sock = (int)s;
					initThread_c(&pid,onvif_c_event_proc,(void *)sock);
				}
			}
		}

		// renew events
		event = onvif->event_list;
		time(&t_now);
		while(event) 
		{
			int dura = event->timeout -4;
			if (dura < 0)
				dura = event->timeout -1;
			if (dura <= 0)
				dura = 3;
			if ((t_now - event->m_sync ) > dura) {
				initThread_c(&pid,onvif_c_event_renew_proc, event);
				time(&event->m_sync);
			}

			event = event->next;
		}
	}

	soap_free(add_soap);	 
	ONVIF_INFO("onvif event_c server daemon !");
	exitThread_c(NULL);
	return (THREAD_RETURN)NULL;
}

static THREAD_RETURN onvif_event_renew_deamon(void *arg)
{
	lpONVIF_C_CONTEXT onvif = (lpONVIF_C_CONTEXT)arg;
	lpONVIF_EVENT_SUBSCRIBE event;
	pthread_t pid;
	time_t t_now;

	ONVIF_INFO("onvif event renew proc start, pid(0x%lx)...", (long)currentThreadId_c());
	
	while(onvif->event_trigger == true) {
		// renew events
		event = onvif->event_list;
		time(&t_now);
		while(event) 
		{
			if ((t_now - event->m_sync ) > ( event->timeout - 3)) {
				initThread_c(&pid,onvif_c_event_renew_proc,event);
				time(&event->m_sync);
			}

			event = event->next;
			sleep_c(100);
		}
		Sleep_c(1);
	}

	ONVIF_INFO("onvif event renew proc stop !");
	exitThread_c(NULL);
	return (THREAD_RETURN)NULL;
}


void ONVIF_event_daemon_start(char *bindip, int port)
{
	lpONVIF_C_CONTEXT onvif  = g_OnvifClientCxt;
	if (onvif)
	{
		if (onvif->event_port == 0) {
			if (port > 0)
				onvif->event_port = port;
			else
				onvif->event_port = ONVIF_DEFAULT_EVENT_LISTEN_PORT;
			strcpy(onvif->event_bindip, bindip);
			onvif->event_trigger = true;
			ONVIF_INFO("onvif event daemon listen @(%s:%d)", onvif->event_bindip, onvif->event_port);
			initThread_c(&onvif->event_pid,  onvif_c_event_listen_proc, onvif);
		}
	}
}

void ONVIF_event_daemon_start2(char *bindip, int port)
{
	lpONVIF_C_CONTEXT onvif  = g_OnvifClientCxt;
	if (onvif)
	{
		onvif->event_port = port;
		strcpy(onvif->event_bindip, bindip);
		onvif->event_trigger = true;
		ONVIF_INFO("onvif event daemon listen @(%s:%d)", onvif->event_bindip, onvif->event_port);
		initThread_c(&onvif->event_pid,  onvif_event_renew_deamon, onvif);
	}
}


static lpONVIF_C_CONTEXT ONVIF_CLIENT_new(int conn_timeout, int send_timeout, int recv_timeout)
{
	lpONVIF_C_CONTEXT onvif = calloc(1, sizeof(stONVIF_C_CONTEXT));
	ONVIF_ASSERT(onvif != NULL, "onvif client new failed!");

	onvif->event_pid = 0;
	onvif->event_trigger = false;
	onvif->event_list = NULL;
	onvif->event_user = 0;
	onvif->event_bindip[0] = 0;
	onvif->mutex = mutex_create();

	onvif->connect_timeout = conn_timeout;
	onvif->send_timeout = send_timeout;
	onvif->recv_timeout = recv_timeout;
	
	onvif->devices = STACK_init(sizeof(stONVIF_DEVICE));
	STACK_set_comp(onvif->devices, _onvif_device_compare);
	onvif->pid_discover = 0;
	onvif->trigger_discover = false;
	start_discover_proc(onvif);

	return onvif;
}

static void ONVIF_CLIENT_delete(lpONVIF_C_CONTEXT onvif)
{
	if (onvif) 
	{
		if (onvif->event_pid ) {
			ONVIF_INFO("try to stop client event thread (0x%lx) ", (long)onvif->event_pid);
			onvif->event_trigger = false;
			joinThread_c(onvif->event_pid);
			onvif->event_pid = 0;
		}		
		stop_discover_proc(onvif);
		STACK_destroy(onvif->devices);
		mutex_destroy(onvif->mutex);

		free(onvif);
		onvif=NULL;
		ONVIF_INFO("%s done ", __FUNCTION__);
	}
}

void ONVIF_CLIENT_init(int conn_timeout, int send_timeout, int recv_timeout,
	bool check_with_arp, int online_timeout)
{
	if (g_OnvifClientCxt == NULL) {
		g_OnvifClientCxt = ONVIF_CLIENT_new(conn_timeout, send_timeout, recv_timeout);
		if (g_OnvifClientCxt){
#ifdef HAVE_ARP
			g_OnvifClientCxt->check_with_arp = check_with_arp;
#else // HAVE_ARP
			if (check_with_arp == true){
				ONVIF_INFO("invalid arg, check_with_arp is true,please Enable arp by -DHAVE_ARP");
				exit(1);
			}
#endif // HAVE_ARP
			g_OnvifClientCxt->online_timeout = online_timeout;
		}
	}
}

void ONVIF_CLIENT_deinit()
{
	if (g_OnvifClientCxt) {
		ONVIF_CLIENT_delete(g_OnvifClientCxt);
		g_OnvifClientCxt=NULL;
	}
}


static inline void SOAP_destroy(struct soap *soap)
{
	if(soap){
		soap_destroy(soap);
		soap_end(soap);
		soap_free(soap);
		//soap_done(soap);
	}
}

int ONVIF_search(int device_type, bool add_hello_dev, int recv_timeout_s, 
	fOnvifSearchFoundHook hook,  const char *bind_host,	
	void *customCtx)
{
	struct soap *soap;
	struct wsdd__ProbeType req;
	struct __wsdd__ProbeMatches resp;
	struct wsdd__ScopesType sScope;
	struct SOAP_ENV__Header header;
	int count = 0;
	int berror = 0;
	int ret = 0;
	const char *uuid=NULL;
	char *ptr = NULL;
	unsigned char ipv4[4];
	char szipv4[16];
	char szname[32];
	char szfirmware[60];
	char szlocation[60];
	unsigned short iport;
	JStack_t *stack = NULL;
	JStack_t *dev_bak = NULL;
	stONVIF_DEVICE device;
	//struct sockaddr_in addr;
	//socklen_t addrlen = sizeof(struct sockaddr_in);
	//in_addr_t if_addr = inet_addr("239.255.255.250"); // optional

#if 0
	if((soap = soap_new1(SOAP_IO_UDP|SOAP_IO_FLUSH | SOAP_IO_LENGTH))==NULL){
		ONVIF_INFO("soap_new failed!");
		return -1;
	}
	//soap->ipv4_multicast_if = (char *)&if_addr;
	soap->connect_flags |= SO_BROADCAST;

	soap->master= SOCK_multicast_udp_init("239.255.255.250", 10000, 2000, "192.168.1.81");
	if(!soap_valid_socket(soap->master))
	{ 
		soap_print_fault(soap, stderr);
		return -1;
	}

#else // 0
	struct ip_mreq mcast;

	soap = soap_new1(SOAP_IO_UDP|SOAP_IO_FLUSH | SOAP_IO_LENGTH);
	if (soap == NULL) {
		ONVIF_INFO("soap_new failed!");
		return -1;
	}
	
	//soap->ipv4_multicast_if = (char *)&if_addr;
	//soap->connect_flags |= SO_BROADCAST;
	
	soap->bind_flags |= SO_REUSEADDR;
	if(!soap_valid_socket(soap_bind(soap, bind_host, 0, 16)))
	{ 
		soap_print_fault(soap, stderr);
		return -1;
	}

	mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	mcast.imr_interface.s_addr = bind_host ? inet_addr(bind_host) : htonl(INADDR_ANY);
	if(setsockopt(soap->master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
		printf("setsockopt error! error code = %d,err string = %s\n",errno,strerror(errno));
		return -1;
	}	
#endif // 0
	uuid = soap_wsa_rand_uuid(soap);
	if (strncmp(uuid, "urn:uuid", strlen("urn:uuid")) == 0) {
		uuid += strlen("urn:");
	}
	ONVIF_TRACE("uuid=%s ",uuid);

	soap_set_namespaces(soap, discovery_namespaces);

	soap->version = 2;
	soap->send_timeout = g_OnvifClientCxt->send_timeout;
	if (recv_timeout_s > 0 && recv_timeout_s <= 60) {
		soap->recv_timeout = recv_timeout_s;
	} else {
		soap->recv_timeout = g_OnvifClientCxt->recv_timeout;
	}
	soap->connect_timeout = g_OnvifClientCxt->connect_timeout;

	soap->bind_flags = soap->bind_flags | SO_REUSEADDR;
	soap_default_SOAP_ENV__Header(soap, &header);
	header.wsa__MessageID = (char *)uuid;
	header.wsa__To     = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
	header.wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
	//header.wsa__Action = "http://schemas.xmllocal_soap.org/ws/2005/04/discovery/Probe";
	soap->header = &header;

	soap_default_wsdd__ScopesType(soap, &sScope);
	sScope.__item = "";     
	//sScope.__item = "onvif://www.onvif.org";
	soap_default_wsdd__ProbeType(soap, &req);
	req.Scopes = &sScope;  
	if (device_type == ONVIF_DEV_NVT)
		req.Types = "dn:NetworkVideoTransmitter";
	else if (device_type == ONVIF_DEV_NVD)
		req.Types = "dn:NetworkVideoDisplay";
	else if (device_type == ONVIF_DEV_NVS)
		req.Types = "dn:NetworkVideoStorage";
	else if (device_type == ONVIF_DEV_NVA)
		req.Types = "dn:NetworkVideoAnalytics";
	else
		req.Types = "dn:Device";
#if 1
	//strcpy(soap->host, "239.255.255.250");
	//soap->port = 3702;
	soap->peer.sin_family = AF_INET;
	soap->peer.sin_addr.s_addr = inet_addr("239.255.255.250") ;
	soap->peer.sin_port = htons(3702);
	soap->peerlen = sizeof(struct sockaddr_in);
	soap->keep_alive = 1; //not forget
	ret = soap_send___wsdd__Probe(soap, "soap.udp://239.255.255.250:3702/", NULL, &req);
	//ret = soap_send___wsdd__Probe(soap, "http://239.255.255.250:3702/", NULL, &req);
#else // 1
	ret = soap_send___wsdd__Probe(soap, "soap.udp://239.255.255.250:3702/", NULL, &req);
#endif // 1

	if(ret != 0)       
	{
		printf("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
		ret = soap->error;
	}
	else
	{
		stack = STACK_init(sizeof(stONVIF_DEVICE));
		if (stack == NULL) {
			ONVIF_INFO("stack init failed!");
			soap_closesock(soap);
			SOAP_destroy(soap);
			return -1;
		}
		STACK_set_comp(stack, _onvif_device_compare);
		//ONVIF_TRACE("soap_send___wsdd__Probe success!");
		do
		{
			char *pt = NULL;
			char temp[512];
			char httpaddr[200] = {0,};
			szname[0] = 0;
			szfirmware[0]= 0;
			szlocation[0]= 0;
			//ONVIF_TRACE("[%d] begin receive probematch... ",count);
			ret = soap_recv___wsdd__ProbeMatches(soap, &resp);
			if (ret != 0 || resp.wsdd__ProbeMatches == NULL) 
			{
				berror = 1;
				printf("Find %d devices probemach:%p ret:%d!\n", count,resp.wsdd__ProbeMatches,ret);
				break;
			}
			else
			{       
				if(resp.wsdd__ProbeMatches->ProbeMatch == NULL){
					continue;
				}
				if(resp.wsdd__ProbeMatches->ProbeMatch->Scopes == NULL){
					continue;
				}
				if(resp.wsdd__ProbeMatches->ProbeMatch->XAddrs == NULL){
					continue;
				}
				if(resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item== NULL){
					continue;
				}
				//#ifdef ONVIF_DEBUG
#if 0
				printf("soap_recv___wsdd__Probe:  __sizeProbeMatch=%d\r\n",resp.wsdd__ProbeMatches->__sizeProbeMatch);
				printf("Target EP Address : %s\r\n",      resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
				printf("Target Type : %s\r\n",            resp.wsdd__ProbeMatches->ProbeMatch->Types);
				printf("Target Service Address : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
				printf("Target Metadata Version : %d\r\n",resp.wsdd__ProbeMatches->ProbeMatch->MetadataVersion);
				printf("Target Scopes Address : %s\r\n",  resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item);
#endif // 0
				ptr = strstr(resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item,"onvif://www.onvif.org/name/");
				if(ptr){
					ptr+=strlen("onvif://www.onvif.org/name/");
					sscanf(ptr,"%s", temp);
					if ((ptr = strstr(temp, "onvif://")) != NULL) {
						*ptr = 0;
					}
					strcpy(szname, temp);
				}
				ptr = strstr(resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item,"onvif://www.onvif.org/hardware/");
				if(ptr){
					ptr+=strlen("onvif://www.onvif.org/hardware/");
					sscanf(ptr,"%s", temp);
					if ((ptr = strstr(temp, "onvif://")) != NULL) {
						*ptr = 0;
					}
					strcpy(szfirmware, temp);
				}
				ptr = strstr(resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item,"onvif://www.onvif.org/location/country/");
				if(ptr){
					ptr+=strlen("onvif://www.onvif.org/location/country/");
					sscanf(ptr,"%s", temp);
					if ((ptr = strstr(temp, "onvif://")) != NULL) {
						*ptr = 0;
					}
					strcpy(szlocation, temp);
				}
				ptr = strstr(resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item,"onvif://www.onvif.org/location/city/");
				if(ptr){
					ptr+=strlen("onvif://www.onvif.org/location/city/");
					sscanf(ptr,"%s", temp);
					if ((ptr = strstr(temp, "onvif://")) != NULL) {
						*ptr = 0;
					}
					strcpy(szlocation, temp);
				}

				count++;
				pt = strstr(resp.wsdd__ProbeMatches->ProbeMatch->XAddrs,  "http://");
				if(pt == NULL) continue;
				if(pt[strlen("http://")] == '['){//ipv6
					pt = strstr(pt+strlen("http://"),  "http://");
					if(pt == NULL) continue;
					if(pt[strlen("http://")] == '['){//ipv6
						continue;
					}
				}
				// get ipv4 address
				sscanf(pt, "%s", httpaddr);
				
				if (http_parse_url(ipv4, &iport, NULL, httpaddr) == 0) {
					strcpy(device.ip, _ip_2string(ipv4,szipv4));
					device.port = iport;
					strcpy(device.name, szname);
					strcpy(device.firmware, szfirmware);
					strcpy(device.location, szlocation);
					device.dev_type = ONVIF_DEV_NVT;
					STACK_push_by_inc(stack, &device, true);
					//if (hook) {
					//	hook(ipv4, iport, szname, szlocation, szfirmware);
					//}
				}
			}
		}while(1);
	}
	
	ONVIF_TRACE("search done , try to pop device from stack....");

	if (add_hello_dev) {
		dev_bak = STACK_dup(g_OnvifClientCxt->devices);
		while(STACK_pop(dev_bak, &device) == 0) {
			// FIX ME , check whether device alive
			// TO DO
			ONVIF_TRACE("pop from discover stack %s:%d", device.ip, device.port);
			STACK_push_by_inc(stack, &device, false);
		}
		STACK_destroy(dev_bak);
	}

	if (hook) {
		count = 0;
		while(STACK_pop(stack, &device) == 0) {
			count++;
			if (ipstr2uint8(ipv4, device.ip) < 0){
				ONVIF_INFO("invalid device ip:%s", device.ip);
			} else {
				hook(bind_host, ipv4, device.port, device.name, device.location, 
					device.firmware, customCtx);
			}
		}
	}
	
	soap_closesock(soap);
	SOAP_destroy(soap);
	if (stack) {
		STACK_destroy(stack);
	}
	
	ONVIF_INFO("onvif search done.");
	return count;
}

static int onvif_check_xaddr(char *service, char *xaddr, bool fix_ip)
{
	unsigned char u_ip[4], u_ip2[4];
	unsigned short port, port2;
	char uri[128], uri2[128];

	if (http_parse_url(u_ip,&port,uri,service) < 0) {
		return -1;
	}
	if (http_parse_url(u_ip2,&port2,uri2,xaddr) < 0) {
		return -1;
	}
	if ((memcmp(u_ip, u_ip2, 4) != 0) ||
		(port != port2)) {
		ONVIF_TRACE("WARNING: xaddr failed(%s, %s)", service, xaddr);
		if (fix_ip){
			sprintf(xaddr, "http://%d.%d.%d.%d:%d%s", u_ip[0],u_ip[1],u_ip[2],u_ip[3],
				port, uri2);
			return 0;
		} else
			return -1;
	}
	return 0;
}

//enum tt__CapabilityCategory { tt__CapabilityCategory__All = 0, tt__CapabilityCategory__Analytics = 1, tt__CapabilityCategory__Device = 2, tt__CapabilityCategory__Events = 3, tt__CapabilityCategory__Imaging = 4, tt__CapabilityCategory__Media = 5, tt__CapabilityCategory__PTZ = 6 };
static int onvif_get_xaddr(lpONVIF_PRIV_CONTEXT context,const char *server, char *user, char *pwd, int type, char *xaddr)
{
	struct soap *soap = NULL;
	enum tt__CapabilityCategory captype;
	struct _tds__GetCapabilities req_uri;
	struct _tds__GetCapabilitiesResponse resp_uri;
	struct SOAP_ENV__Header header; 
	int bAuth = 0;
	bool support[ONVIF_MODULE_CNT];
	char sznonce[128],szcreated[128];
	
	if((soap = soap_new())	== NULL){
		ONVIF_TRACE("soap new failed!");
		return -1;
	}
	soap->userid = user;
	soap->passwd = pwd;
	soap->send_timeout = ONVIF_TIMEOUT;
	soap->recv_timeout = ONVIF_TIMEOUT;
	soap->connect_timeout = ONVIF_TIMEOUT;
	soap_set_namespaces(soap, namespaces);
	soap->bind_flags = soap->bind_flags | SO_REUSEADDR;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	captype = type;
	req_uri.__sizeCategory = 1;
	req_uri.Category = &captype;
	xaddr[0] = 0;

RETRY_WITH_USERTOKEN:
	if(user && pwd && (bAuth == 1)){
		memset(&header, 0 ,sizeof(struct SOAP_ENV__Header));
#if 0
		soap->myBAuth = 1;
		soap->wsse__username = user;  
		soap->wsse__password = szpwd;	
		soap->ns1__customerId = "";
		soap->wsse__nonce = sznonce;
		soap->wsse__created = szcreated;
		update_change(pwd, soap->wsse__nonce, soap->wsse__created, soap->wsse__password, sizeof(szpwd));
#else // 0
		header.wsse__Security = soap_malloc(soap, sizeof(struct _wsse__Security));
		memset(header.wsse__Security, 0, sizeof(struct _wsse__Security));
		header.wsse__Security->UsernameToken = soap_malloc(soap, sizeof(struct _wsse__UsernameToken));
		memset(header.wsse__Security->UsernameToken, 0, sizeof(struct _wsse__UsernameToken));
		header.wsse__Security->UsernameToken->Username = user;
		header.wsse__Security->UsernameToken->wsu__Id = "";
		header.wsse__Security->UsernameToken->Nonce = sznonce;
		header.wsse__Security->UsernameToken->wsu__Created = szcreated;
		header.wsse__Security->UsernameToken->Password = soap_malloc(soap, sizeof(struct _wsse__Password));
		header.wsse__Security->UsernameToken->Password->Type = 
			"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest";
		header.wsse__Security->UsernameToken->Password->__item = soap_malloc(soap, 128);
		update_change(pwd, 
			header.wsse__Security->UsernameToken->Nonce, 
			header.wsse__Security->UsernameToken->wsu__Created, 
			header.wsse__Security->UsernameToken->Password->__item, 128);
#endif // 0
		soap->header = &header;  
	}
	
	soap_call___tds__GetCapabilities( soap, server, NULL, &req_uri, &resp_uri);
	if(soap->error)
	{
		if(bAuth == 0 &&  *soap_faultsubcode(soap) && 
			(strstr(*soap_faultsubcode(soap),"NotAuthorized") 
				|| strstr(*soap_faultsubcode(soap),"FailedAuthentication")
				|| strstr(*soap_faultsubcode(soap),"wsse:InvalidSecurity")
				|| strstr(*soap_faultstring(soap),"Unauthorized"))){
			bAuth = 1;
			goto RETRY_WITH_USERTOKEN;
		}else if(*soap_faultsubcode(soap) && 
			(strstr(*soap_faultsubcode(soap),"NotAuthorized") 
				|| strstr(*soap_faultsubcode(soap),"FailedAuthentication")
				|| strstr(*soap_faultsubcode(soap),"wsse:InvalidSecurity")
				|| strstr(*soap_faultstring(soap),"Unauthorized"))){
			ONVIF_INFO("Auth FAILED!");
			if(context && context->nvp_hook[NVP_AUTH_FAILED]){
				fNVPEventHook fHook =  context->nvp_hook[NVP_AUTH_FAILED];
				fHook(NVP_AUTH_FAILED, 0, 0, context->nvp_hook_custom[NVP_AUTH_FAILED]);
				goto ERR_EXIT;
			}
		}else{
			ONVIF_INFO("soap error:%d,code:%s sub:%s", soap->error, *soap_faultcode(soap),*soap_faultsubcode(soap));
			//ONVIF_INFO("soap error:%d,code:%s\n\tsub:%s\n\treason:%s", soap->error, *soap_faultcode(soap),*soap_faultsubcode(soap), *soap_faultstring(soap) );
			goto ERR_EXIT;
		}
	}else{
		if(resp_uri.Capabilities){
			if (type == tt__CapabilityCategory__Analytics || type == tt__CapabilityCategory__All) {				
				if(resp_uri.Capabilities->Analytics) {
					support[tt__CapabilityCategory__Analytics] = true;
					if (xaddr)
						strcpy(xaddr, resp_uri.Capabilities->Analytics->XAddr);
					if (context) {
						strcpy(context->endpoint[tt__CapabilityCategory__Analytics], resp_uri.Capabilities->Analytics->XAddr);
						context->module_support[tt__CapabilityCategory__Analytics] = true;
					}
				}else{
					support[tt__CapabilityCategory__Analytics] = false;
					if (context) {
						context->module_support[tt__CapabilityCategory__Analytics] = false;
					}
				}
			}
			
			if (type == tt__CapabilityCategory__Device || type == tt__CapabilityCategory__All) {				
				if(resp_uri.Capabilities->Device) {
					support[tt__CapabilityCategory__Device] = true;
					if (xaddr)
						strcpy(xaddr, resp_uri.Capabilities->Device->XAddr);
					if (context) {
						strcpy(context->endpoint[tt__CapabilityCategory__Device], resp_uri.Capabilities->Device->XAddr);
						context->module_support[tt__CapabilityCategory__Device] = true;
					}
				}else{
					support[tt__CapabilityCategory__Device] = false;
					if (context) {
						context->module_support[tt__CapabilityCategory__Device] = false;
					}
				}
			}

			if (type == tt__CapabilityCategory__Events || type == tt__CapabilityCategory__All) {				
				if(resp_uri.Capabilities->Events) {
					support[tt__CapabilityCategory__Events] = true;
					if (xaddr)
						strcpy(xaddr, resp_uri.Capabilities->Events->XAddr);
					if (context) {
						strcpy(context->endpoint[tt__CapabilityCategory__Events], resp_uri.Capabilities->Events->XAddr);
						context->module_support[tt__CapabilityCategory__Events] = true;
					}
				}else{
					support[tt__CapabilityCategory__Events] = false;
					if (context) {
						context->module_support[tt__CapabilityCategory__Events] = false;
					}
				}
			}

			if (type == tt__CapabilityCategory__Imaging || type == tt__CapabilityCategory__All) {				
				if(resp_uri.Capabilities->Imaging) {
					support[tt__CapabilityCategory__Imaging] = true;
					if (xaddr)
						strcpy(xaddr, resp_uri.Capabilities->Imaging->XAddr);
					if (context) {
						strcpy(context->endpoint[tt__CapabilityCategory__Imaging], resp_uri.Capabilities->Imaging->XAddr);
						context->module_support[tt__CapabilityCategory__Imaging] = true;
					}
				}else{
					support[tt__CapabilityCategory__Imaging] = false;
					if (context) {
						context->module_support[tt__CapabilityCategory__Imaging] = false;
					}
				}
			}
			
			if (type == tt__CapabilityCategory__Media || type == tt__CapabilityCategory__All) {				
				if(resp_uri.Capabilities->Media) {
					support[tt__CapabilityCategory__Media] = true;
					if (xaddr)
						strcpy(xaddr, resp_uri.Capabilities->Media->XAddr);
					if (context) {
						strcpy(context->endpoint[tt__CapabilityCategory__Media], resp_uri.Capabilities->Media->XAddr);
						context->module_support[tt__CapabilityCategory__Media] = true;
					}
				}else{
					support[tt__CapabilityCategory__Media] = false;
					if (context) {
						context->module_support[tt__CapabilityCategory__Media] = false;
					}
				}
			}
			
			if (type == tt__CapabilityCategory__PTZ || type == tt__CapabilityCategory__All) {				
				if(resp_uri.Capabilities->PTZ) {
					support[tt__CapabilityCategory__PTZ] = true;
					if (xaddr)
						strcpy(xaddr, resp_uri.Capabilities->PTZ->XAddr);
					if (context) {
						strcpy(context->endpoint[tt__CapabilityCategory__PTZ], resp_uri.Capabilities->PTZ->XAddr);
						context->module_support[tt__CapabilityCategory__PTZ] = true;
					}
				}else{
					support[tt__CapabilityCategory__PTZ] = false;
					if (context) {
						context->module_support[tt__CapabilityCategory__PTZ] = false;
					}
				}
			}


		}
	}

	if (type != tt__CapabilityCategory__All && support[type] == false) {
		goto ERR_EXIT;
	}

	// check ipaddress
	//if (onvif_check_xaddr(server, xaddr, false) < 0) {
	//	goto ERR_EXIT;
	//}
	
	SOAP_destroy(soap);
	ONVIF_TRACE("get server addr : %s", xaddr);
	return 0;

ERR_EXIT:		
	SOAP_destroy(soap);
	ONVIF_TRACE("get server addr failed from:%s", server);
	return -1;	
}

static int onvif_get_services(lpONVIF_PRIV_CONTEXT context,const char *server, char *user, char *pwd, int type, char *xaddr)
{
	struct soap *soap = NULL;
	struct _tds__GetServices req_uri;
	struct _tds__GetServicesResponse resp_uri;
	struct SOAP_ENV__Header header; 
	int bAuth = 0;
	bool support[ONVIF_MODULE_CNT];
	char sznonce[128],szcreated[128];
	
	if((soap = soap_new())	== NULL){
		ONVIF_TRACE("soap new failed!");
		return -1;
	}
	soap->userid = user;
	soap->passwd = pwd;
	soap->send_timeout = ONVIF_TIMEOUT;
	soap->recv_timeout = ONVIF_TIMEOUT;
	soap->connect_timeout = ONVIF_TIMEOUT;
	soap_set_namespaces(soap, namespaces);
	soap->bind_flags = soap->bind_flags | SO_REUSEADDR;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.IncludeCapability = m_NFALSE;
	xaddr[0] = 0;

RETRY_WITH_USERTOKEN:
	if(user && pwd && (bAuth == 1)){
		memset(&header, 0 ,sizeof(struct SOAP_ENV__Header));
#if 0
		soap->myBAuth = 1;
		soap->wsse__username = user;  
		soap->wsse__password = szpwd;	
		soap->ns1__customerId = "";
		soap->wsse__nonce = sznonce;
		soap->wsse__created = szcreated;
		update_change(pwd, soap->wsse__nonce, soap->wsse__created, soap->wsse__password, sizeof(szpwd));
#else // 0
		header.wsse__Security = soap_malloc(soap, sizeof(struct _wsse__Security));
		memset(header.wsse__Security, 0, sizeof(struct _wsse__Security));
		header.wsse__Security->UsernameToken = soap_malloc(soap, sizeof(struct _wsse__UsernameToken));
		memset(header.wsse__Security->UsernameToken, 0, sizeof(struct _wsse__UsernameToken));
		header.wsse__Security->UsernameToken->Username = user;
		header.wsse__Security->UsernameToken->wsu__Id = "";
		header.wsse__Security->UsernameToken->Nonce = sznonce;
		header.wsse__Security->UsernameToken->wsu__Created = szcreated;
		header.wsse__Security->UsernameToken->Password = soap_malloc(soap, sizeof(struct _wsse__Password));
		header.wsse__Security->UsernameToken->Password->Type = 
			"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest";
		header.wsse__Security->UsernameToken->Password->__item = soap_malloc(soap, 128);
		update_change(pwd, 
			header.wsse__Security->UsernameToken->Nonce, 
			header.wsse__Security->UsernameToken->wsu__Created, 
			header.wsse__Security->UsernameToken->Password->__item, 128);
#endif // 0
		soap->header = &header;  
	}
	
	soap_call___tds__GetServices( soap, server, NULL, &req_uri, &resp_uri);
	if(soap->error)
	{
		if(bAuth == 0 &&  *soap_faultsubcode(soap) && 
			(strstr(*soap_faultsubcode(soap),"NotAuthorized") 
				|| strstr(*soap_faultsubcode(soap),"FailedAuthentication")
				|| strstr(*soap_faultstring(soap),"Unauthorized"))){
			bAuth = 1;
			goto RETRY_WITH_USERTOKEN;
		}else if(*soap_faultsubcode(soap) && 
			(strstr(*soap_faultsubcode(soap),"NotAuthorized") 
				|| strstr(*soap_faultsubcode(soap),"FailedAuthentication")
				|| strstr(*soap_faultstring(soap),"Unauthorized"))){
			ONVIF_INFO("Auth FAILED!");
			if(context && context->nvp_hook[NVP_AUTH_FAILED]){
				fNVPEventHook fHook =  context->nvp_hook[NVP_AUTH_FAILED];
				fHook(NVP_AUTH_FAILED, 0, 0, context->nvp_hook_custom[NVP_AUTH_FAILED]);
				goto ERR_EXIT;
			}
		}else{
			ONVIF_INFO("soap error:%d,code:%s sub:%s", soap->error, *soap_faultcode(soap),*soap_faultsubcode(soap));
			//ONVIF_INFO("soap error:%d,code:%s\n\tsub:%s\n\treason:%s", soap->error, *soap_faultcode(soap),*soap_faultsubcode(soap), *soap_faultstring(soap) );
			goto ERR_EXIT;
		}
	}else{
		if(resp_uri.Service && resp_uri.__sizeService > 0){
			int i;
			for (i = 0; i < resp_uri.__sizeService; i++){
				if (strcmp(resp_uri.Service[i].Namespace, "http://www.onvif.org/ver20/analytics/wsdl") == 0) {
					if (type == tt__CapabilityCategory__Analytics || type == tt__CapabilityCategory__All) { 			
						support[tt__CapabilityCategory__Analytics] = true;
						if (xaddr)
							strcpy(xaddr, resp_uri.Service[i].XAddr);
						if (context) {
							strcpy(context->endpoint[tt__CapabilityCategory__Analytics], resp_uri.Service[i].XAddr);
							context->module_support[tt__CapabilityCategory__Analytics] = true;
						}
					}
				}
				else if (strcmp(resp_uri.Service[i].Namespace, "http://www.onvif.org/ver10/device/wsdl") == 0) {
					if (type == tt__CapabilityCategory__Device || type == tt__CapabilityCategory__All) {				
						support[tt__CapabilityCategory__Device] = true;
						if (xaddr)
							strcpy(xaddr, resp_uri.Service[i].XAddr);
						if (context) {
							strcpy(context->endpoint[tt__CapabilityCategory__Device], resp_uri.Service[i].XAddr);
							context->module_support[tt__CapabilityCategory__Device] = true;
						}
					}
				}
				
				else if (strcmp(resp_uri.Service[i].Namespace, "http://www.onvif.org/ver10/events/wsdl") == 0) {
					if (type == tt__CapabilityCategory__Events || type == tt__CapabilityCategory__All) {				
						support[tt__CapabilityCategory__Events] = true;
						if (xaddr)
							strcpy(xaddr, resp_uri.Service[i].XAddr);
						if (context) {
							strcpy(context->endpoint[tt__CapabilityCategory__Events], resp_uri.Service[i].XAddr);
							context->module_support[tt__CapabilityCategory__Events] = true;
						}
					}
				}
				else if (strcmp(resp_uri.Service[i].Namespace, "http://www.onvif.org/ver20/imaging/wsdl") == 0) {
					if (type == tt__CapabilityCategory__Imaging || type == tt__CapabilityCategory__All) {				
						support[tt__CapabilityCategory__Imaging] = true;
						if (xaddr)
							strcpy(xaddr, resp_uri.Service[i].XAddr);
						if (context) {
							strcpy(context->endpoint[tt__CapabilityCategory__Imaging], resp_uri.Service[i].XAddr);
							context->module_support[tt__CapabilityCategory__Imaging] = true;
						}
					}
				}
				else if (strcmp(resp_uri.Service[i].Namespace, "http://www.onvif.org/ver10/media/wsdl") == 0) {
					if (type == tt__CapabilityCategory__Media || type == tt__CapabilityCategory__All) { 			
						support[tt__CapabilityCategory__Media] = true;
						if (xaddr)
							strcpy(xaddr, resp_uri.Service[i].XAddr);
						if (context) {
							strcpy(context->endpoint[tt__CapabilityCategory__Media], resp_uri.Service[i].XAddr);
							context->module_support[tt__CapabilityCategory__Media] = true;
						}
					}
				}
				else if (strcmp(resp_uri.Service[i].Namespace, "http://www.onvif.org/ver20/ptz/wsdl") == 0) {
					if (type == tt__CapabilityCategory__PTZ || type == tt__CapabilityCategory__All) {				
						support[tt__CapabilityCategory__PTZ] = true;
						if (xaddr)
							strcpy(xaddr, resp_uri.Service[i].XAddr);
						if (context) {
							strcpy(context->endpoint[tt__CapabilityCategory__PTZ], resp_uri.Service[i].XAddr);
							context->module_support[tt__CapabilityCategory__PTZ] = true;
						}
					}
				}
			}

		}
	}

	if (type != tt__CapabilityCategory__All && support[type] == false) {
		goto ERR_EXIT;
	}
	// check ipaddress
	if (onvif_check_xaddr((char *)server, xaddr, true) < 0) {
		goto ERR_EXIT;
	}

	SOAP_destroy(soap);
	ONVIF_TRACE("get server xaddr : %s", xaddr);
	return 0;

ERR_EXIT:		
	SOAP_destroy(soap);
	ONVIF_TRACE("get server xaddr failed from:%s", server);
	return -1;	
}



static int 
onvif_request(lpNVP_ARGS args, struct soap **s,
	fSOAP_CALL call, unsigned int type , /*if type > ONVIF_TYPE_CNT, THIS is a endpoint[string] reference */
	void *in_arg, int in_size, 
	void *out_arg, int out_size)
{
	int ret = -1;
	char server[256];
	unsigned char ipv4[4];
	struct soap *soap = NULL;
	struct wsa__EndpointReferenceType replyto;
	char msgid[64];
	struct SOAP_ENV__Header header; 
	lpNVP_INTERFACE iface = (lpNVP_INTERFACE)args->thiz;
	lpONVIF_PRIV_CONTEXT context = (lpONVIF_PRIV_CONTEXT)(iface ? iface->private_ctx : NULL);
	int bAuth = 0;
	char xaddr[128];
	char sznonce[128],szcreated[128];

	//if (args == NULL && context) {
	//	args =&context->args;
	//}
	//
	if(s == NULL || args == NULL || call == NULL  || in_arg == NULL || out_arg == NULL){
		ONVIF_ASSERT(NULL, "invalid arguments in %s", __FUNCTION__);
	}
	if ( context && args != (&context->args)) {
		memcpy( &context->args, args, sizeof(stNVP_ARGS));
	}
	//
	sprintf(server, ONVIF_SERVER_URI, args->ip, args->port);

	if ( http_parse_url(ipv4, NULL, NULL, server) < 0) {
		ONVIF_INFO("invaild server: %s", server);
		return -1;
	}

	if(type < ONVIF_MODULE_CNT){
		if(onvif_get_xaddr(context, server, args->username, args->password, type, xaddr)){
			return -1;
		}
	}else{
		if (call == soap_call___tev__Renew || call == soap_call___tev__Unsubscribe) {
			memset(&replyto, 0, sizeof(replyto));
			memset(&header, 0 ,sizeof(struct SOAP_ENV__Header));
			if (call == soap_call___tev__Renew)
				header.wsa__Action = "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewRequest";
			else if (call == soap_call___tev__Unsubscribe)
				header.wsa__Action = "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeRequest";
			replyto.Address = "http://www.w3.org/2005/08/addressing/anonymous";
			header.wsa__ReplyTo = &replyto;
			header.wsa__To = (char *)type;
		}
		strcpy(xaddr, (char *)type);
	}
	
	if((soap = soap_new())	== NULL){
		ONVIF_TRACE("soap new failed!");
		return -1;
	}
	*s = soap;
	
	if(type > ONVIF_MODULE_CNT){
		if (call == soap_call___tev__Renew || call == soap_call___tev__Unsubscribe) {
			strcpy(msgid, soap_wsa_rand_uuid(soap));
			header.wsa__MessageID = msgid;
			soap->header = &header;  
		}
	}
	soap->userid = args->username;
	soap->passwd = args->password;
	soap->send_timeout = ONVIF_TIMEOUT;
	soap->recv_timeout = ONVIF_TIMEOUT;
	soap->connect_timeout = ONVIF_TIMEOUT;
	soap->bind_flags = soap->bind_flags | SO_REUSEADDR;
	soap_set_namespaces(soap, namespaces);
	
	//memset( in_arg, 0 , in_size);
	//memset( out_arg, 0, out_size);

RETRY_WITH_USERTOKEN:
	if(args->username && args->password && 
		(bAuth == 1 || 
		(context && context->auth == true))){
		memset(&header, 0 ,sizeof(struct SOAP_ENV__Header));

#if 0
		soap->myBAuth = 1;
		soap->wsse__username = args->username;  
		soap->wsse__password = szpwd;	
		soap->ns1__customerId = "";
		soap->wsse__nonce = sznonce;
		soap->wsse__created = szcreated;
		update_change(args->password, soap->wsse__nonce, soap->wsse__created, soap->wsse__password, sizeof(szpwd));
#else // 0
		header.wsse__Security = soap_malloc(soap, sizeof(struct _wsse__Security));
		memset(header.wsse__Security, 0, sizeof(struct _wsse__Security));
		header.wsse__Security->UsernameToken = soap_malloc(soap, sizeof(struct _wsse__UsernameToken));
		memset(header.wsse__Security->UsernameToken, 0, sizeof(struct _wsse__UsernameToken));
		header.wsse__Security->UsernameToken->Username = args->username;
		header.wsse__Security->UsernameToken->wsu__Id = "";
		header.wsse__Security->UsernameToken->Nonce = sznonce;
		header.wsse__Security->UsernameToken->wsu__Created = szcreated;
		header.wsse__Security->UsernameToken->Password = soap_malloc(soap, sizeof(struct _wsse__Password));
		header.wsse__Security->UsernameToken->Password->Type = 
			"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest";
		header.wsse__Security->UsernameToken->Password->__item = soap_malloc(soap, 128);
		update_change(args->password, 
			header.wsse__Security->UsernameToken->Nonce, 
			header.wsse__Security->UsernameToken->wsu__Created, 
			header.wsse__Security->UsernameToken->Password->__item, 128);
#endif // 0
		if(type > ONVIF_MODULE_CNT){			
			if (call == soap_call___tev__Renew || call == soap_call___tev__Unsubscribe) {
				memset(&replyto, 0, sizeof(replyto));
				if (call == soap_call___tev__Renew)
					header.wsa__Action = "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewRequest";
				else if (call == soap_call___tev__Unsubscribe)
					header.wsa__Action = "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeRequest";
				replyto.Address = "http://www.w3.org/2005/08/addressing/anonymous";
				header.wsa__ReplyTo = &replyto;
				header.wsa__To = (char *)type;
				strcpy(msgid, soap_wsa_rand_uuid(soap));
				header.wsa__MessageID = msgid;
			}
		}
		soap->header = &header;  
		
	}

	ONVIF_TRACE("xaddr : %s", xaddr);
	call( soap, xaddr, NULL, in_arg,  out_arg);
	if(soap->error)
	{	
		if(bAuth == 0 &&  *soap_faultsubcode(soap) && 
			(strstr(*soap_faultsubcode(soap),"NotAuthorized") 
				|| strstr(*soap_faultsubcode(soap),"FailedAuthentication")
				|| strstr(*soap_faultsubcode(soap),"wsse:InvalidSecurity")
				|| strstr(*soap_faultstring(soap),"Unauthorized"))){
			ONVIF_INFO("Require Auth: %s", *soap_faultsubcode(soap) );
			if (context ) context->auth = true;
			bAuth = 1;
			goto RETRY_WITH_USERTOKEN;
		}else if(*soap_faultsubcode(soap) && 
			(strstr(*soap_faultsubcode(soap),"NotAuthorized") 
				|| strstr(*soap_faultsubcode(soap),"FailedAuthentication")
				|| strstr(*soap_faultsubcode(soap),"wsse:InvalidSecurity")
				|| strstr(*soap_faultstring(soap),"Unauthorized"))){
			ONVIF_INFO("Auth FAILED sub:%s", *soap_faultsubcode(soap) );
			if(context && context->nvp_hook[NVP_AUTH_FAILED]){
				fNVPEventHook fHook =  context->nvp_hook[NVP_AUTH_FAILED];
				fHook(NVP_AUTH_FAILED, 0, 0, context->nvp_hook_custom[NVP_AUTH_FAILED]);
			}
			ret = NVP_RET_AUTH_FAILED;
			goto ERR_EXIT;
		}else{
			ONVIF_INFO("soap error:%d,code:%s sub:%s", soap->error, *soap_faultcode(soap),*soap_faultsubcode(soap) );
			if (soap->error > 0)
				ret = -soap->error;
			else
				ret = soap->error;
			goto ERR_EXIT;
		}
	}

	// if call success , we must destroy soap manually
	return 0;
ERR_EXIT:
	SOAP_destroy(soap);
	return ret;		
}

int ONVIF_get_devinfo(lpNVP_ARGS args, lpNVP_DEV_INFO info)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tds__GetDeviceInformation req_uri;
	struct _tds__GetDeviceInformationResponse resp_uri;
	
	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));

	if((ret = onvif_request(args, &soap, soap_call___tds__GetDeviceInformation, ONVIF_DEVICE,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0){
		ONVIF_TRACE("%s Brand: %s model:%s FW:%s SN:%s HWID:%s", args->ip, resp_uri.Manufacturer, resp_uri.Model, resp_uri.FirmwareVersion, resp_uri.SerialNumber,
			resp_uri.HardwareId);
		if(info){
			strcpy(info->manufacturer, resp_uri.Manufacturer);
			strcpy(info->model, resp_uri.Model);
			strcpy(info->firmware, resp_uri.FirmwareVersion);
			strcpy(info->sn, resp_uri.SerialNumber);
			strcpy(info->hwid, resp_uri.HardwareId);
		}
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;		
}

static int ONVIF_reboot(lpNVP_ARGS args, void *data )
{
	int ret = -1;
	char sz_server[128];
	struct soap *soap = NULL;
	struct _tds__SystemReboot req_uri;
	struct _tds__SystemRebootResponse resp_uri;
	
	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	sprintf(sz_server, ONVIF_SERVER_URI, args->ip, args->port);
	if((ret = onvif_request(args, &soap, soap_call___tds__SystemReboot, (unsigned int)sz_server,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		ONVIF_INFO("##### reboot message %s ######",resp_uri.Message);
		SOAP_destroy(soap);
		return  0;
	}
	ONVIF_INFO("##### reboot %s failed!",args->ip);
	return ret;
}


int ONVIF_get_system_time(lpNVP_ARGS args, lpNVP_SYS_TIME syst)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tds__GetSystemDateAndTime req_uri;
	struct _tds__GetSystemDateAndTimeResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	
	memset(syst, 0, sizeof(stNVP_SYS_TIME));
	syst->ntp_enable = false;
	
	if((ret = onvif_request(args, &soap, soap_call___tds__GetSystemDateAndTime, ONVIF_DEVICE,
		&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		struct tt__DateTime *pDateTime = NULL;
		int tzone = 0;

		syst->daylightsaving = resp_uri.SystemDateAndTime->DaylightSavings;
		if (resp_uri.SystemDateAndTime->TimeZone) {
			tzone_s2int(resp_uri.SystemDateAndTime->TimeZone->TZ, &tzone);
			syst->tzone = tzone_value_from_second(tzone);
			strncpy(syst->stzone, resp_uri.SystemDateAndTime->TimeZone->TZ, sizeof(syst->stzone) - 1);
		}
		ONVIF_INFO("Daylight: %s, TimeZone:%s", resp_uri.SystemDateAndTime->DaylightSavings ? "true" : "false", 
			resp_uri.SystemDateAndTime->TimeZone ? resp_uri.SystemDateAndTime->TimeZone->TZ : "unknown");
		if(resp_uri.SystemDateAndTime->UTCDateTime){
			pDateTime = resp_uri.SystemDateAndTime->UTCDateTime;
		}else{
			pDateTime = NULL;
		}
		if( pDateTime ){
			ONVIF_INFO("UTCDateTime: %d/%d/%d %d:%d:%d", 
				resp_uri.SystemDateAndTime->UTCDateTime->Date->Year,
				resp_uri.SystemDateAndTime->UTCDateTime->Date->Month,
				resp_uri.SystemDateAndTime->UTCDateTime->Date->Day,
				resp_uri.SystemDateAndTime->UTCDateTime->Time->Hour,
				resp_uri.SystemDateAndTime->UTCDateTime->Time->Minute,
				resp_uri.SystemDateAndTime->UTCDateTime->Time->Second);
			syst->gm_time.date.year = pDateTime->Date->Year - 1900;
			syst->gm_time.date.month = pDateTime->Date->Month -1;
			syst->gm_time.date.day = pDateTime->Date->Day;
			syst->gm_time.time.hour = pDateTime->Time->Hour;
			syst->gm_time.time.minute = pDateTime->Time->Minute;
			syst->gm_time.time.second = pDateTime->Time->Second;
		}
		
		if(resp_uri.SystemDateAndTime->LocalDateTime){
			pDateTime = resp_uri.SystemDateAndTime->LocalDateTime;
		}else{
			pDateTime = NULL;
		}
		if( pDateTime ){
			ONVIF_INFO("LocalDateTime: %d/%d/%d %d:%d:%d", 
				resp_uri.SystemDateAndTime->LocalDateTime->Date->Year,
				resp_uri.SystemDateAndTime->LocalDateTime->Date->Month,
				resp_uri.SystemDateAndTime->LocalDateTime->Date->Day,
				resp_uri.SystemDateAndTime->LocalDateTime->Time->Hour,
				resp_uri.SystemDateAndTime->LocalDateTime->Time->Minute,
				resp_uri.SystemDateAndTime->LocalDateTime->Time->Second);

			syst->local_time.date.year = pDateTime->Date->Year - 1900;
			syst->local_time.date.month = pDateTime->Date->Month -1;
			syst->local_time.date.day = pDateTime->Date->Day;
			syst->local_time.time.hour = pDateTime->Time->Hour;
			syst->local_time.time.minute = pDateTime->Time->Minute;
			syst->local_time.time.second = pDateTime->Time->Second;
		}
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;
}


int ONVIF_sync_system_time(lpNVP_ARGS args, lpNVP_SYS_TIME syst)
{
	int ret = -1;
	struct tt__Time curtime;
	struct tt__Date curdate;	
	struct tt__DateTime dtime;
	struct tt__TimeZone tzone;
	struct soap *soap = NULL;
	struct _tds__SetSystemDateAndTime req_uri;
	struct _tds__SetSystemDateAndTimeResponse resp_uri;
	char stzone[32];

	if (syst == NULL) {
		return -1;
	}
	
	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.DateTimeType = tt__SetDateTimeType__Manual;
		req_uri.DaylightSavings = syst->daylightsaving;
		curtime.Hour = syst->gm_time.time.hour;
		curtime.Minute = syst->gm_time.time.minute;
		curtime.Second = syst->gm_time.time.second;
		curdate.Year = syst->gm_time.date.year + 1900;
		curdate.Month = syst->gm_time.date.month + 1;
		curdate.Day = syst->gm_time.date.day;
	dtime.Time = &curtime;
	dtime.Date = &curdate;
	req_uri.UTCDateTime = &dtime;
	tzone.TZ = stzone;
	if (strlen(syst->stzone) > 0) {
		strcpy(tzone.TZ, syst->stzone);
	} else {
		tzone_int2s(tzone_value_to_second(syst->tzone), tzone.TZ, 32);
	}
	//tzone.TZ = "GMT+00:00";// UTC
	req_uri.TimeZone = &tzone;
	//req_uri.TimeZone = NULL;

	ONVIF_INFO("set time : %d/%d/%d  %d:%d:%d (%s)", curdate.Year, curdate.Month, curdate.Day,
		curtime.Hour, curtime.Minute, curtime.Second,
		tzone.TZ);
	if((ret = onvif_request(args, &soap, soap_call___tds__SetSystemDateAndTime, ONVIF_DEVICE,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		SOAP_destroy(soap);
		return 0;	
	}

	return ret;	
}

static int onvif_get_ntp(lpNVP_ARGS args, lpNVP_SYS_TIME syst)
{
	return -1;		
}

static int onvif_set_ntp(lpNVP_ARGS args, lpNVP_SYS_TIME syst)
{
	return -1;		
}


int ONVIF_set_system_time(lpNVP_ARGS args, lpNVP_SYS_TIME syst)
{
	int ret;
	stNVP_SYS_TIME systime;
	time_t t_now_bak, t_now, ptime, ptime_bak;
	struct tm ptm;
	char strbuf[64];
	int64_t diff;

	memset(&ptm, 0, sizeof(ptm));
	if (syst == NULL) {
		int i;
		
		time(&t_now);
		t_now_bak = t_now;
		ptime = 0;
		diff = 0;
		ONVIF_TRACE("t_now %lu %s", t_now , ctime_r(&t_now, strbuf));
		for (i = 0; i < 3 ; i++ ) {			
			int64_t pt_diff = 0;

			memset(&systime, 0, sizeof(systime));		
			if (( ret = ONVIF_get_system_time(args, &systime)) < 0) {
				return ret;
			}

			ptime_bak = ptime;
			ptm.tm_year = systime.local_time.date.year;
			ptm.tm_mon = systime.local_time.date.month;
			ptm.tm_mday = systime.local_time.date.day;
			ptm.tm_hour = systime.local_time.time.hour;
			ptm.tm_min = systime.local_time.time.minute;
			ptm.tm_sec = systime.local_time.time.second;
			ptime = mktime(&ptm);
			ONVIF_TRACE("gettime %lu %s", ptime , ctime_r(&ptime, strbuf));
			// check time
			if (abs(ptime - t_now_bak) > (5 * i + 5)) {
				if (ptime_bak != 0 && diff != 0) {
					pt_diff =  ptime - ptime_bak;
					diff = diff - pt_diff;
				} else {
					diff =  (int64_t)t_now - (int64_t)ptime ;
				}
				//printf("ptime :%lu sync:%lu diff:%c%lu(%dh) \n", ptime, t_now, (diff < 0) ? '-' : ' ', abs(diff) , (abs(diff) + 1799) / 3600);
				//diff +=  pt_diff;
				ONVIF_TRACE("ptime :%lu sync:%ld diff:%c%d(%dh) pt_diff:%c%d(%dh)", ptime, t_now, (diff < 0) ? '-' : ' ', abs(diff),
					(abs(diff)  + 1799)/3600, 
					(pt_diff < 0) ? '-' : ' ', abs(pt_diff), (abs(pt_diff) + 1799) / 3600);
				if ((uint32_t)ptime < 1000000000L) {
					ONVIF_TRACE("currnet setup time too small");
					diff = 0;
				} else {
					t_now =t_now +  diff;
					if (t_now < 0) {
						t_now = 0;
					}
				}
				
				localtime_c(&t_now, &ptm);
				ONVIF_TRACE("settime %lu %s", t_now , ctime_r(&t_now, strbuf));
				
				systime.gm_time.date.year = ptm.tm_year; 
				systime.gm_time.date.month  = ptm.tm_mon;
				systime.gm_time.date.day  = ptm.tm_mday;
				systime.gm_time.time.hour = ptm.tm_hour;
				systime.gm_time.time.minute = ptm.tm_min;
				systime.gm_time.time.second = ptm.tm_sec;

				if ( (ret = ONVIF_sync_system_time(args, &systime)) < 0) {
					if (ret == -SOAP_NO_TAG)
						ONVIF_TRACE("warning: no tag!");
					else
						return ret;
				}
			}  else {
				ONVIF_INFO("sync time success %lu %s", ptime , ctime_c(&ptime, strbuf));
				break;
			}
		}

		return 0;
	} else {
		return ONVIF_sync_system_time(args, syst);
	}
}


#ifndef WIN32
int ONVIF_setup_main_venc(lpNVP_VENC_CONFIG profile, lpNVP_VENC_OPTIONS options)
{
	int i;
	profile->enc_type = NVP_VENC_H264;
	profile->enc_quality = (options->enc_quality.min + options->enc_quality.max)*4/5;
	profile->enc_fps = 25;
	if(options->enc_bps.max == 0){
		profile->enc_bps = 3072;
	}else if( options->enc_bps.max < 2024){
		profile->enc_bps = options->enc_bps.max;
	}else{
		profile->enc_bps = 3072;
	}
	profile->enc_interval = 1;
	if(options->resolution_nr == 1){
		profile->width = options->resolution[0].width;
		profile->height = options->resolution[0].height;
	}else{
		int b1080p = 0;
		int b960p = 0;
		int b720p = 0;
		// 1920x1080,1280x1024, 1280x960,1280x720,960x540,768x432,720x480, 704x576, 640x480, 640x360,624x352,512x288, 352x288, 352x192, 320x240,176x144
		for(i=0; i<options->resolution_nr; i++){
			if((options->resolution[i].width == 1920) && (options->resolution[i].height == 1080)){
				b1080p = 1;
			}
			if((options->resolution[i].width == 1280) && (options->resolution[i].height == 960)){
				b960p = 1;
			}
			if((options->resolution[i].width == 1280) && (options->resolution[i].height == 720)){
				b720p = 1;
			}
		}
		if(b1080p){
			profile->width = __MIN(1920, MAX_CAM_WIDTH);
			profile->height = __MIN(1080, MAX_CAM_HEIGHT);
			if(profile->height == 960 && b960p == 0){
				profile->width = 1280;
				profile->height = 720;
			}
		}else if(b960p){
			profile->width = __MIN(1280, MAX_CAM_WIDTH);
			profile->height = __MIN(960, MAX_CAM_HEIGHT);
		}else if(b720p){
			profile->width = 1280;
			profile->height = 720;
		}else{
			profile->width = 1280;
			profile->height = 720;
		}
	}

#ifndef WIN32
	NVP_dump_venc_profile(profile);
#endif // WIN32
	return 0;
}
#endif // WIN32

#ifndef WIN32
int ONVIF_setup_sub_venc(lpNVP_VENC_CONFIG profile, lpNVP_VENC_OPTIONS options)
{
	int i;

	profile->enc_type = NVP_VENC_H264;
	//profile->enc_profile = H264_PROFILE_BASELINE;
	profile->enc_quality = (options->enc_quality.min + options->enc_quality.max)*2/3;
	if( options->enc_fps.max < 25){
		if( options->enc_fps.max < 15){
			profile->enc_fps = options->enc_fps.max;
		}else{
			profile->enc_fps = 15;
		}
	}else{
		profile->enc_fps = 25;
	}
	if(options->enc_bps.max == 0){
		profile->enc_bps = 768;
	}else if( options->enc_bps.max < 768){
		profile->enc_bps = options->enc_bps.max;
	}else{
		profile->enc_bps = 768;
	}
	profile->enc_interval = 1;
	if(options->resolution_nr == 1){
		profile->width = options->resolution[0].width;
		profile->height = options->resolution[0].height;
	}else{
		int found = 0;
		// 1920x1080, 1280x960,1280x720, 704x576, 640x480, 640x360, 352x288, 352x192, 320x240,
		for(i=0; i<options->resolution_nr; i++){
			if(((options->resolution[i].width == 320) && (options->resolution[i].height == 240))
				|| ((options->resolution[i].width == 352) && (options->resolution[i].height == 288))
				|| ((options->resolution[i].width == 320) && (options->resolution[i].height == 192))){
				profile->width = options->resolution[i].width;
				profile->height = options->resolution[i].height;
				found = 1;
				break;
			}
		}
		// if not found , use default resolution geting from GetProfiles
		if(found  == 0){
			profile->width = 352;
			profile->height = 288;
			
		}
	}
#ifndef WIN32
	NVP_dump_venc_profile(profile);
#endif // WIN32
	return 0;
}
#endif WIN32

int ONVIF_get_venc_options(lpNVP_ARGS args, lpNVP_VENC_OPTIONS options)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _trt__GetVideoEncoderConfigurationOptions req_uri;
	struct _trt__GetVideoEncoderConfigurationOptionsResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.ConfigurationToken = options->enc_token;
	req_uri.ProfileToken = options->token;

	if((ret = onvif_request(args, &soap, soap_call___trt__GetVideoEncoderConfigurationOptions, ONVIF_MEDIA,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		int i;
		if(resp_uri.Options == NULL) {			
			SOAP_destroy(soap);
			return -1;
		}
		if(resp_uri.Options->QualityRange == NULL || resp_uri.Options->H264 == NULL){
			SOAP_destroy(soap);
			return -1;
		}
		options->enc_quality.min = resp_uri.Options->QualityRange->Min;
		options->enc_quality.max = resp_uri.Options->QualityRange->Max;
		if(resp_uri.Options->H264){
			options->resolution_nr = resp_uri.Options->H264->__sizeResolutionsAvailable;
			for( i = 0; i < resp_uri.Options->H264->__sizeResolutionsAvailable && i < NVP_SIZE_ARRAY(options->resolution); i++){
				options->resolution[i].width = resp_uri.Options->H264->ResolutionsAvailable[i].Width;
				options->resolution[i].height = resp_uri.Options->H264->ResolutionsAvailable[i].Height;
			}
			if (resp_uri.Options->H264->H264ProfilesSupported) {
				options->enc_profile_nr = resp_uri.Options->H264->__sizeH264ProfilesSupported;
				for( i = 0; i < resp_uri.Options->H264->__sizeH264ProfilesSupported && i < NVP_SIZE_ARRAY(options->enc_profile); i++){
					options->enc_profile[i] = resp_uri.Options->H264->H264ProfilesSupported[i];
				}
			} else {
				options->enc_profile_nr = 0;
			}
			options->enc_fps.min = resp_uri.Options->H264->FrameRateRange->Min;
			options->enc_fps.max = resp_uri.Options->H264->FrameRateRange->Max;
			options->enc_gov.min = resp_uri.Options->H264->GovLengthRange->Min;
			options->enc_gov.max = resp_uri.Options->H264->GovLengthRange->Max;
			options->enc_interval.min = resp_uri.Options->H264->EncodingIntervalRange->Min;
			options->enc_interval.max = resp_uri.Options->H264->EncodingIntervalRange->Max;
		}
		if(resp_uri.Options->Extension && resp_uri.Options->Extension->H264){
			options->enc_bps.min = resp_uri.Options->Extension->H264->BitrateRange->Min;
			options->enc_bps.max = resp_uri.Options->Extension->H264->BitrateRange->Max;
		} else { // user defined
			options->enc_bps.min = 32;
			options->enc_bps.max =5000;
		}

#ifndef WIN32
		NVP_dump_venc_options(options);
#endif // WIN32
		SOAP_destroy(soap);
		return 0;
	}

	return ret;	
}

int ONVIF_set_venc_profile(lpNVP_ARGS args, lpNVP_VENC_CONFIG param)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct tt__VideoRateControl rate_control;
	struct tt__H264Configuration h264;
	struct tt__VideoResolution resolution;
	struct tt__IPAddress address;
	struct tt__MulticastConfiguration multicast;
	struct tt__VideoEncoderConfiguration venc_conf;
	struct _trt__SetVideoEncoderConfiguration req_uri;
	struct _trt__SetVideoEncoderConfigurationResponse resp_uri;

#ifndef WIN32
	NVP_dump_venc_profile(param);
#endif // WIN32

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.ForcePersistence = ONVIF_TRUE;
	venc_conf.Name = param->enc_name;
	venc_conf.token = param->enc_token;
	venc_conf.UseCount = 4;
	venc_conf.Quality = (float)param->enc_quality;
	venc_conf.Encoding = param->enc_type;
		resolution.Width = param->width;
		resolution.Height = param->height;
	venc_conf.Resolution = &resolution;
		h264.GovLength = param->enc_gov;
		h264.H264Profile = param->enc_profile;
	venc_conf.H264 = &h264;
		rate_control.FrameRateLimit = param->enc_fps;
		rate_control.EncodingInterval = param->enc_interval;
		rate_control.BitrateLimit = param->enc_bps;
	venc_conf.RateControl = &rate_control;
	venc_conf.SessionTimeout = 10000;
	venc_conf.MPEG4 = NULL;
			address.Type = tt__IPType__IPv4;
			address.IPv4Address = "224.1.2.3";
			address.IPv6Address = NULL;
		multicast.Address = &address;
		multicast.Port = 12345;
		multicast.TTL = 1;
		multicast.AutoStart = ONVIF_TRUE;
		multicast.__size = 0;
		multicast.__any = NULL;
		multicast.__anyAttribute = NULL;
	venc_conf.Multicast = &multicast;
	venc_conf.__size = 0;
	venc_conf.__any = NULL;
	venc_conf.__anyAttribute = NULL;
	req_uri.Configuration = &venc_conf;

	if((ret = onvif_request(args, &soap, soap_call___trt__SetVideoEncoderConfiguration, ONVIF_MEDIA,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		SOAP_destroy(soap);
		return 0;
	}

	return ret;	
}


int ONVIF_get_profiles(lpNVP_ARGS args, lpNVP_PROFILE_CHN profile)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _trt__GetProfiles req_uri;
	struct _trt__GetProfilesResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	profile->profile_nr = 0;
	profile->index = args->chn;

	if(( ret  = onvif_request(args, &soap, soap_call___trt__GetProfiles, ONVIF_MEDIA,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{	
		int i;
		ONVIF_TRACE("soap success,__sizeProfiles:%d",resp_uri.__sizeProfiles);
		for(i=0;i<resp_uri.__sizeProfiles && i < ARRAY_ITEM(profile->venc); i++){		
			memset(&profile->venc[i], 0, sizeof(stNVP_VENC_CONFIG));
			if(resp_uri.Profiles == NULL){
				SOAP_destroy(soap);
				return -1;
			}
			if(resp_uri.Profiles[i].VideoEncoderConfiguration == NULL || resp_uri.Profiles[i].VideoSourceConfiguration == NULL){
				SOAP_destroy(soap);
				return -1;
			}

			profile->venc[i].index = i;
			strcpy(profile->name[i],resp_uri.Profiles[i].Name);
			strcpy(profile->token[i],resp_uri.Profiles[i].token);
			strcpy(profile->venc[i].name,resp_uri.Profiles[i].Name);
			strcpy(profile->venc[i].token,resp_uri.Profiles[i].token);
			strcpy(profile->vin[i].name,resp_uri.Profiles[i].VideoSourceConfiguration->Name);
			strcpy(profile->vin[i].token,resp_uri.Profiles[i].VideoSourceConfiguration->token);
			profile->venc[i].width = resp_uri.Profiles[i].VideoEncoderConfiguration->Resolution->Width;
			profile->venc[i].height = resp_uri.Profiles[i].VideoEncoderConfiguration->Resolution->Height;
			strcpy(profile->venc[i].enc_name, resp_uri.Profiles[i].VideoEncoderConfiguration->Name);
			strcpy(profile->venc[i].enc_token, resp_uri.Profiles[i].VideoEncoderConfiguration->token);
			profile->venc[i].enc_quality = (int)resp_uri.Profiles[i].VideoEncoderConfiguration->Quality;
			profile->venc[i].enc_type = (NVP_VENC_TYPE)resp_uri.Profiles[i].VideoEncoderConfiguration->Encoding;
			if(resp_uri.Profiles[i].VideoEncoderConfiguration->RateControl){
				profile->venc[i].enc_fps = resp_uri.Profiles[i].VideoEncoderConfiguration->RateControl->FrameRateLimit;
				profile->venc[i].enc_bps = resp_uri.Profiles[i].VideoEncoderConfiguration->RateControl->BitrateLimit;
				profile->venc[i].enc_interval = resp_uri.Profiles[i].VideoEncoderConfiguration->RateControl->EncodingInterval;
			}
			if(profile->venc[i].enc_type == NVP_VENC_H264){
				if(resp_uri.Profiles[i].VideoEncoderConfiguration->H264){
					profile->venc[i].enc_gov = resp_uri.Profiles[i].VideoEncoderConfiguration->H264->GovLength;
					profile->venc[i].enc_profile = resp_uri.Profiles[i].VideoEncoderConfiguration->H264->H264Profile;
				}
			}else if(profile->venc[i].enc_type == NVP_VENC_MPEG4){
				if(resp_uri.Profiles[i].VideoEncoderConfiguration->MPEG4){
					profile->venc[i].enc_gov = resp_uri.Profiles[i].VideoEncoderConfiguration->MPEG4->GovLength;
					profile->venc[i].enc_profile = resp_uri.Profiles[i].VideoEncoderConfiguration->MPEG4->Mpeg4Profile;
				}
			}
			if(resp_uri.Profiles[i].VideoAnalyticsConfiguration){
				strcpy(profile->van.token, resp_uri.Profiles[i].VideoAnalyticsConfiguration->token);
			}
			if(resp_uri.Profiles[i].PTZConfiguration){
				strcpy(profile->ptz.token, resp_uri.Profiles[i].PTZConfiguration->token);
			}
#ifndef WIN32
			NVP_dump_venc_profile(&profile->venc[i]);
#endif // WIN32
		}
		profile->venc_nr = resp_uri.__sizeProfiles;
		profile->profile_nr = profile->venc_nr;

		SOAP_destroy(soap);
		if(profile->profile_nr <= 0){
			return -1;
		}
		return 0;
	}

	return ret;	
}

static int ONVIF_get_venc_configs(lpNVP_ARGS args, lpNVP_VENC_CONFIGS vencs)
{
	int ret = -1;

	stNVP_PROFILE_CHN profile;
	
	memset(&profile, 0, sizeof(profile));
	if ((ret = ONVIF_get_profiles(args, &profile)) == 0) {
		int i;
		vencs->nr = profile.profile_nr;
		for (i = 0; i < profile.profile_nr; i++) {
			memcpy(&vencs->entry[i], &profile.venc[i], sizeof(stNVP_VENC_CONFIG));
		}
		return 0;
	}
	return ret;
}

static int onvif_get_video_source(lpNVP_ARGS args, lpNVP_PROFILE profiles)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _trt__GetVideoSources req_uri;
	struct _trt__GetVideoSourcesResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	memset(profiles, 0, sizeof(stNVP_PROFILE));
	
	if(( ret  = onvif_request(args, &soap, soap_call___trt__GetVideoSources, ONVIF_MEDIA,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{	
		int i;
		if(resp_uri.VideoSources == NULL  || resp_uri.__sizeVideoSources == 0){ 	
			ONVIF_INFO("soap success, but video source is empty");
			SOAP_destroy(soap);
			return -1;
		}
		profiles->chn  = resp_uri.__sizeVideoSources;
		ONVIF_TRACE("soap success, video source count is %d", profiles->chn);
		for ( i = 0; i < profiles->chn && i < sizeof(profiles->profile)/sizeof(profiles->profile[0]); i++) {
			ONVIF_TRACE("video source %d(%s) ", i, resp_uri.VideoSources[i].token);
			strcpy(profiles->profile[i].v_source.token, resp_uri.VideoSources[i].token);
			strcpy(profiles->profile[i].v_source.image.src_token, resp_uri.VideoSources[i].token);
			strcpy(profiles->profile[i].v_source.image.color.src_token, resp_uri.VideoSources[i].token);
			profiles->profile[i].v_source.fps = (int)resp_uri.VideoSources[i].Framerate;
			if ( resp_uri.VideoSources[i].Resolution) {
				profiles->profile[i].v_source.resolution.width = resp_uri.VideoSources[i].Resolution->Width;
				profiles->profile[i].v_source.resolution.height = resp_uri.VideoSources[i].Resolution->Height;
			}
			if (resp_uri.VideoSources[i].Imaging) {
				ONVIF_TRACE("Get Video Source Imaging");
				if (resp_uri.VideoSources[i].Imaging->Brightness)
					profiles->profile[i].v_source.image.color.brightness = *resp_uri.VideoSources[i].Imaging->Brightness;
				if (resp_uri.VideoSources[i].Imaging->Contrast)
					profiles->profile[i].v_source.image.color.contrast = *resp_uri.VideoSources[i].Imaging->Contrast;
				if (resp_uri.VideoSources[i].Imaging->ColorSaturation)
					profiles->profile[i].v_source.image.color.saturation = *resp_uri.VideoSources[i].Imaging->ColorSaturation;
				if (resp_uri.VideoSources[i].Imaging->Sharpness)
					profiles->profile[i].v_source.image.color.sharpness= *resp_uri.VideoSources[i].Imaging->Sharpness;
			}
			if (resp_uri.VideoSources[i].Extension && resp_uri.VideoSources[i].Extension->Imaging) {
				ONVIF_TRACE("Get Video Source Extension Imaging");
				if (resp_uri.VideoSources[i].Extension->Imaging->Brightness)
					profiles->profile[i].v_source.image.color.brightness = *resp_uri.VideoSources[i].Extension->Imaging->Brightness;
				if (resp_uri.VideoSources[i].Extension->Imaging->Contrast)
					profiles->profile[i].v_source.image.color.contrast = *resp_uri.VideoSources[i].Extension->Imaging->Contrast;
				if (resp_uri.VideoSources[i].Extension->Imaging->ColorSaturation)
					profiles->profile[i].v_source.image.color.saturation = *resp_uri.VideoSources[i].Extension->Imaging->ColorSaturation;
				if (resp_uri.VideoSources[i].Extension->Imaging->Sharpness)
					profiles->profile[i].v_source.image.color.sharpness= *resp_uri.VideoSources[i].Extension->Imaging->Sharpness;
			}
		}
		
		SOAP_destroy(soap);
		return 0;
	}

	return ret;	
}


int onvif_get_venc_config(lpNVP_ARGS args, lpNVP_VENC_CONFIG venc)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _trt__GetVideoEncoderConfiguration req_uri;
	struct _trt__GetVideoEncoderConfigurationResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.ConfigurationToken = venc->enc_token;

	if(( ret  = onvif_request(args, &soap, soap_call___trt__GetVideoEncoderConfiguration, ONVIF_MEDIA,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{	
		ONVIF_INFO("soap success");
		if(resp_uri.Configuration){ 	
			ONVIF_INFO("soap success, but configuration is null");
			SOAP_destroy(soap);
			return -1;
		}
		strcpy(venc->enc_name,resp_uri.Configuration->Name);
		strcpy(venc->enc_token,resp_uri.Configuration->token);
		if ( resp_uri.Configuration->Resolution) {
			venc->width = resp_uri.Configuration->Resolution->Width;
			venc->height = resp_uri.Configuration->Resolution->Height;
		}
		venc->enc_quality = (int)resp_uri.Configuration->Quality;
		venc->enc_type = (NVP_VENC_TYPE)resp_uri.Configuration->Encoding;
		if(resp_uri.Configuration->RateControl){
			venc->enc_fps = resp_uri.Configuration->RateControl->FrameRateLimit;
			venc->enc_bps = resp_uri.Configuration->RateControl->BitrateLimit;
			venc->enc_interval = resp_uri.Configuration->RateControl->EncodingInterval;
		}
		if(venc->enc_type == NVP_VENC_H264){
			if(resp_uri.Configuration->H264){
				venc->enc_gov = resp_uri.Configuration->H264->GovLength;
				venc->enc_profile = resp_uri.Configuration->H264->H264Profile;
			}
		}else if(venc->enc_type == NVP_VENC_MPEG4){
			if(resp_uri.Configuration->MPEG4){
				venc->enc_gov = resp_uri.Configuration->MPEG4->GovLength;
				venc->enc_profile = resp_uri.Configuration->MPEG4->Mpeg4Profile;
			}
		}
#ifndef WIN32
		NVP_dump_venc_profile(venc);
#endif // WIN32
		SOAP_destroy(soap);
		return 0;
	}

	return ret;	
}


int ONVIF_get_venc_config(lpNVP_ARGS args, lpNVP_VENC_CONFIG venc)
{
	int ret = -1;
	stNVP_PROFILE_CHN profile;
	
	memset(&profile, 0, sizeof(profile));
	if (( ret = ONVIF_get_profiles(args, &profile)) < 0) {
		return ret;
	} else {
		if (venc->index >= profile.profile_nr) {
			return -1;
		}
		memcpy(venc, &profile.venc[venc->index], sizeof(stNVP_VENC_CONFIG));
		return 0;
	}
}

static int ONVIF_get_protocal_port(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tds__GetNetworkProtocols req_uri;
	struct _tds__GetNetworkProtocolsResponse resp_uri;
	
	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	
	if(( ret  = onvif_request(args, &soap, soap_call___tds__GetNetworkProtocols, ONVIF_DEVICE,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		int i;
		ONVIF_INFO("##### __sizeNetworkProtocols %d ######",resp_uri.__sizeNetworkProtocols);
		for(i=0;i<resp_uri.__sizeNetworkProtocols;i++){ // only get one ethernet
			if (resp_uri.NetworkProtocols) {
				if (resp_uri.NetworkProtocols[i].Name == tt__NetworkProtocolType__HTTP) {
					ether->http_port = resp_uri.NetworkProtocols[i].Port ? (resp_uri.NetworkProtocols[i].Port[0]) : 0;
				}
				else if (resp_uri.NetworkProtocols[i].Name == tt__NetworkProtocolType__RTSP) {
					ether->rtsp_port = resp_uri.NetworkProtocols[i].Port ? (resp_uri.NetworkProtocols[i].Port[0]) : 0;
				}
			}
		}
		
		SOAP_destroy(soap);
		return  0;
	}

	return ret;
}

static int ONVIF_set_protocal_port(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether) /* only set http port */
{
	int ret = -1;
	struct soap *soap = NULL;
	struct tt__NetworkProtocol protocals;
	struct _tds__SetNetworkProtocols req_uri;
	struct _tds__SetNetworkProtocolsResponse resp_uri;
	
	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	memset(&protocals, 0, sizeof(protocals));
	req_uri.__sizeNetworkProtocols = 1;
	protocals.Enabled = ONVIF_TRUE;
	protocals.Name = tt__NetworkProtocolType__HTTP;
	protocals.__sizePort = 1;
	protocals.Port = &ether->http_port;
	req_uri.NetworkProtocols = &protocals;
	
	if(( ret  = onvif_request(args, &soap, soap_call___tds__SetNetworkProtocols, ONVIF_DEVICE,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		SOAP_destroy(soap);
		return  0;
	}

	return ret;
}

static int onvif_get_network_interface(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tds__GetNetworkInterfaces req_uri;
	struct _tds__GetNetworkInterfacesResponse resp_uri;
	
	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	
	if(( ret = onvif_request(args, &soap, soap_call___tds__GetNetworkInterfaces, ONVIF_DEVICE,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		int i;
		ONVIF_INFO("##### __sizeNetworkInterfaces %d ######",resp_uri.__sizeNetworkInterfaces);
		for(i=0;i<resp_uri.__sizeNetworkInterfaces && i < 1;i++){ // only get one ethernet
			if(resp_uri.NetworkInterfaces[i].Info){
				if( macstr2uint8(ether->mac, resp_uri.NetworkInterfaces[i].Info->HwAddress) < 0 ) {
					memset(ether->mac, 0, sizeof(ether->mac));					
				}
			}else{
				memset(ether->mac, 0, sizeof(ether->mac));
			}
			strcpy(ether->token, resp_uri.NetworkInterfaces[i].token);
			if(resp_uri.NetworkInterfaces[i].IPv4){
				ether->dhcp = resp_uri.NetworkInterfaces[i].IPv4->Config->DHCP;
				if(resp_uri.NetworkInterfaces[i].IPv4->Config->DHCP == ONVIF_TRUE){
					if( resp_uri.NetworkInterfaces[i].IPv4->Config->FromDHCP){
						if(ipstr2uint8(ether->ip, resp_uri.NetworkInterfaces[i].IPv4->Config->FromDHCP->Address) < 0) {
							SOAP_destroy(soap);
							return -1;
						}
						netmask_from_prefixlength(resp_uri.NetworkInterfaces[i].IPv4->Config->FromDHCP->PrefixLength,ether->netmask);
					}else{
						memset(ether->ip, 0, sizeof(ether->ip));
						memset(ether->netmask, 0, sizeof(ether->netmask));
					}
				}else{
					if( resp_uri.NetworkInterfaces[i].IPv4->Config){
						if (resp_uri.NetworkInterfaces[i].IPv4->Config->Manual) {
							if(ipstr2uint8(ether->ip, resp_uri.NetworkInterfaces[i].IPv4->Config->Manual->Address) < 0) {
								SOAP_destroy(soap);
								return -1;
							}
							netmask_from_prefixlength(resp_uri.NetworkInterfaces[i].IPv4->Config->Manual->PrefixLength,ether->netmask);
						}  /*
						else if (resp_uri.NetworkInterfaces[i].IPv4->Config->LinkLocal) {
							if(ipstr2uint8(ether->ip, resp_uri.NetworkInterfaces[i].IPv4->Config->LinkLocal->Address) < 0) {
								SOAP_destroy(soap);
								return -1;
							}
							netmask_from_prefixlength(resp_uri.NetworkInterfaces[i].IPv4->Config->LinkLocal->PrefixLength,ether->netmask);
						} */
						else {
							SOAP_destroy(soap);
							return -1;
						}
					}else{
						memset(ether->ip, 0, sizeof(ether->ip));
						memset(ether->netmask, 0, sizeof(ether->netmask));
					}
				}
			}
		}

#ifndef WIN32
		NVP_dump_ether_info(ether);
#endif // WIN32

		SOAP_destroy(soap);
		return  0;
	}

	return ret;
}

static int ONVIF_get_network_interface(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether)
{
	int ret = -1;
	if(( ret = ONVIF_get_protocal_port(args, ether)) < 0) {
		return ret;
	}
	if(( ret = onvif_get_network_interface(args, ether)) < 0) {
		return ret;
	}
	return 0;
}

static int onvif_set_network_interface(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether)
{	
#define SET_NETWORK_ALWAYS_REBOOT
	int ret = -1;
	struct soap *soap = NULL;
	char szipv4ip[20];
	stNVP_ARGS _arg;
	int _ret;
	struct tt__PrefixedIPv4Address ipv4addr;
	struct tt__IPv4NetworkInterfaceSetConfiguration ipv4conf;
	struct tt__NetworkInterfaceSetConfiguration netconf;
	struct _tds__SetNetworkInterfaces req_uri;
	struct _tds__SetNetworkInterfacesResponse resp_uri;
	
	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	memset(&netconf, 0, sizeof(netconf));
	memset(&ipv4addr, 0, sizeof(ipv4addr));
	memset(&ipv4conf, 0, sizeof(ipv4conf));
	req_uri.InterfaceToken = ether->token;
		netconf.Enabled = &m_NTRUE;
			if (ether->dhcp == true)
				ipv4conf.DHCP = &m_NTRUE;
			else
				ipv4conf.DHCP = &m_NFALSE;
			ipv4conf.Enabled = &m_NTRUE;
			ipv4conf.__sizeManual = 1;
				ipv4addr.PrefixLength = netmask_to_prefixlength(ether->netmask);
				sprintf(szipv4ip, "%d.%d.%d.%d", ether->ip[0], ether->ip[1], ether->ip[2], ether->ip[3]);
				ipv4addr.Address = szipv4ip;
			ipv4conf.Manual = &ipv4addr;
		netconf.IPv4 = &ipv4conf;
	req_uri.NetworkInterface = &netconf;
	if(( ret = onvif_request(args, &soap, soap_call___tds__SetNetworkInterfaces, ONVIF_DEVICE,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		ONVIF_INFO("##### reboot Needed %d ######",resp_uri.RebootNeeded);		
		SOAP_destroy(soap);
#ifdef SET_NETWORK_ALWAYS_REBOOT
		if (resp_uri.RebootNeeded == ONVIF_TRUE) {
			if (ONVIF_reboot(args, NULL) < 0) {
				return -1;
			}
		}else{
			Sleep_c(4);
			memcpy(&_arg, args, sizeof(_arg));
			sprintf(_arg.ip, "%d.%d.%d.%d", ether->ip[0],
				ether->ip[1],ether->ip[2],ether->ip[3]);
			ONVIF_INFO("Reboot Device: %s", _arg.ip);
			if ((_ret = ONVIF_reboot(&_arg, NULL)) < 0) {
				if (_ret == (-SOAP_TCP_ERROR)) {
					ONVIF_INFO("Retry Reboot Device: %s(%s)", args->ip, _arg.ip);
					_ret = ONVIF_reboot(args, NULL);
					if (_ret < 0) {
						return -1;
					}
				}else{
					return -1;
				}
			}
		}
				
#else // SET_NETWORK_ALWAYS_REBOOT
		if (resp_uri.RebootNeeded == ONVIF_TRUE) {
			if (ONVIF_reboot(args, NULL) < 0) {
				return -1;
			}
		} 
		else if((ether->dhcp == false) && (ether->http_port == 0)){
			// some devices, (such as XM IPC), should reboot 
			char _server[128];
			char _xaddr[128];
			char _xaddr_bak[128];

			sleep(1);
			memcpy(&_arg, args, sizeof(_arg));
			sprintf(_arg.ip, "%d.%d.%d.%d", ether->ip[0],
				ether->ip[1],ether->ip[2],ether->ip[3]);
			sprintf(_server, "http://%d.%d.%d.%d:%d/onvif/device_service", ether->ip[0],
				ether->ip[1],ether->ip[2],ether->ip[3],args->port);
			if (onvif_get_xaddr(NULL, _server,args->username, args->password, ONVIF_DEVICE,_xaddr) == 0) {
				strcpy(_xaddr_bak, _xaddr);
				if((onvif_check_xaddr(_server, _xaddr, true) == 0) &&
					(strcmp(_xaddr, _xaddr_bak) != 0)){
					ONVIF_INFO("Reboot Device: %s", _arg.ip);
					if ((_ret = ONVIF_reboot(&_arg, NULL)) < 0) {
						if (_ret == (-SOAP_TCP_ERROR)) {
							_ret = ONVIF_reboot(args, NULL);
						}
					}
				}
			}
		}
#endif // SET_NETWORK_ALWAYS_REBOOT
		return  0;
	}
	ONVIF_INFO("##### Set NetConf(%s) Faild!",args->ip);		

	return ret;
}

int ONVIF_set_network_interface(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether)
{
	int ret = -1;
	stNVP_ETHER_CONFIG netconf;
	memset(&netconf, 0, sizeof(netconf));
	if (( ret = ONVIF_get_network_interface(args, &netconf)) < 0) {
		return ret;
	}
	strcpy(ether->token, netconf.token);
	if (ether->http_port || ether->rtsp_port) {
		if ((ret = ONVIF_set_protocal_port(args, ether)) < 0) {
			return ret;
		}
	}
	if ((ret = onvif_set_network_interface(args, ether)) < 0) {
		return ret;
	}
	return 0;
}

static int check_rtsp_url(unsigned char *_u8ip, unsigned short *_port, char *_uri, char *_default) 
{ 
	int ret;
	char *ppp= NULL; 
	char ipport[32];
	int tmp[4]; 
	int tmp_port;
	char _uri_temp[128];
	if (_u8ip) {
		_u8ip[0] = 0; 
		_u8ip[1] = 0; 
		_u8ip[2] = 0; 
		_u8ip[3] = 0; 
	}
	_uri_temp[0] = 0; 
	if (_port) *_port = 0;
	if (strncmp(_default, "rtsp://", strlen("rtsp://")) !=0 ) {
		return -1;
	}
	ppp = _default + strlen("rtsp://");
	ret  = sscanf(ppp, "%[^/]%s", ipport, _uri_temp);
	if(ret == 2){
		if(strstr(ipport,":")!=NULL){
			char szip[64]; 
			int port;
			if (sscanf(ipport, "%[^:]:%d", szip, &port) != 2) {
				printf("ERR: parse ip address failed01 : %s!\n", ipport);
				return -1;
			}
			if (check_ipv4_addr(szip) != 0) {
				printf("ERR: parse ip address failed02 : %s!\n", ipport);
				return -1;
			}
			if ((szip[0] == '0') 
				|| (((*(szip + strlen(szip) - 1)) == '0') && ((*(szip + strlen(szip) - 2)) == '.'))
				|| (strstr(szip, "255") != NULL)) {
				printf("ERR: parse ip address failed022 : %s!\n", ipport);
				return -1;
			}
			if (port <= 0 || port >= 65535) {
				printf("ERR: parse ip address failed03 : %s!\n", ipport);
				return -1;
			}
			if(sscanf(ipport, "%d.%d.%d.%d:%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3],&tmp_port) == 5) 
			{ 
				if (_u8ip) {
					_u8ip[0] = tmp[0]; 
					_u8ip[1] = tmp[1]; 
					_u8ip[2] = tmp[2]; 
					_u8ip[3] = tmp[3]; 
				}
				if (_port) *_port = tmp_port;
			}else{
				printf("ERR: parse ip address failed1 : %s!\n", ipport);
				return -1;
			}
		}else{
			if (check_ipv4_addr(ipport) != 0) {
				printf("ERR: parse ip address failed11 : %s!\n", ipport);
				return -1;
			}
			if(sscanf(ipport, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) == 4) 
			{ 
				if (_u8ip) {
					_u8ip[0] = tmp[0]; 
					_u8ip[1] = tmp[1]; 
					_u8ip[2] = tmp[2]; 
					_u8ip[3] = tmp[3]; 
				}
				if (_port) *_port = 80;
			}else{
				printf("ERR: parse ip address failed2: %s!\n", ipport);
				return -1;
			}
		}
	}else{
		printf("ERR: parse ip address failed4 !\n");
		return -1;
	}
	if (_uri) {
		strcpy(_uri, _uri_temp);
	}
	return 0;
}


int ONVIF_get_stream_uri(lpNVP_ARGS args, char *token,char *url)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct tt__Transport transport;
	struct tt__StreamSetup stream;
	struct _trt__GetStreamUri req_uri;
	struct _trt__GetStreamUriResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.ProfileToken = token;
	transport.Protocol = tt__TransportProtocol__RTSP;
	transport.Tunnel = NULL;
	stream.Stream =tt__StreamType__RTP_Unicast;
	stream.Transport = (struct tt__Transport *)&transport;
	stream.__any = NULL;
	stream.__anyAttribute = NULL;
	stream.__size = 0;
	req_uri.StreamSetup = (struct tt__StreamSetup *)&stream;

	if((ret = onvif_request(args, &soap, soap_call___trt__GetStreamUri, ONVIF_MEDIA,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		// check valid url
		if (check_rtsp_url(NULL, NULL, NULL, resp_uri.MediaUri->Uri) < 0) {
			ONVIF_INFO("### invalid rtsp stream url:%s",resp_uri.MediaUri->Uri);
			SOAP_destroy(soap);
			return -1;
		}
		ONVIF_INFO("### rtsp stream url:%s",resp_uri.MediaUri->Uri);
		if(url) strcpy(url, resp_uri.MediaUri->Uri);

		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;	
}


int ONVIF_get_rtsp_uri(lpNVP_ARGS args, lpNVP_RTSP_STREAM rtsp)
{
	int ret = -1;
	int i;
	int success_count = 0;
	stNVP_PROFILE_CHN profile;

	strcpy(rtsp->main_uri, "rtsp://10.11.12.13:554/blank.stream");
	strcpy(rtsp->sub_uri, "rtsp://10.11.12.13:554/blank.stream");

	memset(&profile, 0, sizeof(profile));
	if((ret =  ONVIF_get_profiles(args, &profile)) < 0){
		ONVIF_INFO("GetRtspStreamUri Failed 0, ret=%d", ret);
		return ret;
	}
	for ( i = 0; i < profile.profile_nr; i++){
		if(i  == rtsp->main_index){
			ONVIF_TRACE("main: %d token:%s", i, profile.venc[i].token);
			if(ONVIF_get_stream_uri(args, profile.venc[i].token, rtsp->main_uri)  == 0){
				success_count++;
			}
		}
		//sleep(4);
		if((i == rtsp->sub_index) && (rtsp->sub_index != rtsp->main_index) && (rtsp->sub_index >= 0)){
			ONVIF_TRACE("sub: %d token:%s", i, profile.venc[i].token);
			if(ONVIF_get_stream_uri(args, profile.venc[i].token, rtsp->sub_uri) == 0){
				success_count++;
			}
		}
	}
	if ((rtsp->main_index == rtsp->sub_index) || (rtsp->sub_index == (-1))) {
		if (success_count == 1) {
			return 0;
		} else {
			ONVIF_INFO("GetRtspStreamUri Failed 1");
			return -1;
		}
	} else {
		if(success_count == 2){
			return 0;
		}else{
			ONVIF_INFO("GetRtspStreamUri Failed 2");
			return -1;
		}
	}
}

static int onvif_continueus_move(lpNVP_ARGS args, char *token,  NVP_PTZ_CMD cmd, float speed)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct tt__Vector2D pantilt;
	struct tt__Vector1D zoom;
	struct tt__PTZSpeed velocity;
	struct _tptz__ContinuousMove req_uri;
	struct _tptz__ContinuousMoveResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	pantilt.space = NULL;
	zoom.space = NULL;
	velocity.PanTilt = NULL;
	velocity.Zoom = NULL;
	switch (cmd)
	{
		case NVP_PTZ_CMD_AUTOPAN:
		case NVP_PTZ_CMD_LEFT:
		{
			pantilt.x = -1 * speed;
			pantilt.y = 0;
			velocity.PanTilt = &pantilt;
			break;				
		}
		case NVP_PTZ_CMD_RIGHT:
		{
			pantilt.x = speed;
			pantilt.y = 0;
			velocity.PanTilt = &pantilt;
			break;
		}
		case NVP_PTZ_CMD_UP:
		{
			pantilt.x = 0;
			pantilt.y = speed;
			velocity.PanTilt = &pantilt;
			break;
		}
		case NVP_PTZ_CMD_DOWN:
		{				
			pantilt.x = 0;
			pantilt.y = -1 * speed;
			velocity.PanTilt = &pantilt;
			break;
		}
		case NVP_PTZ_CMD_ZOOM_IN:
		{
			zoom.x = speed;
			velocity.Zoom = &zoom;
			break;
		}
		case NVP_PTZ_CMD_ZOOM_OUT:
		{
			zoom.x = -1 * speed;
			velocity.Zoom = &zoom;
			break;
		}
		default:
			return -1;
	}
	req_uri.ProfileToken = token;
	req_uri.Velocity = &velocity;
	req_uri.Timeout = NULL;

	if(( ret = onvif_request(args, &soap, soap_call___tptz__ContinuousMove, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;	
}

static int onvif_stop(lpNVP_ARGS args, char *token)
{
	int ret = -1;
	struct soap *soap = NULL;
	enum xsd__boolean pantilt, zoom;
	struct _tptz__Stop req_uri;
	struct _tptz__StopResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.ProfileToken = token;
	pantilt = ONVIF_TRUE;
	zoom = ONVIF_TRUE;
	req_uri.PanTilt = &pantilt;
	req_uri.Zoom = &zoom;

	if(( ret = onvif_request(args, &soap, soap_call___tptz__Stop, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;	
}

typedef struct _onvif_preset
{
	int index;
	char name[64];
	char token[64];
}stONVIF_PRESET, *lpONVIF_PRESET;
#define MAX_ONVIF_PRESET	(256)
typedef struct _onvif_presets
{
	int nr;
	stONVIF_PRESET entry[MAX_ONVIF_PRESET];
}stONVIF_PRESETS, *lpONVIF_PRESETS;

static int preset_compare_by_token(const void *aa, const void *bb)
{
	int a_i = 0, b_i = 0;
	char a_format[64];
	char b_format[64];
	lpONVIF_PRESET a = (lpONVIF_PRESET)aa;
	lpONVIF_PRESET b = (lpONVIF_PRESET)bb;

	if ((get_int_string_format(a->token, a_format) == 1)
		&& (get_int_string_format(b->token, b_format) == 1)) {
		if (strcmp(a_format, b_format) != 0) {
			return -1;
		}
		sscanf(a->token, a_format, &a_i);
		sscanf(b->token, b_format, &b_i);
		if (a_i < b_i) 
			return -1;
		else if (a_i > b_i) {
			return 1;
		}else
			return 0;
	} else {
		return -1;
	}
}

typedef struct PTZ_CONF
{
	char name[32];
	char token[32];
	char node_name[32];
	char node_token[32];
	int preset_number;
	bool home_support;
	char absolute_pantilt_space[128];
	char absolute_zoom_space[128];
	char continue_pantilt_space[128];
	char continue_zoom_space[128];
	char relative_pantilt_space[128];
	char relative_zoom_space[128];
	char pantilt_speed_space[128];
	char zoom_speed_space[128];
	float pan_speed;
	float tilt_speed;
	float zoom_speed;
}stPTZ_CONF, *lpPTZ_CONF;

static int onvif_get_ptz_nodes(lpNVP_ARGS args)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tptz__GetNodes req_uri;
	struct _tptz__GetNodesResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));

	if(( ret = onvif_request(args, &soap, soap_call___tptz__GetNodes, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		int i, n;
		for ( i = 0; i < resp_uri.__sizePTZNode; i++) {
			ONVIF_TRACE("NODE%d name:%s token:%s home:%d preset-num:%d", i, resp_uri.PTZNode[i].Name, resp_uri.PTZNode[i].token,
				resp_uri.PTZNode[i].HomeSupported, resp_uri.PTZNode[i].MaximumNumberOfPresets);
			if (resp_uri.PTZNode[i].SupportedPTZSpaces) {
				for (n  = 0; n < resp_uri.PTZNode[i].SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace; n++) {
					ONVIF_TRACE("absolute pantilt space[%d]:%s(%f,%f/%f,%f)", n, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].URI,
						resp_uri.PTZNode[i].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].XRange->Min, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].XRange->Max,
						resp_uri.PTZNode[i].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].YRange->Min,
						resp_uri.PTZNode[i].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].YRange->Max);
				}
				for (n  = 0; n < resp_uri.PTZNode[i].SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace; n++) {
					ONVIF_TRACE("absolute zoom space[%d]:%s(%f,%f)", n, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->AbsoluteZoomPositionSpace[n].URI,
						resp_uri.PTZNode[i].SupportedPTZSpaces->AbsoluteZoomPositionSpace[n].XRange->Min, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->AbsoluteZoomPositionSpace[n].XRange->Max);
				}
				for (n  = 0; n < resp_uri.PTZNode[i].SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace; n++) {
					ONVIF_TRACE("relative pantilt space[%d]:%s(%f,%f/%f,%f)", n, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].URI,
						resp_uri.PTZNode[i].SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].XRange->Min, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].XRange->Max,
						resp_uri.PTZNode[i].SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].YRange->Min,
						resp_uri.PTZNode[i].SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].YRange->Max);
				}
				for (n  = 0; n < resp_uri.PTZNode[i].SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace; n++) {
					ONVIF_TRACE("relative zoom space[%d]:%s(%f,%f)", n, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->RelativeZoomTranslationSpace[n].URI,
						resp_uri.PTZNode[i].SupportedPTZSpaces->RelativeZoomTranslationSpace[n].XRange->Min, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->RelativeZoomTranslationSpace[n].XRange->Max);
				}
				for (n  = 0; n < resp_uri.PTZNode[i].SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace; n++) {
					ONVIF_TRACE("continue pantilt space[%d]:%s(%f,%f/%f,%f)", n, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].URI,
						resp_uri.PTZNode[i].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].XRange->Min, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].XRange->Max,
						resp_uri.PTZNode[i].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].YRange->Min,
						resp_uri.PTZNode[i].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].YRange->Max);
				}
				for (n  = 0; n < resp_uri.PTZNode[i].SupportedPTZSpaces->__sizeContinuousZoomVelocitySpace; n++) {
					ONVIF_TRACE("continue zoom space[%d]:%s(%f,%f)", n, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->ContinuousZoomVelocitySpace[n].URI,
						resp_uri.PTZNode[i].SupportedPTZSpaces->ContinuousZoomVelocitySpace[n].XRange->Min, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->ContinuousZoomVelocitySpace[n].XRange->Max);
				}
				for (n  = 0; n < resp_uri.PTZNode[i].SupportedPTZSpaces->__sizePanTiltSpeedSpace; n++) {
					ONVIF_TRACE("pantilt speed space[%d]:%s(%f,%f)", n, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->PanTiltSpeedSpace[n].URI,
						resp_uri.PTZNode[i].SupportedPTZSpaces->PanTiltSpeedSpace[n].XRange->Min, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->PanTiltSpeedSpace[n].XRange->Max);
				}
				for (n  = 0; n < resp_uri.PTZNode[i].SupportedPTZSpaces->__sizeZoomSpeedSpace; n++) {
					ONVIF_TRACE("zoom speed space[%d]:%s(%f,%f)", n, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->ZoomSpeedSpace[n].URI,
						resp_uri.PTZNode[i].SupportedPTZSpaces->ZoomSpeedSpace[n].XRange->Min, 
						resp_uri.PTZNode[i].SupportedPTZSpaces->ZoomSpeedSpace[n].XRange->Max);
				}
			}
		}

		
		SOAP_destroy(soap);
		return 0;
	}

	return ret;
}

static int onvif_get_ptz_node(lpNVP_ARGS args, char *token, lpPTZ_CONF ptz)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tptz__GetNode req_uri;
	struct _tptz__GetNodeResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.NodeToken = token;

	if(( ret = onvif_request(args, &soap, soap_call___tptz__GetNode, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		int n;
		ONVIF_TRACE("NODE name:%s token:%s home:%d preset-num:%d", resp_uri.PTZNode->Name, resp_uri.PTZNode->token,
			resp_uri.PTZNode->HomeSupported, resp_uri.PTZNode->MaximumNumberOfPresets);
		strcpy(ptz->node_name, resp_uri.PTZNode->Name);
		strcpy(ptz->node_token, resp_uri.PTZNode->token);
		ptz->home_support = (bool)resp_uri.PTZNode->HomeSupported;
		ptz->preset_number = resp_uri.PTZNode->MaximumNumberOfPresets;
		if (resp_uri.PTZNode->SupportedPTZSpaces) {
			for (n  = 0; n < resp_uri.PTZNode->SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace; n++) {
				ONVIF_TRACE("absolute pantilt space[%d]:%s(%f,%f/%f,%f)", n, 
					resp_uri.PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].URI,
					resp_uri.PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].XRange->Min, 
					resp_uri.PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].XRange->Max,
					resp_uri.PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].YRange->Min,
					resp_uri.PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].YRange->Max);
				strcpy(ptz->absolute_pantilt_space, resp_uri.PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[n].URI);
			}
			for (n  = 0; n < resp_uri.PTZNode->SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace; n++) {
				ONVIF_TRACE("absolute zoom space[%d]:%s(%f,%f)", n, 
					resp_uri.PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace[n].URI,
					resp_uri.PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace[n].XRange->Min, 
					resp_uri.PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace[n].XRange->Max);
				strcpy(ptz->absolute_zoom_space, resp_uri.PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace[n].URI);
			}
			for (n  = 0; n < resp_uri.PTZNode->SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace; n++) {
				ONVIF_TRACE("relative pantilt space[%d]:%s(%f,%f/%f,%f)", n, 
					resp_uri.PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].URI,
					resp_uri.PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].XRange->Min, 
					resp_uri.PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].XRange->Max,
					resp_uri.PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].YRange->Min,
					resp_uri.PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].YRange->Max);
				strcpy(ptz->relative_pantilt_space, resp_uri.PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[n].URI);
			}
			for (n  = 0; n < resp_uri.PTZNode->SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace; n++) {
				ONVIF_TRACE("relative zoom space[%d]:%s(%f,%f)", n, 
					resp_uri.PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace[n].URI,
					resp_uri.PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace[n].XRange->Min, 
					resp_uri.PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace[n].XRange->Max);
				strcpy(ptz->relative_zoom_space, resp_uri.PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace[n].URI);
			}
			for (n  = 0; n < resp_uri.PTZNode->SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace; n++) {
				ONVIF_TRACE("continue pantilt space[%d]:%s(%f,%f/%f,%f)", n, 
					resp_uri.PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].URI,
					resp_uri.PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].XRange->Min, 
					resp_uri.PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].XRange->Max,
					resp_uri.PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].YRange->Min,
					resp_uri.PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].YRange->Max);
				strcpy(ptz->continue_pantilt_space, resp_uri.PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[n].URI);
			}
			for (n  = 0; n < resp_uri.PTZNode->SupportedPTZSpaces->__sizeContinuousZoomVelocitySpace; n++) {
				ONVIF_TRACE("continue zoom space[%d]:%s(%f,%f)", n, 
					resp_uri.PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace[n].URI,
					resp_uri.PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace[n].XRange->Min, 
					resp_uri.PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace[n].XRange->Max);
				strcpy(ptz->continue_zoom_space, resp_uri.PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace[n].URI);
			}
			for (n  = 0; n < resp_uri.PTZNode->SupportedPTZSpaces->__sizePanTiltSpeedSpace; n++) {
				ONVIF_TRACE("pantilt speed space[%d]:%s(%f,%f)", n, 
					resp_uri.PTZNode->SupportedPTZSpaces->PanTiltSpeedSpace[n].URI,
					resp_uri.PTZNode->SupportedPTZSpaces->PanTiltSpeedSpace[n].XRange->Min, 
					resp_uri.PTZNode->SupportedPTZSpaces->PanTiltSpeedSpace[n].XRange->Max);
				strcpy(ptz->pantilt_speed_space, resp_uri.PTZNode->SupportedPTZSpaces->PanTiltSpeedSpace[n].URI);
			}
			for (n  = 0; n < resp_uri.PTZNode->SupportedPTZSpaces->__sizeZoomSpeedSpace; n++) {
				ONVIF_TRACE("zoom speed space[%d]:%s(%f,%f)", n, 
					resp_uri.PTZNode->SupportedPTZSpaces->ZoomSpeedSpace[n].URI,
					resp_uri.PTZNode->SupportedPTZSpaces->ZoomSpeedSpace[n].XRange->Min, 
					resp_uri.PTZNode->SupportedPTZSpaces->ZoomSpeedSpace[n].XRange->Max);
				strcpy(ptz->zoom_speed_space, resp_uri.PTZNode->SupportedPTZSpaces->ZoomSpeedSpace[n].URI);
			}
		}

		
		SOAP_destroy(soap);
		return 0;
	}
	ONVIF_TRACE("soap_call___tptz__GetNode failed");

	return ret;
}


static int onvif_get_ptz_configurations(lpNVP_ARGS args)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tptz__GetConfigurations req_uri;
	struct _tptz__GetConfigurationsResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));

	if(( ret = onvif_request(args, &soap, soap_call___tptz__GetConfigurations, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		int i;
		for ( i = 0; i < resp_uri.__sizePTZConfiguration; i++) {
			ONVIF_TRACE("PTZ CONF%d name:%s token:%s node:%s", i, resp_uri.PTZConfiguration[i].Name, 
				resp_uri.PTZConfiguration[i].token,
				resp_uri.PTZConfiguration[i].NodeToken);
			if (resp_uri.PTZConfiguration[i].DefaultAbsolutePantTiltPositionSpace)
				ONVIF_TRACE("\tdefault absolute pantilt space :%s", resp_uri.PTZConfiguration[i].DefaultAbsolutePantTiltPositionSpace);
			if (resp_uri.PTZConfiguration[i].DefaultAbsoluteZoomPositionSpace)
				ONVIF_TRACE("\tdefault absolute zoom space :%s", resp_uri.PTZConfiguration[i].DefaultAbsoluteZoomPositionSpace);
			if (resp_uri.PTZConfiguration[i].DefaultRelativePanTiltTranslationSpace)
				ONVIF_TRACE("\tdefault relative pantilt space :%s", resp_uri.PTZConfiguration[i].DefaultRelativePanTiltTranslationSpace);
			if (resp_uri.PTZConfiguration[i].DefaultRelativeZoomTranslationSpace)
				ONVIF_TRACE("\tdefault relative zoom space :%s", resp_uri.PTZConfiguration[i].DefaultRelativeZoomTranslationSpace);
			if (resp_uri.PTZConfiguration[i].DefaultContinuousPanTiltVelocitySpace)
				ONVIF_TRACE("\tdefault continue pantilt space :%s", resp_uri.PTZConfiguration[i].DefaultContinuousPanTiltVelocitySpace);
			if (resp_uri.PTZConfiguration[i].DefaultContinuousZoomVelocitySpace)
				ONVIF_TRACE("\tdefault continue zoom space :%s", resp_uri.PTZConfiguration[i].DefaultContinuousZoomVelocitySpace);
			if (resp_uri.PTZConfiguration[i].DefaultPTZSpeed &&
				resp_uri.PTZConfiguration[i].DefaultPTZSpeed->PanTilt &&
				resp_uri.PTZConfiguration[i].DefaultPTZSpeed->Zoom) {
				ONVIF_TRACE("\tdefault ptz speed (%f,%f,%f)", resp_uri.PTZConfiguration[i].DefaultPTZSpeed->PanTilt->x,
					resp_uri.PTZConfiguration[i].DefaultPTZSpeed->PanTilt->y,
					resp_uri.PTZConfiguration[i].DefaultPTZSpeed->Zoom->x);
			}
			if (resp_uri.PTZConfiguration[i].PanTiltLimits) {
				ONVIF_TRACE("\tpantilt limit (%f,%f/%f,%f)", 
					resp_uri.PTZConfiguration[i].PanTiltLimits->Range->XRange->Min,
					resp_uri.PTZConfiguration[i].PanTiltLimits->Range->XRange->Max,
					resp_uri.PTZConfiguration[i].PanTiltLimits->Range->YRange->Min,
					resp_uri.PTZConfiguration[i].PanTiltLimits->Range->YRange->Max);
			}
			if (resp_uri.PTZConfiguration[i].ZoomLimits) {
				ONVIF_TRACE("\tzoom limit (%f,%f)", 
					resp_uri.PTZConfiguration[i].ZoomLimits->Range->XRange->Min,
					resp_uri.PTZConfiguration[i].ZoomLimits->Range->XRange->Max);
			}
			
		}

		SOAP_destroy(soap);
		return 0;
	}

	return ret;
}

static int onvif_get_ptz_configuration(lpNVP_ARGS args, char *token, lpPTZ_CONF ptz)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tptz__GetConfiguration req_uri;
	struct _tptz__GetConfigurationResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.PTZConfigurationToken = token;

	if(( ret = onvif_request(args, &soap, soap_call___tptz__GetConfiguration, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		ONVIF_TRACE("PTZ CONF name:%s token:%s node:%s", resp_uri.PTZConfiguration->Name, 
			resp_uri.PTZConfiguration->token,
			resp_uri.PTZConfiguration->NodeToken);
		strcpy(ptz->name, resp_uri.PTZConfiguration->Name);
		strcpy(ptz->token, resp_uri.PTZConfiguration->token);
		strcpy(ptz->node_token, resp_uri.PTZConfiguration->NodeToken);
		if (resp_uri.PTZConfiguration->DefaultAbsolutePantTiltPositionSpace) {
			ONVIF_TRACE("\tdefault absolute pantilt space :%s", resp_uri.PTZConfiguration->DefaultAbsolutePantTiltPositionSpace);
		}
		if (resp_uri.PTZConfiguration->DefaultAbsoluteZoomPositionSpace) {
			ONVIF_TRACE("\tdefault absolute zoom space :%s", resp_uri.PTZConfiguration->DefaultAbsoluteZoomPositionSpace);
		}
		if (resp_uri.PTZConfiguration->DefaultRelativePanTiltTranslationSpace) {
			ONVIF_TRACE("\tdefault relative pantilt space :%s", resp_uri.PTZConfiguration->DefaultRelativePanTiltTranslationSpace);
		}
		if (resp_uri.PTZConfiguration->DefaultRelativeZoomTranslationSpace) {
			ONVIF_TRACE("\tdefault relative zoom space :%s", resp_uri.PTZConfiguration->DefaultRelativeZoomTranslationSpace);
		}
		if (resp_uri.PTZConfiguration->DefaultContinuousPanTiltVelocitySpace) {
			ONVIF_TRACE("\tdefault continue pantilt space :%s", resp_uri.PTZConfiguration->DefaultContinuousPanTiltVelocitySpace);
		}
		if (resp_uri.PTZConfiguration->DefaultContinuousZoomVelocitySpace) {
			ONVIF_TRACE("\tdefault continue zoom space :%s", resp_uri.PTZConfiguration->DefaultContinuousZoomVelocitySpace);
		}
		if (resp_uri.PTZConfiguration->DefaultPTZSpeed &&
			resp_uri.PTZConfiguration->DefaultPTZSpeed->PanTilt &&
			resp_uri.PTZConfiguration->DefaultPTZSpeed->Zoom) {
			ONVIF_TRACE("\tdefault ptz speed (%f,%f,%f)", resp_uri.PTZConfiguration->DefaultPTZSpeed->PanTilt->x,
				resp_uri.PTZConfiguration->DefaultPTZSpeed->PanTilt->y,
				resp_uri.PTZConfiguration->DefaultPTZSpeed->Zoom->x);
		}
		if (resp_uri.PTZConfiguration->PanTiltLimits) {
			ONVIF_TRACE("\tpantilt limit (%f,%f/%f,%f)", 
				resp_uri.PTZConfiguration->PanTiltLimits->Range->XRange->Min,
				resp_uri.PTZConfiguration->PanTiltLimits->Range->XRange->Max,
				resp_uri.PTZConfiguration->PanTiltLimits->Range->YRange->Min,
				resp_uri.PTZConfiguration->PanTiltLimits->Range->YRange->Max);
		}
		if (resp_uri.PTZConfiguration->ZoomLimits) {
			ONVIF_TRACE("\tzoom limit (%f,%f)", 
				resp_uri.PTZConfiguration->ZoomLimits->Range->XRange->Min,
				resp_uri.PTZConfiguration->ZoomLimits->Range->XRange->Max);
		}
			

		SOAP_destroy(soap);
		return 0;
	}

	ONVIF_TRACE("soap_call___tptz__GetConfiguration failed");
	return ret;
}


int onvif_get_presets(lpNVP_ARGS args, char *token, lpONVIF_PRESETS presets)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tptz__GetPresets req_uri;
	struct _tptz__GetPresetsResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.ProfileToken = token;
	memset(presets, 0, sizeof(stONVIF_PRESETS));

	if((ret = onvif_request(args, &soap, soap_call___tptz__GetPresets, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		int i;
		presets->nr = resp_uri.__sizePreset;
		ONVIF_TRACE("preset count: %d", presets->nr);
		for ( i = 0;  i < resp_uri.__sizePreset && i <  MAX_ONVIF_PRESET; i++){
			presets->entry[i].index = i;
			if(resp_uri.Preset[i].Name){
				strncpy(presets->entry[i].name, resp_uri.Preset[i].Name, sizeof(presets->entry[0].name));
			}
			if(resp_uri.Preset[i].token){
				strncpy(presets->entry[i].token, resp_uri.Preset[i].token, sizeof(presets->entry[0].token));
			}
			ONVIF_TRACE("preset %3d name:%s token:%s", i, presets->entry[i].name, presets->entry[i].token);
		}
		// try to sort preset by preset-token		
		ONVIF_TRACE("preset-sorted count: %d", presets->nr);
		qsort(presets->entry, presets->nr, sizeof(presets->entry[0]), preset_compare_by_token);
		for (i = 0; i < presets->nr; i++) {
			ONVIF_TRACE("preset %3d name:%s token:%s", i, presets->entry[i].name, presets->entry[i].token);
		}
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;
}

int ONVIF_set_preset(lpNVP_ARGS args, char *token, char *preset_token, char *preset_name)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tptz__SetPreset req_uri;
	struct _tptz__SetPresetResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.ProfileToken = token;
	req_uri.PresetName = preset_name;
	req_uri.PresetToken = preset_token;

	if((ret = onvif_request(args, &soap, soap_call___tptz__SetPreset, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		ONVIF_TRACE("%s %s success done!", __FUNCTION__, preset_token ? preset_token : "null");
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;	
}

int ONVIF_goto_preset(lpNVP_ARGS args, char *token, char *preset_token, float speed)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct tt__Vector2D pantilt_speed;
	struct tt__Vector1D zoom_speed;
	struct tt__PTZSpeed ptz_speed;
	struct _tptz__GotoPreset req_uri;
	struct _tptz__GotoPresetResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.ProfileToken = token;
	req_uri.PresetToken = preset_token;
	pantilt_speed.x = speed;
	pantilt_speed.y = speed;
	pantilt_speed.space = NULL;
	zoom_speed.x = speed;
	zoom_speed.space = NULL;
	ptz_speed.PanTilt = &pantilt_speed;
	ptz_speed.Zoom = &zoom_speed;
	req_uri.Speed = & ptz_speed;

	if((ret = onvif_request(args, &soap, soap_call___tptz__GotoPreset, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		//ONVIF_TRACE("%s %s success done!", __FUNCTION__, preset_token);
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;	
}

int ONVIF_remove_preset(lpNVP_ARGS args, char *token, char *preset_token)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tptz__RemovePreset req_uri;
	struct _tptz__RemovePresetResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	req_uri.ProfileToken = token;
	req_uri.PresetToken = preset_token;

	if((ret = onvif_request(args, &soap, soap_call___tptz__RemovePreset, ONVIF_PTZ,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		//ONVIF_TRACE("%s %s success done!", __FUNCTION__, preset_token);
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;	
}

int ONVIF_get_image_options(lpNVP_ARGS args, lpNVP_IMAGE_OPTIONS option)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _timg__GetOptions req_uri;
	struct _timg__GetOptionsResponse resp_uri;
	stNVP_PROFILE profiles;
	lpNVP_INTERFACE iface = (lpNVP_INTERFACE)args->thiz;
	lpONVIF_PRIV_CONTEXT context = (lpONVIF_PRIV_CONTEXT)(iface ? iface->private_ctx : NULL);

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	if (option->srcToken[0] == 0) {
		if (context == NULL) {
			ONVIF_INFO("invalid paramter");
			return -1;
		}
		if (context->profiles) {
			req_uri.VideoSourceToken = context->profiles[args->chn].v_source.token;
		} else {
			memset(&profiles, 0, sizeof(profiles));
			if (onvif_get_video_source(args, &profiles) < 0) {
				return -1;
			} 
			if (args->chn < profiles.chn) {
				strcpy(option->srcToken, profiles.profile[args->chn].v_source.token);
				req_uri.VideoSourceToken = profiles.profile[args->chn].v_source.token;
			} else {
				ONVIF_INFO("exceed channel available (%d/%d)", args->chn, profiles.chn);
				return -1;
			}
		}
	} else {
		req_uri.VideoSourceToken = option->srcToken;
	}
	
	if((ret = onvif_request(args, &soap, soap_call___timg__GetOptions, ONVIF_IMAGING,
		&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		if (resp_uri.ImagingOptions == NULL) {
			SOAP_destroy(soap);
			return -1;
		}
		if (resp_uri.ImagingOptions->Brightness) {
			option->brightness.min = resp_uri.ImagingOptions->Brightness->Min;
			option->brightness.max= resp_uri.ImagingOptions->Brightness->Max;
		}
		if (resp_uri.ImagingOptions->Contrast) {
			option->contrast.min = resp_uri.ImagingOptions->Contrast->Min;
			option->contrast.max= resp_uri.ImagingOptions->Contrast->Max;
		}
		if (resp_uri.ImagingOptions->ColorSaturation) {
			option->saturation.min = resp_uri.ImagingOptions->ColorSaturation->Min;
			option->saturation.max= resp_uri.ImagingOptions->ColorSaturation->Max;
		}
		if (resp_uri.ImagingOptions->Sharpness) {
			option->sharpness.min = resp_uri.ImagingOptions->Sharpness->Min;
			option->sharpness.max= resp_uri.ImagingOptions->Sharpness->Max;
		}

		ONVIF_TRACE("hue:[%f, %f] brightness:[%f, %f] contrast:[%f, %f] saturation:[%f, %f] sharpness:[%f, %f]",
			option->hue.min, option->hue.max,
			option->brightness.min, option->brightness.max,
			option->contrast.min, option->brightness.max,
			option->saturation.min, option->saturation.max,
			option->sharpness.min, option->sharpness.max);
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;
}


static int onvif_get_image(lpNVP_ARGS args, lpNVP_IMAGE_CONFIG image)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _timg__GetImagingSettings req_uri;
	struct _timg__GetImagingSettingsResponse resp_uri;
	stNVP_PROFILE profiles;
	lpNVP_INTERFACE iface = (lpNVP_INTERFACE)args->thiz;
	lpONVIF_PRIV_CONTEXT context = (lpONVIF_PRIV_CONTEXT)(iface ? iface->private_ctx : NULL);

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	if (image->src_token[0] == 0) {
		if (context == NULL) {
			ONVIF_INFO("invalid paramter");
			return -1;
		}
		if (context->profiles) {
			req_uri.VideoSourceToken = context->profiles[args->chn].v_source.token;
		} else {
			memset(&profiles, 0, sizeof(profiles));
			if ((ret = onvif_get_video_source(args, &profiles)) < 0) {
				return ret;
			} 
			if (args->chn < profiles.chn) {
				strcpy(image->src_token, profiles.profile[args->chn].v_source.token);
				req_uri.VideoSourceToken = profiles.profile[args->chn].v_source.token;
			} else {
				ONVIF_INFO("exceed channel available (%d/%d)", args->chn, profiles.chn);
				return -1;
			}
		}
	} else {
		req_uri.VideoSourceToken = image->src_token;
	}
	if((ret = onvif_request(args, &soap, soap_call___timg__GetImagingSettings, ONVIF_IMAGING,
		&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		if (resp_uri.ImagingSettings == NULL) {
			SOAP_destroy(soap);
			return -1;
		}

		if (resp_uri.ImagingSettings->Brightness) 
			image->color.brightness = *resp_uri.ImagingSettings->Brightness;
		if (resp_uri.ImagingSettings->Contrast) 
			image->color.contrast = *resp_uri.ImagingSettings->Contrast;
		if (resp_uri.ImagingSettings->ColorSaturation) 
			image->color.saturation = *resp_uri.ImagingSettings->ColorSaturation;
		if (resp_uri.ImagingSettings->Sharpness) 
			image->color.sharpness= *resp_uri.ImagingSettings->Sharpness;
		image->color.hue= 0;
		ONVIF_TRACE("hue:%f brightness:%f contrast:%f saturation:%f sharpness:%f", image->color.hue,
			image->color.brightness, image->color.contrast, image->color.saturation, image->color.sharpness);
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;
}

static int ONVIF_get_color(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color)
{
	int ret = -1;
	stNVP_IMAGE_OPTIONS option;
	stNVP_IMAGE_CONFIG image;

	memset(&option, 0, sizeof(option));	
	memset(&image, 0, sizeof(image));
	strcpy(option.srcToken, color->src_token);
	if ((ret = ONVIF_get_image_options(args, &option)) < 0){
		ONVIF_INFO("get image options failed");
		return ret;
	} else {
		sleep_c(100);
		strcpy(image.src_token, option.srcToken);
		strcpy(color->src_token, option.srcToken);
		if ((ret = onvif_get_image(args, &image)) < 0) {
			return ret;
		} else {
			color->brightness =(image.color.brightness -option.brightness.min) / (option.brightness.max - option.brightness.min);
			color->saturation =(image.color.saturation  - option.saturation.min) / (option.saturation.max - option.saturation.min);
			color->contrast =(image.color.contrast - option.contrast.min) / (option.contrast.max - option.contrast.min);
			color->sharpness =(image.color.sharpness - option.sharpness.min) / (option.sharpness.max - option.sharpness.min);
			color->hue =(image.color.hue - option.hue.min )/ (option.hue.max - option.hue.min);
			return 0;
		}
	}
}

static int onvif_set_image(lpNVP_ARGS args, lpNVP_IMAGE_CONFIG image)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct tt__ImagingSettings20 imgset;
	struct _timg__SetImagingSettings req_uri;
	struct _timg__SetImagingSettingsResponse resp_uri;
	stNVP_PROFILE profiles;
	lpNVP_INTERFACE iface = (lpNVP_INTERFACE)args->thiz;
	lpONVIF_PRIV_CONTEXT context = (lpONVIF_PRIV_CONTEXT)(iface ? iface->private_ctx : NULL);

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	memset(&imgset, 0, sizeof(imgset));
	if (image->src_token[0] == 0) {
		if (context == NULL) {
			ONVIF_INFO("invalid paramter");
			return -1;
		}
		if (context->profiles) {
			req_uri.VideoSourceToken = context->profiles[args->chn].v_source.token;
		} else {
			memset(&profiles, 0, sizeof(profiles));
			if ((ret = onvif_get_video_source(args, &profiles)) < 0) {
				return ret;
			} 
			if (args->chn < profiles.chn) {
				ONVIF_TRACE("get image chn:%d src:%s", args->chn, profiles.profile[args->chn].v_source.token);
				strcpy(image->src_token, profiles.profile[args->chn].v_source.token);
				req_uri.VideoSourceToken = profiles.profile[args->chn].v_source.token;
			} else {
				ONVIF_INFO("exceed channel available (%d/%d)", args->chn, profiles.chn);
				return -1;
			}
		}
	} else {
		req_uri.VideoSourceToken = image->src_token;
	}
	if (image->color.brightness  >= 0) imgset.Brightness = &image->color.brightness;
	if (image->color.contrast >= 0) imgset.Contrast = &image->color.contrast;
	if (image->color.saturation >= 0) imgset.ColorSaturation = &image->color.saturation;
	if (image->color.sharpness >= 0) imgset.Sharpness= &image->color.sharpness;
	req_uri.ImagingSettings = &imgset;
	req_uri.ForcePersistence = &m_NTRUE;

	ONVIF_TRACE("Set Image2(%s) hue:%f brightness:%f contrast:%f saturation:%f sharpness:%f",
		req_uri.VideoSourceToken, image->color.hue,
		image->color.brightness, image->color.contrast, image->color.saturation, image->color.sharpness);	
	
	if(( ret = onvif_request(args, &soap, soap_call___timg__SetImagingSettings, ONVIF_IMAGING,
		&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		SOAP_destroy(soap);
		return 0;
	}
	
	return ret;
}


static int ONVIF_set_image(lpNVP_ARGS args, lpNVP_IMAGE_CONFIG image)
{
	int ret = -1;
	stNVP_IMAGE_OPTIONS option;

	memset(&option, 0, sizeof(option));
	strcpy(option.srcToken, image->src_token);
	if (( ret = ONVIF_get_image_options(args, &option)) < 0){
		// nothing to do, we assume destination range is [0, 1.0]
		ONVIF_INFO("get image options failed");
		return ret;
	} else {
		ONVIF_TRACE("Set Image1 hue:%f brightness:%f contrast:%f saturation:%f sharpness:%f", image->color.hue,
			image->color.brightness, image->color.contrast, image->color.saturation, image->color.sharpness);	
		if (image->color.brightness >= 0)
			image->color.brightness =image->color.brightness * (option.brightness.max - option.brightness.min) + option.brightness.min;
		if (image->color.saturation >= 0)
			image->color.saturation =image->color.saturation * (option.saturation.max - option.saturation.min) + option.saturation.min;
		if (image->color.contrast >= 0)
			image->color.contrast =image->color.contrast * (option.contrast.max - option.contrast.min) + option.contrast.min;
		if (image->color.sharpness >= 0)
			image->color.sharpness =image->color.sharpness * (option.sharpness.max - option.sharpness.min) + option.sharpness.min;
		if (image->color.hue >= 0)
			image->color.hue =image->color.hue * (option.hue.max - option.hue.min) + option.hue.min;
	}
	sleep_c(100);
	strcpy(image->src_token, option.srcToken);
	strcpy(image->color.src_token, option.srcToken);
	return onvif_set_image(args, image);
}

static int ONVIF_set_color(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color)
{
	int ret = -1;
	stNVP_IMAGE_OPTIONS option;
	stNVP_IMAGE_CONFIG image;

	memset(&image, 0, sizeof(image));
	memset(&option, 0, sizeof(option));
	strcpy(option.srcToken, color->src_token);
	if ((ret = ONVIF_get_image_options(args, &option)) < 0){
		// nothing to do, we assume destination range is [0, 1.0]
		return ret;
	} else {
		ONVIF_TRACE("Set Image1 hue:%f brightness:%f contrast:%f saturation:%f sharpness:%f", image.color.hue,
			color->brightness, color->contrast, color->saturation, color->sharpness);	
		if (color->brightness >= 0)
			image.color.brightness =color->brightness * (option.brightness.max - option.brightness.min) + option.brightness.min;
		if (color->saturation >= 0)
			image.color.saturation =color->saturation * (option.saturation.max - option.saturation.min) + option.saturation.min;
		if (color->contrast >= 0)
			image.color.contrast =color->contrast * (option.contrast.max - option.contrast.min) + option.contrast.min;
		if (color->sharpness >= 0)
			image.color.sharpness =color->sharpness * (option.sharpness.max - option.sharpness.min) + option.sharpness.min;
		if (color->hue >= 0)
			image.color.hue =color->hue * (option.hue.max - option.hue.min) + option.hue.min;
	}
	sleep_c(100);
	strcpy(color->src_token, option.srcToken);
	strcpy(image.src_token, option.srcToken);

	return onvif_set_image(args, &image);
}


int ONVIF_set_hue(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color)
{
	color->brightness = -1;
	color->contrast = -1;
	color->saturation = -1;
	color->sharpness = -1;
	return ONVIF_set_color(args, color);
}

int ONVIF_set_brightness(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color)
{
	color->contrast = -1;
	color->saturation = -1;
	color->sharpness = -1;
	color->hue = -1;
	return ONVIF_set_color(args, color);
}

int ONVIF_set_contrast(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color)
{
	color->brightness = -1;
	color->saturation = -1;
	color->sharpness = -1;
	color->hue = -1;
	return ONVIF_set_color(args, color);
}

int ONVIF_set_sharpness(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color)
{
	color->brightness = -1;
	color->contrast = -1;
	color->saturation = -1;
	color->hue = -1;
	return ONVIF_set_color(args, color);
}

int ONVIF_set_saturation(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color)
{
	color->brightness = -1;
	color->contrast = -1;
	color->sharpness = -1;
	color->hue = -1;
	return ONVIF_set_color(args, color);
}

int ONVIF_get_input_config(lpNVP_ARGS args, lpNVP_INPUT_CONFIG input) { return -1; }
int ONVIF_set_input_config(lpNVP_ARGS args, lpNVP_INPUT_CONFIG input) { return -1; }

int ONVIF_get_md_config(lpNVP_ARGS args, lpNVP_MD_CONFIG md) { return -1; }
int ONVIF_set_md_config(lpNVP_ARGS args, lpNVP_MD_CONFIG md) { return -1; }

int ONVIF_set_title_overlay(lpNVP_ARGS args, lpNVP_TITLE_OVERLAY overlay) { return -1; }
int ONVIF_get_title_overlay(lpNVP_ARGS args, lpNVP_TITLE_OVERLAY overlay) { return -1; }
int ONVIF_set_time_overlay(lpNVP_ARGS args, lpNVP_TIME_OVERLAY overlay) { return -1; }
int ONVIF_get_time_overlay(lpNVP_ARGS args, lpNVP_TIME_OVERLAY overlay) { return -1; }

static int onvif_create_pullpoint_subscription
	(lpNVP_ARGS args, char *topic, char *messagecontent, char *address)
{
#define TOPIC_DIALECT		"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet"
#define MESSAGE_DIALECT	"http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter"
	const char *sz_filter = "<wsnt:TopicExpression Dialect=\"%s\">%s</wsnt:TopicExpression>"\
		"<wsnt:MessageContent Dialect=\"%s\">%s</wsnt:MessageContent>";
	char sz_buf[1024];
	int ret = -1;
	char *xml_array[1] = { sz_buf, };
	char terminationtime[64];
	struct soap *soap = NULL;
	struct wsnt__FilterType Filter;
	//struct _tev__CreatePullPointSubscription_SubscriptionPolicy SubscriptionPolicy;
	struct _tev__CreatePullPointSubscription req_uri;
	struct _tev__CreatePullPointSubscriptionResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	address[0] = 0;
	if(topic || messagecontent){
		SNPRINTF(sz_buf, sizeof(sz_buf),  sz_filter, TOPIC_DIALECT, topic, MESSAGE_DIALECT, messagecontent);
		Filter.__any = xml_array;
		Filter.__size = 1 ;
		req_uri.Filter = &Filter;
	}else{
		req_uri.Filter = NULL;
	}
	req_uri.SubscriptionPolicy = NULL;
	if(BASE_NOTIFICATION_TIMEOUT >= 3600){
		SNPRINTF(terminationtime, sizeof(terminationtime), "PT%dH%dM%dS", BASE_NOTIFICATION_TIMEOUT/3600,
			(BASE_NOTIFICATION_TIMEOUT%3600)/60, BASE_NOTIFICATION_TIMEOUT % 60);
	}else if(BASE_NOTIFICATION_TIMEOUT >= 60){
		SNPRINTF(terminationtime, sizeof(terminationtime), "PT%dM%dS", BASE_NOTIFICATION_TIMEOUT/60, BASE_NOTIFICATION_TIMEOUT % 60);
	}else{
		SNPRINTF(terminationtime, sizeof(terminationtime), "PT%dS", BASE_NOTIFICATION_TIMEOUT);
	}
	req_uri.InitialTerminationTime = terminationtime;
	req_uri.__any = NULL;
	req_uri.__size = 0;
	//soap_element_begin_in(struct soap * soap,const char * tag,int nillable,const char * type)
	if(( ret = onvif_request(args, &soap, soap_call___tev__CreatePullPointSubscription, ONVIF_EVENTS,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		ONVIF_TRACE("wsnt__CurrentTime:%s wsnt__TerminationTime:%s\n", ctime(&resp_uri.wsnt__CurrentTime), ctime(&resp_uri.wsnt__TerminationTime));
		if ( resp_uri.SubscriptionReference.Address){
			ONVIF_TRACE("event pullpoint address:%s\n", resp_uri.SubscriptionReference.Address);
			strcpy(address, resp_uri.SubscriptionReference.Address);

		}else {
			ONVIF_INFO("event pullpoint without vaild address!");
			return -1;
		}
			
		SOAP_destroy(soap);
		return 0;
	}

	return ret;	
}

static int onvif_subscribe(lpNVP_ARGS args, char *local_reference, char *topic, char *messagecontent, lpNVP_SUBSCRIBE subs)
{
#define TOPIC_DIALECT		"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet"
#define MESSAGE_DIALECT	"http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter"
	const char *sz_filter = "<wsnt:TopicExpression Dialect=\"%s\">%s</wsnt:TopicExpression>"\
		"<wsnt:MessageContent Dialect=\"%s\">%s</wsnt:MessageContent>";
	char sz_buf[1024];
	int ret = -1;
	char *xml_array[1] = { sz_buf, };
	char sztimeout[32];
	
	//char local_reference[128];
	struct soap *soap = NULL;
	struct wsa5__EndpointReferenceType ConsumerReference;
	struct wsnt__FilterType Filter;
	struct _wsnt__Subscribe req_uri;
	struct _wsnt__SubscribeResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	subs->reference[0] = 0;
	if(topic || messagecontent){
		SNPRINTF(sz_buf, sizeof(sz_buf),  sz_filter, TOPIC_DIALECT, topic, MESSAGE_DIALECT, messagecontent);
		Filter.__any = xml_array;
		Filter.__size = 1 ;
		req_uri.Filter = &Filter;
	}else{
		req_uri.Filter = NULL;
	}
	memset(&ConsumerReference, 0, sizeof(struct wsa5__EndpointReferenceType));
	ConsumerReference.Address = local_reference;
	sztime_from_int(ONVIF_EVENT_TIMEOUT, sztimeout);
	req_uri.ConsumerReference = ConsumerReference;
	req_uri.SubscriptionPolicy = NULL;
	req_uri.InitialTerminationTime = sztimeout;
	req_uri.__any = NULL;
	req_uri.__size = 0;
	//soap_element_begin_in(struct soap * soap,const char * tag,int nillable,const char * type)
	if((ret = onvif_request(args, &soap, soap_call___tev__Subscribe, ONVIF_EVENTS,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{
		if(resp_uri.CurrentTime && resp_uri.TerminationTime){
			ONVIF_INFO("CurrentTime:%s \tTerminationTime:%s", ctime(resp_uri.CurrentTime), ctime(resp_uri.TerminationTime));
		}
		if ( resp_uri.SubscriptionReference.Address){
			ONVIF_INFO("event subscribe address:%s", resp_uri.SubscriptionReference.Address);
			if (http_parse_url(NULL, NULL, NULL, resp_uri.SubscriptionReference.Address) < 0) {
				ONVIF_INFO("event subscribe without vaild reference!");
				return NVP_RET_SERVER_NOT_SUPPORT;
			} else {
				strcpy(subs->reference, resp_uri.SubscriptionReference.Address);
			}

		} else {
			ONVIF_INFO("event subscribe without vaild address!");
			return NVP_RET_SERVER_NOT_SUPPORT;
		}
		SOAP_destroy(soap);
		return 0;
	}

	return ret;	
}



static int onvif_get_event_properties(lpNVP_ARGS args, char *source_token, char *analytics_token, char *topic)
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _tev__GetEventProperties req_uri;
	struct _tev__GetEventPropertiesResponse resp_uri;

	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));

	if((ret = onvif_request(args, &soap, soap_call___tev__GetEventProperties, ONVIF_EVENTS,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{


		SOAP_destroy(soap);
		return 0;
	}

	return ret;	
}


int ONVIF_subscribe_md_event(lpNVP_ARGS args, lpNVP_SUBSCRIBE subs) 
{ 
#define ONVIF_MD_TOPIC		"tns1:RuleEngine/CellMotionDetector/Motion"
	const char *sz_messagecontent =
		"boolean(//tt:SimpleItem[@Name=\"VideoSourceConfigurationToken\" and @Value=\"%s\"])"\
		" and "\
		"boolean(//tt:SimpleItem[@Name=\"VideoAnalyticsConfigurationToken\" and @Value=\"%s\"])"  ;
	char sz_buf[1024];
	int ret = -1;
	stNVP_PROFILE_CHN profile;
	char local_reference[128];
	lpONVIF_C_CONTEXT onvif = g_OnvifClientCxt;
	lpNVP_INTERFACE iface = (lpNVP_INTERFACE)args->thiz;
	lpONVIF_PRIV_CONTEXT context = (lpONVIF_PRIV_CONTEXT)(iface ? iface->private_ctx : NULL);

	memset(&profile, 0, sizeof(profile));
	if( (ret = ONVIF_get_profiles(args, &profile)) < 0){
		return ret;
	}
	if(0 == strlen(profile.van.token)){
		ONVIF_INFO("ONVIF subscribe MD to %s Failed, because not support analytics!", args->ip);
		return -1;
	}
	SNPRINTF(sz_buf, sizeof(sz_buf), sz_messagecontent , profile.v_source.token , 
		profile.van.token);
	
	SNPRINTF(local_reference, sizeof(local_reference), "http://%s:%d%s_c/%ld_%ld",
		onvif->event_bindip, onvif->event_port, ONVIF_EVENT_PULLPOINT_URI_PREFIX, (long)time(NULL), RANDOM());
	if((ret = onvif_subscribe(args, local_reference, NULL, NULL, subs)) < 0)
	//if(onvif_create_pullpoint_subscription(args, ONVIF_MD_TOPIC, sz_buf, address) < 0)
	{
		ONVIF_INFO("ONVIF subscribe MD to %s Failed!", args->ip);
		return ret;
	}

	//subscribe success, insert a new item to subscibe-list
	if ((ret = ONVIF_C_event_subscribe(0, subs->reference, local_reference, ONVIF_EVENT_TIMEOUT,
		args->username, args->password,
		subs->hook, subs->hook_custom, context)) < 0) {
		ONVIF_INFO("ONVIF subscribe MD to %s Failed2!", args->ip);
		return ret;
	}
	
	return 0; 
}

int ONVIF_subscribe_event(lpNVP_ARGS args, lpNVP_SUBSCRIBE subs) 
{ 
	int ret = -1;
	char local_reference[128];
	lpONVIF_C_CONTEXT onvif = g_OnvifClientCxt;
	lpNVP_INTERFACE iface = (lpNVP_INTERFACE)args->thiz;
	lpONVIF_PRIV_CONTEXT context = (lpONVIF_PRIV_CONTEXT)(iface ? iface->private_ctx : NULL);

	SNPRINTF(local_reference, sizeof(local_reference), "http://%s:%d%s_c/%ld_%ld",
		onvif->event_bindip, onvif->event_port, ONVIF_EVENT_PULLPOINT_URI_PREFIX, (long)time(NULL), (long)(RANDOM() % 100));
	printf("localref:%s.\n", local_reference);
	if( (ret = onvif_subscribe(args, local_reference, NULL, NULL, subs)) < 0)
	{
		ONVIF_INFO("ONVIF subscribe All events to %s Failed!", args->ip);
		return ret;
	}

	//subscribe success, insert a new item to subscibe-list
	if (ONVIF_C_event_subscribe(0, subs->reference, local_reference, ONVIF_EVENT_TIMEOUT,
		args->username, args->password,
		subs->hook, subs->hook_custom, context) < 0) {
		ONVIF_INFO("ONVIF subscribe MD to %s Failed2!", args->ip);
		return -1;
	}
	
	return 0; 
}


int ONVIF_renew_event_ex(lpNVP_ARGS args, lpNVP_SUBSCRIBE subs) 
{
	int ret = -1;
	char terminationtime[64];
	struct soap *soap = NULL;
	struct _wsnt__Renew req_uri;
	struct _wsnt__RenewResponse resp_uri;
	
	// check reference
	if ( strlen(subs->reference) == 0 ){
		ONVIF_INFO("Renew withou valid peer reference");
		return -1;
	}
	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));
	if(subs->keeplive_time <= 5){
		subs->keeplive_time = BASE_NOTIFICATION_TIMEOUT;
	}

	sztime_from_int(subs->keeplive_time, terminationtime);
	req_uri.TerminationTime = terminationtime;
	if(( ret = onvif_request(args, &soap, soap_call___tev__Renew, (unsigned int)subs->reference,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{		
		ONVIF_TRACE("CurrentTime:%s",resp_uri.CurrentTime ? ctime(resp_uri.CurrentTime) : "");
		ONVIF_INFO(" <<RENEW : %s>> TerminationTime:%s ", args->ip, ctime(&resp_uri.TerminationTime));
		SOAP_destroy(soap);
		return 0;
	}

	return ret;	
}


int ONVIF_cancel_event_ex(lpNVP_ARGS args, lpNVP_SUBSCRIBE subs) 
{
	int ret = -1;
	struct soap *soap = NULL;
	struct _wsnt__Unsubscribe req_uri;
	struct _wsnt__UnsubscribeResponse resp_uri;
	
	// check reference
	if ( strlen(subs->reference) == 0 ){
		ONVIF_INFO("Cancel withou valid peer(%s) reference ", args->ip);
		return -1;
	}
	memset(&req_uri,0,sizeof(req_uri));
	memset(&resp_uri,0,sizeof(resp_uri));

	if((ret = onvif_request(args, &soap, soap_call___tev__Unsubscribe, (unsigned int)subs->reference,
		(void *)&req_uri, sizeof(req_uri), (void *)&resp_uri, sizeof(resp_uri))) == 0)
	{		
		ONVIF_INFO("Cancel reference: %s", subs->reference);
		SOAP_destroy(soap);
	} else {	
		ONVIF_INFO("Cancel reference: %s FAILED.", subs->reference);
	}

	ONVIF_C_event_unsubscribe(subs->reference);
	return ret;
}


int ONVIF_get_event_status(lpNVP_ARGS args, lpNVP_EVENT event ) { return -1; }

int ONVIF_control_ptz(lpNVP_ARGS args, lpNVP_PTZ_CONTROL ptz)
{
	int ret = -1;
	int p1_index = -1;
	int i_profile = 0;
	stNVP_PROFILE_CHN profile;
	stPTZ_CONF ptz_conf;
	ONVIF_INFO("onvif ptz cmd: %d index:%d ", ptz->cmd, ptz->index);

	memset(&profile, 0, sizeof(profile));
	memset(&ptz_conf, 0, sizeof(ptz_conf));
	if( ( ret = ONVIF_get_profiles(args, &profile)) < 0){
		return ret;
	}
	// check speed value
	if(ptz->speed > 1.0) ptz->speed = 1.0;
	else if(ptz->speed <= 0) ptz->speed = 0.5;

	switch(ptz->cmd)
	{
		case NVP_PTZ_CMD_AUTOPAN:
		case NVP_PTZ_CMD_UP:
		case NVP_PTZ_CMD_DOWN:
		case NVP_PTZ_CMD_LEFT:
		case NVP_PTZ_CMD_RIGHT:
		case NVP_PTZ_CMD_ZOOM_IN:
		case NVP_PTZ_CMD_ZOOM_OUT:
		{
			if((ret = onvif_continueus_move(args, profile.venc[i_profile].token, ptz->cmd, ptz->speed)) < 0){
				return ret;
			}
			break;
		}
		case NVP_PTZ_CMD_SET_PRESET:
		{
			char *token  = NULL, *name = NULL;
			char format[64];
			char sztoken[64];
			char xmtoken[64];
			stONVIF_PRESETS  presets;
			if ((ret = onvif_get_ptz_configuration(args, profile.ptz.token, &ptz_conf)) < 0) 
				return ret;
			if( (ret = onvif_get_ptz_node(args, ptz_conf.node_token, &ptz_conf)) < 0) {
				return ret;
			}
			if( (ret = onvif_get_presets(args, profile.venc[i_profile].token, &presets)) < 0){
				return ret;
			}
			if ((presets.nr == ptz_conf.preset_number)&&(strcmp(presets.entry[ptz_conf.preset_number-1].token, presets.entry[ptz_conf.preset_number-2].token)!=0)) {
				if (get_int_string_format(presets.entry[0].token, format) == 1) {
					if ((sscanf(presets.entry[0].token, format, &p1_index) == 1)
						&& (p1_index == 1)) {
						ptz->index--;
					}
				}
				if( ptz->index < ptz_conf.preset_number){
					token = presets.entry[ptz->index].token;
				} else {
					ONVIF_INFO("preset nr:%d , but request index:%d", presets.nr, ptz->index);
					return -1;
				}
			} else {
				if( ptz->index < ptz_conf.preset_number){
					if (get_int_string_format(presets.entry[0].token, format) == 1) {
						sprintf(sztoken, format, ptz->index);
					} else {
						sprintf(sztoken, "%s", presets.entry[ptz->index].token);
					}
					token = sztoken;
					if (token[0] == 0)
						token = NULL;
				} else {
					ONVIF_INFO("preset nr:%d , but request index:%d", presets.nr, ptz->index);
					return -1;
				}
			}
			if(( ret = ONVIF_set_preset(args, profile.venc[i_profile].token, token, name)) < 0){
				return ret;
			}
			break;
		}
		case NVP_PTZ_CMD_GOTO_PRESET:
		{
			char *token  = NULL;
			char format[64];
			char sztoken[64];
			stONVIF_PRESETS  presets;
			if (onvif_get_ptz_configuration(args, profile.ptz.token, &ptz_conf) < 0) 
				return -1;
			if( onvif_get_ptz_node(args, ptz_conf.node_token, &ptz_conf) < 0) {
				return -1;
			}
			if( onvif_get_presets(args, profile.venc[i_profile].token, &presets) < 0){
				return -1;
			}
			if (presets.nr == ptz_conf.preset_number) {
				if (get_int_string_format(presets.entry[0].token, format) == 1) {
					if ((sscanf(presets.entry[0].token, format, &p1_index) == 1)
						&& (p1_index == 1)) {
						ptz->index--;
					}
				}
				if( ptz->index < ptz_conf.preset_number){
					token = presets.entry[ptz->index].token;
				} else {
					ONVIF_INFO("preset nr:%d , but request index:%d", presets.nr, ptz->index);
					return -1;
				}
			} else {
				if( ptz->index < ptz_conf.preset_number){
					if (get_int_string_format(presets.entry[0].token, format) == 1) {
						sprintf(sztoken, format, ptz->index);
					} else {
						sprintf(sztoken, "%s", presets.entry[ptz->index].token);
					}
					token = sztoken;
					if (sztoken[0] == 0) {
						ONVIF_INFO("preset nr:%d , but request index:%d", presets.nr, ptz->index);
						return -1;
					}
				} else {
					ONVIF_INFO("preset nr:%d , but request index:%d", presets.nr, ptz->index);
					return -1;
				}
			}

			if(ONVIF_goto_preset(args, profile.venc[i_profile].token, token, ptz->speed) < 0){
				return -1;
			}
			break;
		}
		case NVP_PTZ_CMD_CLEAR_PRESET:
		{
			char *token  = NULL;
			char format[64];
			char sztoken[64];
			stONVIF_PRESETS  presets;
			if (onvif_get_ptz_configuration(args, profile.ptz.token, &ptz_conf) < 0) 
				return -1;
			if( onvif_get_ptz_node(args, ptz_conf.node_token, &ptz_conf) < 0) {
				return -1;
			}
			if( onvif_get_presets(args, profile.venc[i_profile].token, &presets) < 0){
				return -1;
			}
			if (presets.nr == ptz_conf.preset_number) {
				if (get_int_string_format(presets.entry[0].token, format) == 1) {
					if ((sscanf(presets.entry[0].token, format, &p1_index) == 1)
						&& (p1_index == 1)) {
						ptz->index--;
					}
				}
				if( ptz->index < ptz_conf.preset_number){
					token = presets.entry[ptz->index].token;
				} else {
					ONVIF_INFO("preset nr:%d , but request index:%d", presets.nr, ptz->index);
					return -1;
				}
			} else {
				if( ptz->index < ptz_conf.preset_number){
					if (get_int_string_format(presets.entry[0].token, format) == 1) {
						sprintf(sztoken, format, ptz->index);
					} else {
						sprintf(sztoken, "%s", presets.entry[ptz->index].token);
					}
					token = sztoken;
					if (sztoken[0] == 0) {
						ONVIF_INFO("preset nr:%d , but request index:%d", presets.nr, ptz->index);
						return -1;
					}
				} else {
					ONVIF_INFO("preset nr:%d , but request index:%d", presets.nr, ptz->index);
					return -1;
				}
			}

			if(ONVIF_remove_preset(args, profile.venc[i_profile].token, token) < 0){
				return -1;
			}
			break;
		}		
		case NVP_PTZ_CMD_STOP:
		{
			if(onvif_stop(args, profile.venc[i_profile].token) < 0){
				return -1;
			}
			break;
		}
		default:
		{
			onvif_get_ptz_configurations(args);
			if(onvif_stop(args, profile.venc[i_profile].token) < 0){
				return -1;
			}
			break;
		}
	}

	return 0;
}

int ONVIF_get_jpeg_image(lpNVP_ARGS args, void *data ) { return -1; }

void ONVIF_context_init(lpONVIF_PRIV_CONTEXT context, lpNVP_ARGS args)
{
	if (context) {
		stNVP_PROFILE profiles;
		char server[NVP_MAX_URI_SIZE];
		
		memcpy(&context->args, args, sizeof(stNVP_ARGS));
		context->auth = false;
		sprintf(server, ONVIF_SERVER_URI, args->ip, args->port);
		
		if (onvif_get_xaddr(context, server, args->username, args->password, ONVIF_MODEULE_ALL, NULL) < 0) {
			return;
		}
		if (context->info == NULL) {
			context->info = calloc(1, sizeof(stNVP_DEV_INFO));
		}
		ONVIF_get_devinfo(args, context->info);
		onvif_get_video_source(args, &profiles);
		// FIX me
		context->chn = 1;
		context->profiles = calloc(context->chn , sizeof(stNVP_PROFILE_CHN));
		ONVIF_get_profiles(args, context->profiles);
	}
}

stNVP_INTERFACE gOnvifInterface =
{
	NULL,                            // 	void *private_ctx; 
	onvif_set_nvp_event_hook,        // 	int (*SetNVPEventHook)			(lpNVP_ARGS args, NVP_HOOK_EVENT event, fNVPEventHook hook, void *custom);
	NULL,                            // 	int (*GetSystemVersion)			(lpNVP_ARGS args, char *version);
	NULL,                            // 	int (*GetProtocalVersion)			(lpNVP_ARGS args, char *version);
	ONVIF_search,                    // 	int (*Search)();
	ONVIF_get_devinfo,               // 	int (*GetDeviceInfo)				(lpNVP_ARGS args, lpNVP_DEV_INFO info);
	ONVIF_get_rtsp_uri,              // 	int (*GetRtspUri)					(lpNVP_ARGS args, lpNVP_RTSP_STREAM rtsps);
	ONVIF_get_system_time,           // 	int (*GetSystemDateTime)			(lpNVP_ARGS args, lpNVP_SYS_TIME dt);
	ONVIF_set_system_time,           // 	int (*SetSystemDateTime)			(lpNVP_ARGS args, lpNVP_SYS_TIME dt /*if dt == NULL, use system local time */);
	ONVIF_get_venc_configs,          // 	int (*GetVideoEncoderConfigs)		(lpNVP_ARGS args, lpNVP_VENC_CONFIGS venc); // get all stream configurations
	ONVIF_get_venc_config,           // 	int (*GetVideoEncoderConfig)		(lpNVP_ARGS args, lpNVP_VENC_CONFIG venc);  // get one stream configuration
	ONVIF_set_venc_profile,          // 	int (*SetVideoEncoderConfig)		(lpNVP_ARGS args, lpNVP_VENC_CONFIG venc);
	ONVIF_get_venc_options,          // 	int (*GetVideoEncoderConfigOption)(lpNVP_ARGS args, lpNVP_VENC_OPTIONS option);
	ONVIF_get_color,                 // 	int (*GetColorConfig)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	ONVIF_set_color,                 // 	int (*SetColorConfig)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	ONVIF_set_hue,                   // 	int (*SetHue)						(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	ONVIF_set_sharpness,             // 	int (*SetSharpness)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	ONVIF_set_contrast,              // 	int (*SetContrast)					(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	ONVIF_set_brightness,            // 	int (*SetBrightness)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	ONVIF_set_saturation,            // 	int (*SetSaturation)				(lpNVP_ARGS args, lpNVP_COLOR_CONFIG color);
	ONVIF_get_network_interface,     // 	int (*SetNetworkInterface)		(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether);
	ONVIF_set_network_interface,     // 	int (*GetNetworkInterface)		(lpNVP_ARGS args, lpNVP_ETHER_CONFIG ether);
	ONVIF_get_title_overlay,         // 	int (*GetTitleOverlay)				(lpNVP_ARGS args, lpNVP_TITLE_OVERLAY overlay);
	ONVIF_get_time_overlay,// 	int (*GetDateTimeOverlay)			(lpNVP_ARGS args, lpNVP_TIME_OVERLAY overlay);
	ONVIF_set_title_overlay,// 	int (*SetTitleOverlay)				(lpNVP_ARGS args, lpNVP_TITLE_OVERLAY overlay);
	ONVIF_set_time_overlay,// 	int (*SetDateTimeOverlay)			(lpNVP_ARGS args, lpNVP_TIME_OVERLAY overlay);
	ONVIF_get_input_config,// 	int (*GetDigitalInputConfig)		(lpNVP_ARGS args, lpNVP_INPUT_CONFIG input);
	ONVIF_set_input_config,// 	int (*SetDigitalInputConfig)		(lpNVP_ARGS args, lpNVP_INPUT_CONFIG input);
	ONVIF_subscribe_md_event,// 	int (*SubscribeEvent)				(lpNVP_ARGS args, lpNVP_SUBSCRIBE subs);
	ONVIF_cancel_event_ex,// 	int (*CancelEvent)					(lpNVP_ARGS args, lpNVP_SUBSCRIBE subs);
	ONVIF_get_event_status,// 	int (*GetEventStatus)				(lpNVP_ARGS args, lpNVP_EVENT event);
	ONVIF_get_md_config,// 	int (*GetMDConfig)					(lpNVP_ARGS args, lpNVP_MD_CONFIG md);
	ONVIF_set_md_config,// 	int (*SetMDConfig)					(lpNVP_ARGS args, lpNVP_MD_CONFIG md);
	ONVIF_control_ptz,// 	int (*ControlPTZ)					(lpNVP_ARGS args, lpNVP_PTZ_CONTROL ptz);	
	ONVIF_reboot,// 	int (*Reboot)						(lpNVP_ARGS args, void *data);
	ONVIF_get_jpeg_image// 	int (*GetJpegImage)				(lpNVP_ARGS args, void *data);
};

lpNVP_INTERFACE NVP_ONVIF_new(lpNVP_ARGS args)
{
	lpNVP_INTERFACE iface=(lpNVP_INTERFACE)calloc(1, sizeof(stNVP_INTERFACE));
	lpONVIF_PRIV_CONTEXT context = NULL;
	assert(iface != NULL);

#ifndef WIN32
	memcpy(iface, &gNVPNULL, sizeof(stNVP_INTERFACE));
#endif // WIN32
	iface->private_ctx = (void *)calloc(1, sizeof(stONVIF_PRIV_CONTEXT));
	assert(iface->private_ctx != NULL);
	context = (lpONVIF_PRIV_CONTEXT)iface->private_ctx;
	context->auth = false;

	iface->SetNVPEventHook		= onvif_set_nvp_event_hook;
	
	iface->Search 				= ONVIF_search;
	iface->GetDeviceInfo			= ONVIF_get_devinfo;
	iface->GetRtspUri 			= ONVIF_get_rtsp_uri;
	iface->GetSystemDateTime		= ONVIF_get_system_time;
	iface->SetSystemDateTime		= ONVIF_set_system_time;
	iface->GetVideoEncoderConfigs = ONVIF_get_venc_configs;
	iface->GetVideoEncoderConfig	= ONVIF_get_venc_config;
	iface->SetVideoEncoderConfig	= ONVIF_set_venc_profile;
	iface->GetVideoEncoderConfigOption	= ONVIF_get_venc_options;	
	iface->GetColorConfig 		= ONVIF_get_color;
	iface->SetColorConfig 		= ONVIF_set_color;
	iface->SetHue 				= ONVIF_set_hue;
	iface->SetSharpness			= ONVIF_set_sharpness;
	iface->SetContrast			= ONVIF_set_contrast;
	iface->SetBrightness			= ONVIF_set_brightness;
	iface->SetSaturation			= ONVIF_set_saturation;
	iface->GetNetworkInterface	= ONVIF_get_network_interface;
	iface->SetNetworkInterface	= ONVIF_set_network_interface;

	iface->GetDigitalInputConfig	= ONVIF_get_input_config;
	iface->SetDigitalInputConfig	= ONVIF_set_input_config;

	iface->GetMDConfig			= ONVIF_get_md_config;
	iface->SetMDConfig			= ONVIF_set_md_config;

	iface->SetTitleOverlay		= ONVIF_set_title_overlay;
	iface->GetTitleOverlay		= ONVIF_get_title_overlay;
	iface->SetDateTimeOverlay 		= ONVIF_set_time_overlay;
	iface->GetDateTimeOverlay 		= ONVIF_get_time_overlay;

	iface->SubscribeEvent		= ONVIF_subscribe_event;
	iface->CancelEvent			= ONVIF_cancel_event_ex;
	iface->GetEventStatus			= ONVIF_get_event_status;

	iface->ControlPTZ 			= ONVIF_control_ptz;

	iface->Reboot 				= ONVIF_reboot;
	iface->GetJpegImage			= ONVIF_get_jpeg_image;

	ONVIF_INFO("%s!", __FUNCTION__);
	return iface;
}

void NVP_ONVIF_delete(lpNVP_INTERFACE iface)
{
	if(iface)
	{
		if(iface->private_ctx){
			lpONVIF_PRIV_CONTEXT priv = (lpONVIF_PRIV_CONTEXT)iface->private_ctx;
			if (priv->info) {
				free(priv->info);
				priv->info = NULL;
			}
			if (priv->profiles) {
				free(priv->profiles);
				priv->profiles = NULL;
			}
			//
			free(iface->private_ctx);
		}
		iface->private_ctx = NULL;
		free(iface);
	}

	ONVIF_INFO("%s :!", __FUNCTION__ );
}

#endif //end of ifdef soap_client
