#ifndef AUDIODECODER_HEAD_FILE_H
#define AUDIODECODER_HEAD_FILE_H

#include "AudioDecoder_global.h"
#include <QtCore/QObject>
#include <QtCore/QMutex>
#include "IAudioDecoder.h"



class AudioDecoder :
	public IAudioDecoder
{
public:
	AudioDecoder();
    virtual ~AudioDecoder();

	virtual int Decode(char *OutPutBuffer,char *InputBuffer,int nInputBufferSize);

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();
private:

	int m_nRef;
	QMutex m_csRef;
};

#endif // AUDIODECODER_HEAD_FILE_H
