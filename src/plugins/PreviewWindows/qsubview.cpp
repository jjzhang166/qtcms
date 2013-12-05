#include "qsubview.h"
#include <guid.h>
#include <QtGui/QPainter>

#include <QtCore>
#include <QSettings>
#include <QMouseEvent>


QSubView::QSubView(QWidget *parent)
	: QWidget(parent),m_IVideoDecoder(NULL),
	m_IVideoRender(NULL),
	m_IDeviceClient(NULL)
{
	setMouseTracking(true);
	this->lower();
	//申请解码器接口
	pcomCreateInstance(CLSID_h264Decoder,NULL,IID_IVideoDecoder,(void**)&m_IVideoDecoder);
	qDebug("IVideoDecoder:%x",m_IVideoDecoder);
	//申请渲染器接口
	pcomCreateInstance(CLSID_SDLRender,NULL,IID_IVideoRender,(void**)&m_IVideoRender);
	//申请DeviceClient接口
	pcomCreateInstance(CLSID_DeviceClient,NULL,IID_IDeviceClient,(void**)&m_IDeviceClient);
}

QSubView::~QSubView()
{
	if (NULL!=m_IDeviceClient)
	{
		m_IDeviceClient->Release();
	}
	if (NULL!=m_IVideoDecoder)
	{
		m_IVideoDecoder->Release();
	}
	if (NULL!=m_IVideoRender)
	{
		m_IVideoRender->Release();
	}

}


void QSubView::paintEvent( QPaintEvent * e)
{
	QPainter p(this);

	QString image;
	QColor LineColor;
	QColor FontColor;
	int FontSize;
	QString FontFamily;

	QString sAppPath = QCoreApplication::applicationDirPath();
	QString path = sAppPath + "/skins/default/css/SubWindowStyle.ini";
	QSettings IniFile(path, QSettings::IniFormat, 0);

	image = IniFile.value("background/background-image", NULL).toString();
	LineColor.setNamedColor(IniFile.value("background/background-color", NULL).toString());
	FontColor.setNamedColor(IniFile.value("font/font-color", NULL).toString());
	FontSize = IniFile.value("font/font-size", NULL).toString().toInt();
	FontFamily = IniFile.value("font/font-family", NULL).toString();

 	QRect rcClient = contentsRect();
 
 	QPixmap pix;
	QString PixPaht = sAppPath + image;
 	bool ret = pix.load(PixPaht);
 
  	pix = pix.scaled(rcClient.width(),rcClient.height(),Qt::KeepAspectRatio);

 	p.drawPixmap(rcClient,pix);

	QPen pen = QPen(LineColor, 2);
 	p.setPen(pen);

	p.drawRect(rcClient);
	if (this->hasFocus())
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		rcClient.getCoords(&x, &y, &width, &height);
		p.drawRect(QRect(x + 2,y + 2,width - 2, height - 2));
	}
	else
	{
	 	p.drawRect(rcClient);
	}

	QFont font(FontFamily, FontSize, QFont::Bold);
	
	p.setFont(font);

 	pen.setColor(FontColor);
	
 	p.setPen(pen);

	p.drawText(rcClient, Qt::AlignCenter, "Pixmap");

}

void QSubView::mouseDoubleClickEvent( QMouseEvent * ev)
{
	emit mouseDoubleClick(this,ev);
}

void QSubView::mouseMoveEvent(QMouseEvent *ev)
{
	QRect rect = contentsRect();
	int x = ev->pos().x();
	int y = ev->pos().y();

	if (rect.contains(x, y))
	{
		setFocus(Qt::MouseFocusReason);
	}
}

void QSubView::mousePressEvent(QMouseEvent *ev)
{
	setFocus(Qt::MouseFocusReason);
}

WId QSubView::GetCurrentWnd()
{
	return winId();
}

