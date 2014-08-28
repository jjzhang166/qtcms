#include "BufferNode.h"


RecBufferNode::RecBufferNode(void):m_nRef(0),
	m_pFrameHead(NULL)
{
	m_nRef=1;
}


RecBufferNode::~RecBufferNode(void)
{
}

int  RecBufferNode::release()
{
	int nRet = 0;
	m_csRef.lock();
	m_nRef -- ;
	nRet = m_nRef;
	m_csRef.unlock();
	if (0 == nRet)
	{
		if (NULL!=m_pFrameHead)
		{
			delete m_pFrameHead;
			m_pFrameHead=NULL;
		}else{
			//do nothing
		}
		delete this;
	}
	return nRet;
}

void RecBufferNode::setDataPointer( tagFrameHead *pFrameHead )
{
	m_pFrameHead=pFrameHead;
}

void RecBufferNode::getDataPointer( tagFrameHead **pFrameHead )
{
	*pFrameHead=m_pFrameHead;
	addRef();
}

int RecBufferNode::addRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

