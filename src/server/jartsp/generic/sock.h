#ifndef __RTSP_SOCK_H__
#define __RTSP_SOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#define UDP_SOCK_BUF_SIZE	(512*1024)

#ifndef RET_OK
#define RET_OK	(0)
#endif
#ifndef RET_FAIL
#define RET_FAIL	(-1)
#endif

#if defined(_WIN32) || defined(_WIN64)
	//#include <winsock.h>
	#include <winsock2.h>
	#include <windows.h>
	#include <windef.h>
	#include <WS2TCPIP.H>
	#include <errno.h>

	typedef SOCKET	SOCK_t;
	typedef int SOCKLEN_t; 
	typedef SOCKADDR SOCKADDR_t;
	typedef SOCKADDR_IN SOCKADDR_IN_t;
	typedef char SOCKOPTARG_t;

	#define SOCK_close(s) \
		do{\
			closesocket(s);\
			WSACleanup();\
		}while(0)
	#define SOCK_IOCTL		ioctlsocket
	#define SOCK_ERR		WSAGetLastError()
	#define SOCK_EAGAIN		WSAEWOULDBLOCK
	#define SOCK_ETIMEOUT	WSAETIMEDOUT
	#define SOCK_EINTR		WSAEINTR

#else

	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
#ifdef HAVE_IFADDRS_H	
	#include <ifaddrs.h>
#endif
	#include <fcntl.h>
	#include <errno.h>

	typedef int	SOCK_t;
	typedef socklen_t SOCKLEN_t; 
	typedef struct sockaddr SOCKADDR_t;
	typedef struct sockaddr_in SOCKADDR_IN_t;
	typedef const void SOCKOPTARG_t;

	#define SOCK_close(s)	close(s)
	#define SOCK_IOCTL		ioctl
	#define SOCK_ERR		errno

	#define SOCK_EAGAIN		EAGAIN
	#define SOCK_ETIMEOUT	ETIMEDOUT
	#define SOCK_EINTR		EINTR

#endif


#define DEFAULT_SOCK_TIMEOUT	(5000)

SOCK_t SOCK_tcp_listen(int listen_port);
SOCK_t SOCK_tcp_connect(char *ip,int port,int rwtimeout); 
int SOCK_tcp_connect2(char *ip, int port, int connect_timeout, int rwtimeout);//non-blocking mode
int SOCK_set_buf_size(SOCK_t sock, unsigned int sndBufSize, unsigned int rcvBufSize); /*if 0 , use default*/
int SOCK_tcp_init(SOCK_t fd,int rwtimeout);
SOCK_t SOCK_udp_init(char *bindip, int bind_port,int rwtimeout);
SOCK_t SOCK_broadcast_udp_init(char *bindip, int port,int rwtimeout/* unit: millisecond */);
SOCK_t SOCK_multicast_udp_init(char *group, int port,int rwtimeout/* unit: millisecond */, char *binda);
SOCK_t SOCK_raw_init(int protocal, int timeout);
int SOCK_raw_sendto(SOCK_t sock, char *bind_ether, char *target_mac, int frame_type, char *buf, int size);
int SOCK_recv(SOCK_t sock,char *buf,int iBufSize,int flag);// recv one time
unsigned int SOCK_recv2(SOCK_t sock,char *buf,unsigned int iToReadSize,int flag);// recv until reach the the size to be read or error occur or EAGAIN
   // if recv EAGAIN, it would return data readed, if peer close , return -1;
unsigned int SOCK_recv3(SOCK_t fd,char *buf,unsigned int size,int flag) ; // recv until peer close
int SOCK_send(SOCK_t sock,char *buf,unsigned int size);
int SOCK_recvfrom(SOCK_t sock,char *ip,int *port,char *buf,int size,int flags);
int SOCK_sendto(SOCK_t sock,char *ip,int port,char *buf,int size); 
int SOCK_gethostbyname(char *name,char *ip);
int SOCK_getallhostip(void (*f_Add)(char *ip)) ;
int SOCK_getpeername(SOCK_t sock,char *peer);
int SOCK_getsockname(SOCK_t sock,char *ip);
int SOCK_getsockport(SOCK_t sock,unsigned short * const port);
//int SOCK_gethostname(char *ip);
int SOCK_isreservedip(char *szIp);
int SOCK_set_broadcast(int sock);
int SOCK_get_ether_ip(char *eth, char * const ip, char * const netmask, char *const mac);
int SOCK_set_rcvlowat(int sock, int count);
int SOCK_set_sndlowat(int sock, int count);
int SOCK_get_gateway(char * const gateway);
int SOCK_get_ip_only(char *eth, char * const ip);
int SOCK_check_nic(char *nic_name);

#ifdef __cplusplus
}
#endif

#endif

