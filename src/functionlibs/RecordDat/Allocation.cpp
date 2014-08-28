#include "Allocation.h"
#include<stdlib.h>

Allocation::Allocation(void)
{
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
	return malloc(uiSize);
}
