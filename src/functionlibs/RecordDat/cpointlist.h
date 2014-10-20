#ifndef CPOINTLIST_H
#define CPOINTLIST_H

class CPointList
{
public:
    CPointList();
    ~CPointList();

	// 由于队列内在大小不够的情况下会扩充，可能有重新分配的情况发生，因此必须用下标，而无法直接使用指针
    typedef struct _tagNode{
        void * pNode;
		int PreIndex; // 上个节点在节点缓冲数组中的下标
        int NextIndex; // 下个节点在节点缓冲数组中的下标
        bool bUesed;
    }Node;

// methods
public:
    void * front();

    void * dequeue();

    void enqueue(void * p);

    bool isEmpty();

    int size() const;

    void prealloc(int count);

private:
    Node * invalidaNode(); // 从缓冲中拿一个可用的节点

    inline void enlargeBuffer(); // 预分配的buffer不足时，扩充空间

	inline int nodeIndex(Node * pNode); // 计算节点的下标

private:
    Node * m_nodes; // 节点缓冲数组，采用预分配
    int m_nHeadIndex;
    int m_nLastIndex;
    int m_nSize; // 队列大小
    int m_nCount; // 预分配缓冲区大小
    int m_nLastRetriveNode; // 最近搜索的缓冲节点索引，从该节点开始往后搜索，优化搜索时间
};

#endif // CPOINTLIST_H
