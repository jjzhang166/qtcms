
#include "soapH.h"
#include "soapStub.h"

#include "wsddapi.h"

#include "generic.h"
#include "onvifS.h"
#include "onvifC.h"
#include "onvif_debug.h"
#include "onvif.h"

extern fON_WSDD_EVENT g_search_hook;
extern void *g_searchCustomCtx;

#ifdef SOAP_SERVER
extern lpONVIF_S_CONTEXT g_OnvifServerCxt;
#endif

#ifndef SOAP_CLIENT //kaga
SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__Hello(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__HelloType *wsdd__Hello)
{	struct __wsdd__Hello soap_tmp___wsdd__Hello;
	if (!soap_action)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Hello";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__Hello.wsdd__Hello = wsdd__Hello;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__Hello(soap, &soap_tmp___wsdd__Hello);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__Hello(soap, &soap_tmp___wsdd__Hello, "-wsdd:Hello", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__Hello(soap, &soap_tmp___wsdd__Hello, "-wsdd:Hello", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_recv___wsdd__Hello(struct soap *soap, struct __wsdd__Hello *_param_2)
{
	soap_default___wsdd__Hello(soap, _param_2);
	soap_begin(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get___wsdd__Hello(soap, _param_2, "-wsdd:Hello", NULL);
	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		soap->error = SOAP_NO_METHOD;
	if (soap->error
	 || soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__Bye(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ByeType *wsdd__Bye)
{	struct __wsdd__Bye soap_tmp___wsdd__Bye;
	if (!soap_action)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Bye";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__Bye.wsdd__Bye = wsdd__Bye;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__Bye(soap, &soap_tmp___wsdd__Bye);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__Bye(soap, &soap_tmp___wsdd__Bye, "-wsdd:Bye", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__Bye(soap, &soap_tmp___wsdd__Bye, "-wsdd:Bye", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_recv___wsdd__Bye(struct soap *soap, struct __wsdd__Bye *_param_3)
{
	soap_default___wsdd__Bye(soap, _param_3);
	soap_begin(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get___wsdd__Bye(soap, _param_3, "-wsdd:Bye", NULL);
	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		soap->error = SOAP_NO_METHOD;
	if (soap->error
	 || soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__Probe(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ProbeType *wsdd__Probe)
{	struct __wsdd__Probe soap_tmp___wsdd__Probe;
	if (!soap_action)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Probe";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__Probe.wsdd__Probe = wsdd__Probe;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__Probe(soap, &soap_tmp___wsdd__Probe);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__Probe(soap, &soap_tmp___wsdd__Probe, "-wsdd:Probe", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__Probe(soap, &soap_tmp___wsdd__Probe, "-wsdd:Probe", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_recv___wsdd__Probe(struct soap *soap, struct __wsdd__Probe *_param_4)
{
	soap_default___wsdd__Probe(soap, _param_4);
	soap_begin(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get___wsdd__Probe(soap, _param_4, "-wsdd:Probe", NULL);
	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		soap->error = SOAP_NO_METHOD;
	if (soap->error
	 || soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__ProbeMatches(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ProbeMatchesType *wsdd__ProbeMatches)
{	struct __wsdd__ProbeMatches soap_tmp___wsdd__ProbeMatches;
	if (!soap_action)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ProbeMatches";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__ProbeMatches.wsdd__ProbeMatches = wsdd__ProbeMatches;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__ProbeMatches(soap, &soap_tmp___wsdd__ProbeMatches, "-wsdd:ProbeMatches", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_recv___wsdd__ProbeMatches(struct soap *soap, struct __wsdd__ProbeMatches *_param_5)
{
	soap_default___wsdd__ProbeMatches(soap, _param_5);
	soap_begin(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get___wsdd__ProbeMatches(soap, _param_5, "-wsdd:ProbeMatches", NULL);
	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		soap->error = SOAP_NO_METHOD;
	if (soap->error
	 || soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__Resolve(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ResolveType *wsdd__Resolve)
{	struct __wsdd__Resolve soap_tmp___wsdd__Resolve;
	if (!soap_action)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/Resolve";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__Resolve.wsdd__Resolve = wsdd__Resolve;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__Resolve(soap, &soap_tmp___wsdd__Resolve);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__Resolve(soap, &soap_tmp___wsdd__Resolve, "-wsdd:Resolve", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__Resolve(soap, &soap_tmp___wsdd__Resolve, "-wsdd:Resolve", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_recv___wsdd__Resolve(struct soap *soap, struct __wsdd__Resolve *_param_6)
{
	soap_default___wsdd__Resolve(soap, _param_6);
	soap_begin(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get___wsdd__Resolve(soap, _param_6, "-wsdd:Resolve", NULL);
	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		soap->error = SOAP_NO_METHOD;
	if (soap->error
	 || soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_send___wsdd__ResolveMatches(struct soap *soap, const char *soap_endpoint, const char *soap_action, struct wsdd__ResolveMatchesType *wsdd__ResolveMatches)
{	struct __wsdd__ResolveMatches soap_tmp___wsdd__ResolveMatches;
	if (!soap_action)
		soap_action = "http://docs.oasis-open.org/ws-dd/ns/discovery/2009/01/ResolveMatches";
	soap->encodingStyle = NULL;
	soap_tmp___wsdd__ResolveMatches.wsdd__ResolveMatches = wsdd__ResolveMatches;
	soap_begin(soap);
	soap_serializeheader(soap);
	soap_serialize___wsdd__ResolveMatches(soap, &soap_tmp___wsdd__ResolveMatches);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___wsdd__ResolveMatches(soap, &soap_tmp___wsdd__ResolveMatches, "-wsdd:ResolveMatches", NULL)
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___wsdd__ResolveMatches(soap, &soap_tmp___wsdd__ResolveMatches, "-wsdd:ResolveMatches", NULL)
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 soap_recv___wsdd__ResolveMatches(struct soap *soap, struct __wsdd__ResolveMatches *_param_7)
{
	soap_default___wsdd__ResolveMatches(soap, _param_7);
	soap_begin(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	soap_get___wsdd__ResolveMatches(soap, _param_7, "-wsdd:ResolveMatches", NULL);
	if (soap->error == SOAP_TAG_MISMATCH && soap->level == 2)
		soap->error = SOAP_NO_METHOD;
	if (soap->error
	 || soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

#endif

/**
@fn void wsdd_event_Hello(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int MetadataVersion)
@brief Handles and registers a Hello event from a TS or DP joining the network.
@param soap context (use soap->user as a pointer to a global state if needed)
@param[in] InstanceId (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] SequenceId (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] MessageNumber (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] MessageID WS-Addressing message ID of the Hello message
@param[in] RelatesTo WS-Addressing RelatesTo message ID of the Hello message
@param[in] EndpointReference of the Target Service or Discovery Proxy that joins
@param[in] Types an unordered string of QNames of services provided by the Target Service or Discovery Proxy where a Discovery Proxy MUST include "wsdd:DiscoveryProxy"
@param[in] Scopes an unordered set of Scopes the Target Service or Discovery Proxy is in
@param[in] MatchBy unused (reserved)
@param[in] XAddrs contains the transport address(es) that MAY be used to communicate with the Target Service or Discovery Proxy
@param[in] MetadataVersion incremented by a positive value (>= 1) whenever there is a change in the metadata of the Target Service

Hello is a one-way message sent by a Target Service to announce its
availability when it joins the network. It is also sent by a Discovery Proxy to
reduce multicast traffic on an ad hoc network.

To maintain a global state between events, for example to internally register
Target Services, Discovery Proxies, and update the status of these, use
void *soap->user to point to a global state (that you need to define).
*/


void wsdd_event_Hello(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int MetadataVersion)
{
	ONVIF_INFO("got hello from %s , type: %s", XAddrs ? XAddrs : "null", Types ? Types : "null");
	if (g_search_hook) {
		g_search_hook((char *)Types, (char *)XAddrs, (char *)Scopes, WSDD_EVENT_HELLO, g_searchCustomCtx);
	}
#ifdef  SOAP_CLIENT
	ONVIFC_wsdd_event_hook((char *)Types, (char *)XAddrs, (char *)Scopes, WSDD_EVENT_HELLO);
#endif
	return;
}

/**
@fn void wsdd_event_Bye(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int *MetadataVersion)
@brief Handles and registers a Bye event from a TS or DP leaving the network.
@param soap context (use soap->user as a pointer to a global state if needed)
@param[in] InstanceId (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] SequenceId (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] MessageNumber (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] MessageID WS-Addressing message ID of the Bye message
@param[in] RelatesTo WS-Addressing RelatesTo message ID of the Bye message
@param[in] EndpointReference of the Target Service or Discovery Proxy
@param[in] Types an unordered string of QNames of services provided by the Target Service or Discovery Proxy where a Discovery Proxy MUST include "wsdd:DiscoveryProxy"
@param[in] Scopes an unordered set of Scopes the Target Service or Discovery
Proxy is in
@param[in] MatchBy unused (reserved)
@param[in] XAddrs contains the transport address(es) that MAY be used to communicate with the Target Service or Discovery Proxy
@param[in] MetadataVersion incremented by a positive value (>= 1) whenever there is a change in the metadata of the Target Service

Bye is a one-way message sent by a Target Service to announce its
unavailability as a best effort when it leaves the network.

To maintain a global state between events, for example to internally register
Target Services, Discovery Proxies, and update the status of these, use
void *soap->user to point to a global state (that you need to define).
*/
void wsdd_event_Bye(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int *MetadataVersion)
{
	ONVIF_INFO("got bye from %s , type: %s", XAddrs ? XAddrs : "null", Types ? Types : "null");
	if (g_search_hook) {
		g_search_hook((char *)Types, (char *)XAddrs, (char *)Scopes, WSDD_EVENT_BYE, g_searchCustomCtx);
	}
#ifdef  SOAP_CLIENT
	ONVIFC_wsdd_event_hook((char *)Types, (char *)XAddrs, (char *)Scopes, WSDD_EVENT_BYE);
#endif
	return;
}


/**
@fn soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *Types, const char *Scopes, const char *MatchBy, struct wsdd__ProbeMatchesType *matches)
@brief Handles a Probe event from a Client.
@param soap context (use soap->user as a pointer to a global state if needed)
@param[in] MessageID WS-Addressing message ID of the message
@param[in] ReplyTo WS-Addressing ReplyTo message ID of the message
@param[in] Types an unordered string of QNames to probe
@param[in] Scopes an unordered set of scopes to probe
@param[in] MatchBy matching rule to apply for this probe
@param[out] matches contains probe matches returned by event handler, use @ref soap_wsdd_add_ProbeMatch to populate the matches in the handler
@return managed (SOAP_WSDD_MANAGED) or ad-hoc (SOAP_WSDD_ADHOC) mode to use to return the matches

A Client sends a probe to find Target Services by the Type of the Target
Service, a Scope in which the Target Service resides, both, or simply all
Target Services. The matches are returned by this server-side event handler
that match the Client's probe.

To maintain a global state between events, for example to internally register
Target Services, Discovery Proxies, and update the status of these, use
void *soap->user to point to a global state (that you need to define).
*/
soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *Types, 
	const char *Scopes, const char *MatchBy, struct wsdd__ProbeMatchesType *matches)
{
#ifdef SOAP_SERVER
	char tmp[32];
	char *xaddr = NULL;
	char *sz_scopes = NULL;
	char *sz_uuid = NULL;
	int i;

	if (g_OnvifServerCxt == NULL) {
		return SOAP_WSDD_MANAGED;
	}

	sz_scopes = ONVIF_MALLOC_SIZE(char , 2500);
	sz_uuid = ONVIF_MALLOC_SIZE(char , 64);
	strcpy(sz_uuid, (char *)soap_wsa_rand_uuid(soap));
	if (strstr(Types, "NetworkVideoTransmitter") != NULL
		/* || strstr(Types, "Device") != NULL */
		) {
		NVP_env_load(&g_OnvifServerCxt->env, OM_NET, 0);
		
		xaddr = (char *)soap_malloc(soap, 128);
		sprintf(xaddr, "http://%s:%d/onvif/device_service", _ip_2string(g_OnvifServerCxt->env.ether.ip, tmp), 
			g_OnvifServerCxt->env.ether.http_port);

		memset(sz_scopes, 0, sizeof(sz_scopes));
		for ( i = 0; i < g_OnvifServerCxt->scope_nr; i++) {
			strcat(sz_scopes, g_OnvifServerCxt->scope_list[i]);
			if ( (i + 1) != g_OnvifServerCxt->scope_nr)
				strcat(sz_scopes, " ");
		}
		ONVIF_INFO("search type:%s", Types);
		ONVIF_TRACE("scopes: %s", sz_scopes);
		
		soap_wsdd_init_ProbeMatches(soap, matches);
		soap_wsdd_add_ProbeMatch(soap, matches, 
			 sz_uuid,
			 "dn:NetworkVideoTransmitter",
			 sz_scopes,
			NULL,
			xaddr,
			1);
	}

	if (soap->header) {
		soap_default_SOAP_ENV__Header(soap, soap->header);
#if 1
		soap->header->wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches";
		soap->header->wsa__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";
		soap->header->wsa__RelatesTo = ONVIF_MALLOC(struct wsa__Relationship);
		soap->header->wsa__RelatesTo->__item= ONVIF_MALLOC_SIZE(char , 128);
		strcpy(soap->header->wsa__RelatesTo->__item, MessageID);
		soap->header->wsa__MessageID = sz_uuid;
#else		
		soap->header->wsa5__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches";
		soap->header->wsa5__To = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";
		soap->header->wsa5__RelatesTo = ONVIF_MALLOC(struct wsa5__RelatesToType);
		soap->header->wsa5__RelatesTo->__item= ONVIF_MALLOC_SIZE(char , 128);
		strcpy(soap->header->wsa5__RelatesTo->__item, MessageID);
		soap->header->wsa5__MessageID = ONVIF_MALLOC_SIZE(char , 64);
		strcpy(soap->header->wsa5__MessageID, sz_uuid);
#endif
	} 
	return SOAP_WSDD_MANAGED;
#else
	return SOAP_WSDD_MANAGED;
#endif
}

/**
@fn void wsdd_event_ProbeMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ProbeMatchesType *matches)
@brief Handles a Probe event from a Client.
@param soap context (use soap->user as a pointer to a global state if needed)
@param[in] InstanceId (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] SequenceId (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] MessageNumber (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] MessageID WS-Addressing message ID of the message
@param[in] RelatesTo WS-Addressing RelatesTo message ID of the message
@param[in] matches contains the probe matches

A Client sends a probe to find Target Services by the Type of the Target
Service, a Scope in which the Target Service resides, both, or simply all
Target Services. The matches are provided to this client-side event handler.

To maintain a global state between events, for example to internally register
Target Services, Discovery Proxies, and update the status of these, use
void *soap->user to point to a global state (that you need to define).
*/
void wsdd_event_ProbeMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ProbeMatchesType *matches)
{
	return;
}

/**
@fn soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *EndpointReference, struct wsdd__ResolveMatchType *match);
@brief Handles a Resolve event from a Client.
@param soap context (use soap->user as a pointer to a global state if needed)
@param[in] MessageID WS-Addressing message ID of the message
@param[in] ReplyTo WS-Addressing ReplyTo message ID of the message
@param[in] EndpointReference of the Target Service or Discovery Proxy
@param[out] match contains the match returned by the event handler
@return managed (SOAP_WSDD_MANAGED) or ad-hoc (SOAP_WSDD_ADHOC) mode to use to return the matches

A Client sends a resolve to locate a Target Service, i.e., to retrieve its
transport address(es). This server-side event handler returns the match(es).

To maintain a global state between events, for example to internally register
Target Services, Discovery Proxies, and update the status of these, use
void *soap->user to point to a global state (that you need to define).
*/
soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *EndpointReference, struct wsdd__ResolveMatchType *match)
{
	return SOAP_WSDD_MANAGED;
}

/**
@fn void wsdd_event_ResolveMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ResolveMatchType *match)
@brief Handles a Probe event from a Client.
@param soap context (use soap->user as a pointer to a global state if needed)
@param[in] InstanceId (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] SequenceId (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] MessageNumber (see WS-Discovery 1.1 Section 7 Application Sequencing)
@param[in] MessageID WS-Addressing message ID of the message
@param[in] RelatesTo WS-Addressing RelatesTo message ID of the message
@param[in] match contains the resolve match

A Client sends a resolve to locate a Target Service, i.e., to retrieve its
transport address(es). This client-side event handler receives the match.

To maintain a global state between events, for example to internally register
Target Services, Discovery Proxies, and update the status of these, use
void *soap->user to point to a global state (that you need to define).
*/
void wsdd_event_ResolveMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ResolveMatchType *match)
{
	return;
}



//////////////////////////////////////////////////////


