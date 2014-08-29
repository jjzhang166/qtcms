#pragma once
class Allocation
{
public:
	Allocation(void);
	~Allocation(void);
public:
	void setSize(unsigned int uiSize);
	void *applySpace(unsigned int uiSize);
	void freeSpace(char *pChar);
private:
	unsigned int m_uiSize;
};

