#ifndef HICHIPSEARCH_GLOBAL_H
#define HICHIPSEARCH_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

typedef char CHAR;

typedef struct HiChipSearchItem
{
	CHAR name[32];
	CHAR dev_model[32];
	CHAR esee_id[32];
	CHAR ip[32];
	CHAR netmask[32];
	CHAR gateway[32];
	CHAR port[16];
	CHAR channelcnt[16];
	CHAR mac[32];
	CHAR devid[64];
}HiChipSearchItem_t;

 typedef struct HiChipSetupItem
 {
 	CHAR name[32];
 	CHAR ip[32];
 	CHAR netmask[2];
 	CHAR gateway[32];
 	CHAR port[16];
 	CHAR mac[32];
 	CHAR devid[64];
 	CHAR username[32];
 	CHAR password[32];
 }HiChipSetupItem_t;

typedef int (__cdecl *IPCSearchCB)(QString name, QVariantMap info, void* pUser);

typedef struct ProcInfoItem
{
	IPCSearchCB proc;
	void		*puser;
}ProcInfoItem_t;

#endif // HICHIPSEARCH_GLOBAL_H
