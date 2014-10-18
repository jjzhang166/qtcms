#pragma once
#include "recorddat_global.h"
class ringBuffer
{
public:
	ringBuffer(void);
	~ringBuffer(void);
public:
	RBHANDLE createRingBuffer(unsigned int nBufferSize);
	void releaseRindBuffer(RBHANDLE hBuf);
	char *getBuffer(RBHANDLE hBuf,unsigned int nSize);
};

