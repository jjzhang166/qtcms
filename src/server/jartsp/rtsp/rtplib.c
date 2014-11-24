/******************************************************************************

  Copyright (C), 2013-2020, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : rtplib.c
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2013/04/25
  Last Modified : 2013/04/25
  Description   : A transport protocal for real time application  utils , reference to rtp( rfc3550)
  	rtp payload format for H264 ( rfc3984),
  	rtp payload format for g711(rfc5391)
 
  History       : 
  1.Date        : 2013/04/25
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sock.h"

#include "rtpbuf.h"
#include "rtplib.h"
#include "vlog.h"
#include "portmanage.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int 	uint32_t;

#define RTP_RET_FAIL	(-1)
#define RTP_RET_OK		(0)

#define RTP_CLIENT 0
#define RTP_SERVER 1


/*
int rtp_init_transport(int cast_type,int protocal,int chn_port)
{
	int sock=-1;
	if(protocal == RTP_TRANSPORT_UDP && cast_type==RTP_UNICAST){
		sock=SOCK_udp_init(NULL, chn_port,RTSP_SOCK_TIMEOUT);
	}else if(protocal == RTP_TRANSPORT_TCP && cast_type==RTP_UNICAST){
		VLOG(VLOG_DEBUG,"rtsp over tcp");
	}else if(protocal == RTP_TRANSPORT_UDP && cast_type==RTP_MULTICAST){
		VLOG(VLOG_ERROR,"unsupport transport: %s,%s",(protocal == RTP_TRANSPORT_TCP) ? "tcp" : "udp",
			(cast_type==RTP_MULTICAST) ? "multicast" : "unicast");
	}else{
		VLOG(VLOG_ERROR,"unsupport transport: %s,%s",(protocal == RTP_TRANSPORT_TCP) ? "tcp" : "udp",
			(cast_type==RTP_MULTICAST) ? "multicast" : "unicast");
	}
	return sock;
}
*/

Rtp_t *RTP_client_new(int protocal,/* udp or tcp */
	int interleaved, /* TRUE or FALSE */
	int sock,
	int payloadType,
	int real_type,
	char *dstip,
	int dstport,
	int buffer_time)
{
	Rtp_t *rtp=NULL;
	RtpPacket_t *p=NULL;
	RtpHeader_t *rtpHeader=NULL;
	RtspInterHeader_t *inHeader=NULL;

	rtp=(Rtp_t *)malloc(sizeof(Rtp_t));
	if(rtp == NULL){
		VLOG(VLOG_ERROR,"malloc rtp failed");
		return NULL;
	}
	memset(rtp,0,sizeof(rtp));
	p=&rtp->packet;
	rtpHeader=&rtp->header;
	inHeader=&rtp->interHeader;
	
	rtp->role = RTP_CLIENT;
	rtp->buffertime = buffer_time;
	rtp->timestamp = 0;
	rtp->seq = 0;
	rtp->raw_data = FALSE;
	rtp->protocal = protocal;
	rtp->interleaved = interleaved;
	rtp->payloadType = payloadType;
	rtp->mediaType =real_type;
	//
	rtp->base_seq = 0;
	rtp->cycle_cnt = 0;
	rtp->packet_cnt = 0;
	rtp->octet_cnt = 0;
	rtp->fraction_lost = 0;
	rtp->comulative_lost = 0;
	// init for packets' buffer
	p->cnt = 0;
	p->malloc_size = 0;
	p->buffer = NULL;
	p->buf_size[0] = 0;
	p->iFrameCnt = 0;
	p->trunk_readed = 0;
		
	memset(rtpHeader,0,sizeof(RtpHeader_t));
	memset(inHeader,0,sizeof(RtspInterHeader_t));
	// init transport
	rtp->sock = sock;
	strcpy(rtp->peername,dstip);
	rtp->peer_chn_port=dstport;

	VLOG(VLOG_CRIT,"[CLIENT]rtp over %s init done, type %d/%d,dst %s:%d sock:%d",
		interleaved ? "RTSP" : "UDP", rtp->payloadType, rtp->mediaType ,rtp->peername,rtp->peer_chn_port,rtp->sock);
	return rtp;

}


Rtp_t *RTP_server_new(uint32_t ssrc,
	int payload_type,
	int protocal,	 /* udp or tcp */
	int interleaved, /* TRUE or FALSE */
	int sock,
	char *dstip,
	int dstport)
{
	Rtp_t *rtp=NULL;
	RtpPacket_t *p=NULL;
	RtpHeader_t *rtpHeader=NULL;
	RtspInterHeader_t *inHeader=NULL;

	rtp=(Rtp_t *)malloc(sizeof(Rtp_t));
	if(rtp == NULL){
		VLOG(VLOG_ERROR,"malloc rtp failed");
		return NULL;
	}
	memset(rtp,0,sizeof(Rtp_t));
	p=&rtp->packet;
	rtpHeader=&rtp->header;
	inHeader=&rtp->interHeader;
	
	rtp->role = RTP_SERVER;
	rtp->timestamp = 0xffffffff;
	rtp->seq = 0;
	rtp->raw_data = FALSE;
	rtp->interleaved = interleaved;
	//
	rtp->base_seq = 1;
	rtp->cycle_cnt = 0;
	rtp->packet_cnt = 0;
	rtp->octet_cnt = 0;
	rtp->fraction_lost = 0;
	rtp->comulative_lost = 0;
	// init for packets' buffer
	p->cnt = 0;
	p->malloc_size = 0;
	p->buffer = NULL;

	// init rtp header
	rtpHeader->version = RTP_VERSION;
	rtpHeader->padding = FALSE;
	rtpHeader->extension = FALSE;
	rtpHeader->csrc_cnt = 0;
	rtpHeader->marker = FALSE;
	rtpHeader->payload_type = 0;
	rtpHeader->sequence_number = 0;
	rtpHeader->timestamp = 0;
	rtpHeader->ssrc = htonl(ssrc);
	//init rtsp interleaved header
	if(interleaved == TRUE){
		inHeader->magic = '$';
		inHeader->channel = dstport;
		inHeader->length = 0;
	}else{
		memset(inHeader,0,sizeof(RtspInterHeader_t));
	}
	// init transport
	rtp->sock = sock;
	strcpy(rtp->peername,dstip);
	rtp->peer_chn_port=dstport;

	VLOG(VLOG_CRIT,"[SERVER]rtp ovr %s init done,sock:%d dst %s:%d",
		interleaved ? "RTSP" : "UDP",rtp->sock,rtp->peername,rtp->peer_chn_port);
	return rtp;
}

int RTP_destroy(Rtp_t *rtp)
{
	CircleBuffer_t *buffer=NULL;
	unsigned short port;
	
	if(rtp == NULL){
        VLOG(VLOG_WARNING,"rtp is null");
		return RTP_RET_OK;
	}
	
	if(rtp->interleaved == FALSE){
		SOCK_getsockport(rtp->sock,&port);
		PORT_MANAGE_free_port(port);
		SOCK_close(rtp->sock);
	}
	if(rtp->packet.buffer){
		if(rtp->role == RTP_SERVER || rtp->packet.cnt == RTP_NORTPBUF_MAGIC){
			free(rtp->packet.buffer);
		}else{
			buffer=(CircleBuffer_t *)rtp->packet.buffer;
			buffer->Destroy(buffer);
		}
		rtp->packet.buffer = NULL;
		rtp->packet.cnt = 0;
	}
	free(rtp);
	VLOG(VLOG_CRIT,"rtp destroy success");

	return RTP_RET_OK;
}

