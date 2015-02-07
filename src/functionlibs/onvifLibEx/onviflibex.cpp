#include "onviflibex.h"

QMutex onvifLibEx::m_tOnvifMutex;
int onvifLibEx::m_nRef(0);
onvifLibEx::onvifLibEx()
{

}

onvifLibEx::~onvifLibEx()
{

}

void onvifLibEx::ONVIF_CLIENT_initEx( int conn_timeout, int send_timeout, int recv_timeout, bool check_with_arp, int online_timeout,char *bindip, int port )
{
	m_tOnvifMutex.lock();
	if (m_nRef==0)
	{
		//do init
		ONVIF_CLIENT_init(conn_timeout,send_timeout,recv_timeout,check_with_arp,online_timeout);
		ONVIF_event_daemon_start(bindip,port);
	}else{
		//do nothing
	}
	m_nRef++;
	m_tOnvifMutex.unlock();
}

void onvifLibEx::ONVIF_CLIENT_deinitEx()
{
	m_tOnvifMutex.lock();
	m_nRef--;
	if (m_nRef==0)
	{
		//do deinit
		ONVIF_CLIENT_deinit();
	}else{
		//do nothing
	}
	if (m_nRef<0)
	{
		m_nRef=0;
	}
	m_tOnvifMutex.unlock();
}

