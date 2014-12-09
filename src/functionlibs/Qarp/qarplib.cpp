#include "qarplib.h"
#include "qarp.h"
#include <QString>
bool qsendarp(unsigned long dstip)
{
	Qarp arp;
	return arp.qsendarp(dstip);
}
bool qsendarpEx(unsigned long dstip,char &pMac)
{
	Qarp arp;
	return arp.qsendarp(dstip,pMac);
}