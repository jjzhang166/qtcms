#include "sock.h"
#include "gnu_win.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#if defined(_WIN32) || defined(_WIN64)
#pragma comment (lib,"WS2_32.lib")  
#endif

#ifndef TRUE
#define TRUE	(1)
#endif
#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct ether_packet
{
	unsigned char targ_hw_addr[6];
	unsigned char src_hw_addr[6];
	unsigned short frame_type;
	//unsigned char *data;
}EtherPack_t;

static int _get_hw_addr(unsigned char* buf, char* str)
{
        int i;
        char c, val;
        for (i = 0; i < 6; i++)
        {
                if (!(c = tolower(*str++)))
                {
			printf("Invalid hardware address!\n");
			return -1;
                }
                if (isdigit(c))
                {
                        val = c - '0';
                }
                else if (c >= 'a' && c <= 'f')
                {
                        val = c - 'a' + 10;
                } 
		else if (c >= 'A' && c <= 'F')
                {
                        val = c - 'A' + 10;
                }
                else
                {
			printf("Invalid hardware address!\n");
			return -1;
                }
                *buf = val << 4;
                if (!(c = tolower(*str++)))
                {
			printf("Invalid hardware address!\n");
			return -1;
                }
                if (isdigit(c))
                {
                        val = c - '0';
                }
		else if (c >= 'a' && c <= 'f')
		{
		        val = c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'F')
		{
			val = c - 'A' + 10;
		}
		 else
		  {
			printf("Invalid hardware address!\n");
			return -1;
		}
		*buf++ |= val;
		if (*str == ':')
		{
			str++;
		}
	}

	return 0;
}


SOCK_t SOCK_new(int af,int type,int protocal)
{
	SOCK_t sock;
#if defined(_WIN32) || defined(_WIN64)
	WSADATA wsaData;
	int ret;
	ret=WSAStartup(MAKEWORD(1,1),&wsaData);
	if(ret!=0){
		printf("WSAStartup failed!\n");
		return -1;
	}
	sock=socket(af,type,protocal);
	if(sock == INVALID_SOCKET){
		printf("create socket failed!\n");
		WSACleanup( );
		return -1;
	}
#else
	sock = socket(af,type,protocal);
	if(sock < 0){
		printf("create socket failed!\n");
		return -1;
	}
#endif	
	
	return sock;
}


SOCK_t SOCK_tcp_listen(int listen_port) 
{
    int ret;
	int on = 1;
	SOCKADDR_IN_t addr;

    SOCK_t sock=SOCK_new(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) 
		return -1;
	
    ret=setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if(ret < 0){
        printf("SOCK-ERROR: set port reuse failed");
		return -1;
	}

    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons((unsigned short)listen_port);
    addr.sin_addr.s_addr=INADDR_ANY;
    ret= bind(sock,(SOCKADDR_t *)&addr,sizeof(SOCKADDR_t));
    if (ret<0) {
        printf("SOCK-ERROR: bind failed @ SOCK_ERR=%d",SOCK_ERR);
        return -1;
    }
    ret = listen(sock,32);
    if (ret<0) {
        printf("SOCK-ERROR: listen failed @ SOCK_ERR=%d",SOCK_ERR);
        return -1;
    } else {
        printf("SOCK-INFO: listen start success @%d(sock:%d)\n",listen_port, sock);
    }

    return sock;
}

int SOCK_tcp_init(SOCK_t sock,int rwtimeout/* unit: millisecond */) 
{
    int ret;
	int on = 1;

#if defined(_WIN32) || defined(_WIN64)
	int timeo = rwtimeout;
#else
    struct timeval timeo = { rwtimeout/1000, (rwtimeout%1000)*1000 };
#endif
    ret=setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(SOCKOPTARG_t *) &timeo, sizeof(timeo));
	if(ret < 0){
    	printf("SOCK-ERROR: set send timeout failed,sock:%d",sock);
		return -1;
	}
    //set receive timeout
    ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(SOCKOPTARG_t *) &timeo,sizeof(timeo));
	if(ret < 0){
    	printf("SOCK-ERROR: set receive timeout failed.");
		return -1;
	}
	
    ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (SOCKOPTARG_t *) &on, sizeof(on));
	if(ret < 0){
    	printf("SOCK-ERROR: set nodelay failed.");
		return -1;
	}
    return ret;
}

int SOCK_set_buf_size(SOCK_t sock, unsigned int sndBufSize, unsigned int rcvBufSize) /*if 0 , use default*/
{
	int ret = 0;
	unsigned int buf_size;
	SOCKLEN_t optlen;
	
	// Get buffer size
	if(sndBufSize > 0){
		optlen = sizeof(buf_size);
		ret = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&buf_size, &optlen);
		if(ret < 0){
	        printf("SOCK-ERROR: get buffer size failed");
			return -1;
		}
		else{
			printf("SOCK-DEBUG: send buffer size = %d\n", buf_size);
		}
		// Set buffer size
		ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&sndBufSize, sizeof(sndBufSize));
		if(ret < 0){
			printf("SOCK-ERROR: set buffer size:%d failed",sndBufSize);
			return -1;
		}
	}
	if(rcvBufSize > 0){
		optlen = sizeof(buf_size);
		ret = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&buf_size, &optlen);
		if(ret < 0){
	        printf("SOCK-ERROR: get recv buffer size failed");
			return -1;
		}
		else{
			printf("SOCK-DEBUG: recv buffer size = %d\n", buf_size);
		}
		// Set buffer size
		ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&rcvBufSize, sizeof(rcvBufSize));
		if(ret < 0){
			printf("SOCK-ERROR: set recv buffer size:%d failed",rcvBufSize);
			return -1;
		}
	}

	return 0;
}