int QSubView::OpenCameraInWnd(const QString sAddress,unsigned int uiPort,const QString & sEseeId ,unsigned int uiChannelId,unsigned int uiStreamId ,const QString & sUsername,const QString & sPassword ,const QString & sCameraname,const QString & sVendor)
{
	//注册事件，需检测是否注册成功
	
	cbInit();
	m_DevCliSetInfo.m_sAddress=sAddress;
	m_DevCliSetInfo.m_uiPort=uiPort;
	m_DevCliSetInfo.m_sEseeId=sEseeId;
	m_DevCliSetInfo.m_uiChannelId=uiChannelId;
	m_DevCliSetInfo.m_uiStreamId=uiStreamId;
	m_DevCliSetInfo.m_sUsername=sUsername;
	m_DevCliSetInfo.m_sPassword=sPassword;
	m_DevCliSetInfo.m_sCameraname=sCameraname;
	m_DevCliSetInfo.m_sVendor=sVendor;
	if (NULL==m_IDeviceClient)
	{
		return 1;
	}
	m_IDeviceClient->setChannelName(sCameraname);
	int nRet=1;
	nRet=m_IDeviceClient->connectToDevice(sAddress,uiPort,sEseeId);
	//需要等待连接成功，再发送请求，采用信号绑定的方法或者计时器
	if (1==nRet)
	{
		return 1;
	}
	m_IDeviceClient->liveStreamRequire(uiChannelId,uiStreamId,true);
	return 0;
}
int QSubView::CloseWndCamera()
{
	if (NULL==m_IDeviceClient)
	{
		return 1;
	}
	m_IDeviceClient->closeAll();
	return 0;
}
int QSubView::GetWindowConnectionStatus()
{
	return m_IDeviceClient->getConnectStatus();
}
int QSubView::cbInit()
{
	//注册设备服务回调函数
	QString evName="LiveStream";
	IEventRegister *pRegist=NULL;
	if (NULL==m_IDeviceClient)
	{
		return 1;
	}
	m_IDeviceClient->QueryInterface(IID_IEventRegister,(void**)&pRegist);
	if (NULL==pRegist)
	{
		return 1;
	}
	pRegist->registerEvent(evName,cbLiveStream,this);
	pRegist->Release();
	pRegist=NULL;
	//注册解码回调函数
	evName.clear();
	evName.append("DecodedFrame");
	if (NULL==m_IVideoDecoder)
	{
		return 1;
	}
	m_IVideoDecoder->QueryInterface(IID_IEventRegister,(void**)&pRegist);
	if (NULL==pRegist)
	{
		return 1;
	}
	pRegist->registerEvent(evName,cbDecodedFrame,this);
	pRegist->Release();
	pRegist=NULL;
	if (NULL==m_IVideoRender)
	{
		return 1;
	}
	m_IVideoRender->setRenderWnd((QWidget*)this);
	m_IVideoRender->init(this->width(),this->height());
	return 0;
}
int QSubView::PrevPlay(QVariantMap evMap)
{
	unsigned int nLength=evMap.value("length").toUInt();
	qDebug()<<nLength;
	char * lpdata=(char *)evMap.value("data").toUInt();
	if (NULL==m_IVideoDecoder)
	{
		return 1;
	}
	m_IVideoDecoder->decode(lpdata,nLength);
	return 0;
}

int QSubView::PrevRender(QVariantMap evMap)
{
	QVariantMap::const_iterator it;
	for (it=evMap.begin();it!=evMap.end();++it)
	{
		QString sKey=it.key();
		QString sValue=it.value().toString();
		qDebug()<<sKey;
		qDebug()<<sValue;
	}
	if (NULL==m_IVideoRender)
	{
		return 1;
	}
//	m_IVideoRender-render();
	return 0;
}

int cbLiveStream(QString evName,QVariantMap evMap,void*pUser)
{
	//检测数据包，把数据包扔给解码器


	if (evName=="LiveStream")
	{
		((QSubView*)pUser)->PrevPlay(evMap);
		return 0;
	}
	else
		return 1;
}

int cbDecodedFrame(QString evName,QVariantMap evMap,void*pUser)
{
	if (evName=="DecodedFrame")
	{
		((QSubView*)pUser)->PrevRender(evMap);
		return 0;
	}
	else 
		return 1;
}