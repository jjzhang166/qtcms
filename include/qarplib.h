#pragma once


extern "C"{
	bool qsendarp(unsigned long dstip);
	bool qsendarpEx(unsigned long dstip,char &pMac);
};