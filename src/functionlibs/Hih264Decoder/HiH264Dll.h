#pragma once
#include <QLibrary>

#define HiH264_API

typedef struct _tagHiH264_UserData
{
	unsigned long  uUserDataType;   //Type of userdata
	unsigned long  uUserDataSize;   //Length of userdata in byte
	unsigned char* pData;           //Buffer contains userdata stuff
	struct _tagHiH264_UserData* pNext;    //Pointer to next userdata
} HiH264_UserData;


typedef struct _tagHiH264DecCreInfo
{
	unsigned long  uPictureFormat;       //Decoded output picture format 0x00:YUV420 0x01:YUV422 0x02:YUV444
	unsigned long  uStreamInType;        //Input stream type
	unsigned long  uPicWidthInMB;        //The width of picture in MB
	unsigned long  uPicHeightInMB;       //The height of picture in MB
	unsigned long  uBufNum;              //Max reference frame num 
	unsigned long  uWorkMode;            //Decoder working mode 
	HiH264_UserData  *pUserData;  //Buffer contains userdata stuff
	unsigned long  uReserved;
} HiH264DecCreInfo;

typedef struct _tagHiH264DecOouputInfo
{
	unsigned long uPicBytes;            //total bytes of one frame
	unsigned long uI4MbNum;             //number of I4x4 macroblocks in one frame
	unsigned long uI8MbNum;             //number of I8x8 macroblocks in one frame
	unsigned long uI16MbNum;            //number of I16x16 macroblocks in one frame
	unsigned long uP16MbNum;            //number of P16x16 macroblocks in one frame
	unsigned long uP16x8MbNum;          //number of P16x8 macroblocks in one frame
	unsigned long uP8x16MbNum;          //number of P8x16 macroblocks in one frame
	unsigned long uP8MbNum;             //number of P8x8 macroblocks in one frame
	unsigned long uPskipMbNum;          //number of PSkip macroblocks in one frame
	unsigned long uIpcmMbNum;           //number of IPCM macroblocks in one frame
} HiH264DecOouputInfo;

typedef struct _tagHiH264DecFrame
{
	unsigned char*  pY;                   //Y plane base address of the picture
	unsigned char*  pU;                   //U plane base address of the picture
	unsigned char*  pV;                   //V plane base address of the picture
	unsigned long  uWidth;               //The width of output picture in pixel
	unsigned long  uHeight;              //The height of output picture in pixel
	unsigned long  uYStride;             //Luma plane stride in pixel
	unsigned long  uUVStride;            //Chroma plane stride in pixel
	unsigned long  uCroppingLeftOffset;  //Crop information in pixel
	unsigned long  uCroppingRightOffset; 
	unsigned long  uCroppingTopOffset;   
	unsigned long  uCroppingBottomOffset;
	unsigned long  uDpbIdx;              //The index of dpb
	unsigned long  uPicFlag;             //0: Frame; 1: Top filed; 2: Bottom field  
	unsigned long  bError;               //0: picture is correct; 1: picture is corrupted
	unsigned long  bIntra;               //0: intra picture; 1:inter picture
	unsigned long long  ullPTS;               //Time stamp
	unsigned long  uPictureID;           //The sequence ID of this output picture decoded
	unsigned long  uReserved;            //Reserved for future
	HiH264_UserData *pUserData;   //Pointer to the first userdata
	HiH264DecOouputInfo *pFrameInfo; //Pointer to the output information of one frame
}HiH264DecFrame;


typedef HiH264_API void* (*lpHi264DecCreate)(HiH264DecCreInfo *pDecAttr);
typedef HiH264_API void (*lpHi264DecDestroy)(void *hDec);
typedef HiH264_API long (*lpHi264DecFrame)(void* hDec,unsigned char* pStream, unsigned long iStreamLen, unsigned long long ullPTS, HiH264DecFrame* pDecFrame, unsigned long uFlags);

class HiH264Dll
{
public:
	HiH264Dll(void);
	~HiH264Dll(void);

	lpHi264DecCreate DecCreate;
	lpHi264DecDestroy DecDestroy;
	lpHi264DecFrame DecFrame;
private:
	QLibrary mylib;

};