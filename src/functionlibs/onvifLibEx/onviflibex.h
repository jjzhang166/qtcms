#ifndef ONVIFLIBEX_H
#define ONVIFLIBEX_H

#include "onviflibex_global.h"
#include <QMutex>
#include "onvif.h"
#pragma comment(lib, "libonvifc.lib")
class ONVIFLIBEX_EXPORT onvifLibEx
{
public:
	onvifLibEx();
	~onvifLibEx();

public:
	void ONVIF_CLIENT_initEx(int conn_timeout, int send_timeout, int recv_timeout,
		bool check_with_arp, int online_timeout,char *bindip, int port);
	void ONVIF_CLIENT_deinitEx();
private:
	static QMutex m_tOnvifMutex;
	static int m_nRef;
};

#endif // ONVIFLIBEX_H