static char *h264_get_startcode(char *p, char *end) {
    typedef int intptr_t;
    const char *a = p + 4 - ((intptr_t)p & 3);

    // get startcode in head of given buffer
    for (end -= 3; p < a && p < end; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1) {//start code
            return p;
        }
    }
	
    for (end -= 3; p < end; p += 4) {
        uint32_t x = *(const uint32_t*)p;
        //      if ((x - 0x01000100) & (~x) & 0x80008000) // little endian
        //      if ((x - 0x00010001) & (~x) & 0x00800080) // big endian
        if ((x - 0x01010101) & (~x) & 0x80808080) { // detect has zero byte or not
			if (p[1] == 0) {
                if (p[0] == 0 && p[2] == 1) {// 00 00 01 xx
                    return p;
                }
                if (p[2] == 0 && p[3] == 1) {// xx 00 00 01
                    return p+1;
                }
            }
            if (p[3] == 0) {
                if (p[2] == 0 && p[4] == 1) {// xx xx 00 00 01
                    return p+2;
                }
                if (p[4] == 0 && p[5] == 1) {// xx xx xx 00 00 01
                    return p+3;
                }
            }
        }
    }

    for (end += 3; p < end; p++) {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1) {
            return p;
        }
    }

    return end + 3;
}

static int rtp_packet_sps_pps_sei(char **src,unsigned int *len,Rtp_t *rtp) 
{
    char *psrc = *src;
    char *end = psrc + *len;
    char *nal_start, *nal_end;
	uint32_t nal_size=0;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)&rtp->header;
	RtpPacket_t *p=&rtp->packet;
	const int ex_header_size=rtp->interleaved ? 4 : 0;
	char (*buffer)[RTP_PACKET_SIZE]=(char (*)[RTP_PACKET_SIZE])rtp->packet.buffer;
	char *ptr=NULL;

	p->cnt = 0;
	rtpHeader->payload_type =RTP_DEFAULT_VIDEO_TYPE;
	rtpHeader->marker = TRUE;
	//VLOG_Hex(VLOG_CRIT,*src,40);

    nal_start = h264_get_startcode(psrc, end);
    if (psrc<nal_start && nal_start<end && !nal_start[-1]) nal_start--;
    while (nal_start < end) {
        while (!*(nal_start++));//jump over zero
        if(((*nal_start) & 0x1f) == H264_IDR){
			break;
		}
        // find next start code
        nal_end = h264_get_startcode(nal_start, end);
		if(nal_end >= end){
			break;
		}
		if (psrc<nal_end && nal_end<end && !nal_end[-1]) nal_end--;
		nal_size=nal_end - nal_start;
		//
		ptr=buffer[p->cnt];
		if(rtp->interleaved == TRUE){
			rtp->interHeader.length = htons((unsigned short)(nal_size +sizeof(RtpHeader_t)));
			memcpy(ptr,&rtp->interHeader,sizeof(RtspInterHeader_t));
			ptr+=sizeof(RtspInterHeader_t);
		}
		rtpHeader->sequence_number=htons((unsigned short)(++rtp->seq));
		if (rtp->seq >= 0xffff) {
			rtp->seq = 0;
		}
		memcpy(ptr,rtpHeader,sizeof(RtpHeader_t));
		ptr+=sizeof(RtpHeader_t);
		memcpy(ptr,nal_start,nal_size);
		//VLOG_Hex(VLOG_CRIT,ptr,nal_size);
		p->buf_size[p->cnt]=nal_size+sizeof(RtpHeader_t)+ex_header_size;
		p->cnt ++;
		//
        nal_start = nal_end;
		//
		*src = nal_end;
		*len -= nal_size + 4;
		
    }
	
	VLOG(VLOG_DEBUG,"total entries:%d",p->cnt);

	if(p->cnt!=0){
		//sps ,pps ,sei ,iframe all in one packet
		if(((*((unsigned char *)*src+4)) & 0x1f) != H264_IDR){
			if(((*((unsigned char *)*src+3)) & 0x1f) != H264_IDR){
				VLOG(VLOG_ERROR,"invalid , please check it ,entry:%d %x",p->cnt, *((unsigned char *)*src+4));
				//VLOG_HexString(VLOG_ERROR,(unsigned char *)(*src),100);
				return -1;
			}
		}
	}
    return 0;
}

//#define WITH_RTP_PADDING
static int rtp_packet_h264(char *src,uint32_t len,Rtp_t *rtp,uint32_t ts)
{
#define H264_NALU_OFFSET		(4)	/*start code */
#define H264_NALU_DATA_OFFSET	(5)	/*start code & nalu header*/
	char *ptr=NULL,*psrc=NULL;
	int frag_size;
	Nalu_t nalu;
	FUIndicator_t *fu_indicator;
	FUHeader_t *fu_header;
	int b_fragment = FALSE;
	int ex_header_size=(rtp->interleaved==TRUE) ? 4 : 0;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)&rtp->header;
	RtpPacket_t *p=&rtp->packet;
	char paddingsize = 0;
	char (*buffer)[RTP_PACKET_SIZE]=(char (*)[RTP_PACKET_SIZE])rtp->packet.buffer;
	if((len-H264_NALU_DATA_OFFSET) > RTP_MTU_SIZE){
		b_fragment = TRUE;
	}
	p->cnt = 0;
	if(ts == 0 && rtp->timestamp != 0xffffffff){
		VLOG(VLOG_CRIT, "ts is %u(last:%u)", ts, rtp->timestamp);
	}

	rtp->header.padding = 0;
	//rtp->timestamp +=ts_inc;
	rtp->timestamp =ts;
	rtpHeader->timestamp = htonl(rtp->timestamp);
	rtp->packet.cnt = 0;
	nalu.padding = src[H264_NALU_OFFSET];
#if defined(NOCROSS)
	if(FALSE)
#else
	if(nalu.type == H264_SPS || nalu.type ==  H264_PPS || nalu.type == H264_SEI)
