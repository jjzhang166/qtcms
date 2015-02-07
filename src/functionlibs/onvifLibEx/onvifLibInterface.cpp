#include "onvifLibInterface.h"
#include "onviflibex.h"
void ONVIF_CLIENT_initEx(int conn_timeout, int send_timeout, int recv_timeout,
	bool check_with_arp, int online_timeout,char *bindip, int port){
	onvifLibEx tOnvifLibEx;
	return tOnvifLibEx.ONVIF_CLIENT_initEx(conn_timeout,send_timeout,recv_timeout,check_with_arp,online_timeout,bindip, port);
}
void ONVIF_CLIENT_deinitEx(){
	onvifLibEx tOnvifLibEx;
	return tOnvifLibEx.ONVIF_CLIENT_deinitEx();
}