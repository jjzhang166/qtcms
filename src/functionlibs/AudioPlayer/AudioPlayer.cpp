#include "AudioPlayer.h"
#include <guid.h>



AudioPlayer::AudioPlayer() :
m_nRef(0),
m_pAudioDec(NULL),
m_pAudioDev(NULL)
{
	m_bEnabledPlay = false;
	//ÉêÇë½âÂëÆ÷
	pcomCreateInstance(CLSID_AudioDecoder,NULL,IID_IAudioDecoder,(void**)&m_pAudioDec);
	//ÉêÇëÉè±¸
	pcomCreateInstance(CLSID_AudioDevice,NULL,IID_IAudioDevice,(void**)&m_pAudioDev);
}

AudioPlayer::~AudioPlayer()
{
	if (NULL != m_pAudioDec)
	{
		m_pAudioDec->Release();
		m_pAudioDec = NULL;
	}
	if (NULL != m_pAudioDev)
	{
		m_pAudioDev->Release();
		m_pAudioDev = NULL;
	}

}

int AudioPlayer::SetPlayWnd(int nWnd)
{
	if (NULL == m_pAudioDev)
	{
		return 1;
	}
	return m_pAudioDev->SetMainWnd(nWnd);
}
int AudioPlayer::SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth)
{
	if (NULL == m_pAudioDev)
	{
		return 1;
	}
	if (nChannel <= 0 || nSampleRate <= 0 || nSampleWidth <= 0)
	{
		return 1;
	}
	int nRet = m_pAudioDev->SetAudioParam(nChannel, nSampleRate, nSampleWidth);
	return nRet;
}
int AudioPlayer::SetVolume(unsigned int uiPersent)
{
    if (NULL == m_pAudioDev || (int)uiPersent < 0 || uiPersent > 100)
	{
		return 1;
	}
	unsigned int volume = uiPersent*(qint64)0xFFFFFFFF/100;
	return m_pAudioDev->SetVolume(volume);
}
int AudioPlayer::Play(char *pBuffer,int nSize)
{
	if (NULL == pBuffer || nSize <= 0)
	{
		return 1;
	}
	if (NULL == m_pAudioDev || NULL == m_pAudioDec)
	{
		return 1;
	}
	if (!m_bEnabledPlay)
	{
		return 2;
	}
	char buffer[1024*4];
	memset(buffer, 0, sizeof(buffer));
	int nLength = m_pAudioDec->Decode(buffer, pBuffer, nSize);
	int nRet = m_pAudioDev->PlayBuffer(buffer, nLength);
	return nRet;
}
int AudioPlayer::EnablePlay(bool bEnabled)
{
	if (NULL == m_pAudioDev)
	{
		return 1;
	}

	m_bEnabledPlay = bEnabled;
	if (!bEnabled)
	{
		return m_pAudioDev->Stop();
	}
	return 0;
}
int AudioPlayer::GetPlayStatus()
{
	return m_bEnabledPlay;
}
int AudioPlayer::Stop()
{
	if (NULL == m_pAudioDev)
	{
		return 1;
	}
	return m_pAudioDev->Stop();
}


long __stdcall AudioPlayer::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IAudioPlayer == iid)
	{
		*ppv = static_cast<IAudioPlayer *>(this);
	}
	else if (IID_IPcomBase == iid)
	{
		*ppv = static_cast<IPcomBase *>(this);
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	static_cast<IPcomBase *>(this)->AddRef();

	return S_OK;
}

unsigned long __stdcall AudioPlayer::AddRef()
{
	m_csRef.lock();
	m_nRef++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall AudioPlayer::Release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef-- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		delete this;
	}
	return nRet;
}