#endif
	//if(FALSE)
	{
		//char *ppp=src+H264_NALU_OFFSET;
		//RTSP_LOG_HEX(ppp,128);
		//VLOG(VLOG_DEBUG,"src:%p len:%d",src,len);
		if(rtp_packet_sps_pps_sei(&src,&len,rtp)==RTP_RET_FAIL)
			return RTP_RET_FAIL;
		//VLOG(VLOG_DEBUG,"after src:%p len:%d cnt:%d seq:%d",src,len,p->cnt,rtp->seq);
	}
	//rtp->packet.cnt = 0;
	nalu.padding = src[H264_NALU_OFFSET];

	if(b_fragment == FALSE){
		p->cnt = 1;
		ptr=buffer[0];
#ifdef WITH_RTP_PADDING
		paddingsize = len % 4;
		if(paddingsize != 0){
			rtp->header.padding = 1;
			paddingsize = 4 - paddingsize;
		}else{
			paddingsize = 0;
		}
#endif	
		if(rtp->interleaved == TRUE){
			rtp->interHeader.length = htons((unsigned short)(sizeof(RtpHeader_t)+ len + paddingsize -H264_NALU_OFFSET));
			memcpy(ptr,&rtp->interHeader,sizeof(RtspInterHeader_t));
			ptr+=sizeof(RtspInterHeader_t);
		}
		rtpHeader->payload_type =RTP_DEFAULT_VIDEO_TYPE;
		rtpHeader->marker = 1;
		rtpHeader->sequence_number=htons((unsigned short)(++rtp->seq));
		memcpy(ptr,rtpHeader,sizeof(RtpHeader_t));
		ptr+=sizeof(RtpHeader_t);
		memcpy(ptr,src+H264_NALU_OFFSET,len-H264_NALU_OFFSET);
		ptr +=  len - H264_NALU_OFFSET;
		p->buf_size[0]=sizeof(RtpHeader_t)+ex_header_size+ len + paddingsize -H264_NALU_OFFSET;
		#ifdef WITH_RTP_PADDING
			if(paddingsize == 1){
				ptr[0] = 1;
			}else if(paddingsize == 2){
				ptr[0] = 0; ptr[1] = 2;
			}else if(paddingsize == 3){
				ptr[0] = 0; ptr[1] = 0; ptr[2] =3;
			}
		#endif
		if(p->buf_size[0] > RTP_PACKET_SIZE){
			VLOG(VLOG_ERROR,"rtp packet size:%d exceed the buffer size",p->buf_size[0]);
			return RTP_RET_FAIL;
		}
	}else{
		int i=0,start=p->cnt,end;
		len -=H264_NALU_DATA_OFFSET;
		end = (len+RTP_MTU_SIZE - 1)/RTP_MTU_SIZE+p->cnt;
		p->cnt = end;
		psrc = src + H264_NALU_DATA_OFFSET;
		for(i=start;i<end;i++){
			ptr = buffer[i];
			if(len > RTP_MTU_SIZE)
				frag_size = RTP_MTU_SIZE;
			else
				frag_size = len;
			
#ifdef WITH_RTP_PADDING
			paddingsize = (frag_size + 2) % 4;
			if(paddingsize != 0){
				paddingsize = 4 - paddingsize;
				rtpHeader->padding = 1;
			}else{
				rtpHeader->padding = 0;
			}
#endif
			
			if(rtp->interleaved == TRUE){
				rtp->interHeader.length = htons((unsigned short)(sizeof(RtpHeader_t) + 2 + paddingsize + frag_size));
				memcpy(ptr,&rtp->interHeader,sizeof(RtspInterHeader_t));
				ptr+=sizeof(RtspInterHeader_t);
			}
			rtpHeader->payload_type =RTP_DEFAULT_VIDEO_TYPE;
			rtpHeader->sequence_number=htons((unsigned short)(++rtp->seq));
			rtpHeader->marker = (i == (end -1))  ? 1 : 0;
			memcpy(ptr,rtpHeader,sizeof(RtpHeader_t));
			ptr+=sizeof(RtpHeader_t);
			fu_indicator=(FUIndicator_t *)ptr;
			fu_indicator->forbidden_zero_bit = 0;
			fu_indicator->nal_ref_idc = nalu.nal_ref_idc;
			fu_indicator->type = RTP_FU_A;
			ptr++;
			fu_header=(FUHeader_t *)ptr;
			fu_header->reserved_bit = 0;
			fu_header->start_bit = (i == start) ? 1 : 0;
			fu_header->stop_bit = (i == (end -1))  ? 1 : 0;
			fu_header->type = nalu.type;
			ptr++;
			memcpy(ptr,psrc,frag_size);
			ptr += frag_size;
			p->buf_size[i]=sizeof(RtpHeader_t)+ex_header_size + paddingsize + 1 + 1 + frag_size ;
#ifdef WITH_RTP_PADDING
			if(paddingsize == 1){
				ptr[0] = 1;
			}else if(paddingsize == 2){
				ptr[0] = 0; ptr[1] = 2;
			}else if(paddingsize == 3){
				ptr[0] = 0; ptr[1] = 0; ptr[2] =3;
			}
#endif
			if(p->buf_size[i] > RTP_PACKET_SIZE){
				VLOG(VLOG_ERROR,"rtp packet size:%d exceed the buffer size",p->buf_size[i]);
				return RTP_RET_FAIL;
			}
			len-=frag_size;
			psrc +=frag_size;
		}
	}
	VLOG(VLOG_DEBUG,"rtp packet h264 done,cnt:%d ts:%u",p->cnt, ts);
	return RTP_RET_OK;
}

static int rtp_packet_ps(char *src,uint32_t len,Rtp_t *rtp,uint32_t ts)
{
	char *ptr=NULL,*psrc=NULL;
	int frag_size;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)&rtp->header;
	RtpPacket_t *p=&rtp->packet;
	char (*buffer)[RTP_PACKET_SIZE]=(char (*)[RTP_PACKET_SIZE])rtp->packet.buffer;	
	int i=0,start=0,end;

	p->cnt = 0;
	rtp->timestamp =ts;
	rtpHeader->timestamp = htonl(rtp->timestamp);
	rtp->packet.cnt = 0;

	end = (len+RTP_MTU_SIZE - 1)/RTP_MTU_SIZE;
	p->cnt = end;
	psrc = src;
	for(i=start;i<end;i++){
		ptr = buffer[i];
		if(len > RTP_MTU_SIZE)
			frag_size = RTP_MTU_SIZE;
		else
			frag_size = len;
		
		rtpHeader->payload_type =RTP_DEFAULT_VIDEO_TYPE;
		rtpHeader->sequence_number=htons((unsigned short)(++rtp->seq));
		rtpHeader->marker = (i == (end -1))  ? 1 : 0;
		memcpy(ptr,rtpHeader,sizeof(RtpHeader_t));
		ptr+=sizeof(RtpHeader_t);
		memcpy(ptr,psrc,frag_size);
		p->buf_size[i]=sizeof(RtpHeader_t)+ frag_size ;
		if(p->buf_size[i] > RTP_PACKET_SIZE){
			VLOG(VLOG_ERROR,"rtp packet size:%d exceed the buffer size",p->buf_size[i]);
			return RTP_RET_FAIL;
		}
		len-=frag_size;
		psrc +=frag_size;
	}
	VLOG(VLOG_DEBUG,"rtp packet ps done,cnt:%d",p->cnt);
	return RTP_RET_OK;
}

