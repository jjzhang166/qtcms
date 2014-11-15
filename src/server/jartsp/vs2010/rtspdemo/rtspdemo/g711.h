#ifndef __G711_HEADER_FILE__
#define __G711_HEADER_FILE__

static int alaw2linear(unsigned char a_val);

int g711a_decode(short amp[], const unsigned char g711a_data[], int g711a_bytes);

#endif