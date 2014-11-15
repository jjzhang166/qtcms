#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "onvif.h"

#include "onvif_common.h"	// test
#include "env_common.h"
#include "sock.h"

extern int nvp_test(int argc, char *argv[]);

void print_usage(char *cmd)
{
	printf("%s {-s|...} {-listen-port}\n", cmd);
	printf("-s , run as server, it this mode, you can specific listen-port on following parameter\n");
	printf("-sc , run as both server and client, it this mode, you can got discovery and hello/bye message,etc.\n");
	printf("..., run as client\n");
	exit(0);
}

void unit_test()
{
	char format[64];
	int ret;
	
	ret = get_int_string_format("Preset_1", format);
	printf("Preset_1 format is %s(%d)\n", format, ret);
	ret = get_int_string_format("ch1_0.264", format);
	printf("ch1_0.264 format is %s(%d)\n", format, ret);
	ret = get_int_string_format("10Chanel_1", format);
	printf("10Chanel_1 format is %s(%d)\n", format, ret);
}

void onvif_wsdd_event_hook(char *type, char *xaddr, char *scopes ,int event, void *customCtx)
{
	printf("wsdd hook %s, %s event:%d\n\t\tscope:%s\n", type ? type : "", xaddr, event, scopes ? scopes : "");
}

static int onvif_search_hook(char *bindip, unsigned char *ip,unsigned short port,
	char *name, char *location, char *firmware,
	void *customCtx)
{
	printf("add ipc %d.%d.%d.%d:%d %s %s @%s\n",ip[0],ip[1],ip[2],ip[3],port,
		name, firmware, location);
	return 0;
}

int main(int argc, char *argv[])
{
	int ch;
	int listen_port = 8080;
	
	if (argc < 2) {
		print_usage(argv[0]);
	} else {
		setenv("DEF_ETHER", "eth0", true);
		if (strcmp(argv[1], "-s") == 0) {
			char szport[32];
			
			if (argc == 3)
				listen_port = atoi(argv[2]);
			sprintf(szport, "%d", listen_port);
			setenv("ONVIF_PORT", szport, true);
			printf("hello onvif, run as server, lisen port : %d ...\n", listen_port);
			ONVIF_SERVER_init(ONVIF_DEV_NVT, "ONVIF_DEMO");
			ONVIF_SERVER_start(listen_port);
			ONVIF_search_daemon_start(NULL, NULL);
		} else if (strcmp(argv[1], "-t") == 0) {
			unit_test();
		} else if (strcmp(argv[1], "-sc") == 0) {	
			printf("hello onvif, run as both server and client, lisen port : %d ...\n", listen_port);
			ONVIF_SERVER_init(ONVIF_DEV_NVT, "ONVIF_DEMO");
			ONVIF_SERVER_start(listen_port);
			ONVIF_CLIENT_init(5, 5, 5, true, 30);
			ONVIF_search_daemon_start(onvif_wsdd_event_hook, NULL);
			//ONVIF_search_daemon_start(NULL, NULL);
		} else {
			char localip[20];
			
			printf("-------------- hello onvif, run as client -------------\n");
			SOCK_get_ether_ip("eth0", localip, NULL, NULL);
			ONVIF_CLIENT_init(4, 4, 4, false, 30);
			ONVIF_event_daemon_start(localip, 9987);
			//ONVIF_search_daemon_start(onvif_wsdd_event_hook, NULL);
			return nvp_test(argc, argv);
		}
	}

	for(; ; )
	{
		ch = getchar();
		if (ch == 'q') {
			break;
		} else if (ch == 'e') {
			ONVIF_notify_event(NVP_EVENT_MD);
		} else if (ch == 's') {
			ONVIF_search(ONVIF_DEV_NVT, true, 0, onvif_search_hook, NULL, NULL);
		}
	}
	
	if (strcmp(argv[1], "-s") == 0) {
		ONVIF_search_daemon_stop();
		ONVIF_SERVER_deinit();
	} else if (strcmp(argv[1], "-t") == 0) {
	} else if (strcmp(argv[1], "-sc") == 0) {	
		ONVIF_search_daemon_stop();
		ONVIF_SERVER_deinit();
		ONVIF_CLIENT_deinit();
	} else {
		ONVIF_search_daemon_stop();
		ONVIF_CLIENT_deinit();
	}
	
	printf("hello onvif, destroy done!\n");

	return 0;
}

