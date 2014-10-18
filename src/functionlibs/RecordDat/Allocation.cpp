#include "Allocation.h"
#include<stdlib.h>

Allocation::Allocation(void):m_pBuffer(NULL)
{
	m_pBuffer=m_tRingBuffer.createRingBuffer(CHLBUFFERSIZE*1024*1024);
}


Allocation::~Allocation(void)
{
	if (m_pBuffer!=NULL)
	{
		m_tRingBuffer.releaseRindBuffer(m_pBuffer);
	}else{

	}
}

void Allocation::setSize(unsigned int uiSize)
{
	m_uiSize=uiSize;
}

void * Allocation::applySpace( unsigned int uiSize )
{
	return (void*)m_tRingBuffer.getBuffer(m_pBuffer,uiSize);
	//return malloc(uiSize);
}

void Allocation::freeSpace( char *pChar )
{
	//if (pChar!=NULL)
	//{
	//	free(pChar);
	//}
	return;
}
