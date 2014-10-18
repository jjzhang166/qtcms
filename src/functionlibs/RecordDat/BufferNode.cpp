#include "BufferNode.h"


RecBufferNode::RecBufferNode(void):m_nRef(0),
	m_pFrameHead(NULL)
{
	m_nRef=1;
}


RecBufferNode::~RecBufferNode(void)
{
	m_tDataLock.lock();
	if (NULL!=m_pFrameHead)
	{
		free(m_pFrameHead);
		m_pFrameHead=NULL;
	}
	m_tDataLock.unlock();
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
		m_tDataLock.lock();
		if (NULL!=m_pFrameHead)
		{
			/*free(m_pFrameHead);*/
			m_pFrameHead=NULL;
		}else{
			//do nothing
		}
		m_tDataLock.unlock();
		delete this;
	}
	return nRet;
}

void RecBufferNode::setDataPointer( tagFrameHead *pFrameHead )
{
	m_tDataLock.lock();
	m_pFrameHead=pFrameHead;
	m_tDataLock.unlock();
}

void RecBufferNode::getDataPointer( tagFrameHead **pFrameHead )
{
	m_tDataLock.lock();
	*pFrameHead=m_pFrameHead;
	m_tDataLock.unlock();
}

int RecBufferNode::addRef()
{
	m_csRef.lock();
	m_nRef ++;
	m_csRef.unlock();
	return m_nRef;
}

