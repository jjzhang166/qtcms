#pragma once
#include <QVariantMap>
#include "BufferNode.h"
#include "Allocation.h"
#include "recorddat_global.h"
#include <QDateTime>
#include <QQueue>
#include <QMutex>
#include <QEventLoop>
#include <QTimer>
#include <QThread>
#define MAXFRAMERATE 30 //最大帧率
#define FRAMERATERESTARTTIME  79200000//计时器 重启时间
class BufferQueue:public QThread
{
	Q_OBJECT
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
	void setRecordStatus(int nStatus);
	int getSize();
private:
	QQueue<RecBufferNode*> m_tDataQueue;
	QMutex m_tDataLock;
	QMutex m_tEnqueueDataLock;
	int m_nQueueMaxSize;
	Allocation m_tAllocation;
	int m_nRecordStatus;
	int m_nLoseFrameCount;
	volatile unsigned int m_uiAvailableSize;
};

