#include "ringBuffer.h"


ringBuffer::ringBuffer(void)
{
}


ringBuffer::~ringBuffer(void)
{
	if (NULL != m_Buffer)
	{
		free(m_Buffer);
	}
}

void ringBuffer::createRingBuffer( unsigned int nBufferSize )
{
	m_Buffer = (char *)malloc(nBufferSize);
	if (NULL == m_Buffer)
	{
		abort();
	}

	m_nBufferSize = nBufferSize;
	m_lpCur = m_Buffer;
}

char * ringBuffer::getBuffer( unsigned int nSize )
{
	//ASSERT(hBuf,"");
	//ASSERT(hBuf->nBufferSize > nSize,"nSize(%d)",nSize);

	char *lpRet = NULL;
	if (m_lpCur + nSize < m_Buffer + m_nBufferSize)
	{
		lpRet = m_lpCur;
		m_lpCur += nSize;
	}
	else
	{
		lpRet = m_Buffer;
		m_lpCur = m_Buffer + nSize;
	}

	return lpRet;
}
