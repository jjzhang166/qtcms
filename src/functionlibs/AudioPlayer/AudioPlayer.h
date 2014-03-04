#ifndef AUDIOPLAYER_HEAD_FILE_H	
#define AUDIOPLAYER_HEAD_FILE_H

#include "AudioPlayer_global.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include "IAudioPlayer.h"
#include "IAudioDecoder.h"
#include "IAudioDevice.h"


class AudioPlayer : public IAudioPlayer
{
public:
	AudioPlayer();
	~AudioPlayer();

	virtual int SetPlayWnd(int nWnd);
	virtual int SetAudioParam(int nChannel,int nSampleRate,int nSampleWidth);
	virtual int SetVolume(int nPersent);
	virtual int Play(char *pBuffer,int nSize);
	virtual int EnablePlay(bool bEnabled);
	virtual int GetPlayStatus();
	virtual int Stop();

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );
	virtual unsigned long __stdcall AddRef();
	virtual unsigned long __stdcall Release();


private:
	bool m_bEnabledPlay;
	IAudioDecoder *m_pAudioDec;
	IAudioDevice *m_pAudioDev;

	int m_nRef;
	QMutex m_csRef;
};

#endif // AUDIOPLAYER_HEAD_FILE_H