SOCK_t SOCK_tcp_connect(char *ip,int port,int rwtimeout) 
{
    int ret;    
	SOCK_t sock;
	SOCKADDR_IN_t addr;

	fd_set w_set;
	unsigned long ul=1;
	struct timeval tm;
	int error=-1;
	int len=sizeof(int);

#if defined(_WIN32) || defined(_WIN64)
	int timeo = rwtimeout;
#else
    struct timeval timeo = { rwtimeout/1000, (rwtimeout%1000)*1000 };
#endif
	
    sock=SOCK_new(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) 
		return -1;

	SOCK_IOCTL(sock,FIONBIO,&ul);//设置为非阻塞模式
	
	memset(&addr, 0, sizeof(SOCKADDR_IN_t));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((unsigned short)port);
	addr.sin_addr.s_addr = inet_addr(ip);
	ret = connect(sock,(SOCKADDR_t *)&addr,sizeof(SOCKADDR_IN_t));
	if(ret < 0){
		tm.tv_sec=5;
		tm.tv_usec=0;
		FD_ZERO(&w_set);
		FD_SET(sock,&w_set);
		if(select(sock+1,NULL,&w_set,NULL,&tm)>0)
		{
			getsockopt(sock,SOL_SOCKET, SO_ERROR, (SOCKOPTARG_t *)&error, (socklen_t *)&len);
			if(error==0)
				ret=1;
			else 
				ret=0;
		}
		else
		{	
			ret=0;
		}
	}
	else
	{
		ret=1;
	}

	ul=0;
	SOCK_IOCTL(sock, FIONBIO, &ul);//设置为阻塞模式

	if(!ret)
	{
		printf("SOCK-ERROR: connect to %s:%d failed.",ip,port);
		SOCK_close(sock);
		return -1;
	}

    ret=setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (SOCKOPTARG_t *) &timeo, sizeof(timeo));
	if(ret < 0){
    	printf("SOCK-ERROR: set send timeout failed.");
		SOCK_close(sock);
		return -1;
	}
    //set receive timeout
    ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(SOCKOPTARG_t *) &timeo,sizeof(timeo));
	if(ret < 0){
    	printf("SOCK-ERROR: set receive timeout failed.");
		SOCK_close(sock);
		return -1;
	}

    //ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (SOCKOPTARG_t *) &on, sizeof(on));
	//if(ret < 0){
   // 	printf("SOCK-ERROR: set nodelay failed.");
	//	SOCK_close(sock);
	//	return -1;
	//}
	//printf("SOCK-INFO: connect to %s:%d success, fd=%d",ip,port,sock);
    return sock;
}

int SOCK_tcp_connect2(char *target_ip, int target_port, int connect_timeout, int rw_timeout)
{
#if defined(_WIN32) || defined(_WIN64)
	return SOCK_tcp_connect(target_ip,target_port,rw_timeout);
#else
	int ret = 0;
	struct sockaddr_in peer_addr;
	unsigned long unblock_flag = 0;

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(target_port);
	peer_addr.sin_addr.s_addr = inet_addr(target_ip);
	bzero(&(peer_addr.sin_zero), 8);
	
	unblock_flag = 1;
	ioctl(sock, FIONBIO, (typeof(unblock_flag)*)(&unblock_flag)); // unblocked connect mode

	///////////////////////////////////////////////////////
	ret = connect(sock, (struct sockaddr *)&peer_addr, sizeof(struct sockaddr_in));
	if(ret < 0){
		if(EINPROGRESS != errno){
			printf("SOCK-ERR: Connect to %s:%d error(\"%s\")!\n", target_ip,target_port,strerror(errno));
			close(sock);
			return -1;
		}else{
			struct timeval poll_timeo;
			fd_set wfd_set;
			FD_ZERO(&wfd_set);
			FD_SET(sock, &wfd_set);
			poll_timeo.tv_sec = connect_timeout/1000;
			poll_timeo.tv_usec = (connect_timeout % 1000) * 1000;
			ret = select(sock + 1, NULL, &wfd_set, NULL, &poll_timeo);
			if(ret <= 0){
				close(sock);
				printf("SOCK-ERR: Connect select error(\"%s\")!\n", strerror(errno));
				return -1;
			}else{
				if(FD_ISSET(sock, &wfd_set)){
					int sock_error;
					SOCKLEN_t sock_err_len = sizeof(sock_error);
					if(getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&sock_error, &sock_err_len) < 0){
						printf("SOCK-ERR: getsockopt: SO_ERROR failed(\"%s\")!\n", strerror(errno));
						close(sock);
						return -1;
					}
					if(0 != sock_error){
						printf("SOCK-ERR: getsockopt: SO_ERROR not zero(\"%s\")!\n", strerror(errno));
						close(sock);
						return -1;
					}
				} else {
					close(sock);
					return -1;
				}
			}
		}
	}

	unblock_flag = 0;
	ioctl(sock, FIONBIO, (typeof(unblock_flag)*)(&unblock_flag));

	do{
		struct timeval sock_timeo;
		//int on = 1;
		sock_timeo.tv_sec = rw_timeout/1000;
		sock_timeo.tv_usec = (rw_timeout%1000)*1000;
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &sock_timeo, sizeof(sock_timeo));
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &sock_timeo, sizeof(sock_timeo));
		//setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (SOCKOPTARG_t *) &on, sizeof(on));
	}while(0);

	return sock;
#endif
}


