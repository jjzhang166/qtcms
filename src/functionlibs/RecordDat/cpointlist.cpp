#include "cpointlist.h"
#include <string.h>
#include <stdlib.h>

CPointList::CPointList() :
m_nodes(NULL)
,m_nCount(0)
,m_nSize(0)
,m_nLastRetriveNode(0)
{
}

CPointList::~CPointList()
{
    if(NULL != m_nodes)
    {
        free(m_nodes);
    }
}

void *CPointList::front()
{
    if(0 == m_nSize) return NULL;

    return m_nodes[m_nHeadIndex].pNode;
}

void *CPointList::dequeue()
{
    if(0 == m_nSize) return NULL;

    void * pRet = m_nodes[m_nHeadIndex].pNode;

    // 从链表中移除
	int nCurNodeIndex = m_nHeadIndex;
	m_nHeadIndex = m_nodes[m_nHeadIndex].NextIndex;
    if(-1 != m_nHeadIndex)
    {
		m_nodes[m_nHeadIndex].PreIndex = -1;
    }

    // 标记为未用
	m_nodes[nCurNodeIndex].bUesed = false;

    m_nSize --;
    return pRet;
}

void CPointList::enqueue(void *p)
{
    // 空间不足，按照1.2倍扩充空间
    if(m_nSize == m_nCount)
    {
        enlargeBuffer();
    }

    // 请求节点，如果空间不够则扩充，直到能请求到节点
    Node * newNode;
    while (NULL == (newNode = invalidaNode()))
    {
        enlargeBuffer();
    }

    // 插入队列
    if(0 == m_nSize) // 空队列
    {
        newNode->pNode = p;
        newNode->PreIndex = -1;
        newNode->NextIndex = -1;
		m_nLastIndex = m_nHeadIndex = nodeIndex(newNode);
    }
    else
    {
        newNode->pNode = p;
        newNode->PreIndex = m_nLastIndex;
        newNode->NextIndex = -1;
		m_nodes[m_nLastIndex].NextIndex = nodeIndex(newNode);
		m_nLastIndex = nodeIndex(newNode);
    }

    m_nSize ++;
}

bool CPointList::isEmpty()
{
    return m_nSize > 0 ? false : true;
}

int CPointList::size() const
{
    return m_nSize;
}

void CPointList::prealloc(int count)
{
    m_nodes = (Node*)malloc(count * sizeof(Node));
    memset(m_nodes,0,sizeof(Node) * count);
    m_nCount = count;
}

CPointList::Node *CPointList::invalidaNode()
{
    int i;
    bool bRetrive = false;
    int nRetrivedIndex = 0;
    for (i = 0; i < m_nCount; i ++) // 全部搜索
    {
        if(!m_nodes[m_nLastRetriveNode].bUesed)
        {
            bRetrive = true;
            nRetrivedIndex = m_nLastRetriveNode;
        }
        m_nLastRetriveNode ++;
        m_nLastRetriveNode %= m_nCount;
        if(bRetrive) break;
    }

    if (!bRetrive) // 未搜索到，返回空指针
    {
        return NULL;
    }

    m_nodes[nRetrivedIndex].bUesed = true;
    return &m_nodes[nRetrivedIndex];
}

inline void CPointList::enlargeBuffer()
{
    int nNewCount = m_nCount + m_nCount/5;
    m_nodes = (Node *)realloc(m_nodes,nNewCount * sizeof(Node));
    // 新请求的节点进行初始化
    memset(m_nodes + m_nCount,0,sizeof(Node) * (nNewCount - m_nCount));
    m_nCount = nNewCount;
}

int CPointList::nodeIndex( Node * pNode )
{
	return ((char *)pNode - (char *)m_nodes) / sizeof(Node);
}
