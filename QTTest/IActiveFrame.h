#pragma once
#include <QWebFrame>

typedef enum _enActiveIns{
	AI_PREVIEW,
	AI_CNT,
}ActiveIns;

class IActiveFrame
{
public:
	virtual ~IActiveFrame(){};
public:
	virtual void Active(QWebFrame *Frame) = 0;
};

int CreatInstance(ActiveIns active,void ** ppv);

