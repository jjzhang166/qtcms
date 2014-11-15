#ifndef __AUDIOACMDEVICE_CLASS_HEAD_FILE__
#define __AUDIOACMDEVICE_CLASS_HEAD_FILE__
#include "AudioDevice.h"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#include <list>
using namespace std ;

typedef struct _tagBufferListNode{
	char * pBuffer;
	int    nBufferSize;
}BufferListNode;

typedef list<BufferListNode> BufferList;

#define WAVEDEV_BLOCK_SIZE      320
#define WAVEDEV_BLOCK_COUNT		20

class CAudioWaveDevice : public CAudioDevice
{
public:
	CAudioWaveDevice();
	virtual ~CAudioWaveDevice();

public:
	virtual int SetMainWnd(HWND hWnd);
	virtual int SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth);
	virtual int SetVolume(int nPersent);
	virtual int PlayBuffer(char *pBuffer,int nSize);
	virtual int Stop();

	void waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwParam1, DWORD dwParam2);
	DWORD AudioPlayThread();
protected:
private:
	void      CloseHwo();
	WAVEHDR * allocateBlocks(int size, int count);
	void      freeBlocks(WAVEHDR *pBlocks);


private:
	HWAVEOUT         m_hWaveout;
	WAVEHDR *        m_pBuffer;
	BufferList       m_BufList;

	int              m_nBufferCount;
	int              m_nPlayBuffercount;
	CRITICAL_SECTION m_csBufferCount;
	CRITICAL_SECTION m_csBufList;

	bool             m_bThreadEnd;
	bool             m_bThreadRunning;

	HWND             m_hPlayWnd;

};
#endif