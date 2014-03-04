#ifndef __INTERFACE_IAUDIODECODER_HEAD_FILE_NDSSDTUJNJGF__
#define __INTERFACE_IAUDIODECODER_HEAD_FILE_NDSSDTUJNJGF__
#include <libpcom.h>

interface IAudioDecoder : public IPComBase
{
	virtual int Decode(char *OutPutBuffer,char *InputBuffer,int nInputBufferSize) = 0;
};

#endif