
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the RUDPLIB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// RUDPLIB_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#pragma once

#ifdef RUDPLIB_EXPORTS
#define RUDPLIB_API __declspec(dllexport)
#else
#define RUDPLIB_API __declspec(dllimport)
#endif

#define LPVOID void*
#define OUT
#define BOOL bool
#define DWORD unsigned long
#define FAR

typedef struct in_addr {
	union {
		struct { unsigned char s_b1,s_b2,s_b3,s_b4; } S_un_b;
		struct { unsigned short s_w1,s_w2; } S_un_w;
		unsigned long S_addr;
	} S_un;
#define s_addr  S_un.S_addr /* can be used for most tcp & ip code */
#define s_host  S_un.S_un_b.s_b2    // host on imp
#define s_net   S_un.S_un_b.s_b1    // network
#define s_imp   S_un.S_un_w.s_w2    // imp
#define s_impno S_un.S_un_b.s_b4    // imp #
#define s_lh    S_un.S_un_b.s_b3    // logical host
}IN_ADDR;

typedef struct sockaddr_in {
	unsigned short sin_family;    

	unsigned short sin_port;
	in_addr sin_addr;
	char sin_zero[8];
}SOCKETADDR_IN;

typedef struct sockaddr {
	unsigned short sa_family;

	char sa_data[14];                   // Up to 14 bytes of direct address.
}SOCKADDR;

class RUDPLIB_API CRudpSession{
public:
	CRudpSession();
	~CRudpSession();

public:
	typedef enum _enErrorCode{
		SUCCESS,
		EOUTOFRANGE,
		ECONNECTED,
		ECREATESOCKET,
		ECREATETHREAD,
		EBIND,
		EINVALIDPARAM,
		EOUTOFMEMORY,
		ESENDFAILED,
		ENOMEMORY,
		EPRESEND,
		ERECV,
		ESYNFAILED,
		ETIMEDOUT,
		ES_SYN,
		ES_SYN_WAIT,
		ERESET,
		ESESSIONDOWN,
	}ErrorCode,*lpErrorCode;

	typedef enum _enEventType{
		EVENT_SEND_TIMEOUT,
		EVENT_SEND_PRE,
		EVENT_LD_PACK,
		EVENT_PRE_CREATEPACK,
		EVENT_RECV,
		EVENT_SESSION_CLOSE,
		EVENT_CREATE_SESSION,

		Event_Cnt
	}EventType,*lpEventType;

	typedef int ( *EventProc)(EventType e,LPVOID pData,int nDataSize,LPVOID pUser);

	typedef struct _tagEventMap{
		EventType e;
		EventProc proc;
		LPVOID pUser;
	}EventMap,*lpEventMap;

	typedef struct _tagPreSendData{
		sockaddr_in	 TargetAddress;
		LPVOID       pHead;
		LPVOID       pData;
		unsigned int uiSendDataSize;
	}PreSendData,*lpPreSendData;

	typedef struct _tagRecv{
		char *            pRealBuffer;
		unsigned int      uiBufferSize;
		struct sockaddr * from;
		int               iFromLen;
		OUT unsigned int  uiRecv;
		OUT char *        pOutBuffer;
		OUT bool          bProduced;
	}Recv,*lpRecv;

	typedef struct _tagLdPackData{
		LPVOID pData;
		unsigned int uiPackSize;
	}LdPackData,*lpLdPackData;

	typedef struct _tagSendTimeout{
		unsigned int uiTotalPack;
		unsigned int uiSendPack;
		unsigned int uiResendcount;
		BOOL         bContinue;
	}SendTimeout,*lpSendtimeout;

public:
	CRudpSession::ErrorCode    Connect(const char *sIp,unsigned short usPort);
	CRudpSession::ErrorCode    SetEventProc(EventType e,EventProc proc,LPVOID pUser);
	void                       SessionTimeout(DWORD dwTimeOut);
	void                       SessionSendTimeOut(DWORD dwTimeOut);
	CRudpSession::ErrorCode    SendData(char *pData,unsigned int uiSize);
	CRudpSession::ErrorCode    Close();
	void                       SessionSubPackTimeout(DWORD dwTimeOut);
	void                       SessionSubPackSendTimeout(DWORD dwTimeout);
	CRudpSession::ErrorCode    DirectSendTo(const char FAR * buf,int len,int flags,const struct sockaddr FAR *to, int tolen);

public:
	void * m_pInst;
};
