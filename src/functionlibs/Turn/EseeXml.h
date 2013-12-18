// EseeXml.h: interface for the CEseeXml class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ESEEXML_H__7A73F6A3_976A_4473_8F88_3E6BFECA7ED0__INCLUDED_)
#define AFX_ESEEXML_H__7A73F6A3_976A_4473_8F88_3E6BFECA7ED0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "rudplib.h"
#pragma comment(lib,"RudpLib.lib")

#include <QtXml/QDomDocument>
//#include <QtCore/QMutex>
//#include <QtCore/QObject>
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */

class CEseeXml/*:public QObject*/
{
public:
	CEseeXml();
	virtual ~CEseeXml();

	typedef struct _tagTurnServerInfo{
		unsigned long ulServerAddr;
		unsigned long ulServerPort;
		unsigned long ulDeviceAddr;
		unsigned long ulDevicePort;
	}TurnServerInfo;

	typedef struct _tagHolePeerInfo{
		union{
			unsigned long ulPeerAddr;
			struct {
				char b1;
				char b2;
				char b3;
				char b4;
			}_b;
		}_U;
		unsigned long ulPeerPort;
	}HolePeerInfo;

	typedef enum _enEventType{
		EVENT_DEV_READY,
		EVENT_HOLE_FROM_DEV,

		EVENT_CNT,
	}EventType;

	typedef int ( *EventProc)(EventType e,LPVOID pData,int nDataSize,LPVOID pUser);

	typedef struct _tagEseeEventMap{
		EventType e;
		EventProc proc;
		LPVOID pUserData;
	}EseeEventMap;

	typedef enum _enEseeCmd{
		Cmd_not_found,
		Cmd_hole_client_req = 20101,
		Cmd_turn_server_info = 21011,
		Cmd_turn_dev_ready,
		Cmd_hole_dev_info = 21101,
		Cmd_hole_dev_ready,
		Cmd_hole_from_dev = 30101,
		Cmd_hole_to_dev = 31101,
	}EseeCmd;

	typedef enum _enErrorCode{
		SUCCESS,
		E_INVALID_PARAM,
		E_EVENT_NOT_SET,
	}ErrorCode;

	typedef struct _tagHoleFromData{
		unsigned int uiRandom;
		struct sockaddr from;
	}HoleFromData;

// Attributes
public:
private:
	CRudpSession *m_s;
	TurnServerInfo m_ServerInfo;
	//HolePeerInfo m_HolePeerInfo;
	bool m_bServerInfoReady;
	bool m_bHoleReqAck;
	bool m_bDevReady;
	EseeEventMap m_eMap[EVENT_CNT];

	bool m_reqing;
// Methods
public:
	void CloseReq();
	TurnServerInfo TurnReq(char *sId);
	HolePeerInfo   HoleReq(char *sId,int nRandom);
	int HoleTo(HolePeerInfo info,int nRandom);
	int WaitForReadySignal();
	void SetSession(CRudpSession *s);
	int DataProc(CRudpSession::EventType type,LPVOID pData,int nDataSize);
	void SetEventPorc(EventType e,EventProc func,LPVOID pUserData);
private:
	//DWORD GetServerAddr();
	unsigned int ParseCmd(QDomElement & RootElement);
	TurnServerInfo ParseServerInfo(QDomElement RootElement);
	HolePeerInfo ParseHolePeerInfo(QDomElement RootElement);
	int GetRandomFromProtocol(QDomElement RootElement);
	int EventCall(EventType e,LPVOID pData,int nDataSize);
protected:

};

#endif // !defined(AFX_ESEEXML_H__7A73F6A3_976A_4473_8F88_3E6BFECA7ED0__INCLUDED_)
