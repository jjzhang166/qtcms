// SoupXml.h: interface for the CSoupXml class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUPXML_H__37ECEB97_B60B_441D_95FF_AF4CE1BB2087__INCLUDED_)
#define AFX_SOUPXML_H__37ECEB97_B60B_441D_95FF_AF4CE1BB2087__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RudpLib.h"
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

typedef unsigned __int64    UINT64;
typedef DWORD           FOURCC;

class CSoupXml  
{
public:
	CSoupXml();
	virtual ~CSoupXml();

	typedef struct _tagFrameHead{
		unsigned int magic;			// magic number 固定为 0x534f55ff , "SOU·“
		unsigned int version;		// 版本信息，当前版本为1.0.0.0，固定为0x01000000
		unsigned int frametype;		// 码流帧类型，当前版本支持三种类型：0x00--音频 0x01--视频I帧 0x02--视频P帧
		unsigned int framesize;		// 码流裸数据长度
		UINT64 pts;				// 帧时间戳，64位数据，精度到微秒
		unsigned int externsize;	// 扩展数据大小，当前版本为0
		union{
			struct _tagVideoParam{
				unsigned int width;	// 视频宽
				unsigned int height;	// 视频高
				FOURCC enc;	// 视频编码，四个ASIIC字符表示，当前支持的有"H264"
			}v;
			struct _tagAudioParam{
				unsigned int samplerate;	// 音频采样率
				unsigned int samplewidth;	// 音频采样位宽
				FOURCC enc;	// 音频编码，四个ASIIC字符表示，当前支持的有"G711"
			}a;
		}_U;
	}FrameHead;

	typedef enum _enErrorCode{
		SUCCESS,
		E_NOT_SOUP,
		E_OUT_RANGE,
	}ErrorCode;

	typedef struct _tagAuthData{
		char sUsername[64];
		char sPassword[64];
		int  nErrorCode;
	}AuthData;

	typedef struct _tagStreamItem{
		char sname[64];
		//char size[64];
		int streamid;
		int width;
		int height;
	}StreamItem;

	typedef enum _enProtocolEvent{
		PE_AUTH,
		PE_DEVINFO,
		PE_SETTINGS,
		PE_CNT,
	}ProtocolEvent;

	typedef int (*EventProc)(ProtocolEvent e,LPVOID pData,unsigned int nDataSize,LPVOID pUser);

	typedef struct _tagProtocolEventMap{
		ProtocolEvent e;
		EventProc proc;
		LPVOID pUser;
	}ProtocolEventMap;

public:
	void SetSession(CRudpSession *s);
	int  PtzCtrl(int nChannel,char * sAction,char cParam1,char cParam2);
	int  OpenChannel(int nChannelId,int nStreamId,bool bOpen);
	//
	int  CheckUserMsg(char *sUsername,char *sPassword);
	int  GetChannelCount();
	int  GetStreamData(int nChannelId);
	//
	int  DataProc(char *sXml,int nSize);
	int  SetProtocolEvent(ProtocolEvent e,EventProc proc,LPVOID pUser);
private:
	int  AuthProc(QDomElement nodeElement);
	int  DevInfoProc(QDomElement nodeElement);
	int  SettingsProc(QDomElement nodeElement);
	int  ProtocolEventCall(ProtocolEvent e,LPVOID pData,unsigned int nDataSize);

private:
	CRudpSession * m_ps;
	ProtocolEventMap m_eMap[PE_CNT];

};

#endif // !defined(AFX_SOUPXML_H__37ECEB97_B60B_441D_95FF_AF4CE1BB2087__INCLUDED_)
