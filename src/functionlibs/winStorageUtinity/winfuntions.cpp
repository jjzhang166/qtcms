#include <Windows.h>

bool wfGetDiskFreeSpaceEx( char * path ,unsigned long long & FreeBytesAvailable ,unsigned long long & TotalNuberOfBytes ,unsigned long long & FreeBytes )
{
	ULARGE_INTEGER freeBytesAvailable;
	ULARGE_INTEGER totalNumberOfBytes;
	ULARGE_INTEGER totalNumberOfFreeBytes;
	bool bRet = GetDiskFreeSpaceExA(path,&freeBytesAvailable,&totalNumberOfBytes,&totalNumberOfFreeBytes);
	FreeBytesAvailable = freeBytesAvailable.QuadPart;
	TotalNuberOfBytes = totalNumberOfBytes.QuadPart;
	FreeBytes = totalNumberOfFreeBytes.QuadPart;

	return bRet;
}

