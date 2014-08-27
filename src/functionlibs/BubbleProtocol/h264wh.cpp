#include <math.h>
#include <stdio.h>
#include <string.h>
#include <qglobal.h>

typedef unsigned int UINT;
typedef unsigned char BYTE;
typedef unsigned int DWORD;

UINT Ue(BYTE *pBuff, UINT nLen, UINT &nStartBit)
{
	//计算0bit的个数
	UINT nZeroNum = 0;
	while (nStartBit < nLen * 8)
	{
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) //&:按位与，%取余
		{
			break;
		}
		nZeroNum++;
		nStartBit++;
	}
	nStartBit ++;


	//计算结果
	DWORD dwRet = 0;
	for (UINT i=0; i<nZeroNum; i++)
	{
		dwRet <<= 1;
		if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return (1 << nZeroNum) - 1 + dwRet;
}


int Se(BYTE *pBuff, UINT nLen, UINT &nStartBit)
{


	int UeVal=Ue(pBuff,nLen,nStartBit);
	double k=UeVal;
	int nValue=ceil(k/2);//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
	if (UeVal % 2==0)
		nValue=-nValue;
	return nValue;


}


DWORD u(UINT BitCount,BYTE * buf,UINT &nStartBit)
{
	DWORD dwRet = 0;
	for (UINT i=0; i<BitCount; i++)
	{
		dwRet <<= 1;
		if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
		{
			dwRet += 1;
		}
		nStartBit++;
	}
	return dwRet;
}

static const unsigned char default_scaling4[2][16]={
	{   6,13,20,28,
	13,20,28,32,
	20,28,32,37,
	28,32,37,42
	},{
		10,14,20,24,
			14,20,24,27,
			20,24,27,30,
			24,27,30,34
	}};

static const unsigned char default_scaling8[2][64]={
{   6,10,13,16,18,23,25,27,
10,11,16,18,23,25,27,29,
13,16,18,23,25,27,29,31,
16,18,23,25,27,29,31,33,
18,23,25,27,29,31,33,36,
23,25,27,29,31,33,36,38,
25,27,29,31,33,36,38,40,
27,29,31,33,36,38,40,42
},{
	9,13,15,17,19,21,22,24,
		13,13,17,19,21,22,24,25,
		15,17,19,21,22,24,25,27,
		17,19,21,22,24,25,27,28,
		19,21,22,24,25,27,28,30,
		21,22,24,25,27,28,30,32,
		22,24,25,27,28,30,32,33,
		24,25,27,28,30,32,33,35
}};

