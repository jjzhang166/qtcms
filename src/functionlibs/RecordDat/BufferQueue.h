#pragma once
#include <QVariantMap>
#include "BufferNode.h"
#include "Allocation.h"
#include "recorddat_global.h"
#include <QDateTime>
#include <QQueue>
#include <QMutex>
class BufferQueue
{
public:
	BufferQueue();
	~BufferQueue();
public:
	bool enqueue(QVariantMap tFrameInfo);
	bool setSize(int nMax);
	bool isEmpty();
	void enqueueDataLock();
	void enqueueDataUnLock();
	void clear();
	RecBufferNode *dequeue();
	RecBufferNode *front();
private:
	QQueue<RecBufferNode*> m_tDataQueue;
	QMutex m_tDataLock;
	QMutex m_tEnqueueDataLock;
	int m_nQueueMaxSize;
	Allocation m_tAllocation;
};
