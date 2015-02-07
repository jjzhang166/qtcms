#pragma once


extern "C"{
	 void ONVIF_CLIENT_initEx(int conn_timeout, int send_timeout, int recv_timeout,
		bool check_with_arp, int online_timeout,char *bindip, int port);
	 void ONVIF_CLIENT_deinitEx();
};