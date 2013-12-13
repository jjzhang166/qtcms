// SoupXml.cpp: implementation of the CSoupXml class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "SoupXml.h"
#include <QtCore/QTime>
#include "netlib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSoupXml::CSoupXml() :
m_ps(NULL)
{
	memset(m_eMap,0,sizeof(m_eMap));

}

CSoupXml::~CSoupXml()
{

}

int CSoupXml::PtzCtrl(int nChannel,char * sAction,char cParam1,char cParam2)
{
	if (NULL == m_ps)
	{
		return -1;
	}
	QTime ctime;
	char sPtzDataXml[1024] = {0};
	sprintf(sPtzDataXml,
		"<SOUP version=\"1.0\"><ptz chl=\"%d\" act=\"%s\" param1=\"%d\" param2=\"%d\" ticket=\"%d\"/></SOUP>",
		nChannel,
		sAction,
		cParam1,
		cParam2,
		GetTickCountQ());

	return m_ps->SendData(sPtzDataXml,strlen(sPtzDataXml));;
}


int CSoupXml::OpenChannel(int nChannelId,int nStreamId,bool bOpen)
{
	char sOSData[] = "<SOUP version=\"1.0\"><streamreq ch=\"vin%d\" stream=\"stream%d\" opt=\"%s\"/></SOUP>";
	char sOSDataXml[1024] = {0};
	sprintf(sOSDataXml,sOSData,nChannelId,nStreamId,bOpen ? "start" : "stop");
	return 	m_ps->SendData(sOSDataXml,sizeof(sOSDataXml));
}

int CSoupXml::CheckUserMsg(char *sUsername,char *sPassword)
{
	char sCheckUserMsg[] = "<SOUP version=\"1.0\"><auth usr=\"%s\" psw=\"%s\"/></SOUP>";
	char sCheckUserMsgXml[1024] = {0};
	sprintf(sCheckUserMsgXml,sCheckUserMsg,sUsername,sPassword);
	return 	m_ps->SendData(sCheckUserMsgXml,sizeof(sCheckUserMsgXml));
}
int  CSoupXml::GetChannelCount()
{
	char sGetChannelMsg[] = "<SOUP version=\"1.0\"><devinfo camcnt=\"0\"/></SOUP>";
	return 	m_ps->SendData(sGetChannelMsg,sizeof(sGetChannelMsg));
}
int  CSoupXml::GetStreamData(int nChannelId)
{
	char sGetStreamDataMsg[] = "<SOUP version=\"1.0\"><settings method=\"read\" ticket=\"%d\"><vin%d/></settings></SOUP>";
	char sGetStreamDataMsgXml[1024] = {0};
	sprintf(sGetStreamDataMsgXml,sGetStreamDataMsg,GetTickCountQ(),nChannelId);
	return 	m_ps->SendData(sGetStreamDataMsgXml,sizeof(sGetStreamDataMsgXml));
}

int CSoupXml::DataProc(char *sXml,int nSize)
{
	QString cMsg=sXml;
	if (! cMsg.startsWith("<SOUP"))
	{
		return E_NOT_SOUP;
	}

	QDomDocument SoupXml;
	SoupXml.setContent(cMsg);
	QDomElement nodeElement = SoupXml.documentElement();
	nodeElement =  nodeElement.firstChildElement();
	while(!nodeElement.isNull())
	{
		QString tagname = nodeElement.tagName();
		qDebug("data proc :--%s---",tagname.toAscii().data());
		if(tagname=="auth")
		{
			AuthProc(nodeElement);//校验用户返回
		}
		else if (tagname=="devinfo")
		{
			DevInfoProc(nodeElement);//获取通道数返回
		}
		else if (tagname=="settings")
		{
			SettingsProc(nodeElement);//查询码流返回
		}
		nodeElement = nodeElement.nextSiblingElement();
	}
	return 0;
}

void CSoupXml::SetSession(CRudpSession *s)
{
	m_ps = s;
}

int CSoupXml::AuthProc(QDomElement nodeElement)
{
	AuthData data = {0};
	QString temp = nodeElement.attribute("usr");
	if (!temp.isNull())
	{
		qstrcpy(data.sUsername,temp.toAscii().data());
	}
	temp = nodeElement.attribute("psw");
	if (!temp.isNull())
	{
		qstrcpy(data.sPassword,temp.toAscii().data());
	}
	temp = nodeElement.attribute("error");
	if (!temp.isNull())
	{
		data.nErrorCode = temp.toInt();
	}

	ProtocolEventCall(PE_AUTH,&data,sizeof(data));

	return SUCCESS;
}
int  CSoupXml::DevInfoProc(QDomElement nodeElement)
{
	int data=0;
	QString temp = nodeElement.attribute("camcnt");
	if (!temp.isNull())
	{
		data=temp.toInt();
	}
	ProtocolEventCall(PE_DEVINFO,&data,sizeof(data));

	return SUCCESS;
}
int  CSoupXml::SettingsProc(QDomElement nodeElement)
{
	QDomElement nextElement=nodeElement.firstChildElement();
	if (nextElement.isNull() || !nextElement.tagName().startsWith("vin"))
	{
		return -1;
	}
	
	typedef struct _streamStruct{
		int amount;
		StreamItem streamdata[4];
	}streamStruct;
	streamStruct data;
	int streamindex=0;
	nextElement=nextElement.firstChildElement();
	while(!nextElement.isNull())
	{
		//
		QString temp;
		StreamItem* psdata=&data.streamdata[streamindex];
		psdata->streamid=streamindex;
		temp = nodeElement.attribute("name");
		if (!temp.isNull())
			strncpy(psdata->sname,temp.toAscii().data(),temp.length());
		temp = nodeElement.attribute("size");
		if (!temp.isNull())
			sscanf(temp.toAscii().data(),"%dx%d",&(psdata->width),&(psdata->height));
		nextElement=nextElement.nextSiblingElement();
		streamindex++;
	}
	data.amount=streamindex;
	ProtocolEventCall(PE_SETTINGS,&data,sizeof(data));
	return SUCCESS;
}

int CSoupXml::SetProtocolEvent(ProtocolEvent e,EventProc proc,LPVOID pUser)
{
	if (e >= PE_CNT)
	{
		return E_OUT_RANGE;
	}
	m_eMap[e].e = e;
	m_eMap[e].proc = proc;
	m_eMap[e].pUser = pUser;
	return SUCCESS;
}

int CSoupXml::ProtocolEventCall(ProtocolEvent e,LPVOID pData,unsigned int nDataSize)
{
	if (e >= PE_CNT)
	{
		return E_OUT_RANGE;
	}

	if (NULL != m_eMap[e].proc)
	{
		return m_eMap[e].proc(e,pData,nDataSize,m_eMap[e].pUser);
	}

	return SUCCESS;
}