SOCK_t SOCK_udp_init(char *bindip, int port,int rwtimeout/* unit: millisecond */)
{
	int ret;
	int on = 1;
	int buf_size;
	SOCKLEN_t optlen;
	SOCK_t sock=SOCK_new(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN_t my_addr;

#if defined(_WIN32) || defined(_WIN64)
	int timeo = rwtimeout;
#else
    struct timeval timeo = { rwtimeout/1000, (rwtimeout%1000)*1000 };
#endif
	if (sock <=0 ) {
		return -1;
	}

	// set addr reuse
    ret=setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if(ret < 0){
        printf("SOCK-ERROR: set port reuse failed");
		SOCK_close(sock);
		return -1;
	}
	//set send timeout
	ret=setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeo, sizeof(timeo));
	if(ret < 0){
        printf("SOCK-ERROR: set send timeout failed");
		SOCK_close(sock);
		return -1;
	}
	//set receive timeout
	ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(SOCKOPTARG_t *) &timeo,sizeof(timeo));
	if(ret < 0){
        printf("SOCK-ERROR: set recv timeout failed");
		SOCK_close(sock);
		return -1;
	}
	// set buffer size
	// Get buffer size
	optlen = sizeof(buf_size);
	ret = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&buf_size, &optlen);
	if(ret < 0){
        printf("SOCK-ERROR: get buffer size failed");
		SOCK_close(sock);
		return -1;
	}
	//else{
	//	printf("SOCK-DEBUG: send buffer size = %d\n", buf_size);
	//}
	// Set buffer size
	buf_size = 16*1024;
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char *)&buf_size, sizeof(buf_size));
	if(ret < 0){
		printf("SOCK-ERROR: set buffer size:%d failed",buf_size);
		SOCK_close(sock);
		return -1;
	}
	optlen = sizeof(buf_size);
	ret = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&buf_size, &optlen);
	if(ret < 0){
        printf("SOCK-ERROR: get recv buffer size failed");
		SOCK_close(sock);
		return -1;
	}
	//else{
	//	printf("SOCK-DEBUG: recv buffer size = %d\n", buf_size);
	//}
	// Set buffer size
	buf_size = UDP_SOCK_BUF_SIZE/2;
	ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&buf_size, sizeof(buf_size));
	if(ret < 0){
		printf("SOCK-ERROR: set recv buffer size:%d failed",buf_size);
		SOCK_close(sock);
		return -1;
	}
	
	//
	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons((unsigned short)port);
	my_addr.sin_addr.s_addr = bindip ? inet_addr(bindip) : INADDR_ANY;
	//bind 
	ret = bind(sock, (SOCKADDR_t*)&my_addr, sizeof(SOCKADDR_t));
	if(ret < 0){
        printf("SOCK-ERROR: udp bind failed");
		SOCK_close(sock);
		return -1;
	}
	//printf("SOCK-DEBUG:create udp port:%d(sock:%d) ok.",port,sock);

	return sock;
}

SOCK_t SOCK_broadcast_udp_init(char *bindip, int port,int rwtimeout/* unit: millisecond */)
{
	int ret;
	int on = 1;
	SOCK_t sock=SOCK_new(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN_t my_addr;

#if defined(_WIN32) || defined(_WIN64)
	int timeo = rwtimeout;
#else
    struct timeval timeo = { rwtimeout/1000, (rwtimeout%1000)*1000 };
#endif
	if (sock <=0 ) {
		return -1;
	}

	// set addr reuse
	on = 1;
    	ret=setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if(ret < 0){
       	 	printf("SOCK-ERROR: set port reuse failed");
		SOCK_close(sock);
		return -1;
	}

	on = 1;
	ret=setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof(on));
	if(ret < 0){
	    	printf("SOCK-ERROR: set broadcast failed");
		SOCK_close(sock);
		return -1;
	}
	
	//set send timeout
	ret=setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeo, sizeof(timeo));
	if(ret < 0){
        		printf("SOCK-ERROR: set send timeout failed");
		SOCK_close(sock);
		return -1;
	}
	//set receive timeout
	ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(SOCKOPTARG_t *) &timeo,sizeof(timeo));
	if(ret < 0){
        		printf("SOCK-ERROR: set recv timeout failed");
		SOCK_close(sock);
		return -1;
	}
	//
	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons((unsigned short)port);
	my_addr.sin_addr.s_addr = bindip ? inet_addr(bindip) : INADDR_ANY;
	//bind 
	ret = bind(sock, (SOCKADDR_t*)&my_addr, sizeof(SOCKADDR_t));
	if(ret < 0){
        		printf("SOCK-ERROR: udp bind failed");
		SOCK_close(sock);
		return -1;
	}
	//printf("SOCK-DEBUG:create udp port:%d(sock:%d) ok.",port,sock);

	return sock;
}


SOCK_t SOCK_multicast_udp_init(char *group, int port,int rwtimeout/* unit: millisecond */, char *binda)
{
	int ret;
	int on = 1;
	struct ip_mreq mcaddr;
	SOCK_t sock=SOCK_new(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN_t my_addr;

#if defined(_WIN32) || defined(_WIN64)
	int timeo = rwtimeout;
#else
    struct timeval timeo = { rwtimeout/1000, (rwtimeout%1000)*1000 };
#endif

	if (sock <= 0)
		return -1;

	// set addr reuse
        ret=setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if(ret < 0){
        printf("SOCK-ERROR: set port reuse failed");
		SOCK_close(sock);
		return -1;
	}
	//set send timeout
	ret=setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeo, sizeof(timeo));
	if(ret < 0){
        		printf("SOCK-ERROR: set send timeout failed");
		SOCK_close(sock);
		return -1;
	}
	//set receive timeout
	ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(SOCKOPTARG_t *) &timeo,sizeof(timeo));
	if(ret < 0){
        		printf("SOCK-ERROR: set recv timeout failed");
		SOCK_close(sock);
		return -1;
	}

	//
	memset(&my_addr,0,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons((unsigned short)port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	//my_addr.sin_addr.s_addr = binda ? inet_addr(binda) : INADDR_ANY;
	//bind 
	ret = bind(sock, (SOCKADDR_t*)&my_addr, sizeof(SOCKADDR_t));
	if(ret < 0){
        		printf("SOCK-ERROR: udp bind %s failed errno:%d\n", binda ? binda : "", SOCK_ERR);
		SOCK_close(sock);
		return -1;
	}
	//printf("SOCK-INFO: udp bind %s ok\n", binda ? binda : "");

	memset(&mcaddr,0,sizeof(struct ip_mreq));
	// set src ip
	mcaddr.imr_interface.s_addr = binda ? inet_addr(binda) : INADDR_ANY;
	// set multicast address
	mcaddr.imr_multiaddr.s_addr = inet_addr(group);
	//add membership
	if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&mcaddr,sizeof(struct ip_mreq))<0){
		printf("add to membership failed,errno:%d!\n",SOCK_ERR);		
		SOCK_close(sock);
		return -1;
	}
	//printf("add to membership:%s ok!\n",group);

	return sock;
}

