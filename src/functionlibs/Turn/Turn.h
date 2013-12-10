#ifndef TURN_H
#define TURN_H

#include "RudpLib.h"
#include "Turn_global.h"
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include "IDeviceConnection.h"
#include "IEventRegister.h"
#include "IRemotePreview.h"
#include "EseeXml.h"
#include "SoupXml.h"

#include "aes.h"

class Turn :
	public IDeviceConnection,
	public IRemotePreview,
	public IEventRegister
{
public:
	Turn();
	~Turn();
public:
	enum {PROTOCOL_HEAD = 0x5455524E};

	typedef enum _enTurnCmd{
		CS_Turn_Req = 0x3001,
		CS_Turn_Data,
		SC_Dev_Info = 0x4001,
		SC_Invalide_Id,
		SC_No_Resource,
		SC_Dev_Ready,
	}TurnCmd;

	typedef struct _tagTurnHead{
		unsigned int uiHeader;
		unsigned int uiCmd;
		unsigned int uiRadom;
		union{
			struct _tagTurnReq{
				char id[16];
			}TurnReq;

			struct _tagDevInfo{
				unsigned int uiIp;
				unsigned int uiPort;
			}DevInfo;

			struct _tagTurnData{
				unsigned int uiIp;
				unsigned int uiPort;
			}TurnData;

			char cReverse[20];
		}_U;
		_tagTurnHead(){memset(this,0,sizeof(TurnHead));uiHeader = Turn::PROTOCOL_HEAD;};
	}TurnHead;
	// Event Procs
	int CreateSession(CRudpSession::EventType e,LPVOID pData,int nDataSize);
	int RecvProc(CRudpSession::EventType e,LPVOID pData,int nDataSize);
	int PreCreatePack(CRudpSession::EventType type,LPVOID pData,int nDataSize);
	int SendPre(CRudpSession::EventType e,LPVOID pData,int nDataSize);
	int LdPack(CRudpSession::EventType type,LPVOID pData,int nDataSize);
	int SessionClose(CRudpSession::EventType type,LPVOID pData,int nDataSize);
	int HoleFromDev(CEseeXml::EventType e,LPVOID pData,int nDataSize);
	int DevReady(CEseeXml::EventType e,LPVOID pData,int nDataSize);
	int SoupAuth(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize);//校验用户
	int DevInfo(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize);//获取通道数
	int Settings(CSoupXml::ProtocolEvent e,LPVOID pData,unsigned int uiDataSize);//获取码流信息

    //---------connect--------
	virtual int setDeviceHost(const QString & sAddr);
	virtual int setDevicePorts(const QVariantMap & ports);
	virtual int setDeviceId(const QString & sAddress);
	virtual int setDeviceAuthorityInfomation(QString username,QString password);
	virtual int connectToDevice();
	virtual int disconnect();
	virtual int getCurrentStatus();
	virtual QString getDeviceHost();
	virtual QString getDeviceid();
	virtual QVariantMap getDevicePorts();
	virtual int authority();
	//---------stream----------
	virtual int getLiveStream(int nChannel, int nStream);
	virtual int stopStream();
	virtual int pauseStream(bool bPaused);
	virtual int getStreamCount();
	virtual int getStreamInfo(int nStreamId,QVariantMap &streamInfo);

	typedef int (__cdecl *eventcallback)(QString,QVariantMap,void *);
	virtual QStringList eventList();
	virtual int queryEvent(QString eventName,QStringList& eventParams);
	virtual int registerEvent(QString eventName,int (__cdecl *proc)(QString,QVariantMap,void *),void *pUser);

	virtual QString getModeName();

	virtual long __stdcall QueryInterface( const IID & iid,void **ppv );

	virtual unsigned long __stdcall AddRef();

	virtual unsigned long __stdcall Release();

private:
	void StreamData(LPVOID pData,int nDataSize);
	int applyEventProc(QString eventName,QVariantMap datainfo);

	typedef struct _tagServerInfo{
		struct sockaddr_in ServerAddr;
	}ServerInfo;

	ServerInfo GetServerInfo(char *sId);
	int GetTurnInfo();

	int DataProc(CRudpSession::EventType type,LPVOID pData,int nDataSize);
private:
	int m_nRef;
	QMutex m_csRef;
	typedef struct __EventData{ 
		eventcallback eventproc;
		void* puser;
	}EventData;
	QMap<QString,EventData> eventMap;

	//turn
	CRudpSession           m_s;
	CEseeXml               m_esee;
	int                    m_nRandom;
	QString m_sAddr;
	QString m_sId;
	//
	CEseeXml::TurnServerInfo m_ServerInfo;
	bool m_bGetTurnInfo;
	unsigned int m_uiTurnIp;
	unsigned int m_uiTurnPort;
	struct sockaddr_in m_TurnServerAddr;

	AES *m_pCypher;
	//
	CSoupXml               m_soup;

	bool                   m_bConnected;
	bool                   m_bUserVerified;
	bool                   m_bUserCheck;

	QMutex m_cstreamDec;

	QVariantMap m_ports;
	QMap<int,QVariantMap> m_StaeamList;
	QString m_username;
	QString m_password;
	_enConnectionStatus m_cStatus;

	bool  m_bDevReady;
	bool  m_bHoleRecved;
	bool  m_bstrdataRecved;
	int m_Camcnt;
	int m_Channel, m_Stream;
};

#endif // TURN_H