static int rtp_packet_g711(char *src,uint32_t len,Rtp_t *rtp,uint32_t ts)
{
	char *ptr=NULL;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)&rtp->header;
	RtpPacket_t *p=&rtp->packet;
	int ex_header_size=(rtp->interleaved==TRUE) ? 4 : 0;
	char (*buffer)[RTP_PACKET_SIZE]=(char (*)[RTP_PACKET_SIZE])rtp->packet.buffer;

	if((sizeof(RtpHeader_t) + len + ex_header_size) > RTP_PACKET_SIZE){
		VLOG(VLOG_ERROR,"the buffer out of range");
		return RTP_RET_FAIL;
	}
	//rtp->timestamp +=ts_inc;	
	rtp->timestamp =ts;
	rtpHeader->timestamp = htonl(rtp->timestamp);

	p->cnt = 1;
	ptr=buffer[0];
	if(rtp->interleaved == TRUE){
		rtp->interHeader.length = htons((unsigned short)(sizeof(RtpHeader_t)+ len));
		memcpy(ptr,&rtp->interHeader,sizeof(RtspInterHeader_t));
		ptr+=sizeof(RtspInterHeader_t);
	}
	rtpHeader->payload_type =RTP_TYPE_PCMA;
	rtpHeader->marker = TRUE;
	rtpHeader->sequence_number=htons((unsigned short)(++rtp->seq));
	memcpy(ptr,rtpHeader,sizeof(RtpHeader_t));
	ptr+=sizeof(RtpHeader_t);
	memcpy(ptr,src,len);
	p->buf_size[0]=sizeof(RtpHeader_t) + len + ex_header_size;
	
	VLOG(VLOG_DEBUG,"rtp packet g711 done,cnt:%d",p->cnt);
	return RTP_RET_OK;
}

#define TRUNCK_CNTS_FOR_DELAY	(8)
#define TRUCNT_CNTS	(5)

int RTP_send_packet(Rtp_t *rtp,char *src,uint32_t len,uint32_t ts,int payload_type)
{
	int i,ret;
	uint32_t expect_size = 0;
	uint32_t total_sended = 0;
	char (*buffer)[RTP_PACKET_SIZE]=(char (*)[RTP_PACKET_SIZE])NULL;
	// check buffer size
	//expect_size = (len/RTP_MTU_SIZE+1+3)*RTP_PACKET_SIZE;
	
	expect_size = RTP_MAX_FRAGMENTATION*RTP_PACKET_SIZE;
	if(rtp->packet.buffer==NULL){
		rtp->packet.buffer = malloc(expect_size);
		if(rtp->packet.buffer == NULL){
			VLOG(VLOG_ERROR,"maloc for rtp failed");
			return RTP_RET_FAIL;
		}else{
			//VLOG(VLOG_DEBUG,"maloc size:%d @%p ->%p", expect_size,rtp->packet.buffer,rtp->packet.buffer+expect_size);
		}
		rtp->packet.malloc_size = expect_size;
	}else if(rtp->packet.malloc_size < expect_size){
		rtp->packet.buffer = realloc(rtp->packet.buffer,expect_size);
		if(rtp->packet.buffer == NULL){
			VLOG(VLOG_ERROR,"remalloc for rtp failed");
			return RTP_RET_FAIL;
		}else{
			VLOG(VLOG_DEBUG,"remaloc size:%d @%p ", expect_size,rtp->packet.buffer);
		}
		rtp->packet.malloc_size = expect_size;
	}
	buffer = (char (*)[RTP_PACKET_SIZE])rtp->packet.buffer;

	// setup rtp packets
	if(payload_type == RTP_TYPE_PS)
		ret=rtp_packet_ps(src,len,rtp,ts);
	else if(payload_type == RTP_TYPE_PCMA)
		ret=rtp_packet_g711(src,len,rtp,ts);
	else if(payload_type >= RTP_TYPE_DYNAMIC)
		ret=rtp_packet_h264(src,len,rtp,ts);
	else{
		VLOG(VLOG_ERROR,"unsupport stream type:%d",payload_type);
		return RTP_RET_FAIL;
	}
	if(ret==RTP_RET_OK)
	{
		for(i=0;i<rtp->packet.cnt;i++){
			if(rtp->interleaved == TRUE){
				ret=SOCK_send(rtp->sock,buffer[i],rtp->packet.buf_size[i]);
			}else{
				ret=SOCK_sendto(rtp->sock,rtp->peername,rtp->peer_chn_port,
					buffer[i],rtp->packet.buf_size[i]);
			}
			if(ret==RTP_RET_FAIL){
				return RTP_RET_FAIL;
			}
			else{
				//VLOG(VLOG_DEBUG,"%d: send frame ok,size:%d @%p->%p",i,rtp->packet.buf_size[i],buffer[i],buffer[i]+rtp->packet.buf_size[i]);
			}
			rtp->packet_cnt ++;
			rtp->octet_cnt +=rtp->packet.buf_size[i];
			total_sended += rtp->packet.buf_size[i];
			//
			//if(rtp->packet.cnt > TRUNCK_CNTS_FOR_DELAY){
			//	if(i%TRUNCK_CNTS_FOR_DELAY == (TRUNCK_CNTS_FOR_DELAY/2)) MSLEEP(1);
			//}
		}
	}else{
		return RTP_RET_FAIL;
	}
	VLOG(VLOG_DEBUG,"rtp send frame done,total:%u",total_sended);
	return RTP_RET_OK;
}

int RTP_pack(Rtp_t *rtp,char *src,uint32_t len,uint32_t ts,int payload_type)
{
	int ret;
	uint32_t expect_size = 0;
	// check buffer size
	//expect_size = (len/RTP_MTU_SIZE+1+3)*RTP_PACKET_SIZE;
	
	expect_size = RTP_MAX_FRAGMENTATION*RTP_PACKET_SIZE;
	if(rtp->packet.buffer==NULL){
		rtp->packet.buffer = malloc(expect_size);
		if(rtp->packet.buffer == NULL){
			VLOG(VLOG_ERROR,"maloc for rtp failed");
			return RTP_RET_FAIL;
		}else{
			//VLOG(VLOG_DEBUG,"maloc size:%d @%p ->%p", expect_size,rtp->packet.buffer,rtp->packet.buffer+expect_size);
		}
		rtp->packet.malloc_size = expect_size;
	}else if(rtp->packet.malloc_size < expect_size){
		rtp->packet.buffer = realloc(rtp->packet.buffer,expect_size);
		if(rtp->packet.buffer == NULL){
			VLOG(VLOG_ERROR,"remalloc for rtp failed");
			return RTP_RET_FAIL;
		}else{
			VLOG(VLOG_DEBUG,"remaloc size:%d @%p ", expect_size,rtp->packet.buffer);
		}
		rtp->packet.malloc_size = expect_size;
	}

	// setup rtp packets
	if(payload_type == RTP_TYPE_PS)
		ret=rtp_packet_ps(src,len,rtp,ts);
	else if(payload_type == RTP_TYPE_PCMA)
		ret=rtp_packet_g711(src,len,rtp,ts);
	else if(payload_type >= RTP_TYPE_DYNAMIC)
		ret=rtp_packet_h264(src,len,rtp,ts);
	else{
		VLOG(VLOG_ERROR,"unsupport stream type:%d",payload_type);
		return RTP_RET_FAIL;
	}

	return ret;
}

