// libutinity.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "libutinity.h"
#include "sqlite3.h"
#include "stdio.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


__stdcall int setCmsLanguage( char * language,char * installedpath )
{
	char databasepath[1024];
	sprintf(databasepath,"%s\\system.db",installedpath);
	sqlite3 * db;
	char * errormsg;
	int nret = sqlite3_open(databasepath,&db);
	if (SQLITE_OK != nret)
	{
		return 2;
	}
	char query[1024];
	sprintf(query,"update general_setting set value='%s' where name='misc_slanguage';",language);
	nret = sqlite3_exec(db,query,NULL,NULL,NULL);
	if (SQLITE_OK != nret)
	{

	}
	sqlite3_close(db);
	return 0;
}
