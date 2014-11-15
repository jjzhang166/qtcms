#include "StdAfx.h"
#include "AudioPlayer.h"
//#include "AudioDxDevice.h"
#include "G711Decoder.h"
#include "AudioWaveDevice.h"

#define	BUFFERSIZE		1024*128
CAudioPlayer::CAudioPlayer()
{
	m_bEnablePlay = false;
//	m_dev = new CAudioDxDevice();
	m_dec = new CG711Decoder();
	m_dev = new CAudioWaveDevice();
	
//	m_dev->SetAudioParam(1,8000,16);
}

CAudioPlayer::~CAudioPlayer()
{
	if (NULL != m_dec)
	{
		delete m_dec;
	}

	if (NULL != m_dev)
	{
		delete m_dev;
	}
}

int CAudioPlayer::SetPlayWnd(HWND hWnd)
{
	return m_dev->SetMainWnd(hWnd);
}

int CAudioPlayer::SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth)
{
	return m_dev->SetAudioParam(nChannel,nSampleRate,nSampleWidth);
}

// int CAudioPlayer::Init()
// {
// 	return m_dev->InitDevice();
// }

int CAudioPlayer::SetVolume(int nPersent)
{
	return m_dev->SetVolume(nPersent);
}

void CAudioPlayer::Play(char *pBuffer,int nBufferSize)
{
	if (!m_bEnablePlay)
	{
		return;
	}
	char buffer[BUFFERSIZE];
	memset(buffer, 0, BUFFERSIZE);
	int nRet = m_dec->Decode(buffer, pBuffer, nBufferSize);
	m_dev->PlayBuffer(buffer, nRet);
}

void CAudioPlayer::EnablePlay(bool bEnabled)
{
	m_bEnablePlay = bEnabled;
	if (!bEnabled)
	{
		m_dev->Stop();
	}
}

bool CAudioPlayer::GetPlayStatus()
{
	return m_bEnablePlay;
}

void CAudioPlayer::Stop(void)
{
	m_dev->Stop();
}