const unsigned char ff_zigzag_direct[64] = {
	0,   1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

static const unsigned char zigzag_scan[16+1] = {
	0 + 0 * 4, 1 + 0 * 4, 0 + 1 * 4, 0 + 2 * 4,
	1 + 1 * 4, 2 + 0 * 4, 3 + 0 * 4, 2 + 1 * 4,
	1 + 2 * 4, 0 + 3 * 4, 1 + 3 * 4, 2 + 2 * 4,
	3 + 1 * 4, 3 + 2 * 4, 2 + 3 * 4, 3 + 3 * 4,
};


static void decode_scaling_list(BYTE *buffer,unsigned int nLen,unsigned int & startbit, unsigned char *factors, int size,
	const unsigned char *jvt_list, const unsigned char *fallback_list){
		int i, last = 8, next = 8;
		const unsigned char *scan = size == 16 ? zigzag_scan : ff_zigzag_direct;
		if(!u(1,buffer,startbit)) /* matrix not written, we use the predicted one */
			memcpy(factors, fallback_list, size*sizeof(unsigned char));
		else
			for(i=0;i<size;i++){
				if(next)
					next = (last + Se(buffer,nLen,startbit)) & 0xff;
				if(!i && !next){ /* matrix not written, we use the preset one */
					memcpy(factors, jvt_list, size*sizeof(unsigned char));
					break;
				}
				last = factors[scan[i]] = next ? next : last;
            }
}


bool h264_decode_seq_parameter_set(BYTE * buf,UINT nLen,int *Width,int *Height)
{
	UINT StartBit=0; 
	int forbidden_zero_bit=u(1,buf,StartBit);
	int nal_ref_idc=u(2,buf,StartBit);
	int nal_unit_type=u(5,buf,StartBit);
    Q_UNUSED(forbidden_zero_bit);
    Q_UNUSED(nal_ref_idc);
	if(nal_unit_type==7)
	{
		int profile_idc=u(8,buf,StartBit);
		int constraint_set0_flag=u(1,buf,StartBit);//(buf[1] & 0x80)>>7;
		int constraint_set1_flag=u(1,buf,StartBit);//(buf[1] & 0x40)>>6;
		int constraint_set2_flag=u(1,buf,StartBit);//(buf[1] & 0x20)>>5;
		int constraint_set3_flag=u(1,buf,StartBit);//(buf[1] & 0x10)>>4;
		int constraint_set4_flag=u(1,buf,StartBit);
		int constraint_set5_flag=u(1,buf,StartBit);
		int reserved_zero_4bits=u(2,buf,StartBit);
		int level_idc=u(8,buf,StartBit);

		int seq_parameter_set_id=Ue(buf,nLen,StartBit);
        Q_UNUSED(constraint_set0_flag);
        Q_UNUSED(constraint_set1_flag);
        Q_UNUSED(constraint_set2_flag);
        Q_UNUSED(constraint_set3_flag);
        Q_UNUSED(constraint_set4_flag);
        Q_UNUSED(constraint_set5_flag);
        Q_UNUSED(reserved_zero_4bits);
        Q_UNUSED(level_idc);
        Q_UNUSED(seq_parameter_set_id);

		if( profile_idc == 100 || profile_idc == 110 ||
			profile_idc == 122 || profile_idc == 144 )
		{
            unsigned int chroma_format_idc=Ue(buf,nLen,StartBit);
			if (chroma_format_idc > 3U )
			{
				return false;
			}
			else if( chroma_format_idc == 3 )
			{
				int residual_colour_transform_flag=u(1,buf,StartBit);
				if (residual_colour_transform_flag)
				{
					return false;
				}
			}
            unsigned int bit_depth_luma_minus8=Ue(buf,nLen,StartBit) + 8;
            unsigned int bit_depth_chroma_minus8=Ue(buf,nLen,StartBit) + 8;
			if (bit_depth_luma_minus8 > 14U
				|| bit_depth_chroma_minus8 > 14U
				|| bit_depth_chroma_minus8 != bit_depth_luma_minus8)
			{
				return false;
			}
			int qpprime_y_zero_transform_bypass_flag=u(1,buf,StartBit);
            Q_UNUSED(qpprime_y_zero_transform_bypass_flag);
			int seq_scaling_matrix_present_flag=u(1,buf,StartBit);
			if (seq_scaling_matrix_present_flag)
			{
				const unsigned char * fallback[4] = {
					default_scaling4[0],
					default_scaling8[1],
					default_scaling4[0],
					default_scaling8[1]
				};

				unsigned char scaling_matrix4[6][16];
				unsigned char scaling_matrix8[6][64];

				decode_scaling_list(buf,nLen,StartBit,scaling_matrix4[0],16,default_scaling4[0],fallback[0]);
				decode_scaling_list(buf,nLen,StartBit,scaling_matrix4[1],16,default_scaling4[0],scaling_matrix4[0]); // Intra, Cr
				decode_scaling_list(buf,nLen,StartBit,scaling_matrix4[2],16,default_scaling4[0],scaling_matrix4[1]); // Intra, Cb
				decode_scaling_list(buf,nLen,StartBit,scaling_matrix4[3],16,default_scaling4[1],fallback[1]); // Inter, Y
				decode_scaling_list(buf,nLen,StartBit,scaling_matrix4[4],16,default_scaling4[1],scaling_matrix4[3]); // Inter, Cr
				decode_scaling_list(buf,nLen,StartBit,scaling_matrix4[5],16,default_scaling4[1],scaling_matrix4[4]); // Inter, Cb
				decode_scaling_list(buf,nLen,StartBit,scaling_matrix8[0],64,default_scaling8[0],fallback[2]);  // Intra, Y
				decode_scaling_list(buf,nLen,StartBit,scaling_matrix8[3],64,default_scaling8[1],fallback[3]);  // Inter, Y
				if (3 == chroma_format_idc)
				{
					decode_scaling_list(buf,nLen,StartBit,scaling_matrix8[1],64,default_scaling8[0],scaling_matrix8[0]);  // Intra, Cr
					decode_scaling_list(buf,nLen,StartBit,scaling_matrix8[4],64,default_scaling8[1],scaling_matrix8[3]);  // Inter, Cr
					decode_scaling_list(buf,nLen,StartBit,scaling_matrix8[2],64,default_scaling8[0],scaling_matrix8[1]);  // Intra, Cb
					decode_scaling_list(buf,nLen,StartBit,scaling_matrix8[5],64,default_scaling8[1],scaling_matrix8[4]);  // Inter, Cb
				}
			}
		}
		int log2_max_frame_num_minus4=Ue(buf,nLen,StartBit);
        Q_UNUSED(log2_max_frame_num_minus4);
		int pic_order_cnt_type=Ue(buf,nLen,StartBit);
		if( pic_order_cnt_type == 0 )
        {
			int log2_max_pic_order_cnt_lsb_minus4=Ue(buf,nLen,StartBit);
            Q_UNUSED(log2_max_pic_order_cnt_lsb_minus4);
        }
		else if( pic_order_cnt_type == 1 )
		{
			int delta_pic_order_always_zero_flag=u(1,buf,StartBit);
			int offset_for_non_ref_pic=Se(buf,nLen,StartBit);
			int offset_for_top_to_bottom_field=Se(buf,nLen,StartBit);
            Q_UNUSED(delta_pic_order_always_zero_flag);
            Q_UNUSED(offset_for_non_ref_pic);
            Q_UNUSED(offset_for_top_to_bottom_field);
			int num_ref_frames_in_pic_order_cnt_cycle=Ue(buf,nLen,StartBit);

			int *offset_for_ref_frame=new int[num_ref_frames_in_pic_order_cnt_cycle];
			for( int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
				offset_for_ref_frame[i]=Se(buf,nLen,StartBit);
			delete [] offset_for_ref_frame;
		}
		int num_ref_frames=Ue(buf,nLen,StartBit);
		int gaps_in_frame_num_value_allowed_flag=u(1,buf,StartBit);
        Q_UNUSED(num_ref_frames);
        Q_UNUSED(gaps_in_frame_num_value_allowed_flag);
        int pic_width_in_mbs_minus1=Ue(buf,nLen,StartBit);
		int pic_height_in_map_units_minus1=Ue(buf,nLen,StartBit);

		*Width=(pic_width_in_mbs_minus1+1)*16;
		*Height=(pic_height_in_map_units_minus1+1)*16;

		return true;
	}
	else
		return false;
}

bool isIFrame(char *stream,int stream_len)
{
	BYTE startcode4[4] = {0x00,0x00,0x00,0x01};
	BYTE startcode3[3] = {0x00,0x00,0x01};

	int i = 0;	
	while(i < stream_len-4 )
	{
		if(0 == memcmp((stream+i),startcode4,4))
		{
			UINT StartBit=0; 
			unsigned char * buf = (unsigned char *)stream +i +4;
			int forbidden_zero_bit=u(1,buf,StartBit);
			int nal_ref_idc=u(2,buf,StartBit);
            Q_UNUSED(forbidden_zero_bit);
            Q_UNUSED(nal_ref_idc);
            int nal_unit_type=u(5,buf,StartBit);
			if (5 == nal_unit_type)
			{
				return true;
			}

		}
		if(0 == memcmp((stream+i),startcode3,3))
		{

			UINT StartBit=0; 
			unsigned char * buf = (unsigned char *)stream +i +3;
			int forbidden_zero_bit=u(1,buf,StartBit);
			int nal_ref_idc=u(2,buf,StartBit);
            Q_UNUSED(forbidden_zero_bit);
            Q_UNUSED(nal_ref_idc);
            int nal_unit_type=u(5,buf,StartBit);
			if (5 == nal_unit_type)
			{
				return true;
			}		
		}
		i++;
	}	
	return false;
}

int GetWidthHeight(char *stream,int stream_len,int *width,int *height)
{
	BYTE startcode4[4] = {0x00,0x00,0x00,0x01};
	BYTE startcode3[3] = {0x00,0x00,0x01};

	int i = 0;	
	while(i < stream_len-4 )
	{
		if(0 == memcmp((stream+i),startcode4,4))
		{
			if (h264_decode_seq_parameter_set((unsigned char *)stream +i +4, stream_len, width,height))
			{			
				return 0;
			}		
		}
		if(0 == memcmp((stream+i),startcode3,3))
		{
			if (h264_decode_seq_parameter_set((unsigned char *)stream +i +3, stream_len, width,height))
			{			
				return 0;				
			}		
		}
		i++;
	}	
	return -1;
}

