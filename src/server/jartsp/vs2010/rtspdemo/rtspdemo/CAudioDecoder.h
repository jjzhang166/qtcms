#pragma once
#ifndef __AUDIODECODER_CLASS_HEAD_FILE__
#define __AUDIODECODER_CLASS_HEAD_FILE__

class CAudioDecoder
{
public:
	CAudioDecoder();
	virtual ~CAudioDecoder();

public:
	virtual int Decode(char *OutPutBuffer,char *InputBuffer,int nInputBufferSize) = 0;

protected:
private:
};


#endif