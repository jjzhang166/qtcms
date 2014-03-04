#ifndef __INTERFACE_IAUDIODEVICE_HEAD_FILE_GIDPOEKFD__
#define __INTERFACE_IAUDIODEVICE_HEAD_FILE_GIDPOEKFD__
#include <libpcom.h>

interface IAudioDevice : public IPComBase
{
	virtual int SetMainWnd(int nWnd) = 0;
	virtual int SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth) = 0;
	virtual int SetVolume(unsigned int uiPersent) = 0;
	virtual int PlayBuffer(char *pBuffer,int nSize) = 0;
	virtual int Stop() = 0;
};

#endif