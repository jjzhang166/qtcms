#ifndef __WAVEDEVICE_HEAD_FILE__
#define __WAVEDEVICE_HEAD_FILE__



typedef struct _tagWavehdr {
	char* pData;            
	unsigned long ulBufferLength;    
	unsigned long ulBytesRecorded;   
	unsigned long ulUser;            
	unsigned long ulFlags;           
	unsigned long ulLoops;           
	struct _tagWavehdr *pNext;
	unsigned long ulreserved;          
}WaveInfo;

typedef void (*bufferCountChg)(void* hWaveOut, const unsigned int &msg, void* pUser);

class WaveDevice
{
public:
	WaveDevice();
	virtual ~WaveDevice();

public:
	int winWaveOutInitCb(bufferCountChg pro, void *pUser);
	int winWaveOutSetVolume(void* hWaveOut, unsigned int volume);
	int winWaveOutOpen(void** hWaveOut, int nChannel,int nSampleRate,int nSampleWidth);
	void winWaveOutRestart(void* hWaveOut);
	void winWaveOutReset(void* hWaveOut);
	void winWaveOutPause(void* hWaveOut);
	void winWaveOutClose(void* hWaveOut);
	int winWaveOutUnprepareHeader(void* hWaveOut, WaveInfo* wavehdr);
	int winWaveOutPrepareHeader(void* hWaveOut, WaveInfo* wavehdr);
	int winWaveOutWrite(void* hWaveOut, WaveInfo* wavehdr);
	void WaveProc(void* hWaveOut, const unsigned int &msg);
private:
	bufferCountChg m_proc;
	void* m_pUser;
};
#endif