#include "qarp.h"
#include "stdafx.h"

#include "./wpdpack/IPhlpapi.h"
#pragma comment(lib,"ws2_32")
Qarp::Qarp()
{

}

Qarp::~Qarp()
{

}

bool Qarp::qsendarp(unsigned long dstip)
{
	IPAddr destip=dstip;
	IPAddr srcip=0;
	unsigned long macaddr[2];
	unsigned long physaddrlen=6;
	DWORD dewretval;
	BYTE *bphysaddr;
	memset(&macaddr,0xff,sizeof(macaddr));
	dstip=htonl(dstip);
	dewretval=SendARP(destip,srcip, (ULONG*)&macaddr,&physaddrlen);
	if (dewretval==NO_ERROR)
	{
		bphysaddr=(BYTE*)&macaddr;
		if (physaddrlen)
		{
			for (int i = 0; i < (int) physaddrlen; i++) {
				if (i == (physaddrlen - 1))
					printf("%.2X\n", (int) bphysaddr[i]);
				else
					printf("%.2X-", (int) bphysaddr[i]);
			}
			return false;
		}else{
			return true;
		}
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"this ip can be use";
		return true;
	}
}
