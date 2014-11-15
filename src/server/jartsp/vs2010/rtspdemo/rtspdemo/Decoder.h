#pragma once

class CDecoder
{
public:
	CDecoder(void);
    virtual BOOL InitDec()=0;
	virtual BOOL InputDate(LPVOID pDate,int nDateLen)=0;
	virtual BOOL Flush()=0;


public:
	~CDecoder(void);
};
