#pragma once
#include <QString>
#include <string.h>
#include <QByteArray>
#include <QDebug>
class byteArraySpace
{
public:
	byteArraySpace(void);
	~byteArraySpace(void);
public:
	char *getEndLocation(int &nResidueSize);
	void addDataLenth(int nSize);
	void setSize(int nSize);
	bool contains(const char * str);
	int size();
	bool startsWith( char * str);
	char *data();
	void remove(int nPos,int len);
	bool clear();
	int indexOf(char *str);
	QString mid(int nPos,int nLen);
private:

private:
	int m_nTotalSize;
	int m_nDataLength;
	char *m_pMemoryHead;
	char *m_pDataHead;
};

