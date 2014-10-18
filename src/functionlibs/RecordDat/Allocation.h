#pragma once
#include "ringBuffer.h"
class Allocation
{
public:
	Allocation(void);
	~Allocation(void);
public:
	void setSize(unsigned int uiSize);
	void *applySpace(unsigned int uiSize);
	void freeSpace(char *pChar);
private:
	unsigned int m_uiSize;
	ringBuffer m_tRingBuffer;
	tagRingBuffer * m_pBuffer;
};

