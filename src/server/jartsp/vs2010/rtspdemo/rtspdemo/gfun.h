#pragma once
#include <DDRAW.H>

extern CRITICAL_SECTION g_csDecInit;


extern int             DdrawInit(HWND hWnd);

extern void			   DdrawDeinit();





