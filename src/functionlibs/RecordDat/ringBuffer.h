#pragma once
#include "recorddat_global.h"
class ringBuffer
{
public:
	ringBuffer(void);
	~ringBuffer(void);
public:
	void createRingBuffer(unsigned int nBufferSize);
	char *getBuffer(unsigned int nSize);

private:
	char * m_Buffer;
	unsigned int m_nBufferSize;
	char * m_lpCur;
};