SOCK_t SOCK_raw_init(int protocal, int rwtimeout)
{
#if defined(_WIN32) || defined(_WIN64)
	printf("ERROR : not implement!!!!\n");
	return -1;
#else

        int ret, sock;
#if defined(_WIN32) || defined(_WIN64)
	int timeo = rwtimeout;
#else
	struct timeval timeo = { rwtimeout/1000, (rwtimeout%1000)*1000 };
#endif

	/*
	if (type == OP_ARP_REQUEST || type == OP_ARP_REPLY) {
		protocal = ETH_P_ARP;
	}else if (type == OP_RARP_REQUEST || type == OP_RARP_REPLY) {
		protocal = ETH_P_RARP;
	} */
	sock = SOCK_new(AF_INET, SOCK_PACKET, htons(protocal));
	if (sock < 0)
	{
		return -1;
	}
		
	if (rwtimeout > 0) {
		ret=setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO,(SOCKOPTARG_t *) &timeo, sizeof(timeo));
		if(ret < 0){
			printf("HIK-ERROR: set send timeout failed,sock:%d \n",sock);
			SOCK_close(sock);
			return -1;
		}
		//set receive timeout
		ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(SOCKOPTARG_t *) &timeo,sizeof(timeo));
		if(ret < 0){
			printf("HIK-ERROR: set receive timeout failed.\n");
			SOCK_close(sock);
			return -1;
		}
	}

	return sock;
#endif
}

/*
	if target_mac is NULL, given should contain the ether header
	else you should given target_mac & frame_type
*/
int SOCK_raw_sendto(SOCK_t sock, char *bind_ether, char *target_mac, int frame_type, char *buf, int size)
{
#if defined(_WIN32) || defined(_WIN64)
    printf("ERROR : not implement!!!!\n");
	return -1;
#else
	int ret;
	SOCKADDR_t addr;
	int data_size = size;
	char *pbuf = buf;

	if (bind_ether == NULL) return -1;
	
	if (target_mac) {
		EtherPack_t *ether;
		char thiz_mac[32];
		
		pbuf = (char  *)malloc(size + sizeof(EtherPack_t));
		if (pbuf == NULL) return -1;

		ether = (EtherPack_t *)pbuf;
		if (SOCK_get_ether_ip(bind_ether, NULL, NULL, thiz_mac) < 0) {
			free(pbuf);
			return -1;
		}
		if (_get_hw_addr(ether->src_hw_addr, thiz_mac) < 0) {
			free(pbuf);
			return -1;
		}
		if (_get_hw_addr(ether->targ_hw_addr, target_mac) < 0) {
			free(pbuf);
			return -1;
		}
		ether->frame_type = htons(frame_type);
		
		memcpy(pbuf + sizeof(EtherPack_t) , buf, size);
		data_size += sizeof(EtherPack_t);
	} 

	memset(&addr, 0, sizeof(SOCKADDR_t));
	strcpy(addr.sa_data, bind_ether);

	ret=sendto(sock,pbuf,data_size,0,(SOCKADDR_t *)&addr,sizeof(SOCKADDR_IN_t));
	if (target_mac) 
		free(pbuf);
	if(ret != data_size){
		printf("SOCK-ERROR: raw packet send failed, SOCK_ERR:%d\n", SOCK_ERR);
		return RET_FAIL;
	}

	return RET_OK;
#endif
	
}

int SOCK_recv(SOCK_t sock,char *buf,int size,int flag) 
{
    int ret=0;
    ret=recv(sock,buf,size,flag);
	if(ret < 0){
		printf("SOCK-ERROR: read failed, errno: %d", SOCK_ERR);
		return -1;
	}else if(ret == 0){
		printf("SOCK-ERROR: peer is shut down");
		return -1;
	}    
	//printf("SOCK-DEBUG:tcp recv %d\n",ret);
	buf[ret] = 0;
    return ret;
}


unsigned int SOCK_recv2(SOCK_t fd,char *buf,unsigned int size,int flag) 
{
    int ret=0;
    unsigned int received=0;
    char *pbuf=buf;
	if(size <= 0) return 0;
    while (1) {
        ret=recv(fd,pbuf,size-received,flag);
        if (ret < 0) {
            if (SOCK_ERR==SOCK_EINTR) {
                printf("SOCK-DEBUG: tcp recv error SOCK_EINTR \n");
                continue;
            }else if (SOCK_ERR==SOCK_EAGAIN) {
                //printf("SOCK-DEBUG: tcp recv error SOCK_EAGAIN in nonblocking mode, %u/%u\n", received, size);
                return received;
            } else if (SOCK_ERR==SOCK_ETIMEOUT) {
                printf("SOCK-ERROR:  tcp recv time out \n");
                return 0;
            }
            printf("SOCK-ERROR: tcp recv error @%d \n",SOCK_ERR);
            return -1;
        } else if (ret==0) {
            return -1;
        } else {
            pbuf+=ret;
            received+=ret;
        }
		if(received == size) break;
    }
    //buf[received]=0;
    //printf("SOCK-DEBUG: tcp recv %d\n",received);
    return received;
}

unsigned int SOCK_recv3(SOCK_t fd,char *buf,unsigned int size,int flag) 
{
    int ret=0;
    unsigned int received=0;
    char *pbuf=buf;
	if(size <= 0) return 0;
    while (1) {
        ret=recv(fd,pbuf,size-received,flag);
        if (ret < 0) {
            if (SOCK_ERR==SOCK_EINTR) {
                printf("SOCK-DEBUG: tcp recv error SOCK_EINTR \n");
                continue;
            }else if (SOCK_ERR==SOCK_EAGAIN) {
                //printf("SOCK-DEBUG: tcp recv error SOCK_EAGAIN in nonblocking mode, %u/%u\n", received, size);
                return -1;
            } else if (SOCK_ERR==SOCK_ETIMEOUT) {
                printf("SOCK-ERROR:  tcp recv time out \n");
                return 0;
            }
            printf("SOCK-ERROR: tcp recv error @%d \n",SOCK_ERR);
            return -1;
        } else if (ret==0) {
        	   //printf("peer close\n");
            return received;
        } else {
            pbuf+=ret;
            received+=ret;
        }
		if(received == size) break;
    }
    //buf[received]=0;
    //printf("SOCK-DEBUG: tcp recv %d\n",received);
    return received;
}


