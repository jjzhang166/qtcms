#ifndef __ONVIF_COMMON_H__
#define __ONVIF_COMMON_H__
#include "stdinc.h"
#include "cross.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "stdsoap2.h"
#include "nvp_define.h"

#define ONVIF_EVENT_STATE_INIT	0
#define ONVIF_EVENT_STATE_CHANGED	1
#define ONVIF_EVENT_STATE_DELETE		2

#define ONVIF_EVENT_MODE_NOTIFY	0
#define ONVIF_EVENT_MODE_PULL	1

typedef struct ONVIF_EVENT_SUBSCRIBE
{
	pthread_t pid;
	int sock;
	bool trigger;
	char local_reference[200];
	char peer_reference[200];
	char peer_addr[20];
	unsigned short peer_port;
	char username[32];
	char userpwd[32];
	unsigned short port;
	int event_type;
	int event_mode;
	int event_state[NVP_EVENT_CNT];
	time_t m_occur[NVP_EVENT_CNT];
	char event_topic[64];
	int64_t timeout;
	time_t m_sync;
	
	fNVPEventHook hook;
	void *hook_custom; // hock custom paramter

	void *private;

	struct ONVIF_EVENT_SUBSCRIBE *next;
}stONVIF_EVENT_SUBSCRIBE, *lpONVIF_EVENT_SUBSCRIBE;


extern SOAP_NMAC struct Namespace namespaces[];
extern SOAP_NMAC struct Namespace discovery_namespaces[];

#define ONVIF_SESSION_TIMEOUT		(65000) //unit : ms
#define ONVIF_EVENT_TIMEOUT			(60) //UNIT :SECOND
#define ONVIF_EVENT_PULLPOINT_URI_PREFIX		"/onvif/event/subs"
#define ONVIF_DEFAULT_EVENT_LISTEN_PORT		(9098)

#define ONVIF_MALLOC(Type) (Type *)				memset(soap_malloc(soap, sizeof(Type)), 0, sizeof(Type))
#define ONVIF_MALLOC_SIZE(Type, size) (Type *)	memset(soap_malloc(soap, (size) * sizeof(Type)), 0, (size) * sizeof(Type))

// timezone parser
extern int tzone_s2int(char *szone, int *value /* unit: second */);
extern char *tzone_int2s(int gmt, char *result, int size);
extern int tzone_value_to_second(int gmt);
extern int tzone_value_from_second(int second);

//
extern int get_int_string_format(char *src, char *format);
// celllayout format converter
extern int md_celllayout_s2hex(char *sz_layout, uint8_t *out, int size);
extern void md_celllayout_hex2s(char *sz_layout, int out_size, uint8_t *input, int in_size);
extern void md_celllayout_hex2s_ex(char *sz_layout, int out_size, uint8_t *input, int row, int column);
// netmask prefix converter, for example prefix of '255.255.255.0' is 24
extern int netmask_to_prefixlength(unsigned char *netmask);
extern int netmask_to_prefixlength2(char *netmask);
extern void netmask_from_prefixlength(int length, unsigned char *netmask);
extern void netmask_from_prefixlength2(int length, char *sznetmask);
// PT time parser, format : "PT[%dH][%dM][%dS].[...]
extern int sztime_to_int(char *sz_time);
extern void sztime_from_int(int time, char *sz_time);
//date time parser: format, for example 2014-02-21T15:29:07Z
extern int datetime_s2tm(char *sztime, struct tm *ptm, time_t *timet);
// get curent and terminal string time
extern void get_current_time(char *sztime);
extern void get_terminal_time(int64_t timeout, char *sztime);
//
extern int isValidIp4 (char *str);

extern int ONVIF_send_notify(int event, int state, char *local_reference, char *peer_reference);
extern int ONVIF_send_hello();
extern int ONVIF_send_bye();

extern int ONVIF_C_event_subscribe(int event_type, char *peer_reference, char *local_reference, int timeout,
	char *username, char *password,
	fNVPEventHook hook, void *param, void *private);
extern int ONVIF_C_event_renew(char *reference, int64_t *terminal_duration);
extern int ONVIF_C_event_find(char *reference, lpONVIF_EVENT_SUBSCRIBE entry);
extern int ONVIF_C_event_unsubscribe(char *peer_reference);
//
extern int ONVIF_event_unsubscribe(char *local_reference);

extern const char *g_onvif_topic[];
extern const char *g_event_property[];
extern const char *g_onvif_event_msg[];

#ifdef __cplusplus
};
#endif

#endif // __ONVIF_COMMON_H__
