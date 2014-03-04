#ifndef VEDIODEVICE_GLOBAL_H
#define VEDIODEVICE_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QList>

#define WAVEDEV_BLOCK_SIZE      1024
#define WAVEDEV_BLOCK_COUNT		20
#define WHDR_PREPARED 0x00000002

typedef struct _tagBufferListNode{
	char * pBuffer;
	int    nBufferSize;
}BufferListNode;

typedef QList<BufferListNode> BufferList;

#endif // VEDIODEVICE_GLOBAL_H