int SOCK_send(SOCK_t sock,char *buf,unsigned int size) 
{
	fd_set wset;
	int ret;
	struct timeval timeo;
	unsigned int remind = size;
	char *ptr = buf;
	
	while(remind > 0){
		timeo.tv_sec = 3;
		timeo.tv_usec = 0;
		FD_ZERO(&wset);
		FD_SET(sock, &wset);
		ret = select(sock + 1, NULL, &wset, NULL, & timeo);
		if(ret > 0){
			if(FD_ISSET(sock, &wset)){
				ret=send(sock, ptr, remind,0);
				if(ret < 0){
					if(SOCK_ERR == SOCK_EAGAIN){
						MSLEEP(1);
						continue;
					}
					else if(SOCK_ERR == SOCK_EINTR){
						continue;
					}else{
						goto ERR_EXIT;
					}
				}else{
					ptr += ret;
					remind -= ret;
				}
				
			}else{
				goto ERR_EXIT;
			}
		}else if(ret == 0){
			continue;
		}else{
			goto ERR_EXIT;
		}
	}
	
	//printf("sock send (size:%d) ok\n", size);
	return RET_OK;

ERR_EXIT:
	printf("sock send (size:%d) failed #errno:%d\n", size, SOCK_ERR);
	return RET_FAIL;
}

int SOCK_send2(SOCK_t sock,char *buf,unsigned int size) 
{
	fd_set wset;
	int ret;
	struct timeval timeo;
	unsigned int remind = size;
	char *ptr = buf;
	
	while(remind > 0){
		timeo.tv_sec = 3;
		timeo.tv_usec = 0;
		FD_ZERO(&wset);
		FD_SET(sock, &wset);
		ret = select(sock + 1, NULL, &wset, NULL, & timeo);
		if(ret > 0){
			if(FD_ISSET(sock, &wset)){
				ret=send(sock, ptr, (remind >= 1280) ? 1280 : remind,0);
				if(ret < 0){
					if(SOCK_ERR == SOCK_EAGAIN){
						MSLEEP(1);
						continue;
					}
					else if(SOCK_ERR == SOCK_EINTR){
						continue;
					}else{
						goto ERR_EXIT;
					}
				}else{
					ptr += ret;
					remind -= ret;
				}
			}else{
				goto ERR_EXIT;
			}
		}else if( ret == 0){
			continue;
		}else{
			goto ERR_EXIT;
		}
	}
	
	//printf("sock send (size:%d) ok\n", size);
	return RET_OK;

ERR_EXIT:
	printf("sock send (size:%d) failed #errno:%d\n", size, SOCK_ERR);
	return RET_FAIL;
}


int SOCK_recvfrom(SOCK_t sock,char *ip,int *port,char *buf,int size,int flags) 
{
	int ret;
	SOCKADDR_IN_t addr;
	SOCKLEN_t addrlen=sizeof(SOCKADDR_IN_t);
	memset(&addr, 0, sizeof(SOCKADDR_IN_t));
	/*
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(ip == NULL)
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		addr.sin_addr.s_addr = inet_addr(ip);
		*/
	ret=recvfrom(sock,buf,size,flags,(SOCKADDR_t *)&addr,&addrlen);
	if(ret < 0){
		if (SOCK_ERR == SOCK_EAGAIN) {
			return SOCK_EAGAIN;
		}
		printf("SOCK-ERROR: udp recvfrom failed,size:%d sock:%d buf:%p SOCK_ERR:%d",
			size,sock,buf,SOCK_ERR);
		return RET_FAIL;
	}else if(ret == 0){
		printf("SOCK-ERROR: peer is shut down");
		return RET_FAIL;
	}
	//printf("udp recvfrom(%s:%d) success,size:%d",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),ret);
	//printf("SOCK-DEBUG: udp recvfrom(%s:%d) success,size:%d",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),ret);

	strcpy(ip,inet_ntoa(addr.sin_addr));
	*port = ntohs(addr.sin_port);
	
	return ret;
}

int SOCK_sendto(SOCK_t sock,char *ip,int port,char *buf,int size) 
{
	int ret;
	SOCKADDR_IN_t addr;
	memset(&addr, 0, sizeof(SOCKADDR_IN_t));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((unsigned short)port);
	addr.sin_addr.s_addr = inet_addr(ip);

	ret=sendto(sock,buf,size,0,(SOCKADDR_t *)&addr,sizeof(SOCKADDR_IN_t));
	if(ret != size){
		printf("SOCK-ERROR: udp send to %s:%d failed,size:%d sock:%d buf:%p SOCK_ERR:%d\n",
			ip,port,size,sock,buf,SOCK_ERR);
		return RET_FAIL;
	}
	//printf("SOCK-DEBUG: udp send to%s:%d uccess,size:%d\n",ip,port,size);
	return RET_OK;
}

int SOCK_getpeername(SOCK_t sock,char *peer)
{
	SOCKADDR_t addr;
	SOCKADDR_IN_t *addr_in=(SOCKADDR_IN_t *)&addr;
	SOCKLEN_t sock_len=sizeof(SOCKADDR_t);
	if(getpeername(sock,&addr,&sock_len)!=0){
		printf("SOCK-ERROR: get peer name failed");
		return RET_FAIL;
	}
	strcpy(peer,inet_ntoa(addr_in->sin_addr));
	//printf("SOCK-DEBUG: peer name:%s",peer);

	return RET_OK;
}

