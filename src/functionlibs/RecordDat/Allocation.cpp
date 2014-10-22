#include "Allocation.h"
#include<stdlib.h>

Allocation::Allocation(void)
{
	m_tRingBuffer.createRingBuffer(CHLBUFFERSIZE*1024*1024);
}


Allocation::~Allocation(void)
{
}

void Allocation::setSize(unsigned int uiSize)
{
	m_uiSize=uiSize;
}

void * Allocation::applySpace( unsigned int uiSize )
{
	return (void*)m_tRingBuffer.getBuffer(uiSize);
	//return malloc(uiSize);
}

void Allocation::freeSpace( char *pChar )
{
	return;
}
