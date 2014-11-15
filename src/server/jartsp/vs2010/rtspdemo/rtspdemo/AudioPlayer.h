#ifndef __AUDIOPLAYER_CLASS_HEAD_FILE__
#define __AUDIOPLAYER_CLASS_HEAD_FILE__

#include "AudioDevice.h"
#include "CAudioDecoder.h"
//#include "AudioDxDevice.h"

#pragma pack(4)
typedef struct _tagAudioBufAttr{
	int entries;
	int packsize;
	UINT64 pts;
	time_t gtime;
	char encode[8];
	int samplerate;
	int samplewidth;
}AudioBufAttr;
#pragma pack()

class CAudioPlayer
{
public:
	CAudioPlayer();
	~CAudioPlayer();

public:
	int SetPlayWnd(HWND hWnd);
	int SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth);
	//int Init();
	int SetVolume(int nPersent); 
	void Play(char *pBuffer,int nBufferSize);
	void EnablePlay(bool bEnabled);
	bool GetPlayStatus();
	void Stop(void);
protected:
private:
	bool	m_bEnablePlay;
	CAudioDevice * m_dev;
	CAudioDecoder * m_dec;
};

#endif