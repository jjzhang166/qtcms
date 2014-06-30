#pragma once
class CDDMutex
{
public:
	CDDMutex(void);
	~CDDMutex(void);

	void Lock();
	void Unlock();
private:
	CRITICAL_SECTION m_cs;
};