int SOCK_getsockname(SOCK_t sock,char *ip)
{
	SOCKADDR_t addr;
	SOCKADDR_IN_t *addr_in=(SOCKADDR_IN_t *)&addr;
	SOCKLEN_t sock_len=sizeof(SOCKADDR_t);
	if(getsockname(sock,&addr,&sock_len)!=0){
		printf("SOCK-ERROR: get peer name failed");
		return RET_FAIL;
	}
	strcpy(ip,inet_ntoa(addr_in->sin_addr));
	//printf("SOCK-DEBUG: sock name:%s",ip);

	return RET_OK;
}

int SOCK_getsockport(SOCK_t sock,unsigned short * const port)
{
	SOCKADDR_t addr;
	SOCKADDR_IN_t *addr_in=(SOCKADDR_IN_t *)&addr;
	SOCKLEN_t sock_len=sizeof(SOCKADDR_t);
	if(getsockname(sock,&addr,&sock_len)!=0){
		printf("SOCK-ERROR: get peer name failed");
		return RET_FAIL;
	}
	*port = ntohs(addr_in->sin_port);
	printf("SOCK-PORT: sock port:%d",*port);

	return RET_OK;
}

int SOCK_gethostbyname(char *name,char *ip) 
{
    struct hostent *hent;
	struct in_addr addr;
	int i;

    hent = gethostbyname(name);
	if(hent == NULL){
		printf("gethostbyname failed!\n");
		return -1;
	}
	for(i = 0; hent->h_aliases[i];i++)
		printf("aliase%d %s\n",i+1,hent->h_aliases[i]);
    printf("hostname: %s addrtype:%d\naddress list: ", hent->h_name,hent->h_addrtype);
	if(hent->h_addrtype == AF_INET){
		for(i = 0; hent->h_addr_list[i]; i++) {
#if defined(_WIN32) || defined(_WIN64)
			addr.s_addr = *(u_long *)hent->h_addr_list[i];
#else
			addr.s_addr = *(unsigned int *)hent->h_addr_list[i];
#endif
			printf("<<%d>> %s\n", i+1,inet_ntoa(addr));
			if(i == 0){
				strcpy(ip,inet_ntoa(addr));
			}
		}
		
	}else{
		return -1;
	}
    return 0;
}

int SOCK_getallhostip(void (*f_Add)(char *ip)) 
{
#if defined(_WIN32) || defined(_WIN64)          
	printf("not support!!!!!!!!!!\n"); 
	return -1;
#else
#ifdef HAVE_IFADDRS_H	
    struct ifaddrs * ifAddrStruct=NULL,*ifaddr_bak;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);
	ifaddr_bak = ifAddrStruct;

    while (ifAddrStruct!=NULL) {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4
	            // is a valid IP4 Address
	            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
	            char addressBuffer[INET_ADDRSTRLEN];
	            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
	            printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
		if(strcmp(ifAddrStruct->ifa_name,"lo")!=0 ){
			if(f_Add) f_Add(addressBuffer);
		}
        } else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer); 
        } 
        ifAddrStruct=ifAddrStruct->ifa_next;
    }	
	if(ifaddr_bak) freeifaddrs(ifaddr_bak);
	return 0;
#else
	printf("not support!!!!!!!!!!\n"); 
	return -1;
#endif
#endif
    return 0;
}



int SOCK_getsockinfo(SOCK_t sock,char *ip,char *mask)
{
#if defined(_WIN32) || defined(_WIN64)
	return -1;
#else
	SOCKADDR_IN_t *my_ip;
	SOCKADDR_IN_t *addr;
	SOCKADDR_IN_t myip;	
	struct ifreq ifr;

	my_ip = &myip;
	// get local ip of eth0
	strcpy(ifr.ifr_name, "eth0");
	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		printf("SOCK-ERROR: get local ip failed");
		return -1;
	}
	my_ip->sin_addr = ((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr;
	strcpy(ip,inet_ntoa(my_ip->sin_addr));

	// get local netmask
	if( ioctl( sock, SIOCGIFNETMASK, &ifr) == -1 ){
		printf("SOCK-ERROR: get netmask failed");
		return -1;
	}
	addr = (struct sockaddr_in *) & (ifr.ifr_addr);
	strcpy(mask,inet_ntoa( addr->sin_addr));

	printf("SOCK-INFO: sockinfo: ip:%s mask:%s",ip,mask);
	return 0;
#endif
}

int SOCK_get_blockmode(SOCK_t sock)
{
	int ret=0;
#if defined(_WIN32) || defined(_WIN64)
	return -1;
#else
	ret= fcntl(sock,F_GETFL,0);
	if(ret < 0){
		printf("SOCK-ERROR: SOCK fcntl failed\n");
		return RET_FAIL;
	}
	if(ret & O_NONBLOCK){
		printf("SOCK-INFO: SOCK in nonblock mode\n");
		return FALSE;
	}else{
		printf("SOCK-INFO: SOCK in block mode\n");
		return TRUE;
	}
#endif
}


int SOCK_set_blockmode(SOCK_t sock,int enable)
{
	int ret=0;
#if defined(_WIN32) || defined(_WIN64)

#else
	ret= fcntl(sock,F_GETFL,0);
	if(ret < 0){
		printf("SOCK-ERROR: SOCK fcntl failed");
		return RET_FAIL;
	}	
	if(ret & O_NONBLOCK){
		printf("SOCK-INFO: SOCK in nonblock mode\n");
	}else{
		printf("SOCK-INFO: SOCK in block mode\n");
	}
	if(enable == TRUE){
		ret &= ~O_NONBLOCK;
	}else{
		ret |= O_NONBLOCK;
	}
	if(fcntl(sock,F_SETFL,ret) < 0){
		printf("SOCK-ERROR: SOCK set fcntl failed\n");
		return RET_FAIL;
	}
#endif

	return RET_OK;
}

int SOCK_set_rcvlowat(int sock, int count)
{
	int ret;
	ret = setsockopt(sock,SOL_SOCKET,SO_RCVLOWAT,(char *)&count,sizeof(count));
	if(ret < 0){
		printf("SOCK_set_rcvlowat failed1\n");
		return -1;
	}
	return 0;
}

int SOCK_set_sndlowat(int sock, int count)
{
	int ret;
	ret = setsockopt(sock,SOL_SOCKET,SO_SNDLOWAT,(char *)&count,sizeof(count));
	if(ret < 0){
		printf("SOCK_set_sndlowat failed1\n");
		return -1;
	}
	return 0;
}


int SOCK_isreservedip(char *szIp)
{
	int ret;
	int flag = FALSE;
	int ip[4];
	ret=sscanf(szIp,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
	if(ret != 4){
		printf("SOCK-ERROR: ipaddr:%s invalid",szIp);
		return -1;
	}
	if(ip[0] == 10) flag = TRUE;
	if((ip[0] == 192) && (ip[1] == 168)) flag = TRUE;
	if((ip[0] == 172) && (ip[1] >= 16) && (ip[1] <= 31)) flag = TRUE;
	
	return flag;
}

// multicast ip address
// use for router and other function:224.0.0.0~224.0.0.255
// reserved: 224.0.1.0~238.255.255.255
// 239.0.0.0~239.255.255.255
int SOCK_add_membership(SOCK_t sock,char *multi_ip)
{
	struct ip_mreq mcaddr;
	memset(&mcaddr,0,sizeof(struct ip_mreq));
	// set src ip
	mcaddr.imr_interface.s_addr = htonl(INADDR_ANY);
	// set multicast address
	mcaddr.imr_multiaddr.s_addr = inet_addr(multi_ip);
	//if(inet_pton(AF_INET,multi_ip,&mcaddr.imr_multiaddr) <=0){
	//	printf("wrong multicast ipaddress!\n");
	//	return -1;
	//}
	//add membership
	if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&mcaddr,sizeof(struct ip_mreq))<0){
		printf("add to membership failed,errno:%d!\n",SOCK_ERR);
		return -1;
	}
	printf("add to membership:%s ok!\n",multi_ip);
	return 0;
}

