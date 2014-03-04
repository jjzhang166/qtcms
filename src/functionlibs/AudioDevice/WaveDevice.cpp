#include "WaveDevice.h"
#include <WTypes.h>
#include <mmsystem.h>

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	((WaveDevice*)dwInstance)->WaveProc((void*)hwo, uMsg);
	return;
}

WaveDevice::WaveDevice()
	:m_proc(NULL)
{

}

WaveDevice::~WaveDevice()
{

}

int WaveDevice::winWaveOutInitCb(bufferCountChg pro, void *pUser)
{
	if (NULL != pro && NULL != pUser)
	{
		m_proc = pro;
		m_pUser = pUser;
	}
	return 0;
}

int WaveDevice::winWaveOutSetVolume(void* hWaveOut, unsigned int volume)
{
	if (volume < 0)
	{
		return 1;
	}
	
	waveOutSetVolume((HWAVEOUT)hWaveOut, (DWORD)volume);
	return 0;
}

int WaveDevice::winWaveOutOpen(void** hWaveOut, int nChannel,int nSampleRate,int nSampleWidth)
{
	WAVEFORMATEX wfx;
	MMRESULT hr;
	wfx.wBitsPerSample = nSampleWidth;
	wfx.nSamplesPerSec = nSampleRate;
	wfx.nChannels = nChannel;
	wfx.cbSize = 0;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;


	hr = waveOutOpen((LPHWAVEOUT)hWaveOut, WAVE_MAPPER, &wfx, (DWORD)::waveOutProc, (DWORD)this, CALLBACK_FUNCTION);

	if (MMSYSERR_NOERROR != hr)
	{
		return 1;
	}
	return 0;
}
void WaveDevice::winWaveOutRestart(void* hWaveOut)
{
	waveOutRestart((HWAVEOUT)hWaveOut);
}
void WaveDevice::winWaveOutReset(void* hWaveOut)
{
	waveOutReset((HWAVEOUT)hWaveOut);
}
void WaveDevice::winWaveOutPause(void* hWaveOut)
{
	waveOutPause((HWAVEOUT)hWaveOut);
}
void WaveDevice::winWaveOutClose(void* hWaveOut)
{
	waveOutClose((HWAVEOUT)hWaveOut);
}
int WaveDevice::winWaveOutUnprepareHeader(void* hWaveOut, WaveInfo *wavehdr)
{
	if (NULL == hWaveOut || NULL == wavehdr)
	{
		return 1;
	}

	waveOutUnprepareHeader((HWAVEOUT)hWaveOut, (WAVEHDR*)wavehdr, sizeof(WAVEHDR));
	
	return 0;
}
int WaveDevice::winWaveOutPrepareHeader(void* hWaveOut, WaveInfo* wavehdr)
{
	if (NULL == hWaveOut || NULL == wavehdr)
	{
		return 1;
	}
	MMRESULT result = waveOutPrepareHeader((HWAVEOUT)hWaveOut, (WAVEHDR *)wavehdr, sizeof(WAVEHDR));
	if (0 != result)
	{
		return 1;
	}

	return 0;
}
int WaveDevice::winWaveOutWrite(void* hWaveOut, WaveInfo* wavehdr)
{
	if (NULL == hWaveOut || NULL == wavehdr)
	{
		return 1;
	}
	MMRESULT result = waveOutWrite((HWAVEOUT)hWaveOut, (WAVEHDR *)wavehdr, sizeof(WAVEHDR));
	if (0 != result)
	{
		return 1;
	}
	return 0;
}
void WaveDevice::WaveProc(void* hWaveOut, const unsigned int &msg)
{
	if (NULL != m_proc && m_pUser)
	{
		m_proc(hWaveOut, msg, m_pUser);
	}
}
