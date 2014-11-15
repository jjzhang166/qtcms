#ifndef __G711DECODER_HEADER_FILE__
#define __G711DECODER_HEADER_FILE__

#include "CAudioDecoder.h"

class CG711Decoder:public CAudioDecoder 
{
public:
	CG711Decoder();
	virtual ~CG711Decoder();

public:
	int Decode(char *OutPutBuffer,char *InputBuffer,int nInputBufferSize);
protected:
private:
};

#endif