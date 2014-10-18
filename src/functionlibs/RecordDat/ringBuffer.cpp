#include "ringBuffer.h"


ringBuffer::ringBuffer(void)
{
}


ringBuffer::~ringBuffer(void)
{
}

RBHANDLE ringBuffer::createRingBuffer( unsigned int nBufferSize )
{
	tagRingBuffer * newBuffer;
	newBuffer = (tagRingBuffer *)malloc(sizeof(tagRingBuffer));
	if (NULL == newBuffer)
	{
		return NULL;
	}

	newBuffer->Buffer = (char *)malloc(nBufferSize);
	if (NULL == newBuffer->Buffer)
	{
		free(newBuffer);
		return NULL;
	}

	newBuffer->nBufferSize = nBufferSize;
	newBuffer->lpCur = newBuffer->Buffer;
	return newBuffer;
}

void ringBuffer::releaseRindBuffer( RBHANDLE hBuf )
{
	free(hBuf->Buffer);
	hBuf->Buffer = NULL;

	free(hBuf);
}

char * ringBuffer::getBuffer( RBHANDLE hBuf,unsigned int nSize )
{
	//ASSERT(hBuf,"");
	//ASSERT(hBuf->nBufferSize > nSize,"nSize(%d)",nSize);

	char *lpRet = NULL;
	if (hBuf->lpCur + nSize < hBuf->Buffer + hBuf->nBufferSize)
	{
		lpRet = hBuf->lpCur;
		hBuf->lpCur += nSize;
	}
	else
	{
		lpRet = hBuf->Buffer;
		hBuf->lpCur = hBuf->Buffer;
	}

	return lpRet;
}
