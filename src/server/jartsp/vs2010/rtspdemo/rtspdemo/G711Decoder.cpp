#include "StdAfx.h"
#include "G711Decoder.h"
#include "g711.h"

CG711Decoder::CG711Decoder()
{

}

CG711Decoder::~CG711Decoder()
{

}

int CG711Decoder::Decode(char *OutPutBuffer,char *InputBuffer,int nInputBufferSize)
{
	return g711a_decode((short *)OutPutBuffer,(unsigned char *)InputBuffer,nInputBufferSize);
}