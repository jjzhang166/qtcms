#ifndef QARP_H
#define QARP_H

#include "qarp_global.h"
#include <QDebug>
class QARP_EXPORT Qarp
{
public:
	Qarp();
	~Qarp();
public:
	bool qsendarp(unsigned long dstip);
	bool qsendarp(unsigned long dstip,char &pMAC);
private:

};

#endif // QARP_H
