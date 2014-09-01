#pragma once
#include <QMutex>
#include "recorddat_global.h"
class RecBufferNode
{
public:
	RecBufferNode(void);
	~RecBufferNode(void);
public:
	int release();
	void setDataPointer(tagFrameHead *pFrameHead);
	void getDataPointer(tagFrameHead **pFrameHead);
	int addRef();
private:
	QMutex m_csRef;
	QMutex m_tDataLock;
	int m_nRef;
	tagFrameHead *m_pFrameHead;
};

