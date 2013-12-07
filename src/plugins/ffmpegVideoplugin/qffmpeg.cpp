#include "qffmpeg.h"
#include <QtWebKit/QWebView>
#include <QtWebKit/QWebFrame>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include "AvLibDll.h"
#include "DDrawRender.h"
#include "SDLRender.h"
qffmpeg::qffmpeg(QWidget * parent)
	: QWidget(parent),
	QWebPluginFWBase(this)
{
	
}

qffmpeg::~qffmpeg(void)
{
	m_mapEventProc.clear();
}

void qffmpeg::Init()
{
	CDDrawRender::InitGlobalResource(winId());
	m_dec.SetRenderWnd(winId());
	qDebug("Init OK!!!!!");
}
void qffmpeg::Open()
{
	QString filename = QFileDialog::getOpenFileName((QWidget*)this,"open file",".","(All File(*.*)");
	if (!filename.isNull())
	{
		filepath = filename;
	}
	qDebug("filepath:%s",filepath.toLatin1().data());
}

void qffmpeg::Play()
{
	int i = 0;
	int len = 0;
	char data[1280*720];
	FILE *fp;
	if (NULL == (fp = fopen(filepath.toLatin1().data(),"r+b")))
	{
		qDebug("open file falied");
		QMessageBox::warning(this,"Warining","please select file!!!!!","OK","Cancel");
		return;
	}
	NALU_HEADER_t nhead;
	for (;;)
	{
		memset(&nhead,0,sizeof(NALU_HEADER_t));
		if ((len = fread(&nhead,1,sizeof(NALU_HEADER_t),fp)) < 0)
		{
			qDebug("read video error");
			break;
		}
		memset(data,0,sizeof(data));
		if ((len = fread(data,1,nhead.size,fp)) <= 0)
		{
			rewind(fp);
			continue;
		}
		qDebug("Read Video Frame %d",++i);
		m_dec.Decode(data,nhead.size);
	}
}

void qffmpeg::Pause()
{
	qDebug("pause()");
}

void qffmpeg::Stop()
{
	qDebug("stop()");
}