int RTP_send(Rtp_t *rtp)
{
	int i, ret;
	uint32_t total_sended = 0;
	char (*buffer)[RTP_PACKET_SIZE]=(char (*)[RTP_PACKET_SIZE])rtp->packet.buffer;

	struct iovec *vecs = NULL;
	
	if(rtp->interleaved == TRUE){
#if defined(_WIN32) || defined(_WIN64)
		for(i=0;i<rtp->packet.cnt;i++){
			if(SOCK_send(rtp->sock,buffer[i],rtp->packet.buf_size[i]) < 0)
				return RTP_RET_FAIL;
			rtp->packet_cnt ++;
			rtp->octet_cnt +=rtp->packet.buf_size[i];
			total_sended += rtp->packet.buf_size[i];
		}
#else
		vecs = (struct iovec *)malloc(sizeof(struct iovec) * rtp->packet.cnt);
		for(i=0;i<rtp->packet.cnt;i++){
			struct iovec *pvec = vecs + i;
			pvec->iov_base = buffer[i];
			pvec->iov_len = rtp->packet.buf_size[i];
			//
			rtp->packet_cnt ++;
			rtp->octet_cnt +=rtp->packet.buf_size[i];
			total_sended += rtp->packet.buf_size[i];
		}
		if((ret = writev(rtp->sock, vecs, rtp->packet.cnt)) < 0){
			free(vecs);
			return RTP_RET_FAIL;
		}
		free(vecs);
#endif
	}else{
		for(i=0;i<rtp->packet.cnt;i++){
			if(SOCK_sendto(rtp->sock,rtp->peername,rtp->peer_chn_port,
					buffer[i],rtp->packet.buf_size[i]) < 0)
				return RTP_RET_FAIL;
			rtp->packet_cnt ++;
			rtp->octet_cnt +=rtp->packet.buf_size[i];
			total_sended += rtp->packet.buf_size[i];
			//
			//if(rtp->packet.cnt > TRUNCK_CNTS_FOR_DELAY){
			//	if(i%TRUNCK_CNTS_FOR_DELAY == (TRUNCK_CNTS_FOR_DELAY/2)) MSLEEP(1);
			//}
		}
	}
	
	return RTP_RET_OK;
}


int rtp_alaw_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)ptr;

	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->start_flag = TRUE;
	info->stop_flag = TRUE;

	ptr += sizeof(RtpHeader_t);
	info->frame_pos=(uint32_t)ptr;
	info->frame_size = size - sizeof(RtpHeader_t);
	
	return RTP_RET_OK;
}

int rtp_h264_singlenalu_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)ptr;
	Nalu_t nalu;

	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->start_flag = TRUE;
	info->stop_flag = TRUE;

	ptr += sizeof(RtpHeader_t);
	nalu.padding = ptr[0];
	if((nalu.type == H264_IDR) || (nalu.type == H264_SPS) || (nalu.type == H264_PPS)){
		info->key_flag = TRUE;
	}else{
		info->key_flag = FALSE;
	}
	// add start code
	ptr -= 4;
	ptr[0] = ptr[1] = ptr[2] =0;
	ptr[3] = 1;
	info->frame_pos=(uint32_t)ptr;
	info->frame_size = size - sizeof(RtpHeader_t) + 4;
	
	return RTP_RET_OK;
}


int rtp_h264_rawdata_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)ptr;
	Nalu_t nalu;

	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->stop_flag = rtpHeader->marker;

	ptr += sizeof(RtpHeader_t);
	if(((ptr[0] == 0) && (ptr[1] == 0) && (ptr[2] == 0) && (ptr[3] == 1))
		|| ((ptr[0] == 0) && (ptr[1] == 0) && (ptr[2] == 1))){ 
		info->start_flag = 1;
		if(ptr[2] == 1){
			nalu.padding = ptr[3];
			ptr--;// add a zero byte
			*ptr=0;
			size++;
		}else
			nalu.padding = ptr[4];
		if((nalu.type == H264_IDR) || (nalu.type == H264_SPS) || (nalu.type == H264_PPS)){
			info->key_flag = 1;
		}
	}else{
		//info->key_flag = 0;
	}
	info->frame_pos=(uint32_t)ptr;
	info->frame_size = size - sizeof(RtpHeader_t);
	
	return RTP_RET_OK;
}

int rtp_h264_fua_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)ptr;
	Nalu_t nalu;
	FUHeader_t fuh;

	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->stop_flag = rtpHeader->marker;

	ptr += sizeof(RtpHeader_t);
	nalu.padding = ptr[0];
	fuh.padding = ptr[1];
	info->start_flag = fuh.start_bit;
	info->stop_flag =fuh.stop_bit;
	if((fuh.type == H264_IDR) || (fuh.type == H264_SPS) || (fuh.type == H264_PPS)){ 
		info->key_flag = TRUE;
	}else{
		info->key_flag = FALSE;
	}
	if(info->start_flag == 1){
		nalu.type = fuh.type;
		*(ptr+1)= nalu.padding;
	}
	info->frame_size = size - sizeof(RtpHeader_t) - ((info->start_flag == 1) ? 1 : 2);
	info->frame_pos = (uint32_t)ptr + ((info->start_flag == 1) ? 1 : 2);
	if(info->start_flag == 1){
		info->frame_size += 4;
		info->frame_pos -=  4;
		ptr = (char *)info->frame_pos;
		ptr[0]=0;ptr[1]=0;ptr[2]=0;
		ptr[3]=1;
	}

	return RTP_RET_OK;
}

int rtp_h264_fub_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	VLOG(VLOG_ERROR,"unsupport nalu type:FU-B");
	return RTP_RET_FAIL;
}

#define MAX_MULTI_PACKETS 16
#define MAX_MULTI_OCTECT	2000
typedef struct _multi_packet
{
	int count;
	int keyflag[MAX_MULTI_PACKETS];
	int size[MAX_MULTI_PACKETS];
	char data[MAX_MULTI_PACKETS][MAX_MULTI_OCTECT];
}MultiPacket_t;

int rtp_h264_stapa_decode(char *buf,int size,RtpFrameInfo_t *info, MultiPacket_t *packets)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)ptr;
	Nalu_t nalu;
	int remind = size;
	int index = 0;

	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->stop_flag = TRUE;
	info->start_flag = TRUE;
	info->key_flag = FALSE;

	ptr += sizeof(RtpHeader_t);
	nalu.padding = ptr[0];//STAP-A HDR
	ptr++;
	remind -= sizeof(RtpHeader_t) + 1;
	while(remind > 0){
		int nsize = ntohs(*((unsigned short *)ptr));
		if(index >= MAX_MULTI_PACKETS){
			VLOG(VLOG_ERROR, "exceed max multi packets!\n");
			return RTP_RET_FAIL;			
		}
		if((nsize + 4) > MAX_MULTI_OCTECT){
			VLOG(VLOG_ERROR, "exceed max multi octect!\n");
			return RTP_RET_FAIL;
		}
		ptr += 2;
		nalu.padding = *ptr;
		if(nalu.type == H264_IDR){ 
			packets->keyflag[index] = TRUE;
		}else{
			packets->keyflag[index] = FALSE;
		}
		packets->size[index] = nsize + 4;
		packets->data[index][0]=0;
		packets->data[index][1]=0;
		packets->data[index][2]=0;
		packets->data[index][3]=1;	
		memcpy(&packets->data[index][4], ptr, nsize);
		ptr += nsize;
		
		remind -= 2 + nsize;
		index++;
	}
	packets->count = index;

	return RTP_RET_OK;
}

