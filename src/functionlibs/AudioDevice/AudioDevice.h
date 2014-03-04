#ifndef AUDIODEVICE_HEAD_FILE_H	
#define AUDIODEVICE_HEAD_FILE_H

#include "AudioDevice_global.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QThread>
#include "IAudioDevice.h"
#include "WaveDevice.h"


void CountChange(void* hWaveOut, const unsigned int &msg, void* pUser);

class AudioDevice : public QThread,
	public IAudioDevice
{
public:
	AudioDevice();
	~AudioDevice();

	virtual int SetMainWnd(int nWnd);
	virtual int SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth);
	virtual int SetVolume(unsigned int uiPersent);
	virtual int PlayBuffer(char *pBuffer,int nSize);
	virtual int Stop();

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();

	void CountChg(void* hWaveOut, const unsigned int &msg);

protected:
	void run();
private:
	WaveInfo* AllocateBlocks(int blockSize, int blockCount);
	void FreeBlocks(WaveInfo *pBlock);
private:
	int m_uiWind;
	WaveDevice m_waveDev;
	WaveInfo *m_pBuffer;
	int m_nBufferCount;
	void *m_hWaveOut;
	BufferList m_bufferList;
	QMutex m_mutexBufList;
	QMutex m_mutexBufCount;
	bool m_bIsRunning;
	bool m_bIsEnd;

	int m_nRef;
	QMutex m_csRef;
};

#endif // AUDIODEVICE_HEAD_FILE_H
