#include "BufferQueue.h"
#include <QDebug>

BufferQueue::BufferQueue():m_nQueueMaxSize(120)
{
}


BufferQueue::~BufferQueue()
{
}

bool BufferQueue::enqueue( QVariantMap tFrameInfo )
{
	m_tEnqueueDataLock.lock();
	m_tDataLock.lock();
	if (m_tDataQueue.size()>m_nQueueMaxSize)
	{
		RecBufferNode *pRecBufferNode=NULL;
		pRecBufferNode=m_tDataQueue.dequeue();
		if (NULL!=pRecBufferNode)
		{
			pRecBufferNode->release();
			pRecBufferNode=NULL;
		}else{
			//do nothing
		}
	}else{
		//keep going
	}
	int nDataLength=tFrameInfo["length"].toInt();
	int nFrameHeadLength=sizeof(tagFrameHead);
	int nApplyLength=nDataLength+nFrameHeadLength-sizeof(char*);
	if (tFrameInfo["frametype"].toUInt()==IFRAME)
	{
		nApplyLength=nApplyLength+sizeof(tagVideoConfigFrame);
	}else if (tFrameInfo["frametype"].toUInt()==AFRMAE)
	{
		nApplyLength=nApplyLength+sizeof(tagAudioConfigFrame);
	}else{
		//do nothing
	}
	RecBufferNode tRecBufferNode;
	tagFrameHead *pFrameHead=NULL;
	pFrameHead=(tagFrameHead*)m_tAllocation.applySpace(nApplyLength);
	if (pFrameHead!=NULL)
	{
		pFrameHead->uiChannel=tFrameInfo["winid"].toUInt();
		pFrameHead->uiExtension=0;//
		pFrameHead->uiGentime=QDateTime::currentDateTime().toTime_t();
		pFrameHead->uiLength=nDataLength;
		pFrameHead->uiPts=tFrameInfo["pts"].toULongLong()/1000;
		pFrameHead->uiRecType=0;//
		pFrameHead->uiSessionId=0;//
		pFrameHead->uiType=tFrameInfo["frametype"].toUInt();
		char *pData=(char*)tFrameInfo["data"].toUInt();
		memcpy(pFrameHead->pBuffer,pData,nDataLength);
		if (pFrameHead->uiType==AFRMAE)
		{
			tagVideoConfigFrame *pVideoConfigFrame=(tagVideoConfigFrame*)(pFrameHead+sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(char*)+1);
			pVideoConfigFrame->uiHeight=tFrameInfo["height"].toInt();
			pVideoConfigFrame->uiWidth=tFrameInfo["width"].toInt();
			pVideoConfigFrame->ucReversed;//δ֪
			pVideoConfigFrame->ucVideoDec;//δ֪
		}else if (pFrameHead->uiType==PFRAME)
		{
			tagAudioConfigFrame *pAudioConfigFrame=(tagAudioConfigFrame*)(pFrameHead+sizeof(tagFrameHead)+pFrameHead->uiLength-sizeof(char*)+1);
			pAudioConfigFrame->uiSamplebit=tFrameInfo["samplewidth"].toInt();
			pAudioConfigFrame->uiSamplerate
		}else{
			//do nothing
		}
		tRecBufferNode.setDataPointer(pFrameHead);
		m_tDataQueue.enqueue(&tRecBufferNode);
		m_tDataLock.unlock();
		m_tEnqueueDataLock.unlock();
		return true;
	}else{
		qDebug()<<__FUNCTION__<<__LINE__<<"enqueue fail as allocation fail,there may be run out of memory";
		abort();
	}
	m_tDataLock.unlock();
	m_tEnqueueDataLock.unlock();
	return false;
}

bool BufferQueue::setSize( int nMax )
{
	return false;
}

bool BufferQueue::isEmpty()
{
	m_tDataLock.lock();
	if (m_tDataQueue.isEmpty())
	{
		m_tDataLock.unlock();
		return true;
	}else{
		m_tDataLock.unlock();
		return false;
	}
}

void BufferQueue::clear()
{
	m_tDataLock.lock();
	while(!m_tDataQueue.isEmpty()){
		RecBufferNode *pRecBufferNode=NULL;
		pRecBufferNode=m_tDataQueue.dequeue();
		if (NULL!=pRecBufferNode)
		{
			pRecBufferNode->release();
			pRecBufferNode=NULL;
		}else{
			//do nothing
		}
	}
	m_tDataLock.unlock();
}

RecBufferNode* BufferQueue::dequeue()
{
	RecBufferNode* pRecBufferNode=NULL;
	m_tDataLock.lock();
	if (m_tDataQueue.size()>0)
	{
		pRecBufferNode=m_tDataQueue.dequeue();
	}else{

	}
	m_tDataLock.unlock();
	return pRecBufferNode;
}

RecBufferNode * BufferQueue::front()
{
	RecBufferNode* pRecBufferNode=NULL;
	m_tDataLock.lock();
	if (m_tDataQueue.size()>0)
	{
		pRecBufferNode=m_tDataQueue.front();
		pRecBufferNode->addRef();
	}else{

	}
	m_tDataLock.unlock();
	return pRecBufferNode;
}

void BufferQueue::enqueueDataLock()
{
	m_tEnqueueDataLock.lock();
}

void BufferQueue::enqueueDataUnLock()
{
	m_tEnqueueDataLock.unlock();
}