int SOCK_set_broadcast(int sock)
{
	int so_broadcast = 1;
	int ret;
	ret = setsockopt(sock,SOL_SOCKET,SO_BROADCAST,(char *)&so_broadcast,sizeof(so_broadcast));
	if(ret < 0){
		printf("set broadcast failed1\n");
		return -1;
	}
	return 0;
}

int SOCK_get_ether_ip(char *eth, char * const ip, char * const netmask, char * const mac)
{
#if defined(_WIN32) || defined(_WIN64)
	printf("ERROR: %s unsupperted in this os!\n", __FUNCTION__);
	return -1;
#else
	int sock;
	//
	struct sockaddr_in sin;
	struct ifreq ifr;

	if(ip) 			ip[0] = 0;
	if(netmask) 	netmask[0] = 0;
	if(mac) 		mac[0]= 0;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		perror("socket");
		return -1;	 
	}  

	strncpy(ifr.ifr_name, eth, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0; 

	if(ip){
		if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) 
		{
			perror("ioctl");
			close(sock);
			return -1;
		}
		memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
		strcpy(ip, inet_ntoa(sin.sin_addr));
	}

	if(netmask){
		if (ioctl(sock, SIOCGIFNETMASK, &ifr)< 0) 
		{
			perror("ioctl");
			close(sock);
			return -1;
		}
		memcpy(&sin, &ifr.ifr_addr, sizeof(sin));  
		strcpy(netmask, inet_ntoa(sin.sin_addr));
	}

	if(mac){
		if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0){
			perror("ioctl");
			close(sock);
			return -1;
		}
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", ifr.ifr_hwaddr.sa_data[0] & 0xff,ifr.ifr_hwaddr.sa_data[1] & 0xff,ifr.ifr_hwaddr.sa_data[2] & 0xff,
			ifr.ifr_hwaddr.sa_data[3] & 0xff,ifr.ifr_hwaddr.sa_data[4] & 0xff,ifr.ifr_hwaddr.sa_data[5] & 0xff);
	}

	close(sock);
	return 0;
#endif
}

int SOCK_get_gateway(char * const gateway)
{
#if defined(_WIN32) || defined(_WIN64)
	printf("ERROR: %s unsupperted in this os!\n", __FUNCTION__);
	return -1;
#else
	FILE *fp;	
	int i;
	char buf[256]; // 128 is enough for linux
	char iface[16];
	unsigned char _gateway[4];
	unsigned long dest_addr;
	unsigned long gate_addr;
	int is_found = 0;

	gateway[0] = 0;
	fp = fopen("/proc/net/route", "r");
	if(fp != NULL)
	{
		fgets(buf, sizeof(buf), fp);
		while (fgets(buf, sizeof(buf), fp))
		{
			if (sscanf(buf, "%s\t%lX\t%lX", iface, &dest_addr, &gate_addr) != 3 || dest_addr != 0)
			{
				continue;
			}
			else
			{
				is_found = 1;
				break;
			}
		}
		fclose(fp);
	}


	if(is_found == 1)
	{
		for(i = 0; i < 4; i++)
		{
			*(_gateway+i) = (gate_addr >> (i*8)) & 0xff;
		}
		sprintf(gateway, "%d.%d.%d.%d", _gateway[0], _gateway[1], _gateway[2], _gateway[3]);
		return 0;
	}
	else
	{
		return -1;
	}
#endif
}


