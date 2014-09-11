#include "mdworkobject.h"
#include "bubbleprotocolex.h"

MDWorkObject::MDWorkObject(QObject *parent)
	: QObject(parent)
{

}

MDWorkObject::~MDWorkObject()
{

}

void MDWorkObject::motionDetectionEcho()
{
	BubbleProtocolEx * pParent = (BubbleProtocolEx *)parent();
	if (NULL != pParent)
	{
		pParent->motionDetectionEcho();
	}
}
