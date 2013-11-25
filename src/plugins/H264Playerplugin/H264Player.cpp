
#include <QtCore/QVariantMap>
#include <QtXml\qdom.h>
#include <QtCore\QFile>
#include <QtCore\QIODevice>
#include <QtCore\QCoreApplication>
#include "H264Player.h"
#include "IEventRegister.h"
#include "guid.h"

typedef struct test_nalu_header{
	unsigned long flag;
	unsigned long size;
	unsigned long isider;  //1 ÊÇIÖ¡
}NALU_HEADER_t;

typedef struct _frame_data{
	//unsigned char * pY;
	//unsigned char * pU;
	//unsigned char * pV;
	unsigned char* data[4];
	int nWidth;
	int nHeight;
}FRAME_DATA;

int __cdecl RenderProc(QString param1,QVariantMap renderparam,void * pUser);

H264Player::H264Player(void):
m_pfile(NULL),
playing(NULL),
m_pDecoder(NULL),
m_pRender(NULL)
{
}


H264Player::~H264Player(void)
{
	if (m_pDecoder)
	{
		m_pDecoder->Release();
		m_pDecoder=NULL;
	}

	if (m_pRender)
	{
		m_pRender->Release();
		m_pRender=NULL;
	}
}
int H264Player::loadRender( QString renderName )
{
	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);

	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	int n;
	bool bFound = false;
	for (n = 0; n < itemList.count(); n++)
	{
		QDomNode item = itemList.at(n);
		QString sItemName = item.toElement().attribute("name");

		if (sItemName.left(strlen("render.")) == QString("render."))
		{
			QString searchMode = item.toElement().attribute("type");
			if (renderName == searchMode)
			{
				bFound = true;
				if (NULL != m_pRender)
				{
					m_pRender->deinit();
					m_pRender->Release();
					m_pRender = NULL;

				}
				CLSID RenderClsid = pcomString2GUID(item.toElement().attribute("clsid"));
				pcomCreateInstance(RenderClsid,NULL,IID_IVideoRender,(void **)&m_pRender);
				if (NULL != m_pRender)
				{
					return true;
				}
			}
		}
	}

	file->close();
	delete file;

	return false;
}
int H264Player::loadDecoder( QString decoderName )
{
	if(!m_pRender)
		return false;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QFile * file = new QFile(sAppPath + "/pcom_config.xml");
	file->open(QIODevice::ReadOnly);
	QDomDocument ConfFile;
	ConfFile.setContent(file);

	QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
	QDomNodeList itemList = clsidNode.childNodes();
	int n;
	bool bFound = false;
	for (n = 0; n < itemList.count(); n++)
	{
		QDomNode item = itemList.at(n);
		QString sItemName = item.toElement().attribute("name");

		if (sItemName.left(strlen("decoder.")) == QString("decoder."))
		{
			QString searchMode = item.toElement().attribute("type");
			if (decoderName == searchMode)
			{
				bFound = true;
				if (NULL != m_pDecoder)
				{
					m_pDecoder->deinit();
					m_pDecoder->Release();
					m_pDecoder = NULL;

				}
				CLSID DecoderClsid = pcomString2GUID(item.toElement().attribute("clsid"));
				pcomCreateInstance(DecoderClsid,NULL,IID_IVideoDecoder,(void **)&m_pDecoder);
				if (NULL != m_pDecoder)
				{
					//IEventRegister* pEventInstance =;
					//pEventInstance->registerEvent("DecodedFrame",RenderProc,m_pRender);
					return true;
				}
			}
		}
	}

	file->close();
	delete file;

	return false;
}

bool H264Player::InitSearver(QWidget* pwnd)
{
	Stop();
	loadRender("sdlrender");
	loadDecoder("h264decoder");
	IEventRegister* pevent ;
	m_pDecoder->QueryInterface(IID_IEventRegister,(void**)&pevent);/*dynamic_cast<IEventRegister*>(m_pDecoder)*/
	pevent->registerEvent("DecodedFrame",RenderProc,m_pRender);
	pevent->Release();

	m_pDecoder->init(352,240);
	m_pRender->setRenderWnd(pwnd);
	m_pRender->init(352,240);
	return true;
}
bool H264Player::OpenFile(QString filePath)
{
	if (!m_pfile)
	{
		if (NULL == (m_pfile = fopen(filePath.toLatin1().data(),"r+b")))
		{
			qDebug("open file falied");
			//QMessageBox::warning(this,"Warining","please select file!!!!!","OK","Cancel");
			return false;
		}
	}

	return true;
}
void H264Player::Play()
{
	if (!playing)
	{
		if (!m_pfile)
		{
			qDebug("the video file isn't been open!!!!!");
			return;
		}
		playing=true;
		start();
	}
}
void H264Player::Pause()
{
	playing=false;
	wait();
}
void H264Player::Stop()
{
	Pause();
	if(m_pfile)
	{
		fclose(m_pfile);
		m_pfile=NULL;
	}
}

void H264Player::run()
{
	int len = 0;
	char data[1280*720];
	NALU_HEADER_t nhead;

	while (playing)
	{
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		if ((len = fread(&nhead,1,sizeof(NALU_HEADER_t),m_pfile)) < 0)
		{
			qDebug("read video error");
			break;
		}
		memset(data,0,sizeof(data));
		if ((len = fread(data,1,nhead.size,m_pfile)) <= 0)
		{
			rewind(m_pfile);
			continue;


		}
		FRAME_DATA frame_data;
		if (m_pDecoder)
		{
			if (! m_pDecoder->decode(data,nhead.size))
			    qDebug("decode faile !!!!!!");
		}
		

	}

	exit();
}


int __cdecl RenderProc(QString param1,QVariantMap renderparam,void * pUser)
{
	/*qDebug("\nname:%s\ndev_mode:%s\nesee_id:%s\nip:%s\nnetmask:%s\ngetaway:%s\nport:%s\nchannelcnt:%s\nmac:%s\ndevid:%s\n",
	item.name,item.dev_model,item.esee_id,item.ip,item.netmask,item.gateway,item.port,item.channelcnt,item.mac,item.devid); */
	char* data =(char*) renderparam["data"].toInt();
	char* Ydata =(char*) renderparam["Ydata"].toInt();
	char* Udata =(char*) renderparam["Udata"].toInt();
	char* Vdata =(char*) renderparam["Vdata"].toInt();
	int width=renderparam["width"].toInt();
	int height=renderparam["height"].toInt();
	int YStride=renderparam["YStride"].toInt();
	int UVStride=renderparam["UVStride"].toInt();
	int lineStride=renderparam["lineStride"].toInt();
	QString pixelFormat=renderparam["pixelFormat"].toString();
	int flags=renderparam["flags"].toInt();

	if(pUser)
		((IVideoRender*)pUser)->render(data,Ydata,Udata,Vdata,width,height,YStride,UVStride,lineStride,pixelFormat,flags);
	return 0;
}