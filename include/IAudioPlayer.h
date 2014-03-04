#ifndef __INTERFACE_IAUDIOPLAYER_HEAD_FILE_GIDPOEKFD__
#define __INTERFACE_IAUDIOPLAYER_HEAD_FILE_GIDPOEKFD__
#include <libpcom.h>

interface IAudioPlayer : public IPComBase
{
	virtual int SetPlayWnd(int uiWnd) = 0;
	virtual int SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth) = 0;
	virtual int SetVolume(int nPersent) = 0;
	virtual int Play(char *pBuffer,int nSize) = 0;
	virtual int EnablePlay(bool bEnabled) = 0;
	virtual int GetPlayStatus() = 0;
	virtual int Stop() = 0;
};

#endif