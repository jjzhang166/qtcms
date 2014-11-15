
/*
This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 */ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packbit.h"

/********************************************************************************
PackBits is a fast, simple lossless compression scheme for run-length encoding of data.
A PackBits data stream consists of packets with a one-byte header followed by data. 
The header is a signed byte; the data can be signed, unsigned, or packed (such as MacPaint pixels).
In the following table, n is the value of the header byte as a signed integer.

_________________________________________________________________________________
Header byte  	|	Data following the header byte
_________________________________________________________________________________
0 to 127			|	(1 + n)  literal bytes of data
_________________________________________________________________________________
-1 to -127		|	One byte of data, repeated (1 ¨C n) times in the decompressed output
_________________________________________________________________________________
-128			|	No operation (skip and treat next byte as a header byte)
_________________________________________________________________________________

for example :
packbit coded bytes : 'FE AA 02 80 00 2A FD AA 03 80 00 2A 22 F7 AA'
unpack : 'AA AA AA 80 00 2A AA AA AA AA 80 00 2A 22 AA AA AA AA AA AA AA AA AA AA'

*********************************************************************************/
 
// Assuming compressor logic is maximally efficient,
// worst case input with no duplicate runs of 3 or more bytes
// will be compressed into a series of verbatim runs no longer
// than 128 bytes, each preceded by length byte.
// i.e. worst case output length is not more than 129*ceil(n/128)
// or slightly tighter, 129*floor(n/128) + 1 + (n%128)

unsigned int PACKBITS_encode(unsigned char *src, unsigned char *dst, unsigned int n)
{
	unsigned char *p, *q, *run, *dataend;
	int count, maxrun;

	dataend = src + n;
	for( run = src, q = dst; n > 0; run = p, n -= count ){
		// A run cannot be longer than 128 bytes.
		maxrun = n < 128 ? n : 128;
		if(run <= (dataend-3) && run[1] == run[0] && run[2] == run[0]){
			// 'run' points to at least three duplicated values.
			// Step forward until run length limit, end of input,
			// or a non matching byte:
			for( p = run+3; p < (run+maxrun) && *p == run[0]; )
				++p;
			count = p - run;

			// replace this run in output with two bytes:
			*q++ = 1+256-count; /* flag byte, which encodes count (129..254) */

			*q++ = run[0];      /* byte value that is duplicated */

		}else{
			// If the input doesn't begin with at least 3 duplicated values,
			// then copy the input block, up to the run length limit,
			// end of input, or until we see three duplicated values:
			for( p = run; p < (run+maxrun); )
				if(p <= (dataend-3) && p[1] == p[0] && p[2] == p[0])
					break; // 3 bytes repeated end verbatim run
				else
					++p;
			count = p - run;
			*q++ = count-1;        /* flag byte, which encodes count (0..127) */
			memcpy(q, run, count); /* followed by the bytes in the run */
			q += count;
		}
	}
	return q - dst;
}   


unsigned int PACKBITS_decode(unsigned char *outp, unsigned char *inp,
			unsigned int outlen, unsigned int inlen)
{
	unsigned int i, len;
	int val;

	/* i counts output bytes; outlen = expected output size */
	for(i = 0; inlen > 1 && i < outlen;){
		/* get flag byte */
		len = *inp++;
		--inlen;

		if(len == 128) /* ignore this flag value */
			; // warn_msg("RLE flag byte=128 ignored");
		else{
			if(len > 128){
				len = 1+256-len;

				/* get value to repeat */
				val = *inp++;
				--inlen;

				if((i+len) <= outlen)
					memset(outp, val, len);
				else{
					memset(outp, val, outlen-i); // fill enough to complete row
					printf("unpacked RLE data would overflow row (run)\n");
					len = 0; // effectively ignore this run, probably corrupt flag byte
				}
			}else{
				++len;
				if((i+len) <= outlen){
					if(len > inlen)
						break; // abort - ran out of input data
					/* copy verbatim run */
					memcpy(outp, inp, len);
					inp += len;
					inlen -= len;
				}else{
					memcpy(outp, inp, outlen-i); // copy enough to complete row
					printf("unpacked RLE data would overflow row (copy)\n");
					len = 0; // effectively ignore
				}
			}
			outp += len;
			i += len;
		}
	}
	if(i < outlen)
		printf("not enough RLE data for row %d/%d\n", i, outlen);
	return i;
}

#ifdef PACKBIT_TEST
int main()
{
	//packbit coded bytes : 'FE AA 02 80 00 2A FD AA 03 80 00 2A 22 F7 AA'
	//unpack : 'AA AA AA 80 00 2A AA AA AA AA 80 00 2A 22 AA AA AA AA AA AA AA AA AA AA'
	int i, ret;
	unsigned char input[15] = {0xfe, 0xaa, 0x02, 0x80, 0x00, 0x2a, 0xfd, 0xaa, 0x03, 0x80, 0x00, 0x2a, 0x22, 0xf7, 0xaa};
	unsigned char expected_output[] = {
		0xAA, 0xAA, 0xAA, 0x80, 0x00, 0x2A, 0xAA, 0xAA, 0xAA, 0xAA, 0x80, 0x00, 0x2A, 0x22, 
		0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
	unsigned char output[128], temp[128];

	ret = PACKBITS_decode(output, input, sizeof(output), sizeof(input));
	if ( (ret  == sizeof(expected_output))
		&& (memcmp(output, expected_output, sizeof(expected_output))== 0)) {
		printf("packbit decode check pass!\n");
	} else {
		printf("packbit decode check failed %d/%d!\n", ret, sizeof(expected_output));
		for (i = 0; i < ret; i++) {
			printf("%02X ",output[i]);
		}
		printf("\r\n");
		return -1;
	}

	ret = PACKBITS_encode(output, temp, ret);
	if ( ret  ==sizeof(input)
		&& memcmp(temp, input, sizeof(input))== 0) {
		printf("packbit encode check pass!\n");
	} else {
		printf("packbit encode check failed %d/%d!\n", ret, sizeof(input));
		for (i = 0; i < ret; i++) {
			printf("%02X ", temp[i]);
		}
		printf("\r\n");
		return -1;
	}

	return 0;
}
#endif