int rtp_h264_stapb_decode(char *buf,int size,RtpFrameInfo_t *info, MultiPacket_t *packets)
{
	char *ptr = buf;
	RtpHeader_t *rtpHeader=(RtpHeader_t *)ptr;
	Nalu_t nalu;
	int remind = size;
	int index = 0;

	memset(info,0,sizeof(RtpFrameInfo_t));
	info->magic = RTP_FRAME_INFO_MAGIC;
	info->type = rtpHeader->payload_type;
	info->seq = ntohs(rtpHeader->sequence_number);
	info->timestamp = ntohl(rtpHeader->timestamp);
	info->stop_flag = TRUE;
	info->start_flag = TRUE;
	info->key_flag = FALSE;

	ptr += sizeof(RtpHeader_t);
	nalu.padding = ptr[0];//STAP-A HDR
	ptr++;
	remind -= sizeof(RtpHeader_t) + 1;
	while(remind > 0){
		int nsize = ntohs(*((unsigned short *)(ptr+2)));
		if(index >= MAX_MULTI_PACKETS){
			VLOG(VLOG_ERROR, "exceed max multi packets!\n");
			return RTP_RET_FAIL;			
		}
		if((nsize + 4) > MAX_MULTI_OCTECT){
			VLOG(VLOG_ERROR, "exceed max multi octect!\n");
			return RTP_RET_FAIL;
		}
		ptr += 4;
		nalu.padding = *ptr;
		if(nalu.type == H264_IDR){ 
			packets->keyflag[index] = TRUE;
		}else{
			packets->keyflag[index] = FALSE;
		}
		packets->size[index] = nsize + 4;
		packets->data[index][0]=0;
		packets->data[index][1]=0;
		packets->data[index][2]=0;
		packets->data[index][3]=1;	
		memcpy(&packets->data[index][4], ptr, nsize);
		ptr += nsize;
		
		remind -= 4 + nsize;
		index++;
	}
	packets->count = index;

	return RTP_RET_OK;
}

int rtp_h264_mtap16_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	VLOG(VLOG_ERROR,"unsupport nalu type:MTAP16");
	return RTP_RET_FAIL;
}

int rtp_h264_mtap24_decode(char *buf,int size,RtpFrameInfo_t *info)
{
	VLOG(VLOG_ERROR,"unsupport nalu type:MTAP24");
	return RTP_RET_FAIL;
}


/*
int RTP_handle_packet(Rtp_t *rtp,void *payload,int payload_size)
{
	int ret,size;
	char buf[1024*16],*ptr=NULL,*pbuf=NULL;
	RtpHeader_t *rtpHeader=NULL;
	Nalu_t nalu;
	uint32_t expect_size = 0;
	CircleBuffer_t *buffer=NULL;
	RtpFrameInfo_t info;
	MultiPacket_t mpack;

	if(rtp->interleaved == TRUE){
		ptr=(char *)payload+sizeof(RtspInterHeader_t);
		size = payload_size-sizeof(RtspInterHeader_t);
	}else{
		ret=SOCK_recvfrom(rtp->sock,rtp->peername,&rtp->peer_chn_port,buf,sizeof(buf),0);
		if(ret == RTP_RET_FAIL){
			return RTP_RET_FAIL;
		}
		ptr = buf;
		size = ret;
	}

	pbuf=ptr;
	rtpHeader = (RtpHeader_t *)ptr;
	ptr += sizeof(RtpHeader_t);
	nalu.padding = ptr[0];

	rtp->ssrc = ntohl(rtpHeader->ssrc);

	if(rtpHeader->padding == TRUE){
		size-=pbuf[size -1];
	}
	
	// update received packets and octect and max seq number
	rtp->packet_cnt ++;
	rtp->octet_cnt += size;
	if(rtp->interleaved == TRUE) rtp->octet_cnt += 4;
	if(ntohs(rtpHeader->sequence_number) > rtp->seq ){
		rtp->seq = ntohs(rtpHeader->sequence_number);
	}
	// malloc circle buffer
	if(rtp->packet.buffer == NULL){
		if(rtpHeader->payload_type == RTP_TYPE_PCMA || rtpHeader->payload_type == RTP_TYPE_PCMU){
			expect_size = (uint32_t)(RTP_AUDIO_RECV_BUFFER_SIZE);
		}else{
			expect_size = (uint32_t)(RTP_VIDEO_RECV_BUFFER_SIZE);
		}
		rtp->packet.buffer=(CircleBuffer_t *)CIRCLEBUFFER_new(expect_size,rtp->buffertime);
		if(rtp->packet.buffer == NULL){
			return RTP_RET_FAIL;
		}
		rtp->packet.cnt=1;
		rtp->packet.buf_size[0]=expect_size;
		rtp->packet.malloc_size = expect_size;
	}
	buffer = (CircleBuffer_t *)rtp->packet.buffer;
	
	if(rtpHeader->payload_type == RTP_TYPE_PCMA || rtpHeader->payload_type == RTP_TYPE_PCMU){
		ret=rtp_alaw_decode(pbuf,size,&info);
	}else if(rtpHeader->payload_type >= RTP_TYPE_DYNAMIC){
		if(rtp->raw_data == TRUE){
			ret=rtp_h264_rawdata_decode(pbuf,size,&info);
		}else if((rtp->raw_data==FALSE) && 
			(((ptr[0] == 0) && (ptr[1]==0) && (ptr[2]==0) && (ptr[3] == 1))
			|| ((ptr[0] == 0) && (ptr[1]==0) && (ptr[2]==1)))){
			VLOG(VLOG_DEBUG,"h264 format : raw data");
			//rtp->raw_data =TRUE;
			ret=rtp_h264_rawdata_decode(pbuf,size,&info);
		}else if(nalu.type <= H264_FILLER_DATA){
			ret=rtp_h264_singlenalu_decode(pbuf,size,&info);
		}else if(nalu.type == RTP_FU_A){
			ret=rtp_h264_fua_decode(pbuf,size,&info);
		}else if(nalu.type == RTP_FU_B){
			ret=rtp_h264_fub_decode(pbuf,size,&info);
		}else if(nalu.type == RTP_STAP_A){
			ret=rtp_h264_stapa_decode(pbuf,size,&info, &mpack);
		}else if(nalu.type == RTP_STAP_B){
			ret=rtp_h264_stapb_decode(pbuf,size,&info, &mpack);
		}else if(nalu.type == RTP_MTAP16){
			ret=rtp_h264_mtap16_decode(pbuf,size,&info);
		}else if(nalu.type == RTP_MTAP24){
			ret=rtp_h264_mtap24_decode(pbuf,size,&info);
		}else{
			VLOG(VLOG_ERROR,"unknown nalu type:%d",nalu.type);
			return RTP_RET_FAIL;
		}
	}else{
		VLOG(VLOG_ERROR,"unsupport payload type:%d interleaved:%d",rtpHeader->payload_type,rtp->interleaved);
		return RTP_RET_FAIL;
	}
	if(ret == RTP_RET_FAIL)
		return RTP_RET_FAIL;
	
#ifdef RTSP_BUFFER_ENTER_KEYFRAME_FIRST
	if((buffer->IsAvailable(buffer) ==FALSE) && (buffer->GetUsedSize(buffer)==0) && (info.type >= RTP_TYPE_DYNAMIC)){
		if(info.key_flag == FALSE){
			VLOG(VLOG_WARNING,"RTP: recv a packet,size:%d ts:%u seq:%u,$$ignore$$",size,info.timestamp,info.seq);
			return RTP_RET_OK;
		}
	}
#endif

	if(nalu.type == RTP_STAP_A || nalu.type == RTP_STAP_B){
		int i=0;
		for ( i = 0; i < mpack.count; i++){
			info.key_flag = mpack.keyflag[i];
			info.frame_size = mpack.size[i];
			info.frame_pos = (unsigned int)mpack.data[i];			
			ret=buffer->AddRtpFrame(buffer,&info);
			if(ret==RTP_RET_FAIL){
				return RTP_RET_FAIL;
			}else if(ret == CBUFFER_RET_NODATA){
				buffer->Flush(buffer);
				ret=buffer->AddRtpFrame(buffer,&info);
				if(ret != RTP_RET_OK) return RTP_RET_FAIL;
			}
		}
	}else{
		ret=buffer->AddRtpFrame(buffer,&info);
		if(ret==RTP_RET_FAIL){
			return RTP_RET_FAIL;
		}else if(ret == CBUFFER_RET_NODATA){
			buffer->Flush(buffer);
			ret=buffer->AddRtpFrame(buffer,&info);
			if(ret != RTP_RET_OK) return RTP_RET_FAIL;
		}
	}
	VLOG(VLOG_DEBUG,"RTP:recv a packet,type:%d size:%d ts:%u seq:%u",info.type,size,info.timestamp,info.seq);
	return RTP_RET_OK;
}
*/

