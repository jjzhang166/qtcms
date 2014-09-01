#ifndef __WIN_FUNCTIONS_HEAD_FILE__
#define __WIN_FUNCTIONS_HEAD_FILE__

bool wfGetDiskFreeSpaceEx(char * path
	,unsigned long long & FreeBytesAvailable
	,unsigned long long & TotalNuberOfBytes
	,unsigned long long & FreeBytes);

#endif