int SOCK_keep_alive(int sock, int keepidle, int keepintvl, int keepcnt)
{
#if defined(_WIN32) || defined(_WIN64)
	printf("ERROR: %s unsupperted in this os!\n", __FUNCTION__);
	return -1;
#else
	int ret;
	int keepalive = 1;
	ret=setsockopt(sock,SOL_SOCKET,SO_KEEPALIVE,(SOCKOPTARG_t *) &keepalive,sizeof(keepalive));
	if(ret < 0){
        		printf("SOCK-ERROR: set sockopt failed, error:%d\n", SOCK_ERR);
		return -1;
	}
	ret=setsockopt(sock,SOL_TCP,TCP_KEEPIDLE,(SOCKOPTARG_t *) &keepidle,sizeof(keepidle));
	if(ret < 0){
        		printf("SOCK-ERROR: set sockopt failed, error:%d\n", SOCK_ERR);
		return -1;
	}
	ret=setsockopt(sock,SOL_TCP,TCP_KEEPINTVL,(SOCKOPTARG_t *) &keepintvl,sizeof(keepintvl));
	if(ret < 0){
        		printf("SOCK-ERROR: set sockopt failed, error:%d\n", SOCK_ERR);
		return -1;
	}
	ret=setsockopt(sock,SOL_TCP,TCP_KEEPCNT,(SOCKOPTARG_t *) &keepcnt,sizeof(keepcnt));
	if(ret < 0){
        		printf("SOCK-ERROR: set sockopt failed, error:%d\n", SOCK_ERR);
		return -1;
	}
	return 0;
#endif
}

int SOCK_get_ip_only(char *eth, char * const ip)
{
#if defined(_WIN32) || defined(_WIN64)
	printf("ERROR: %s unsupperted in this os!\n", __FUNCTION__);
	return -1;
#else
        int sock;
        //
        struct sockaddr_in sin;
        struct ifreq ifr;

        if(ip)                  ip[0] = 0;
        
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == -1)
        {
                perror("socket");
                return -1;       
        }  

        strncpy(ifr.ifr_name, eth, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = 0; 

        if(ip){
                if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) 
                {
                        perror("ioctl");
                        close(sock);
                        return -1;
                }
                memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
                //strcpy(ip, inet_ntoa(sin.sin_addr));
                memcpy(ip, &sin.sin_addr, sizeof(sin.sin_addr));
        }

        close(sock);
        return 0;
#endif
}

int SOCK_check_nic(char *nic_name)
{
#if defined(_WIN32) || defined(_WIN64)
	printf("ERROR: %s unsupperted in this os!\n", __FUNCTION__);
	return -1;
#else
        struct ifreq ifr;
        int skfd = socket(AF_INET, SOCK_DGRAM, 0);

        if ( skfd < 0 ) return 0;
                strcpy(ifr.ifr_name, nic_name);
        if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
        {
                close(skfd);
                return 0;
        }
        close(skfd);
        if(ifr.ifr_flags & IFF_RUNNING) {               
                 return 1;
        } else {
                return 0;
        }
#endif
}


#ifdef SOCK_TEST
int main(int argc, char *argv[])
{
	int ret;
	SOCK_t sock;	
	char linestr[256];
	char ip[20] = "127.0.0.1";
	unsigned short port =0;
	fd_set wset, rset;
	struct timeval timeout;
	const char *usage=\
		"/******************sock module test engin********************/\r\n"
		"/****************** -tcplisten *****************************/\r\n"	
		"/****************** -tcpsend ******************************/\r\n"
		"/****************** -udplisten *****************************/\r\n"	
		"/****************** -udpsend *****************************/\r\n";		
	if(argc < 2){
		printf(usage);
		return 0;
	}
	if(argc >= 3){
		port = atoi(argv[2]);
	}
	if(argc >= 4){
		strcpy(ip, argv[3]);
	}
	printf("ip: %s port:%u cmd:%s\n", ip, port, argv[1]);
	if (strcmp(argv[1], "-tcplisten") == 0) {
		sock = SOCK_tcp_listen(port);
		int i;
		for (i = 0; i < 512; i++) {
			SOCK_t s = SOCK_tcp_listen(12000+i);
			if (s < 0) {
				printf("tcp sock(port @ %d) new failed!", 12000+i);
			}
		}
		if (sock) {
			for (; ; )
			{
				FD_ZERO(&rset);
				FD_SET(sock, &rset);
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;
				ret = select( sock + 1, &rset, NULL, NULL, &timeout);
				if (ret < 0) {
					printf("select error : %d \n" , SOCK_ERR);
					break;
				} else if (ret == 0){
					printf("select timeout \n");
				} else {
					if ((ret = SOCK_recv(sock, linestr, sizeof(linestr), 0)) <= 0) {
						break;
					}
					printf("recv %d :\n%s\n", ret, linestr);
				}
			}
		}
	}
	else if (strcmp(argv[1], "-tcpsend") == 0) {
		sock = SOCK_tcp_connect2(ip, port, 2000, 5000);
		if (sock > 0) {
			while(gets(linestr) != NULL) {
				printf("getstr:%s\n", linestr);
				if (strcmp(linestr, "quit")  == 0) break;
				SOCK_send(sock, linestr, strlen(linestr));
			}
			printf("getstr failed\n");
		}
	}
	else if (strcmp(argv[1], "-udplisten") == 0) {
		sock = SOCK_udp_init(NULL, port, 5000);
		
		int i;
		for (i = 0; i < 1024; i++) {
			SOCK_t s = SOCK_udp_init(12000+i, 1000);
			if (s < 0) {
				printf("udp sock(port @ %d) new failed!", 12000+i);
			}
		}
		if (sock) {
			for (; ; )
			{
				FD_ZERO(&rset);
				FD_SET(sock, &rset);
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;
				ret = select( sock + 1, &rset, NULL, NULL, &timeout);
				if (ret < 0) {
					printf("select error : %d \n" , SOCK_ERR);
					break;
				} else if (ret == 0){
					printf("select timeout \n");
				} else {
					if ((ret = SOCK_recvfrom(sock, NULL, NULL, linestr, sizeof(linestr), 0)) <= 0) {
						break;
					}
					printf("recv %d :\n%s\n", ret, linestr);
				}
			}
		}
	}
	else if (strcmp(argv[1], "-udpsend") == 0) {
		sock = SOCK_udp_init(NULL, 0, 5000);
		if (sock > 0) {
			while(gets(linestr) != NULL) {
				if (strcmp(linestr, "quit")  == 0) break;
				SOCK_sendto(sock, ip, port, linestr, strlen(linestr));
			}
		}
	}
	
	while(getchar() != 'q') ;
	return 0;
}
#endif

