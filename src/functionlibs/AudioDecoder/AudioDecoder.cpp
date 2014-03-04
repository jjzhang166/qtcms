#include "AudioDecoder.h"
#include "g711.h"
#include <guid.h>



AudioDecoder::AudioDecoder() :
m_nRef(0)
{

}

AudioDecoder::~AudioDecoder()
{

}

int AudioDecoder::Decode(char *OutPutBuffer,char *InputBuffer,int nInputBufferSize)
{
	return g711a_decode((short *)OutPutBuffer,(unsigned char *)InputBuffer,nInputBufferSize);
}


long __stdcall AudioDecoder::QueryInterface( const IID & iid,void **ppv )
{
	if (IID_IAudioDecoder == iid)
	{
		*ppv = static_cast<IAudioDecoder *>(this);
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

unsigned long __stdcall AudioDecoder::AddRef()
{
	m_csRef.lock();
	m_nRef++;
	m_csRef.unlock();
	return m_nRef;
}

unsigned long __stdcall AudioDecoder::Release()
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