int RTP_handle_packet_nortpbuf(Rtp_t *rtp,void *payload,int payload_size)
{
#define RTP_RECV(BUF,SIZE,FLAG) \
		memcpy(BUF, pbuf, SIZE);\
		pbuf += SIZE;\
		ret = SIZE;\
		rtp->octet_cnt += ret

	int ret;
	char *ptr=NULL, *pbuf=NULL;
	char tmp[128];
	RtpHeader_t rtpHeader;
	Nalu_t nalu;
	FUHeader_t fuh;
	unsigned int expect_size = 0;
	int remind = rtp->interleaved ? payload_size : sizeof(rtp->packet.trunk_buf);

	if(rtp->interleaved == FALSE){
		pbuf = rtp->packet.trunk_buf;
		ret = SOCK_recvfrom(rtp->sock,rtp->peername,&rtp->peer_chn_port,pbuf,sizeof(rtp->packet.trunk_buf),0);
		if (ret == SOCK_EAGAIN) {
			return SOCK_EAGAIN;
		} else if (ret < 0) {
			return -1;
		}
		remind = ret;
		payload_size = ret;
	}
	else {
		unsigned int nret = 0;
		unsigned int need_rcv = payload_size;
		pbuf = rtp->packet.trunk_buf;
		if (payload_size > sizeof(rtp->packet.trunk_buf)) {
			printf("rtp packet too big %d/%d\r\n", payload_size, sizeof(rtp->packet.trunk_buf));
			return -1;
		}
		if (rtp->packet.trunk_readed > 0) {
			//printf("rtp has readed %d/%d\r\n", rtp->packet.trunk_readed , payload_size);
			need_rcv -= rtp->packet.trunk_readed;
			pbuf += rtp->packet.trunk_readed;
		}
		nret = SOCK_recv2(rtp->sock,pbuf, need_rcv,0);
		if (nret < 0) {
			return -1;
		} else if (nret  != need_rcv) {
			 //printf("rtp read %d/%d\r\n", nret, need_rcv);
			 rtp->packet.trunk_readed += nret;
			return SOCK_EAGAIN;
		}
		remind = payload_size;
		pbuf = rtp->packet.trunk_buf;
	}
	rtp->packet.trunk_readed = 0;

	// statistics	
	rtp->packet_cnt++;
	rtp->octet_cnt += payload_size;
	if(rtp->interleaved == TRUE){
		rtp->octet_cnt += 4;
	}
	// receive rtp header first
	RTP_RECV((char *)&rtpHeader, sizeof(RtpHeader_t), 0);
	remind -= sizeof(RtpHeader_t);
	// rtp header
	if (rtpHeader.version != 2) {
		VLOG(VLOG_ERROR, "got rtp packet wrong , version is %d!", rtpHeader.version);
		return -1;
	}
	if (rtp->payloadType != rtpHeader.payload_type) {
		VLOG(VLOG_ERROR, "got rtp packet wrong , payload type is %d(%d)!", rtpHeader.payload_type, rtp->payloadType);
		return -1;
	}
	if(rtpHeader.csrc_cnt){
		RTP_RECV(tmp, 4*rtpHeader.csrc_cnt, 0);
		remind -= 4*rtpHeader.csrc_cnt;
		VLOG(VLOG_DEBUG, "got csrc_cnt!\n");
	}
	if (rtpHeader.extension == 1) {
		RtpExtHeader_t extHeader;
		RTP_RECV(&extHeader, 4, 0);
		remind -= 4;
		if (ntohs(extHeader.length) > 0)  {
			VLOG(VLOG_DEBUG, "got ext length: %d/%d!", extHeader.length, ntohs(extHeader.length));
			RTP_RECV(tmp, 4 * ntohs(extHeader.length), 0);
			remind -= 4 * ntohs(extHeader.length);
		}
	}
	if (rtpHeader.sequence_number == 0) {
		if (rtp->seq == 0xffff || rtp->seq == (0xffff -1) || rtp->seq == 0) {
		} else {
			VLOG(VLOG_DEBUG, "1 rtp:%s lost %d packets (%d->%d)", rtp->peername,
				(uint32_t)ntohs(rtpHeader.sequence_number) + 0xffff - rtp->seq -1, rtp->seq, ntohs(rtpHeader.sequence_number));
		}
	} else {
		if (rtp->seq != 0) {
			if ((rtp->seq + 1) != ntohs(rtpHeader.sequence_number)) {
				VLOG(VLOG_DEBUG, "2 rtp:%s lost %d packets (%d->%d)", rtp->peername,
					(uint32_t)ntohs(rtpHeader.sequence_number)  - rtp->seq - 1, rtp->seq, ntohs(rtpHeader.sequence_number));
			}
		}
	}
	rtp->seq = ntohs(rtpHeader.sequence_number);
	rtp->timestamp = ntohl(rtpHeader.timestamp);
	rtp->ssrc = ntohl(rtpHeader.ssrc);

	
	// check rtp buffer 
	if(rtp->packet.buffer == NULL){
		if(rtpHeader.payload_type == RTP_TYPE_PCMA || rtpHeader.payload_type == RTP_TYPE_PCMU){
			expect_size = RTP_AUDIO_RECV_BUFFER_SIZE;
		}else{
			expect_size = RTP_VIDEO_RECV_BUFFER_SIZE;
		}
		rtp->packet.buffer = (void *)malloc(expect_size);
		if(rtp->packet.buffer == NULL){
			VLOG(VLOG_ERROR,"malloc for rtp buffer failed!\n");
			exit(0);
		}
		rtp->packet.cnt = RTP_NORTPBUF_MAGIC; //note:: when nortpbuf, use it to markup
		rtp->packet.malloc_size = expect_size;
		rtp->packet.buf_size[0] = 0;
	}

	//receive frame data
	if(rtp->mediaType == RTP_TYPE_PCMA || rtp->mediaType == RTP_TYPE_PCMU){
		// read audio frame data
		ptr = (char *)rtp->packet.buffer;
		RTP_RECV(ptr, remind, 0);
		rtp->packet.buf_size[0] = ret;
		if(rtpHeader.padding){
			rtp->packet.buf_size[0] -= ptr[ret-1];
		}
		rtp->packet.iFrameCnt = 1;
	}else if(rtp->mediaType == RTP_TYPE_H264){
		//read video frame data
		// --peek 1 bytes firstly
		RTP_RECV(tmp, 1, 0);
		remind -= 1;
		nalu.padding = tmp[0];
		// check and read the real frame data
		if(nalu.type <= 23){ // H.264 Nalu unit type
			if(nalu.type < H264_IDR){ // p slice
				ptr = (char *)rtp->packet.buffer;
				rtp->packet.buf_size[0] = 0;
			}else{// idr or sps or pps or sei
				ptr = (char *)rtp->packet.buffer + rtp->packet.buf_size[0];
			}
			ptr[0] = 0;ptr[1] = 0;ptr[2]=0;ptr[3] = 1;			
			ptr[4] = tmp[0];
			RTP_RECV(ptr + 5, remind, 0);
			rtp->packet.buf_size[0] += ret + 5;
			if(rtpHeader.padding){
				rtp->packet.buf_size[0] -= (ptr+5)[ret-1];
			}
			if(nalu.type == H264_SPS || nalu.type == H264_PPS || nalu.type == H264_SEI){
				rtp->packet.iFrameCnt = 0;
			}else{
				rtp->packet.iFrameCnt = 1;
			}
		}else if(nalu.type == RTP_FU_A){	
			//printf("ptr:%p pbuf:%p %u %d\n",ptr,pbuf,size,rtp->packet.buf_size[0]);			
			RTP_RECV(tmp, 1, 0);
			remind -= 1;
			fuh.padding = tmp[0];
			ptr = (char *)rtp->packet.buffer+rtp->packet.buf_size[0];
			if(fuh.start_bit == 1){
				// reset buffer pos
				if(fuh.type < H264_IDR){ // p slice
					ptr = (char *)rtp->packet.buffer;
					rtp->packet.buf_size[0] = 0;
				}
				rtp->packet.iFrameCnt = 0;
				//
				nalu.type = fuh.type;
				ptr[0] = 0;
				ptr[1] = 0;
				ptr[2] = 0;
				ptr[3] = 1;
				ptr[4] = nalu.padding;
				RTP_RECV(ptr + 5, remind, 0);
				rtp->packet.buf_size[0] += ret + 5;
				if(rtpHeader.padding){
					rtp->packet.buf_size[0] -= (ptr+5)[ret-1];
				}
			}else{
				if((rtp->packet.buf_size[0] + remind) >= rtp->packet.malloc_size){
					VLOG(VLOG_ERROR,"rtp video buffer exceed malloc size!!!");
					return RTP_RET_FAIL;
				}
				RTP_RECV(ptr, remind, 0);
				rtp->packet.buf_size[0] += ret;
				if(rtpHeader.padding){
					rtp->packet.buf_size[0] -= ptr[ret-1];
				}
			}
			
			if(fuh.stop_bit == 1){
				rtp->packet.iFrameCnt = 1;
			}
		}else if(nalu.type == RTP_STAP_A || nalu.type == RTP_STAP_B){
			int count = 0;
			int don = 0;
			int nsize = 0;
			int nonsps = 0;
			ptr = (char *)rtp->packet.buffer;
			rtp->packet.buf_size[0] = 0;
			rtp->packet.iFrameCnt = 0;
			while(remind > 0){				
				if(nalu.type == RTP_STAP_B){
					RTP_RECV(tmp, 4, 0);
					remind -= 4;
					don = ntohs(*((unsigned short *)tmp));
					nsize = ntohs(*((unsigned short *)(tmp + 2)));
				}else if(nalu.type == RTP_STAP_A){					
					RTP_RECV(tmp, 2, 0);
					remind -= 2;
					don = 0;
					nsize = ntohs(*((unsigned short *)(tmp)));
				}
				*ptr++ = 0;
				*ptr++ = 0;
				*ptr++ = 0;
				*ptr++ = 1;
				RTP_RECV(ptr, nsize, 0);
				remind -= ret;
				rtp->packet.buf_size[0] += ret + 4;
				VLOG(VLOG_DEBUG, "%d nalu : %d(size:%d)!", count, ret, nsize);			

				if((ptr[0] & 0x1F) <= H264_IDR){
					nonsps = 1;
				}
				count++;
				ptr += ret;
				if(rtpHeader.padding){
					if((remind < 4) && (remind > 0) && (nalu.type == RTP_STAP_B)){
						RTP_RECV(ptr, remind, 0);
					}else if((remind < 2) && (remind > 0) && (nalu.type == RTP_STAP_A)){
						RTP_RECV(ptr, remind, 0);
					}
				}else{
					if((remind < 4) && (remind > 0) && (nalu.type == RTP_STAP_B)){
						VLOG(VLOG_ERROR, "invalid stap format!");
						return -1;
					}else if((remind < 2) && (remind > 0) && (nalu.type == RTP_STAP_A)){
						VLOG(VLOG_ERROR, "invalid stap format!");
						return -1;
					}
				}
			}
			VLOG(VLOG_DEBUG, "TOTOAL nalu count: %d(size:%d)!", count, rtp->packet.buf_size[0]);	

			if(nonsps == 1){
				rtp->packet.iFrameCnt = count;
			}
		}else{
			VLOG(VLOG_ERROR, "unknown nalu type: %d!", nalu.type);
			return RTP_RET_FAIL;
		}
	}else{
		VLOG(VLOG_ERROR, "unknown payload type: %d (realtype:%d)!", rtpHeader.payload_type, rtp->mediaType);
		return RTP_RET_FAIL;
	}

	//if(rtp->packet.iFrameCnt == 1)
		VLOG(VLOG_DEBUG,"RTP::recv a packet,type:%d realtype:%d size:%d ts:%u seq:%u",
			rtpHeader.payload_type, rtp->mediaType,rtp->packet.buf_size[0],rtp->timestamp,rtp->seq);
	return RTP_RET_OK;
}


